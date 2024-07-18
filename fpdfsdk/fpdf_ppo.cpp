// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_ppo.h"

#include <algorithm>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <utility>
#include <vector>

#include "constants/page_object.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_formobject.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageimagecache.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_string_wrappers.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/cpp/fpdf_scopers.h"

struct XObjectContext {
  UnownedPtr<CPDF_Document> dest_doc;
  RetainPtr<CPDF_Stream> xobject;
};

namespace {

// Struct that stores sub page origin and scale information.  When importing
// more than one pages onto the same page, most likely the pages will need to be
// scaled down, and scale is in range of (0, 1) exclusive.
struct NupPageSettings {
  CFX_PointF sub_page_start_point;
  float scale = 0.0f;
};

// Calculates the N-up parameters.  When importing multiple pages into one page.
// The space of output page is evenly divided along the X axis and Y axis based
// on the input `pages_on_x_axis` and `pages_on_y_axis`.
class NupState {
 public:
  NupState(const CFX_SizeF& pagesize,
           size_t pages_on_x_axis,
           size_t pages_on_y_axis);

  // Calculate sub page origin and scale with the source page of `pagesize` and
  // new page of `sub_page_size_`.
  NupPageSettings CalculateNewPagePosition(const CFX_SizeF& pagesize);

 private:
  // Helper function to get the `sub_x`, `sub_y` pair based on
  // `sub_page_index_`. The space of output page is evenly divided into slots
  // along x and y axis. `sub_x` and `sub_y` are 0-based indices that indicate
  // which allocation slot to use.
  std::pair<size_t, size_t> ConvertPageOrder() const;

  // Given the `sub_x` and `sub_y` subpage position within a page, and a source
  // page with dimensions of `pagesize`, calculate the sub page's origin and
  // scale.
  NupPageSettings CalculatePageEdit(size_t sub_x,
                                    size_t sub_y,
                                    const CFX_SizeF& pagesize) const;

  const CFX_SizeF dest_page_size_;
  const size_t pages_on_x_axis_;
  const size_t pages_on_y_axis_;
  const size_t pages_per_sheet_;
  CFX_SizeF sub_page_size_;

