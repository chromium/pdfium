// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <tuple>
#include <utility>

#include "constants/page_object.h"
#include "core/fpdfapi/edit/cpdf_contentstream_write_utils.h"
#include "core/fpdfapi/edit/cpdf_pagecontentmanager.h"
#include "core/fpdfapi/edit/cpdf_stringarchivestream.h"
#include "core/fpdfapi/font/cpdf_truetypefont.h"
#include "core/fpdfapi/font/cpdf_type1font.h"
#include "core/fpdfapi/page/cpdf_contentmarks.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_formobject.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_path.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fpdfapi/page/cpdf_textobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fpdfapi/parser/object_tree_traversal_util.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/span.h"

namespace {

// TODO(thestig): Remove/restore other unused resource types:
// - ColorSpace
// - Pattern
// - Shading
constexpr const char* kResourceKeys[] = {"ExtGState", "Font", "XObject"};

// Key: The resource type.
// Value: The resource names of a given type.
using ResourcesMap = std::map<ByteString, std::set<ByteString>>;

// Returns whether it wrote to `buf` or not.
bool WriteColorToStream(fxcrt::ostringstream& buf, const CPDF_Color* color) {
  if (!color || (!color->IsColorSpaceRGB() && !color->IsColorSpaceGray())) {
    return false;
  }

  std::optional<FX_RGB_STRUCT<float>> colors = color->GetRGB();
  if (!colors.has_value()) {
    return false;
  }

  WriteFloat(buf, colors.value().red) << " ";
  WriteFloat(buf, colors.value().green) << " ";
  WriteFloat(buf, colors.value().blue);
  return true;
}

// Balances the "q" operator ProcessGraphics() emitted.
void EndProcessGraphics(fxcrt::ostringstream& buf) {
  buf << " Q\n";
}

void RecordPageObjectResourceUsage(const CPDF_PageObject* page_object,
                                   ResourcesMap& seen_resources) {
  const ByteString& resource_name = page_object->GetResourceName();
  if (!resource_name.IsEmpty()) {
    switch (page_object->GetType()) {
      case CPDF_PageObject::Type::kText:
        seen_resources["Font"].insert(resource_name);
        break;
      case CPDF_PageObject::Type::kImage:
      case CPDF_PageObject::Type::kForm:
        seen_resources["XObject"].insert(resource_name);
        break;
      case CPDF_PageObject::Type::kPath:
        break;
      case CPDF_PageObject::Type::kShading:
        break;
    }
  }
  for (const auto& name : page_object->GetGraphicsResourceNames()) {
    CHECK(!name.IsEmpty());
    seen_resources["ExtGState"].insert(name);
  }
}

CPDF_PageObjectHolder::RemovedResourceMap RemoveUnusedResources(
    CPDF_Dictionary* resource_dict,
    const std::vector<ByteString>& keys,
    const std::set<ByteString>* resource_in_use_of_current_type) {
  CPDF_PageObjectHolder::RemovedResourceMap removed_resource_map;
  for (const ByteString& key : keys) {
    if (resource_in_use_of_current_type &&
        pdfium::Contains(*resource_in_use_of_current_type, key)) {
      continue;
    }

    removed_resource_map[key] = resource_dict->RemoveFor(key.AsStringView());
  }
  return removed_resource_map;
}

void SaveUnusedResources(
    CPDF_PageObjectHolder::RemovedResourceMap& removed_resource_map,
    CPDF_PageObjectHolder::RemovedResourceMap& saved_resource_map) {
  for (auto& entry : removed_resource_map) {
    saved_resource_map[entry.first] = std::move(entry.second);
  }
}

std::vector<ByteString> FindKeysToRestore(
    const std::vector<ByteString>& keys,
    const std::set<ByteString>* resource_in_use_of_current_type,
    const CPDF_PageObjectHolder::RemovedResourceMap& saved_resource_map) {
  if (!resource_in_use_of_current_type) {
    return {};
  }

  std::vector<ByteString> missing_keys;
  for (const ByteString& key : *resource_in_use_of_current_type) {
    if (!pdfium::Contains(keys, key)) {
      missing_keys.push_back(key);
    }
  }

  std::vector<ByteString> keys_to_restore;
  for (const ByteString& key : missing_keys) {
    if (pdfium::Contains(saved_resource_map, key)) {
      keys_to_restore.push_back(key);
    }
  }
  return keys_to_restore;
}

// `keys_to_restore` entries must exist in `saved_resource_map`.
void RestoreUsedResources(
    CPDF_Dictionary* resource_dict,
    const std::vector<ByteString>& keys_to_restore,
    CPDF_PageObjectHolder::RemovedResourceMap& saved_resource_map) {
  for (const ByteString& key : keys_to_restore) {
    auto node = saved_resource_map.extract(key);
    CHECK(!node.empty());
    resource_dict->SetFor(node.key(), std::move(node.mapped()));
  }
}

void RemoveOrRestoreUnusedResources(
    RetainPtr<CPDF_Dictionary> resources_dict,
    const ResourcesMap& resources_in_use,
    CPDF_PageObjectHolder::AllRemovedResourcesMap& all_removed_resources_map) {
  for (const char* resource_key : kResourceKeys) {
    auto resources_in_use_it = resources_in_use.find(resource_key);
    const std::set<ByteString>* resource_in_use_of_current_type =
        resources_in_use_it != resources_in_use.end()
            ? &resources_in_use_it->second
            : nullptr;

    RetainPtr<CPDF_Dictionary> current_resource_dict =
        resources_dict->GetMutableDictFor(resource_key);
    if (!current_resource_dict && !resource_in_use_of_current_type) {
      continue;
    }

    const std::vector<ByteString> keys = current_resource_dict->GetKeys();
    CPDF_PageObjectHolder::RemovedResourceMap removed_resource_map =
        RemoveUnusedResources(current_resource_dict, keys,
                              resource_in_use_of_current_type);
    if (!removed_resource_map.empty()) {
      // Note that this may create a new entry in `all_removed_resources_map`.
      // Only do this if `removed_resource_map` is not empty.
      SaveUnusedResources(removed_resource_map,
                          all_removed_resources_map[resource_key]);
    }

    // Restore in-use objects if possible. To do so, first see if there is a
    // saved resource map to restore from.
    auto saved_resource_map_it = all_removed_resources_map.find(resource_key);
    if (saved_resource_map_it == all_removed_resources_map.end()) {
      continue;
    }

    auto& saved_resource_map = saved_resource_map_it->second;
    std::vector<ByteString> keys_to_restore = FindKeysToRestore(
        keys, resource_in_use_of_current_type, saved_resource_map);
    if (keys_to_restore.empty()) {
      continue;
    }

    // Create the dictionary if needed, so there is a place to restore to.
    if (!current_resource_dict) {
      current_resource_dict =
          resources_dict->SetNewFor<CPDF_Dictionary>(resource_key);
    }
    RestoreUsedResources(current_resource_dict, keys_to_restore,
                         saved_resource_map);
  }
}

bool IsPageResourceShared(CPDF_Document* doc,
                          RetainPtr<const CPDF_Dictionary> page_dict,
                          RetainPtr<const CPDF_Dictionary> resources_dict) {
  CHECK(doc);
  CHECK(page_dict);
  CHECK(resources_dict);

  const uint32_t resources_object_number = resources_dict->GetObjNum();
  if (resources_object_number) {
    // If `resources_dict` is not an inline object, then check to see if is a
    // shared object.
    if (pdfium::Contains(GetObjectsWithMultipleReferences(doc),
                         resources_object_number)) {
      return true;
    }
  }

  // The check above may not catch all cases. e.g. inline objects. Check all
  // pages in the document to see if another page is using the same resources.
  int page_dict_seen = 0;
  int resources_dict_seen = 0;
  for (int i = 0; i < doc->GetPageCount(); ++i) {
    RetainPtr<CPDF_Dictionary> current_page_dict =
        doc->GetMutablePageDictionary(i);
    if (!current_page_dict) {
      continue;
    }

    // Check to see if the current page's page dictionary is seen twice: Once
    // for this page, and once for another. If the same page dictionary is in
    // use twice, then the resource dictionary within must also be shared.
    if (current_page_dict == page_dict) {
      ++page_dict_seen;
      if (page_dict_seen == 2) {
        return true;
      }
    }

    // Similar check as above, for the current page's resource dictionary.
    auto page =
        pdfium::MakeRetain<CPDF_Page>(doc, std::move(current_page_dict));
    if (resources_dict == page->GetMutableResources()) {
      ++resources_dict_seen;
      if (resources_dict_seen == 2) {
        return true;
      }
    }
  }

  return false;
}

void CloneResourcesDictEntries(CPDF_Document* doc,
                               RetainPtr<CPDF_Dictionary> resources_dict) {
  struct KeyAndObject {
    ByteString key;
    RetainPtr<const CPDF_Object> object;
  };
  std::vector<KeyAndObject> entries_to_maybe_clone;
  {
    CPDF_DictionaryLocker locker(resources_dict);
    for (const auto& it : locker) {
      const ByteString& key = it.first;
      const RetainPtr<CPDF_Object>& object = it.second;
      // Clone resource dictionaries that may get modified. While out for
      // self-referencing loops.
      if (object != resources_dict &&
          object->GetType() == CPDF_Object::kReference &&
          pdfium::Contains(kResourceKeys, key)) {
        RetainPtr<const CPDF_Object> direct_object = object->GetDirect();
        if (direct_object) {
          entries_to_maybe_clone.emplace_back(key, std::move(direct_object));
        }
      }
    }
  }

  if (entries_to_maybe_clone.empty()) {
    return;
  }

  std::set<uint32_t> shared_objects = GetObjectsWithMultipleReferences(doc);
  if (shared_objects.empty()) {
    return;
  }

  // Must modify `resources_dict` after `locker` goes out of scope.
  for (const auto& entry : entries_to_maybe_clone) {
    if (pdfium::Contains(shared_objects, entry.object->GetObjNum())) {
      const uint32_t clone_object_number =
          doc->AddIndirectObject(entry.object->Clone());
      resources_dict->SetNewFor<CPDF_Reference>(entry.key, doc,
                                                clone_object_number);
    }
  }
}

}  // namespace