  // A 0-based index, in range of [0, pages_per_sheet_ - 1).
  size_t sub_page_index_ = 0;
};

NupState::NupState(const CFX_SizeF& pagesize,
                   size_t pages_on_x_axis,
                   size_t pages_on_y_axis)
    : dest_page_size_(pagesize),
      pages_on_x_axis_(pages_on_x_axis),
      pages_on_y_axis_(pages_on_y_axis),
      pages_per_sheet_(pages_on_x_axis * pages_on_y_axis) {
  DCHECK(pages_on_x_axis_ > 0);
  DCHECK(pages_on_y_axis_ > 0);
  DCHECK(dest_page_size_.width > 0);
  DCHECK(dest_page_size_.height > 0);

  sub_page_size_.width = dest_page_size_.width / pages_on_x_axis_;
  sub_page_size_.height = dest_page_size_.height / pages_on_y_axis_;
}

std::pair<size_t, size_t> NupState::ConvertPageOrder() const {
  size_t sub_x = sub_page_index_ % pages_on_x_axis_;
  size_t sub_y = sub_page_index_ / pages_on_x_axis_;

  // Y Axis, pages start from the top of the output page.
  sub_y = pages_on_y_axis_ - sub_y - 1;

  return {sub_x, sub_y};
}

NupPageSettings NupState::CalculatePageEdit(size_t sub_x,
                                            size_t sub_y,
                                            const CFX_SizeF& pagesize) const {
  NupPageSettings settings;
  settings.sub_page_start_point.x = sub_x * sub_page_size_.width;
  settings.sub_page_start_point.y = sub_y * sub_page_size_.height;

  const float x_scale = sub_page_size_.width / pagesize.width;
  const float y_scale = sub_page_size_.height / pagesize.height;
  settings.scale = std::min(x_scale, y_scale);

  float sub_width = pagesize.width * settings.scale;
  float sub_height = pagesize.height * settings.scale;
  if (x_scale > y_scale) {
    settings.sub_page_start_point.x += (sub_page_size_.width - sub_width) / 2;
  } else {
    settings.sub_page_start_point.y += (sub_page_size_.height - sub_height) / 2;
  }
  return settings;
}

NupPageSettings NupState::CalculateNewPagePosition(const CFX_SizeF& pagesize) {
  if (sub_page_index_ >= pages_per_sheet_) {
    sub_page_index_ = 0;
  }

  size_t sub_x;
  size_t sub_y;
  std::tie(sub_x, sub_y) = ConvertPageOrder();
  ++sub_page_index_;
  return CalculatePageEdit(sub_x, sub_y, pagesize);
}

RetainPtr<const CPDF_Object> PageDictGetInheritableTag(
    RetainPtr<const CPDF_Dictionary> dict,
    const ByteString& src_tag) {
  if (!dict || src_tag.IsEmpty()) {
    return nullptr;
  }
  if (!dict->KeyExist(pdfium::page_object::kParent) ||
      !dict->KeyExist(pdfium::page_object::kType)) {
    return nullptr;
  }

  RetainPtr<const CPDF_Name> name =
      ToName(dict->GetObjectFor(pdfium::page_object::kType)->GetDirect());
  if (!name || name->GetString() != "Page") {
    return nullptr;
  }

  RetainPtr<const CPDF_Dictionary> pp = ToDictionary(
      dict->GetObjectFor(pdfium::page_object::kParent)->GetDirect());
  if (!pp)
    return nullptr;

  if (dict->KeyExist(src_tag)) {
    return dict->GetObjectFor(src_tag);
  }

  while (pp) {
    if (pp->KeyExist(src_tag)) {
      return pp->GetObjectFor(src_tag);
    }
    if (!pp->KeyExist(pdfium::page_object::kParent))
      break;

    pp = ToDictionary(
        pp->GetObjectFor(pdfium::page_object::kParent)->GetDirect());
  }
  return nullptr;
}

bool CopyInheritable(RetainPtr<CPDF_Dictionary> dest_page_dict,
                     RetainPtr<const CPDF_Dictionary> src_page_dict,
                     const ByteString& key) {
  if (dest_page_dict->KeyExist(key)) {
    return true;
  }

  RetainPtr<const CPDF_Object> inheritable =
      PageDictGetInheritableTag(std::move(src_page_dict), key);
  if (!inheritable) {
    return false;
  }

  dest_page_dict->SetFor(key, inheritable->Clone());
  return true;
}

std::vector<uint32_t> GetPageIndices(const CPDF_Document& doc,
                                     const ByteString& page_range) {
  uint32_t count = doc.GetPageCount();
  if (!page_range.IsEmpty()) {
    return ParsePageRangeString(page_range, count);
  }

  std::vector<uint32_t> page_indices(count);
  std::iota(page_indices.begin(), page_indices.end(), 0);
  return page_indices;
}

class CPDF_PageOrganizer {
 protected:
  CPDF_PageOrganizer(CPDF_Document* dest_doc, CPDF_Document* src_doc);
  ~CPDF_PageOrganizer();

  // Must be called after construction before doing anything else.
  bool Init();

  bool UpdateReference(RetainPtr<CPDF_Object> obj);

  CPDF_Document* dest() { return dest_doc_; }
  const CPDF_Document* dest() const { return dest_doc_; }

  CPDF_Document* src() { return src_doc_; }
  const CPDF_Document* src() const { return src_doc_; }

  void AddObjectMapping(uint32_t old_page_obj_num, uint32_t new_page_obj_num) {
    object_number_map_[old_page_obj_num] = new_page_obj_num;
  }

  void ClearObjectNumberMap() { object_number_map_.clear(); }

 private:
  bool InitDestDoc();

  uint32_t GetNewObjId(CPDF_Reference* ref);

  UnownedPtr<CPDF_Document> const dest_doc_;
  UnownedPtr<CPDF_Document> const src_doc_;