CPDF_PageContentGenerator::CPDF_PageContentGenerator(
    CPDF_PageObjectHolder* pObjHolder)
    : obj_holder_(pObjHolder), document_(pObjHolder->GetDocument()) {
  // Copy all page objects, even if they are inactive. They are needed in
  // GenerateModifiedStreams() below.
  for (const auto& pObj : *pObjHolder) {
    page_objects_.emplace_back(pObj.get());
  }
}

CPDF_PageContentGenerator::~CPDF_PageContentGenerator() = default;

void CPDF_PageContentGenerator::GenerateContent() {
  std::map<int32_t, fxcrt::ostringstream> new_stream_data =
      GenerateModifiedStreams();
  // If no streams were regenerated or removed, nothing to do here.
  if (new_stream_data.empty()) {
    return;
  }

  UpdateContentStreams(std::move(new_stream_data));
  UpdateResourcesDict();
}

std::map<int32_t, fxcrt::ostringstream>
CPDF_PageContentGenerator::GenerateModifiedStreams() {
  // Figure out which streams are dirty.
  std::set<int32_t> all_dirty_streams;
  for (auto& pPageObj : page_objects_) {
    // Must include dirty page objects even if they are marked as inactive.
    // Otherwise an inactive object will not be detected that its stream needs
    // to be removed as part of regeneration.
    if (pPageObj->IsDirty()) {
      all_dirty_streams.insert(pPageObj->GetContentStream());
    }
  }
  std::set<int32_t> marked_dirty_streams = obj_holder_->TakeDirtyStreams();
  all_dirty_streams.insert(marked_dirty_streams.begin(),
                           marked_dirty_streams.end());

  // Start regenerating dirty streams.
  std::map<int32_t, fxcrt::ostringstream> streams;
  std::set<int32_t> empty_streams;
  std::unique_ptr<const CPDF_ContentMarks> empty_content_marks =
      std::make_unique<CPDF_ContentMarks>();
  std::map<int32_t, const CPDF_ContentMarks*> current_content_marks;

  for (int32_t dirty_stream : all_dirty_streams) {
    fxcrt::ostringstream buf;

    // Set the default graphic state values. Update CTM to be the identity
    // matrix for the duration of this stream, if it is not already.
    buf << "q\n";
    const CFX_Matrix ctm = obj_holder_->GetCTMAtBeginningOfStream(dirty_stream);
    if (!ctm.IsIdentity()) {
      WriteMatrix(buf, ctm.GetInverse()) << " cm\n";
    }

    ProcessDefaultGraphics(&buf);
    streams[dirty_stream] = std::move(buf);
    empty_streams.insert(dirty_stream);
    current_content_marks[dirty_stream] = empty_content_marks.get();
  }

  // Process the page objects, write into each dirty stream.
  for (auto& pPageObj : page_objects_) {
    if (!pPageObj->IsActive()) {
      continue;
    }

    int stream_index = pPageObj->GetContentStream();
    auto it = streams.find(stream_index);
    if (it == streams.end()) {
      continue;
    }

    fxcrt::ostringstream* buf = &it->second;
    empty_streams.erase(stream_index);
    current_content_marks[stream_index] =
        ProcessContentMarks(buf, pPageObj, current_content_marks[stream_index]);
    ProcessPageObject(buf, pPageObj);
  }

  // Finish dirty streams.
  for (int32_t dirty_stream : all_dirty_streams) {
    CFX_Matrix prev_ctm;
    CFX_Matrix ctm;
    bool affects_ctm;
    if (dirty_stream == 0) {
      // For the first stream, `prev_ctm` is the identity matrix.
      ctm = obj_holder_->GetCTMAtEndOfStream(dirty_stream);
      affects_ctm = !ctm.IsIdentity();
    } else if (dirty_stream > 0) {
      prev_ctm = obj_holder_->GetCTMAtEndOfStream(dirty_stream - 1);
      ctm = obj_holder_->GetCTMAtEndOfStream(dirty_stream);
      affects_ctm = prev_ctm != ctm;
    } else {
      CHECK_EQ(CPDF_PageObject::kNoContentStream, dirty_stream);
      // This is the last stream, so there is no subsequent stream that it can
      // affect.
      affects_ctm = false;
    }

    const bool is_empty = pdfium::Contains(empty_streams, dirty_stream);

    fxcrt::ostringstream* buf = &streams[dirty_stream];
    if (is_empty && !affects_ctm) {
      // Clear to show that this stream needs to be deleted.
      buf->str("");
      continue;
    }

    if (!is_empty) {
      FinishMarks(buf, current_content_marks[dirty_stream]);
    }

    // Return graphics to original state.
    *buf << "Q\n";

    if (affects_ctm) {
      // Update CTM so the next stream gets the expected value.
      CFX_Matrix ctm_difference = prev_ctm.GetInverse() * ctm;
      if (!ctm_difference.IsIdentity()) {
        WriteMatrix(*buf, ctm_difference) << " cm\n";
      }
    }
  }

  return streams;
}

void CPDF_PageContentGenerator::UpdateContentStreams(
    std::map<int32_t, fxcrt::ostringstream>&& new_stream_data) {
  CHECK(!new_stream_data.empty());

  // Make sure default graphics are created.
  default_graphics_name_ = GetOrCreateDefaultGraphics();

  CPDF_PageContentManager page_content_manager(obj_holder_, document_);
  for (auto& pair : new_stream_data) {
    int32_t stream_index = pair.first;
    fxcrt::ostringstream* buf = &pair.second;

    if (stream_index == CPDF_PageObject::kNoContentStream) {
      int new_stream_index =
          pdfium::checked_cast<int>(page_content_manager.AddStream(buf));
      UpdateStreamlessPageObjects(new_stream_index);
      continue;
    }

    if (page_content_manager.HasStreamAtIndex(stream_index)) {
      page_content_manager.UpdateStream(stream_index, buf);
    } else {
      page_content_manager.AddStream(buf);
    }
  }
}

void CPDF_PageContentGenerator::UpdateResourcesDict() {
  RetainPtr<CPDF_Dictionary> resources = obj_holder_->GetMutableResources();
  if (!resources) {
    return;
  }

  // Do not modify shared resource dictionaries. Give this page its own copy.
  if (IsPageResourceShared(document_, obj_holder_->GetDict(), resources)) {
    resources = pdfium::WrapRetain(resources->Clone()->AsMutableDictionary());
    const uint32_t clone_object_number =
        document_->AddIndirectObject(resources);
    obj_holder_->SetResources(resources);
    obj_holder_->GetMutableDict()->SetNewFor<CPDF_Reference>(
        pdfium::page_object::kResources, document_, clone_object_number);
  }

  // Even though `resources` itself is not shared, its dictionary entries may be
  // shared. Checked for that and clone those as well.
  CloneResourcesDictEntries(document_, resources);

  ResourcesMap seen_resources;
  for (auto& page_object : page_objects_) {
    if (!page_object->IsActive()) {
      continue;
    }
    RecordPageObjectResourceUsage(page_object, seen_resources);
  }
  if (!default_graphics_name_.IsEmpty()) {
    seen_resources["ExtGState"].insert(default_graphics_name_);
  }

  RemoveOrRestoreUnusedResources(std::move(resources), seen_resources,
                                 obj_holder_->all_removed_resources_map());
}