  // Mapping of source object number to destination object number.
  std::map<uint32_t, uint32_t> object_number_map_;
};

CPDF_PageOrganizer::CPDF_PageOrganizer(CPDF_Document* dest_doc,
                                       CPDF_Document* src_doc)
    : dest_doc_(dest_doc), src_doc_(src_doc) {}

CPDF_PageOrganizer::~CPDF_PageOrganizer() = default;

bool CPDF_PageOrganizer::Init() {
  DCHECK(dest_doc_);
  DCHECK(src_doc_);

  return InitDestDoc();
}

bool CPDF_PageOrganizer::InitDestDoc() {
  RetainPtr<CPDF_Dictionary> root = dest()->GetMutableRoot();
  if (!root) {
    return false;
  }

  RetainPtr<CPDF_Dictionary> info = dest()->GetInfo();
  if (info) {
    info->SetNewFor<CPDF_String>("Producer", "PDFium");
  }

  if (root->GetByteStringFor("Type", ByteString()).IsEmpty()) {
    root->SetNewFor<CPDF_Name>("Type", "Catalog");
  }

  RetainPtr<CPDF_Dictionary> pages;
  if (RetainPtr<CPDF_Object> current_pages = root->GetMutableObjectFor("Pages");
      current_pages) {
    pages = ToDictionary(current_pages->GetMutableDirect());
  }
  if (!pages) {
    pages = dest()->NewIndirect<CPDF_Dictionary>();
    root->SetNewFor<CPDF_Reference>("Pages", dest(), pages->GetObjNum());
  }
  if (pages->GetByteStringFor("Type", ByteString()).IsEmpty()) {
    pages->SetNewFor<CPDF_Name>("Type", "Pages");
  }

  if (!pages->GetArrayFor("Kids")) {
    auto kids_array = dest()->NewIndirect<CPDF_Array>();
    pages->SetNewFor<CPDF_Number>("Count", 0);
    pages->SetNewFor<CPDF_Reference>("Kids", dest(), kids_array->GetObjNum());
  }
  return true;
}

bool CPDF_PageOrganizer::UpdateReference(RetainPtr<CPDF_Object> obj) {
  switch (obj->GetType()) {
    case CPDF_Object::kReference: {
      CPDF_Reference* reference = obj->AsMutableReference();
      uint32_t newobjnum = GetNewObjId(reference);
      if (newobjnum == 0)
        return false;
      reference->SetRef(dest(), newobjnum);
      return true;
    }
    case CPDF_Object::kDictionary: {
      CPDF_Dictionary* dict = obj->AsMutableDictionary();
      std::vector<ByteString> bad_keys;
      {
        CPDF_DictionaryLocker locker(dict);
        for (const auto& it : locker) {
          const ByteString& key = it.first;
          if (key == "Parent" || key == "Prev" || key == "First")
            continue;
          RetainPtr<CPDF_Object> next_obj = it.second;
          if (!UpdateReference(next_obj)) {
            bad_keys.push_back(key);
          }
        }
      }
      for (const auto& key : bad_keys)
        dict->RemoveFor(key.AsStringView());
      return true;
    }
    case CPDF_Object::kArray: {
      CPDF_Array* array = obj->AsMutableArray();
      for (size_t i = 0; i < array->size(); ++i) {
        if (!UpdateReference(array->GetMutableObjectAt(i))) {
          return false;
        }
      }
      return true;
    }
    case CPDF_Object::kStream: {
      return UpdateReference(obj->AsMutableStream()->GetMutableDict());
    }
    default:
      return true;
  }
}

uint32_t CPDF_PageOrganizer::GetNewObjId(CPDF_Reference* ref) {
  if (!ref) {
    return 0;
  }

  uint32_t obj_num = ref->GetRefObjNum();
  uint32_t new_obj_num = 0;
  const auto it = object_number_map_.find(obj_num);
  if (it != object_number_map_.end()) {
    new_obj_num = it->second;
  }
  if (new_obj_num) {
    return new_obj_num;
  }

  RetainPtr<const CPDF_Object> direct = ref->GetDirect();
  if (!direct) {
    return 0;
  }

  RetainPtr<CPDF_Object> clone = direct->Clone();
  const CPDF_Dictionary* dict_clone = clone->AsDictionary();
  if (dict_clone && dict_clone->KeyExist("Type")) {
    ByteString type = dict_clone->GetByteStringFor("Type");
    if (type.EqualNoCase("Pages")) {
      return 4;
    }
    if (type.EqualNoCase("Page")) {
      return 0;
    }
  }

  new_obj_num = dest()->AddIndirectObject(clone);
  AddObjectMapping(obj_num, new_obj_num);
  if (!UpdateReference(std::move(clone))) {
    return 0;
  }

  return new_obj_num;
}

// Copies pages from a source document into a destination document.
// This class is intended to be used once via ExportPages() and then destroyed.
class CPDF_PageExporter final : public CPDF_PageOrganizer {
 public:
  CPDF_PageExporter(CPDF_Document* dest_doc, CPDF_Document* src_doc);
  ~CPDF_PageExporter();