ByteString CPDF_PageContentGenerator::RealizeResource(
    const CPDF_Object* pResource,
    ByteStringView type) const {
  DCHECK(pResource);
  if (!obj_holder_->GetResources()) {
    obj_holder_->SetResources(document_->NewIndirect<CPDF_Dictionary>());
    obj_holder_->GetMutableDict()->SetNewFor<CPDF_Reference>(
        pdfium::page_object::kResources, document_,
        obj_holder_->GetResources()->GetObjNum());
  }

  RetainPtr<CPDF_Dictionary> resource_dict =
      obj_holder_->GetMutableResources()->GetOrCreateDictFor(type);
  const auto& all_removed_resources_map =
      obj_holder_->all_removed_resources_map();
  auto it = all_removed_resources_map.find(type);
  const CPDF_PageObjectHolder::RemovedResourceMap* removed_resource_map =
      it != all_removed_resources_map.end() ? &it->second : nullptr;
  ByteString name;
  int idnum = 1;
  while (true) {
    name = ByteString::Format("FX%c%d", type[0], idnum);
    // Avoid name collisions with existing `resource_dict` entries.
    if (resource_dict->KeyExist(name.AsStringView())) {
      idnum++;
      continue;
    }

    // Also avoid collisions with entries in `removed_resource_map`, since those
    // entries may move back into `resource_dict`.
    if (removed_resource_map && pdfium::Contains(*removed_resource_map, name)) {
      idnum++;
      continue;
    }

    resource_dict->SetNewFor<CPDF_Reference>(name, document_,
                                             pResource->GetObjNum());
    return name;
  }
}

bool CPDF_PageContentGenerator::ProcessPageObjects(fxcrt::ostringstream* buf) {
  bool bDirty = false;
  std::unique_ptr<const CPDF_ContentMarks> empty_content_marks =
      std::make_unique<CPDF_ContentMarks>();
  const CPDF_ContentMarks* content_marks = empty_content_marks.get();

  for (auto& pPageObj : page_objects_) {
    if (obj_holder_->IsPage() &&
        (!pPageObj->IsDirty() || !pPageObj->IsActive())) {
      continue;
    }

    bDirty = true;
    content_marks = ProcessContentMarks(buf, pPageObj, content_marks);
    ProcessPageObject(buf, pPageObj);
  }
  FinishMarks(buf, content_marks);
  return bDirty;
}

void CPDF_PageContentGenerator::UpdateStreamlessPageObjects(
    int new_content_stream_index) {
  for (auto& pPageObj : page_objects_) {
    if (!pPageObj->IsActive()) {
      continue;
    }

    if (pPageObj->GetContentStream() == CPDF_PageObject::kNoContentStream) {
      pPageObj->SetContentStream(new_content_stream_index);
    }
  }
}

const CPDF_ContentMarks* CPDF_PageContentGenerator::ProcessContentMarks(
    fxcrt::ostringstream* buf,
    const CPDF_PageObject* pPageObj,
    const CPDF_ContentMarks* pPrev) {
  const CPDF_ContentMarks* pNext = pPageObj->GetContentMarks();
  const size_t first_different = pPrev->FindFirstDifference(pNext);

  // Close all marks that are in prev but not in next.
  // Technically we should iterate backwards to close from the top to the
  // bottom, but since the EMC operators do not identify which mark they are
  // closing, it does not matter.
  for (size_t i = first_different; i < pPrev->CountItems(); ++i) {
    *buf << "EMC\n";
  }

  // Open all marks that are in next but not in prev.
  for (size_t i = first_different; i < pNext->CountItems(); ++i) {
    const CPDF_ContentMarkItem* item = pNext->GetItem(i);

    // Write mark tag.
    *buf << "/" << PDF_NameEncode(item->GetName()) << " ";

    // If there are no parameters, write a BMC (begin marked content) operator.
    if (item->GetParamType() == CPDF_ContentMarkItem::kNone) {
      *buf << "BMC\n";
      continue;
    }

    // If there are parameters, write properties, direct or indirect.
    switch (item->GetParamType()) {
      case CPDF_ContentMarkItem::kDirectDict: {
        CPDF_StringArchiveStream archive_stream(buf);
        item->GetParam()->WriteTo(&archive_stream, nullptr);
        *buf << " ";
        break;
      }
      case CPDF_ContentMarkItem::kPropertiesDict: {
        *buf << "/" << item->GetPropertyName() << " ";
        break;
      }
      case CPDF_ContentMarkItem::kNone:
        NOTREACHED();
    }

    // Write BDC (begin dictionary content) operator.
    *buf << "BDC\n";
  }

  return pNext;
}

void CPDF_PageContentGenerator::FinishMarks(
    fxcrt::ostringstream* buf,
    const CPDF_ContentMarks* pContentMarks) {
  // Technically we should iterate backwards to close from the top to the
  // bottom, but since the EMC operators do not identify which mark they are
  // closing, it does not matter.
  for (size_t i = 0; i < pContentMarks->CountItems(); ++i) {
    *buf << "EMC\n";
  }
}

void CPDF_PageContentGenerator::ProcessPageObject(fxcrt::ostringstream* buf,
                                                  CPDF_PageObject* pPageObj) {
  if (CPDF_ImageObject* pImageObject = pPageObj->AsImage()) {
    ProcessImage(buf, pImageObject);
  } else if (CPDF_FormObject* pFormObj = pPageObj->AsForm()) {
    ProcessForm(buf, pFormObj);
  } else if (CPDF_PathObject* pPathObj = pPageObj->AsPath()) {
    ProcessPath(buf, pPathObj);
  } else if (CPDF_TextObject* pTextObj = pPageObj->AsText()) {
    ProcessText(buf, pTextObj);
  }
  pPageObj->SetDirty(false);
}