  // For the pages from the source document with `page_indices` as their page
  // indices, insert them into the destination document at page `nIndex`.
  // `page_indices` and `nIndex` are 0-based.
  bool ExportPages(pdfium::span<const uint32_t> page_indices, int nIndex);
};

CPDF_PageExporter::CPDF_PageExporter(CPDF_Document* dest_doc,
                                     CPDF_Document* src_doc)
    : CPDF_PageOrganizer(dest_doc, src_doc) {}

CPDF_PageExporter::~CPDF_PageExporter() = default;

bool CPDF_PageExporter::ExportPages(pdfium::span<const uint32_t> page_indices,
                                    int nIndex) {
  if (!Init())
    return false;

  int curpage = nIndex;
  for (uint32_t pageIndex : page_indices) {
    RetainPtr<CPDF_Dictionary> dest_page_dict = dest()->CreateNewPage(curpage);
    RetainPtr<const CPDF_Dictionary> src_page_dict =
        src()->GetPageDictionary(pageIndex);
    if (!src_page_dict || !dest_page_dict) {
      return false;
    }

    // Clone the page dictionary
    CPDF_DictionaryLocker locker(src_page_dict);
    for (const auto& it : locker) {
      const ByteString& src_key = it.first;
      const RetainPtr<CPDF_Object>& obj = it.second;
      if (src_key == pdfium::page_object::kType ||
          src_key == pdfium::page_object::kParent) {
        continue;
      }
      dest_page_dict->SetFor(src_key, obj->Clone());
    }

    // inheritable item
    // Even though some entries are required by the PDF spec, there exist
    // PDFs that omit them. Set some defaults in this case.
    // 1 MediaBox - required
    if (!CopyInheritable(dest_page_dict, src_page_dict,
                         pdfium::page_object::kMediaBox)) {
      // Search for "CropBox" in the source page dictionary.
      // If it does not exist, use the default letter size.
      RetainPtr<const CPDF_Object> inheritable = PageDictGetInheritableTag(
          src_page_dict, pdfium::page_object::kCropBox);
      if (inheritable) {
        dest_page_dict->SetFor(pdfium::page_object::kMediaBox,
                               inheritable->Clone());
      } else {
        // Make the default size letter size (8.5"x11")
        static const CFX_FloatRect kDefaultLetterRect(0, 0, 612, 792);
        dest_page_dict->SetRectFor(pdfium::page_object::kMediaBox,
                                   kDefaultLetterRect);
      }
    }

    // 2 Resources - required
    if (!CopyInheritable(dest_page_dict, src_page_dict,
                         pdfium::page_object::kResources)) {
      // Use a default empty resources if it does not exist.
      dest_page_dict->SetNewFor<CPDF_Dictionary>(
          pdfium::page_object::kResources);
    }

    // 3 CropBox - optional
    CopyInheritable(dest_page_dict, src_page_dict,
                    pdfium::page_object::kCropBox);
    // 4 Rotate - optional
    CopyInheritable(dest_page_dict, src_page_dict,
                    pdfium::page_object::kRotate);

    // Update the reference
    uint32_t old_page_obj_num = src_page_dict->GetObjNum();
    uint32_t new_page_obj_num = dest_page_dict->GetObjNum();
    AddObjectMapping(old_page_obj_num, new_page_obj_num);
    UpdateReference(dest_page_dict);
    ++curpage;
  }

  return true;
}

// Copies pages from a source document into a destination document. Creates 1
// page in the destination document for every N source pages. This class is
// intended to be used once via ExportNPagesToOne() and then destroyed.
class CPDF_NPageToOneExporter final : public CPDF_PageOrganizer {
 public:
  CPDF_NPageToOneExporter(CPDF_Document* dest_doc, CPDF_Document* src_doc);
  ~CPDF_NPageToOneExporter();