void CPDF_PageContentGenerator::ProcessImage(fxcrt::ostringstream* buf,
                                             CPDF_ImageObject* pImageObj) {
  const CFX_Matrix& matrix = pImageObj->matrix();
  if ((matrix.a == 0 && matrix.b == 0) || (matrix.c == 0 && matrix.d == 0)) {
    return;
  }

  RetainPtr<CPDF_Image> pImage = pImageObj->GetImage();
  if (pImage->IsInline()) {
    return;
  }

  RetainPtr<const CPDF_Stream> pStream = pImage->GetStream();
  if (!pStream) {
    return;
  }

  ProcessGraphics(buf, pImageObj);

  if (!matrix.IsIdentity()) {
    WriteMatrix(*buf, matrix) << " cm ";
  }

  bool bWasInline = pStream->IsInline();
  if (bWasInline) {
    pImage->ConvertStreamToIndirectObject();
  }

  ByteString name = RealizeResource(pStream, "XObject");
  pImageObj->SetResourceName(name);

  if (bWasInline) {
    auto* pPageData = CPDF_DocPageData::FromDocument(document_);
    pImageObj->SetImage(pPageData->GetImage(pStream->GetObjNum()));
  }

  *buf << "/" << PDF_NameEncode(name) << " Do";
  EndProcessGraphics(*buf);
}

void CPDF_PageContentGenerator::ProcessForm(fxcrt::ostringstream* buf,
                                            CPDF_FormObject* pFormObj) {
  CPDF_Form* form_xobject = pFormObj->form();
  if (form_xobject->HasDirtyStreams()) {
    // The Form XObject itself has modified content (e.g., an object was
    // removed). We need to regenerate its content stream before using it.
    CPDF_PageContentGenerator form_content_generator(form_xobject);
    form_content_generator.GenerateContent();
  }

  const CFX_Matrix& matrix = pFormObj->form_matrix();
  if ((matrix.a == 0 && matrix.b == 0) || (matrix.c == 0 && matrix.d == 0)) {
    return;
  }

  RetainPtr<const CPDF_Stream> pStream = pFormObj->form()->GetStream();
  if (!pStream) {
    return;
  }

  ByteString name = RealizeResource(pStream.Get(), "XObject");
  pFormObj->SetResourceName(name);

  ProcessGraphics(buf, pFormObj);

  if (!matrix.IsIdentity()) {
    WriteMatrix(*buf, matrix) << " cm ";
  }

  *buf << "/" << PDF_NameEncode(name) << " Do";
  EndProcessGraphics(*buf);
}

// Processing path construction with operators from Table 4.9 of PDF spec 1.7:
// "re" appends a rectangle (here, used only if the whole path is a rectangle)
// "m" moves current point to the given coordinates
// "l" creates a line from current point to the new point
// "c" adds a Bezier curve from current to last point, using the two other
// points as the Bezier control points
// Note: "l", "c" change the current point
// "h" closes the subpath (appends a line from current to starting point)
void CPDF_PageContentGenerator::ProcessPathPoints(fxcrt::ostringstream* buf,
                                                  CPDF_Path* pPath) {
  pdfium::span<const CFX_Path::Point> points = pPath->GetPoints();
  if (pPath->IsRect()) {
    CFX_PointF diff = points[2].point_ - points[0].point_;
    WritePoint(*buf, points[0].point_) << " ";
    WritePoint(*buf, diff) << " re";
    return;
  }
  for (size_t i = 0; i < points.size(); ++i) {
    if (i > 0) {
      *buf << " ";
    }

    WritePoint(*buf, points[i].point_);

    CFX_Path::Point::Type point_type = points[i].type_;
    if (point_type == CFX_Path::Point::Type::kMove) {
      *buf << " m";
    } else if (point_type == CFX_Path::Point::Type::kLine) {
      *buf << " l";
    } else if (point_type == CFX_Path::Point::Type::kBezier) {
      if (i + 2 >= points.size() ||
          !points[i].IsTypeAndOpen(CFX_Path::Point::Type::kBezier) ||
          !points[i + 1].IsTypeAndOpen(CFX_Path::Point::Type::kBezier) ||
          points[i + 2].type_ != CFX_Path::Point::Type::kBezier) {
        // If format is not supported, close the path and paint
        *buf << " h";
        break;
      }
      *buf << " ";
      WritePoint(*buf, points[i + 1].point_) << " ";
      WritePoint(*buf, points[i + 2].point_) << " c";
      i += 2;
    }
    if (points[i].close_figure_) {
      *buf << " h";
    }
  }
}

// Processing path painting with operators from Table 4.10 of PDF spec 1.7:
// Path painting operators: "S", "n", "B", "f", "B*", "f*", depending on
// the filling mode and whether we want stroking the path or not.
void CPDF_PageContentGenerator::ProcessPath(fxcrt::ostringstream* buf,
                                            CPDF_PathObject* pPathObj) {
  ProcessGraphics(buf, pPathObj);

  const CFX_Matrix& matrix = pPathObj->matrix();
  if (!matrix.IsIdentity()) {
    WriteMatrix(*buf, matrix) << " cm ";
  }

  ProcessPathPoints(buf, &pPathObj->path());

  if (pPathObj->has_no_filltype()) {
    *buf << (pPathObj->stroke() ? " S" : " n");
  } else if (pPathObj->has_winding_filltype()) {
    *buf << (pPathObj->stroke() ? " B" : " f");
  } else if (pPathObj->has_alternate_filltype()) {
    *buf << (pPathObj->stroke() ? " B*" : " f*");
  }
  EndProcessGraphics(*buf);
}

// This method supports color operators rg and RGB from Table 4.24 of PDF spec
// 1.7. A color will not be set if the colorspace is not DefaultRGB or the RGB
// values cannot be obtained. The method also adds an external graphics
// dictionary, as described in Section 4.3.4.
// "rg" sets the fill color, "RG" sets the stroke color (using DefaultRGB)
// "w" sets the stroke line width.
// "ca" sets the fill alpha, "CA" sets the stroke alpha.
// "W" and "W*" modify the clipping path using the nonzero winding rule and
// even-odd rules, respectively.
// "q" saves the graphics state, so that the settings can later be reversed
void CPDF_PageContentGenerator::ProcessGraphics(fxcrt::ostringstream* buf,
                                                CPDF_PageObject* pPageObj) {
  *buf << "q ";
  if (WriteColorToStream(*buf, pPageObj->color_state().GetFillColor())) {
    *buf << " rg ";
  }
  if (WriteColorToStream(*buf, pPageObj->color_state().GetStrokeColor())) {
    *buf << " RG ";
  }
  float line_width = pPageObj->graph_state().GetLineWidth();
  if (line_width != 1.0f) {
    WriteFloat(*buf, line_width) << " w ";
  }
  CFX_GraphStateData::LineCap lineCap = pPageObj->graph_state().GetLineCap();
  if (lineCap != CFX_GraphStateData::LineCap::kButt) {
    *buf << static_cast<int>(lineCap) << " J ";
  }
  CFX_GraphStateData::LineJoin lineJoin = pPageObj->graph_state().GetLineJoin();
  if (lineJoin != CFX_GraphStateData::LineJoin::kMiter) {
    *buf << static_cast<int>(lineJoin) << " j ";
  }
  std::vector<float> dash_array = pPageObj->graph_state().GetLineDashArray();
  if (dash_array.size()) {
    *buf << "[";
    for (size_t i = 0; i < dash_array.size(); ++i) {
      if (i > 0) {
        *buf << " ";
      }
      WriteFloat(*buf, dash_array[i]);
    }
    *buf << "] ";
    WriteFloat(*buf, pPageObj->graph_state().GetLineDashPhase()) << " d ";
  }

  const CPDF_ClipPath& clip_path = pPageObj->clip_path();
  if (clip_path.HasRef()) {
    for (size_t i = 0; i < clip_path.GetPathCount(); ++i) {
      CPDF_Path path = clip_path.GetPath(i);
      ProcessPathPoints(buf, &path);
      switch (clip_path.GetClipType(i)) {
        case CFX_FillRenderOptions::FillType::kWinding:
          *buf << " W ";
          break;
        case CFX_FillRenderOptions::FillType::kEvenOdd:
          *buf << " W* ";
          break;
        case CFX_FillRenderOptions::FillType::kNoFill:
          NOTREACHED();
      }

      // Use a no-op path-painting operator to terminate the path without
      // causing any marks to be placed on the page.
      *buf << "n ";
    }
  }

  GraphicsData graphD;
  graphD.fillAlpha = pPageObj->general_state().GetFillAlpha();
  graphD.strokeAlpha = pPageObj->general_state().GetStrokeAlpha();
  graphD.blendType = pPageObj->general_state().GetBlendType();
  if (graphD.fillAlpha == 1.0f && graphD.strokeAlpha == 1.0f &&
      graphD.blendType == BlendMode::kNormal) {
    return;
  }

  ByteString name;
  std::optional<ByteString> maybe_name = obj_holder_->GraphicsMapSearch(graphD);
  if (maybe_name.has_value()) {
    name = std::move(maybe_name.value());
  } else {
    auto gsDict = pdfium::MakeRetain<CPDF_Dictionary>();
    if (graphD.fillAlpha != 1.0f) {
      gsDict->SetNewFor<CPDF_Number>("ca", graphD.fillAlpha);
    }

    if (graphD.strokeAlpha != 1.0f) {
      gsDict->SetNewFor<CPDF_Number>("CA", graphD.strokeAlpha);
    }

    if (graphD.blendType != BlendMode::kNormal) {
      gsDict->SetNewFor<CPDF_Name>("BM",
                                   pPageObj->general_state().GetBlendMode());
    }
    document_->AddIndirectObject(gsDict);
    name = RealizeResource(std::move(gsDict), "ExtGState");
    pPageObj->mutable_general_state().SetGraphicsResourceNames({name});
    obj_holder_->GraphicsMapInsert(graphD, name);
  }
  *buf << "/" << PDF_NameEncode(name) << " gs ";
}