  // For the pages from the source document with `page_indices` as their page
  // indices, insert them into the destination document, starting at page index
  // 0.
  // `page_indices` is 0-based.
  // `dest_page_size` is the destination document page dimensions, measured in
  // PDF "user space" units.
  // `pages_on_x_axis` and `nPagesOnXAxis` together defines how many source
  // pages fit on one destination page.
  bool ExportNPagesToOne(pdfium::span<const uint32_t> page_indices,
                         const CFX_SizeF& dest_page_size,
                         size_t pages_on_x_axis,
                         size_t pages_on_y_axis);

  std::unique_ptr<XObjectContext> CreateXObjectContextFromPage(
      int src_page_index);

 private:
  // Map page object number to XObject object name.
  using PageXObjectMap = std::map<uint32_t, ByteString>;

  // Creates an XObject from `src_page`, or find an existing XObject that
  // represents `src_page`. The transformation matrix is specified in
  // `settings`.
  // Returns the XObject reference surrounded by the transformation matrix.
  ByteString AddSubPage(const RetainPtr<CPDF_Page>& src_page,
                        const NupPageSettings& settings);

  // Creates an XObject from `src_page`. Updates mapping as needed.
  // Returns the name of the newly created XObject.
  ByteString MakeXObjectFromPage(RetainPtr<CPDF_Page> src_page);
  RetainPtr<CPDF_Stream> MakeXObjectFromPageRaw(RetainPtr<CPDF_Page> src_page);

  // Adds `content` as the Contents key in `dest_page_dict`.
  // Adds the objects in `xobject_name_to_number_map_` to the XObject
  // dictionary in `dest_page_dict`'s Resources dictionary.
  void FinishPage(RetainPtr<CPDF_Dictionary> dest_page_dict,
                  const ByteString& content);

  // Counter for giving new XObjects unique names.
  uint32_t object_number_ = 0;

  // Keeps track of created XObjects in the current page.
  // Map XObject's object name to it's object number.
  std::map<ByteString, uint32_t> xobject_name_to_number_map_;

  // Mapping of source page object number and XObject name of the entire doc.
  // If there are multiple source pages that reference the same object number,
  // they can also share the same created XObject.
  PageXObjectMap src_page_xobject_map_;
};

CPDF_NPageToOneExporter::CPDF_NPageToOneExporter(CPDF_Document* dest_doc,
                                                 CPDF_Document* src_doc)
    : CPDF_PageOrganizer(dest_doc, src_doc) {}

CPDF_NPageToOneExporter::~CPDF_NPageToOneExporter() = default;

bool CPDF_NPageToOneExporter::ExportNPagesToOne(
    pdfium::span<const uint32_t> page_indices,
    const CFX_SizeF& dest_page_size,
    size_t pages_on_x_axis,
    size_t pages_on_y_axis) {
  if (!Init())
    return false;

  FX_SAFE_SIZE_T safe_pages_per_sheet = pages_on_x_axis;
  safe_pages_per_sheet *= pages_on_y_axis;
  if (!safe_pages_per_sheet.IsValid()) {
    return false;
  }

  ClearObjectNumberMap();
  src_page_xobject_map_.clear();
  size_t pages_per_sheet = safe_pages_per_sheet.ValueOrDie();
  NupState n_up_state(dest_page_size, pages_on_x_axis, pages_on_y_axis);

  FX_SAFE_INT32 curpage = 0;
  const CFX_FloatRect dest_page_rect(0, 0, dest_page_size.width,
                                     dest_page_size.height);
  for (size_t outer_page_index = 0; outer_page_index < page_indices.size();
       outer_page_index += pages_per_sheet) {
    xobject_name_to_number_map_.clear();

    RetainPtr<CPDF_Dictionary> dest_page_dict =
        dest()->CreateNewPage(curpage.ValueOrDie());
    if (!dest_page_dict) {
      return false;
    }

    dest_page_dict->SetRectFor(pdfium::page_object::kMediaBox, dest_page_rect);
    ByteString content;
    size_t inner_page_max =
        std::min(outer_page_index + pages_per_sheet, page_indices.size());
    for (size_t i = outer_page_index; i < inner_page_max; ++i) {
      RetainPtr<CPDF_Dictionary> src_page_dict =
          src()->GetMutablePageDictionary(page_indices[i]);
      if (!src_page_dict) {
        return false;
      }

      auto src_page = pdfium::MakeRetain<CPDF_Page>(src(), src_page_dict);
      src_page->AddPageImageCache();
      NupPageSettings settings =
          n_up_state.CalculateNewPagePosition(src_page->GetPageSize());
      content += AddSubPage(src_page, settings);
    }

    FinishPage(dest_page_dict, content);
    ++curpage;
  }

  return true;
}

ByteString CPDF_NPageToOneExporter::AddSubPage(
    const RetainPtr<CPDF_Page>& src_page,
    const NupPageSettings& settings) {
  uint32_t src_page_obj_num = src_page->GetDict()->GetObjNum();
  const auto it = src_page_xobject_map_.find(src_page_obj_num);
  ByteString xobject_name = it != src_page_xobject_map_.end()
                                ? it->second
                                : MakeXObjectFromPage(src_page);

  CFX_Matrix matrix;
  matrix.Scale(settings.scale, settings.scale);
  matrix.Translate(settings.sub_page_start_point.x,
                   settings.sub_page_start_point.y);

  fxcrt::ostringstream contentStream;
  contentStream << "q\n"
                << matrix.a << " " << matrix.b << " " << matrix.c << " "
                << matrix.d << " " << matrix.e << " " << matrix.f << " cm\n"
                << "/" << xobject_name << " Do Q\n";
  return ByteString(contentStream);
}

RetainPtr<CPDF_Stream> CPDF_NPageToOneExporter::MakeXObjectFromPageRaw(
    RetainPtr<CPDF_Page> src_page) {
  RetainPtr<const CPDF_Dictionary> src_page_dict = src_page->GetDict();
  RetainPtr<const CPDF_Object> src_contents =
      src_page_dict->GetDirectObjectFor(pdfium::page_object::kContents);

  auto new_xobject =
      dest()->NewIndirect<CPDF_Stream>(dest()->New<CPDF_Dictionary>());
  RetainPtr<CPDF_Dictionary> new_xobject_dict = new_xobject->GetMutableDict();
  static const char kResourceString[] = "Resources";
  if (!CopyInheritable(new_xobject_dict, src_page_dict, kResourceString)) {
    // Use a default empty resources if it does not exist.
    new_xobject_dict->SetNewFor<CPDF_Dictionary>(kResourceString);
  }
  uint32_t src_page_obj_num = src_page_dict->GetObjNum();
  uint32_t new_xobject_obj_num = new_xobject_dict->GetObjNum();
  AddObjectMapping(src_page_obj_num, new_xobject_obj_num);
  UpdateReference(new_xobject_dict);
  new_xobject_dict->SetNewFor<CPDF_Name>("Type", "XObject");
  new_xobject_dict->SetNewFor<CPDF_Name>("Subtype", "Form");
  new_xobject_dict->SetNewFor<CPDF_Number>("FormType", 1);
  new_xobject_dict->SetRectFor("BBox", src_page->GetBBox());
  new_xobject_dict->SetMatrixFor("Matrix", src_page->GetPageMatrix());
  if (!src_contents) {
    return new_xobject;
  }
  const CPDF_Array* src_contents_array = src_contents->AsArray();
  if (!src_contents_array) {
    RetainPtr<const CPDF_Stream> stream(src_contents->AsStream());
    auto acc = pdfium::MakeRetain<CPDF_StreamAcc>(std::move(stream));
    acc->LoadAllDataFiltered();
    new_xobject->SetDataAndRemoveFilter(acc->GetSpan());
    return new_xobject;
  }
  ByteString src_content_stream;
  for (size_t i = 0; i < src_contents_array->size(); ++i) {
    RetainPtr<const CPDF_Stream> stream = src_contents_array->GetStreamAt(i);
    auto acc = pdfium::MakeRetain<CPDF_StreamAcc>(std::move(stream));
    acc->LoadAllDataFiltered();
    src_content_stream += ByteStringView(acc->GetSpan());
    src_content_stream += "\n";
  }
  new_xobject->SetDataAndRemoveFilter(src_content_stream.unsigned_span());
  return new_xobject;
}

ByteString CPDF_NPageToOneExporter::MakeXObjectFromPage(
    RetainPtr<CPDF_Page> src_page) {
  RetainPtr<CPDF_Stream> new_xobject = MakeXObjectFromPageRaw(src_page);

  // TODO(xlou): A better name schema to avoid possible object name collision.
  ByteString xobject_name = ByteString::Format("X%d", ++object_number_);
  xobject_name_to_number_map_[xobject_name] = new_xobject->GetObjNum();
  src_page_xobject_map_[src_page->GetDict()->GetObjNum()] = xobject_name;
  return xobject_name;
}

std::unique_ptr<XObjectContext>
CPDF_NPageToOneExporter::CreateXObjectContextFromPage(int src_page_index) {
  RetainPtr<CPDF_Dictionary> src_page_dict =
      src()->GetMutablePageDictionary(src_page_index);
  if (!src_page_dict)
    return nullptr;

  auto src_page = pdfium::MakeRetain<CPDF_Page>(src(), src_page_dict);
  auto xobject = std::make_unique<XObjectContext>();
  xobject->dest_doc = dest();
  xobject->xobject.Reset(MakeXObjectFromPageRaw(src_page));
  return xobject;
}

void CPDF_NPageToOneExporter::FinishPage(
    RetainPtr<CPDF_Dictionary> dest_page_dict,
    const ByteString& content) {
  RetainPtr<CPDF_Dictionary> resources =
      dest_page_dict->GetOrCreateDictFor(pdfium::page_object::kResources);
  RetainPtr<CPDF_Dictionary> xobject = resources->GetOrCreateDictFor("XObject");
  for (auto& it : xobject_name_to_number_map_) {
    xobject->SetNewFor<CPDF_Reference>(it.first, dest(), it.second);
  }

  auto stream =
      dest()->NewIndirect<CPDF_Stream>(dest()->New<CPDF_Dictionary>());
  stream->SetData(content.unsigned_span());
  dest_page_dict->SetNewFor<CPDF_Reference>(pdfium::page_object::kContents,
                                            dest(), stream->GetObjNum());
}

// Make sure arrays only contain objects of basic types.
bool IsValidViewerPreferencesArray(const CPDF_Array* array) {
  CPDF_ArrayLocker locker(array);
  for (const auto& obj : locker) {
    if (obj->IsArray() || obj->IsDictionary() || obj->IsReference() ||
        obj->IsStream()) {
      return false;
    }
  }
  return true;
}

bool IsValidViewerPreferencesObject(const CPDF_Object* obj) {
  // Per spec, there are no valid entries of these types.
  if (obj->IsDictionary() || obj->IsNull() || obj->IsReference() ||
      obj->IsStream()) {
    return false;
  }

  const CPDF_Array* array = obj->AsArray();
  if (!array) {
    return true;
  }

  return IsValidViewerPreferencesArray(array);
}

}  // namespace

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_ImportPagesByIndex(FPDF_DOCUMENT dest_doc,
                        FPDF_DOCUMENT src_doc,
                        const int* page_indices,
                        unsigned long length,
                        int index) {
  CPDF_Document* cdest_doc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!cdest_doc) {
    return false;
  }

  CPDF_Document* csrc_doc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!csrc_doc) {
    return false;
  }

  CPDF_PageExporter exporter(cdest_doc, csrc_doc);

  if (!page_indices) {
    std::vector<uint32_t> page_indices_vec(csrc_doc->GetPageCount());
    std::iota(page_indices_vec.begin(), page_indices_vec.end(), 0);
    return exporter.ExportPages(page_indices_vec, index);
  }
  if (length == 0) {
    return false;
  }
  auto page_span = UNSAFE_TODO(pdfium::make_span(
      reinterpret_cast<const uint32_t*>(page_indices), length));
  return exporter.ExportPages(page_span, index);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_ImportPages(FPDF_DOCUMENT dest_doc,
                                                     FPDF_DOCUMENT src_doc,
                                                     FPDF_BYTESTRING pagerange,
                                                     int index) {
  CPDF_Document* cdest_doc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!cdest_doc) {
    return false;
  }

  CPDF_Document* csrc_doc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!csrc_doc) {
    return false;
  }

  std::vector<uint32_t> page_indices = GetPageIndices(*csrc_doc, pagerange);
  if (page_indices.empty())
    return false;

  CPDF_PageExporter exporter(cdest_doc, csrc_doc);
  return exporter.ExportPages(page_indices, index);
}