void CPDF_PageContentGenerator::ProcessDefaultGraphics(
    fxcrt::ostringstream* buf) {
  *buf << "0 0 0 RG 0 0 0 rg 1 w "
       << static_cast<int>(CFX_GraphStateData::LineCap::kButt) << " J "
       << static_cast<int>(CFX_GraphStateData::LineJoin::kMiter) << " j\n";
  default_graphics_name_ = GetOrCreateDefaultGraphics();
  *buf << "/" << PDF_NameEncode(default_graphics_name_) << " gs ";
}

ByteString CPDF_PageContentGenerator::GetOrCreateDefaultGraphics() const {
  GraphicsData defaultGraphics;
  defaultGraphics.fillAlpha = 1.0f;
  defaultGraphics.strokeAlpha = 1.0f;
  defaultGraphics.blendType = BlendMode::kNormal;

  std::optional<ByteString> maybe_name =
      obj_holder_->GraphicsMapSearch(defaultGraphics);
  if (maybe_name.has_value()) {
    return maybe_name.value();
  }

  auto gsDict = pdfium::MakeRetain<CPDF_Dictionary>();
  gsDict->SetNewFor<CPDF_Number>("ca", defaultGraphics.fillAlpha);
  gsDict->SetNewFor<CPDF_Number>("CA", defaultGraphics.strokeAlpha);
  gsDict->SetNewFor<CPDF_Name>("BM", "Normal");
  document_->AddIndirectObject(gsDict);
  ByteString name = RealizeResource(std::move(gsDict), "ExtGState");
  obj_holder_->GraphicsMapInsert(defaultGraphics, name);
  return name;
}

// This method adds text to the buffer, BT begins the text object, ET ends it.
// Tm sets the text matrix (allows positioning and transforming text).
// Tf sets the font name (from Font in Resources) and font size.
// Tr sets the text rendering mode.
// Tj sets the actual text, <####...> is used when specifying charcodes.
void CPDF_PageContentGenerator::ProcessText(fxcrt::ostringstream* buf,
                                            CPDF_TextObject* pTextObj) {
  ProcessGraphics(buf, pTextObj);
  *buf << "BT ";

  const CFX_Matrix& matrix = pTextObj->GetTextMatrix();
  if (!matrix.IsIdentity()) {
    WriteMatrix(*buf, matrix) << " Tm ";
  }

  RetainPtr<CPDF_Font> font(pTextObj->GetFont());
  if (!font) {
    font = CPDF_Font::GetStockFont(document_, "Helvetica");
  }

  FontData data;
  const CPDF_FontEncoding* pEncoding = nullptr;
  if (font->IsType1Font()) {
    data.type = "Type1";
    pEncoding = font->AsType1Font()->GetEncoding();
  } else if (font->IsTrueTypeFont()) {
    data.type = "TrueType";
    pEncoding = font->AsTrueTypeFont()->GetEncoding();
  } else if (font->IsCIDFont()) {
    data.type = "Type0";
  } else {
    return;
  }
  data.baseFont = font->GetBaseFontName();

  ByteString dict_name;
  std::optional<ByteString> maybe_name = obj_holder_->FontsMapSearch(data);
  if (maybe_name.has_value()) {
    dict_name = std::move(maybe_name.value());
  } else {
    RetainPtr<const CPDF_Object> pIndirectFont = font->GetFontDict();
    if (pIndirectFont->IsInline()) {
      // In this case we assume it must be a standard font
      auto font_dict = pdfium::MakeRetain<CPDF_Dictionary>();
      font_dict->SetNewFor<CPDF_Name>("Type", "Font");
      font_dict->SetNewFor<CPDF_Name>("Subtype", data.type);
      font_dict->SetNewFor<CPDF_Name>("BaseFont", data.baseFont);
      if (pEncoding) {
        font_dict->SetFor("Encoding",
                          pEncoding->Realize(document_->GetByteStringPool()));
      }
      document_->AddIndirectObject(font_dict);
      pIndirectFont = std::move(font_dict);
    }
    dict_name = RealizeResource(std::move(pIndirectFont), "Font");
    obj_holder_->FontsMapInsert(data, dict_name);
  }
  pTextObj->SetResourceName(dict_name);

  *buf << "/" << PDF_NameEncode(dict_name) << " ";
  WriteFloat(*buf, pTextObj->GetFontSize()) << " Tf ";
  *buf << static_cast<int>(pTextObj->GetTextRenderMode()) << " Tr ";
  ByteString text;
  for (uint32_t charcode : pTextObj->GetCharCodes()) {
    if (charcode != CPDF_Font::kInvalidCharCode) {
      font->AppendChar(&text, charcode);
    }
  }
  *buf << PDF_HexEncodeString(text.AsStringView()) << " Tj ET";
  EndProcessGraphics(*buf);
}