FPDF_EXPORT FPDF_DOCUMENT FPDF_CALLCONV
FPDF_ImportNPagesToOne(FPDF_DOCUMENT src_doc,
                       float output_width,
                       float output_height,
                       size_t pages_on_x_axis,
                       size_t pages_on_y_axis) {
  CPDF_Document* csrc_doc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!csrc_doc) {
    return nullptr;
  }

  if (output_width <= 0 || output_height <= 0 || pages_on_x_axis <= 0 ||
      pages_on_y_axis <= 0) {
    return nullptr;
  }

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  if (!output_doc)
    return nullptr;

  CPDF_Document* dest_doc = CPDFDocumentFromFPDFDocument(output_doc.get());
  DCHECK(dest_doc);

  std::vector<uint32_t> page_indices = GetPageIndices(*csrc_doc, ByteString());
  if (page_indices.empty())
    return nullptr;

  if (pages_on_x_axis == 1 && pages_on_y_axis == 1) {
    CPDF_PageExporter exporter(dest_doc, csrc_doc);
    if (!exporter.ExportPages(page_indices, 0)) {
      return nullptr;
    }
    return output_doc.release();
  }

  CPDF_NPageToOneExporter exporter(dest_doc, csrc_doc);
  if (!exporter.ExportNPagesToOne(page_indices,
                                  CFX_SizeF(output_width, output_height),
                                  pages_on_x_axis, pages_on_y_axis)) {
    return nullptr;
  }
  return output_doc.release();
}

FPDF_EXPORT FPDF_XOBJECT FPDF_CALLCONV
FPDF_NewXObjectFromPage(FPDF_DOCUMENT dest_doc,
                        FPDF_DOCUMENT src_doc,
                        int src_page_index) {
  CPDF_Document* dest = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!dest)
    return nullptr;

  CPDF_Document* src = CPDFDocumentFromFPDFDocument(src_doc);
  if (!src)
    return nullptr;

  CPDF_NPageToOneExporter exporter(dest, src);
  std::unique_ptr<XObjectContext> xobject =
      exporter.CreateXObjectContextFromPage(src_page_index);
  return FPDFXObjectFromXObjectContext(xobject.release());
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_CloseXObject(FPDF_XOBJECT xobject) {
  std::unique_ptr<XObjectContext> xobject_deleter(
      XObjectContextFromFPDFXObject(xobject));
}

FPDF_EXPORT FPDF_PAGEOBJECT FPDF_CALLCONV
FPDF_NewFormObjectFromXObject(FPDF_XOBJECT xobject) {
  XObjectContext* xobj = XObjectContextFromFPDFXObject(xobject);
  if (!xobj)
    return nullptr;

  auto form = std::make_unique<CPDF_Form>(xobj->dest_doc, nullptr,
                                          xobj->xobject, nullptr);
  form->ParseContent(nullptr, nullptr, nullptr);
  auto form_object = std::make_unique<CPDF_FormObject>(
      CPDF_PageObject::kNoContentStream, std::move(form), CFX_Matrix());
  return FPDFPageObjectFromCPDFPageObject(form_object.release());
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_CopyViewerPreferences(FPDF_DOCUMENT dest_doc, FPDF_DOCUMENT src_doc) {
  CPDF_Document* cdest_doc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!cdest_doc) {
    return false;
  }

  CPDF_Document* csrc_doc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!csrc_doc) {
    return false;
  }

  RetainPtr<const CPDF_Dictionary> pref_dict =
      csrc_doc->GetRoot()->GetDictFor("ViewerPreferences");
  if (!pref_dict) {
    return false;
  }

  RetainPtr<CPDF_Dictionary> dest_dict = cdest_doc->GetMutableRoot();
  if (!dest_dict) {
    return false;
  }

  auto cloned_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  CPDF_DictionaryLocker locker(pref_dict);
  for (const auto& it : locker) {
    if (IsValidViewerPreferencesObject(it.second)) {
      cloned_dict->SetFor(it.first, it.second->Clone());
    }
  }

  dest_dict->SetFor("ViewerPreferences", std::move(cloned_dict));
  return true;
}
