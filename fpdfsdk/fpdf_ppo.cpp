// Copyright 2014 PDFium Authors. All rights reserved.
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
#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_string_wrappers.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/cpp/fpdf_scopers.h"
#include "third_party/base/check.h"
#include "third_party/base/span.h"

struct XObjectContext {
  CPDF_Document* dest_doc;
  RetainPtr<CPDF_Stream> xobject;
};

namespace {

// Struct that stores sub page origin and scale information.  When importing
// more than one pages onto the same page, most likely the pages will need to be
// scaled down, and scale is in range of (0, 1) exclusive.
struct NupPageSettings {
  CFX_PointF subPageStartPoint;
  float scale;
};

// Calculates the N-up parameters.  When importing multiple pages into one page.
// The space of output page is evenly divided along the X axis and Y axis based
// on the input |nPagesOnXAxis| and |nPagesOnYAxis|.
class NupState {
 public:
  NupState(const CFX_SizeF& pagesize,
           size_t nPagesOnXAxis,
           size_t nPagesOnYAxis);

  // Calculate sub page origin and scale with the source page of |pagesize| and
  // new page of |m_subPageSize|.
  NupPageSettings CalculateNewPagePosition(const CFX_SizeF& pagesize);

 private:
  // Helper function to get the |iSubX|, |iSubY| pair based on |m_subPageIndex|.
  // The space of output page is evenly divided into slots along x and y axis.
  // |iSubX| and |iSubY| are 0-based indices that indicate which allocation
  // slot to use.
  std::pair<size_t, size_t> ConvertPageOrder() const;

  // Given the |iSubX| and |iSubY| subpage position within a page, and a source
  // page with dimensions of |pagesize|, calculate the sub page's origin and
  // scale.
  NupPageSettings CalculatePageEdit(size_t iSubX,
                                    size_t iSubY,
                                    const CFX_SizeF& pagesize) const;

  const CFX_SizeF m_destPageSize;
  const size_t m_nPagesOnXAxis;
  const size_t m_nPagesOnYAxis;
  const size_t m_nPagesPerSheet;
  CFX_SizeF m_subPageSize;

  // A 0-based index, in range of [0, m_nPagesPerSheet - 1).
  size_t m_subPageIndex = 0;
};

NupState::NupState(const CFX_SizeF& pagesize,
                   size_t nPagesOnXAxis,
                   size_t nPagesOnYAxis)
    : m_destPageSize(pagesize),
      m_nPagesOnXAxis(nPagesOnXAxis),
      m_nPagesOnYAxis(nPagesOnYAxis),
      m_nPagesPerSheet(nPagesOnXAxis * nPagesOnYAxis) {
  DCHECK(m_nPagesOnXAxis > 0);
  DCHECK(m_nPagesOnYAxis > 0);
  DCHECK(m_destPageSize.width > 0);
  DCHECK(m_destPageSize.height > 0);

  m_subPageSize.width = m_destPageSize.width / m_nPagesOnXAxis;
  m_subPageSize.height = m_destPageSize.height / m_nPagesOnYAxis;
}

std::pair<size_t, size_t> NupState::ConvertPageOrder() const {
  size_t iSubX = m_subPageIndex % m_nPagesOnXAxis;
  size_t iSubY = m_subPageIndex / m_nPagesOnXAxis;

  // Y Axis, pages start from the top of the output page.
  iSubY = m_nPagesOnYAxis - iSubY - 1;

  return {iSubX, iSubY};
}

NupPageSettings NupState::CalculatePageEdit(size_t iSubX,
                                            size_t iSubY,
                                            const CFX_SizeF& pagesize) const {
  NupPageSettings settings;
  settings.subPageStartPoint.x = iSubX * m_subPageSize.width;
  settings.subPageStartPoint.y = iSubY * m_subPageSize.height;

  const float xScale = m_subPageSize.width / pagesize.width;
  const float yScale = m_subPageSize.height / pagesize.height;
  settings.scale = std::min(xScale, yScale);

  float subWidth = pagesize.width * settings.scale;
  float subHeight = pagesize.height * settings.scale;
  if (xScale > yScale)
    settings.subPageStartPoint.x += (m_subPageSize.width - subWidth) / 2;
  else
    settings.subPageStartPoint.y += (m_subPageSize.height - subHeight) / 2;
  return settings;
}

NupPageSettings NupState::CalculateNewPagePosition(const CFX_SizeF& pagesize) {
  if (m_subPageIndex >= m_nPagesPerSheet)
    m_subPageIndex = 0;

  size_t iSubX;
  size_t iSubY;
  std::tie(iSubX, iSubY) = ConvertPageOrder();
  ++m_subPageIndex;
  return CalculatePageEdit(iSubX, iSubY, pagesize);
}

const CPDF_Object* PageDictGetInheritableTag(const CPDF_Dictionary* pDict,
                                             const ByteString& bsSrcTag) {
  if (!pDict || bsSrcTag.IsEmpty())
    return nullptr;
  if (!pDict->KeyExist(pdfium::page_object::kParent) ||
      !pDict->KeyExist(pdfium::page_object::kType)) {
    return nullptr;
  }

  const CPDF_Name* pName =
      ToName(pDict->GetObjectFor(pdfium::page_object::kType)->GetDirect());
  if (!pName || pName->GetString() != "Page")
    return nullptr;

  const CPDF_Dictionary* pp = ToDictionary(
      pDict->GetObjectFor(pdfium::page_object::kParent)->GetDirect());
  if (!pp)
    return nullptr;

  if (pDict->KeyExist(bsSrcTag))
    return pDict->GetObjectFor(bsSrcTag);

  while (pp) {
    if (pp->KeyExist(bsSrcTag))
      return pp->GetObjectFor(bsSrcTag);
    if (!pp->KeyExist(pdfium::page_object::kParent))
      break;
    pp = ToDictionary(
        pp->GetObjectFor(pdfium::page_object::kParent)->GetDirect());
  }
  return nullptr;
}

bool CopyInheritable(CPDF_Dictionary* pDestPageDict,
                     const CPDF_Dictionary* pSrcPageDict,
                     const ByteString& key) {
  if (pDestPageDict->KeyExist(key))
    return true;

  const CPDF_Object* pInheritable =
      PageDictGetInheritableTag(pSrcPageDict, key);
  if (!pInheritable)
    return false;

  pDestPageDict->SetFor(key, pInheritable->Clone());
  return true;
}

std::vector<uint32_t> GetPageIndices(const CPDF_Document& doc,
                                     const ByteString& bsPageRange) {
  uint32_t nCount = doc.GetPageCount();
  if (!bsPageRange.IsEmpty())
    return ParsePageRangeString(bsPageRange, nCount);

  std::vector<uint32_t> page_indices(nCount);
  std::iota(page_indices.begin(), page_indices.end(), 0);
  return page_indices;
}

class CPDF_PageOrganizer {
 protected:
  CPDF_PageOrganizer(CPDF_Document* pDestDoc, CPDF_Document* pSrcDoc);
  ~CPDF_PageOrganizer();

  // Must be called after construction before doing anything else.
  bool Init();

  bool UpdateReference(CPDF_Object* pObj);

  CPDF_Document* dest() { return m_pDestDoc.Get(); }
  const CPDF_Document* dest() const { return m_pDestDoc.Get(); }

  CPDF_Document* src() { return m_pSrcDoc.Get(); }
  const CPDF_Document* src() const { return m_pSrcDoc.Get(); }

  void AddObjectMapping(uint32_t dwOldPageObj, uint32_t dwNewPageObj) {
    m_ObjectNumberMap[dwOldPageObj] = dwNewPageObj;
  }

  void ClearObjectNumberMap() { m_ObjectNumberMap.clear(); }

 private:
  uint32_t GetNewObjId(CPDF_Reference* pRef);

  UnownedPtr<CPDF_Document> const m_pDestDoc;
  UnownedPtr<CPDF_Document> const m_pSrcDoc;

  // Mapping of source object number to destination object number.
  std::map<uint32_t, uint32_t> m_ObjectNumberMap;
};

CPDF_PageOrganizer::CPDF_PageOrganizer(CPDF_Document* pDestDoc,
                                       CPDF_Document* pSrcDoc)
    : m_pDestDoc(pDestDoc), m_pSrcDoc(pSrcDoc) {}

CPDF_PageOrganizer::~CPDF_PageOrganizer() = default;

bool CPDF_PageOrganizer::Init() {
  DCHECK(m_pDestDoc);
  DCHECK(m_pSrcDoc);

  CPDF_Dictionary* pNewRoot = dest()->GetRoot();
  if (!pNewRoot)
    return false;

  CPDF_Dictionary* pDocInfoDict = dest()->GetInfo();
  if (!pDocInfoDict)
    return false;

  pDocInfoDict->SetNewFor<CPDF_String>("Producer", "PDFium", false);

  ByteString cbRootType = pNewRoot->GetStringFor("Type", ByteString());
  if (cbRootType.IsEmpty())
    pNewRoot->SetNewFor<CPDF_Name>("Type", "Catalog");

  CPDF_Object* pElement = pNewRoot->GetObjectFor("Pages");
  CPDF_Dictionary* pNewPages =
      pElement ? ToDictionary(pElement->GetDirect()) : nullptr;
  if (!pNewPages) {
    pNewPages = dest()->NewIndirect<CPDF_Dictionary>();
    pNewRoot->SetNewFor<CPDF_Reference>("Pages", dest(),
                                        pNewPages->GetObjNum());
  }
  ByteString cbPageType = pNewPages->GetStringFor("Type", ByteString());
  if (cbPageType.IsEmpty())
    pNewPages->SetNewFor<CPDF_Name>("Type", "Pages");

  if (!pNewPages->GetArrayFor("Kids")) {
    auto* pNewArray = dest()->NewIndirect<CPDF_Array>();
    pNewPages->SetNewFor<CPDF_Number>("Count", 0);
    pNewPages->SetNewFor<CPDF_Reference>("Kids", dest(),
                                         pNewArray->GetObjNum());
  }
  return true;
}

bool CPDF_PageOrganizer::UpdateReference(CPDF_Object* pObj) {
  switch (pObj->GetType()) {
    case CPDF_Object::kReference: {
      CPDF_Reference* pReference = pObj->AsReference();
      uint32_t newobjnum = GetNewObjId(pReference);
      if (newobjnum == 0)
        return false;
      pReference->SetRef(dest(), newobjnum);
      return true;
    }
    case CPDF_Object::kDictionary: {
      CPDF_Dictionary* pDict = pObj->AsDictionary();
      std::vector<ByteString> bad_keys;
      {
        CPDF_DictionaryLocker locker(pDict);
        for (const auto& it : locker) {
          const ByteString& key = it.first;
          if (key == "Parent" || key == "Prev" || key == "First")
            continue;
          CPDF_Object* pNextObj = it.second.Get();
          if (!UpdateReference(pNextObj))
            bad_keys.push_back(key);
        }
      }
      for (const auto& key : bad_keys)
        pDict->RemoveFor(key);
      return true;
    }
    case CPDF_Object::kArray: {
      CPDF_Array* pArray = pObj->AsArray();
      for (size_t i = 0; i < pArray->size(); ++i) {
        if (!UpdateReference(pArray->GetObjectAt(i)))
          return false;
      }
      return true;
    }
    case CPDF_Object::kStream: {
      CPDF_Stream* pStream = pObj->AsStream();
      CPDF_Dictionary* pDict = pStream->GetDict();
      return pDict && UpdateReference(pDict);
    }
    default:
      return true;
  }
}

uint32_t CPDF_PageOrganizer::GetNewObjId(CPDF_Reference* pRef) {
  if (!pRef)
    return 0;

  uint32_t dwObjnum = pRef->GetRefObjNum();
  uint32_t dwNewObjNum = 0;
  const auto it = m_ObjectNumberMap.find(dwObjnum);
  if (it != m_ObjectNumberMap.end())
    dwNewObjNum = it->second;
  if (dwNewObjNum)
    return dwNewObjNum;

  CPDF_Object* pDirect = pRef->GetDirect();
  if (!pDirect)
    return 0;

  RetainPtr<CPDF_Object> pClone = pDirect->Clone();
  if (CPDF_Dictionary* pDictClone = pClone->AsDictionary()) {
    if (pDictClone->KeyExist("Type")) {
      ByteString strType = pDictClone->GetStringFor("Type");
      if (strType.EqualNoCase("Pages"))
        return 4;
      if (strType.EqualNoCase("Page"))
        return 0;
    }
  }
  CPDF_Object* pUnownedClone = dest()->AddIndirectObject(std::move(pClone));
  dwNewObjNum = pUnownedClone->GetObjNum();
  AddObjectMapping(dwObjnum, dwNewObjNum);
  if (!UpdateReference(pUnownedClone))
    return 0;

  return dwNewObjNum;
}

// Copies pages from a source document into a destination document.
// This class is intended to be used once via ExportPage() and then destroyed.
class CPDF_PageExporter final : public CPDF_PageOrganizer {
 public:
  CPDF_PageExporter(CPDF_Document* pDestDoc, CPDF_Document* pSrcDoc);
  ~CPDF_PageExporter();

  // For the pages from the source document with |pageIndices| as their page
  // indices, insert them into the destination document at page |nIndex|.
  // |pageIndices| and |nIndex| are 0-based.
  bool ExportPage(pdfium::span<const uint32_t> pageIndices, int nIndex);
};

CPDF_PageExporter::CPDF_PageExporter(CPDF_Document* pDestDoc,
                                     CPDF_Document* pSrcDoc)
    : CPDF_PageOrganizer(pDestDoc, pSrcDoc) {}

CPDF_PageExporter::~CPDF_PageExporter() = default;

bool CPDF_PageExporter::ExportPage(pdfium::span<const uint32_t> pageIndices,
                                   int nIndex) {
  if (!Init())
    return false;

  int curpage = nIndex;
  for (uint32_t pageIndex : pageIndices) {
    CPDF_Dictionary* pDestPageDict = dest()->CreateNewPage(curpage);
    auto* pSrcPageDict = src()->GetPageDictionary(pageIndex);
    if (!pSrcPageDict || !pDestPageDict)
      return false;

    // Clone the page dictionary
    CPDF_DictionaryLocker locker(pSrcPageDict);
    for (const auto& it : locker) {
      const ByteString& cbSrcKeyStr = it.first;
      if (cbSrcKeyStr == pdfium::page_object::kType ||
          cbSrcKeyStr == pdfium::page_object::kParent) {
        continue;
      }

      CPDF_Object* pObj = it.second.Get();
      pDestPageDict->SetFor(cbSrcKeyStr, pObj->Clone());
    }

    // inheritable item
    // Even though some entries are required by the PDF spec, there exist
    // PDFs that omit them. Set some defaults in this case.
    // 1 MediaBox - required
    if (!CopyInheritable(pDestPageDict, pSrcPageDict,
                         pdfium::page_object::kMediaBox)) {
      // Search for "CropBox" in the source page dictionary.
      // If it does not exist, use the default letter size.
      const CPDF_Object* pInheritable = PageDictGetInheritableTag(
          pSrcPageDict, pdfium::page_object::kCropBox);
      if (pInheritable) {
        pDestPageDict->SetFor(pdfium::page_object::kMediaBox,
                              pInheritable->Clone());
      } else {
        // Make the default size letter size (8.5"x11")
        static const CFX_FloatRect kDefaultLetterRect(0, 0, 612, 792);
        pDestPageDict->SetRectFor(pdfium::page_object::kMediaBox,
                                  kDefaultLetterRect);
      }
    }

    // 2 Resources - required
    if (!CopyInheritable(pDestPageDict, pSrcPageDict,
                         pdfium::page_object::kResources)) {
      // Use a default empty resources if it does not exist.
      pDestPageDict->SetNewFor<CPDF_Dictionary>(
          pdfium::page_object::kResources);
    }

    // 3 CropBox - optional
    CopyInheritable(pDestPageDict, pSrcPageDict, pdfium::page_object::kCropBox);
    // 4 Rotate - optional
    CopyInheritable(pDestPageDict, pSrcPageDict, pdfium::page_object::kRotate);

    // Update the reference
    uint32_t dwOldPageObj = pSrcPageDict->GetObjNum();
    uint32_t dwNewPageObj = pDestPageDict->GetObjNum();
    AddObjectMapping(dwOldPageObj, dwNewPageObj);
    UpdateReference(pDestPageDict);
    ++curpage;
  }

  return true;
}

// Copies pages from a source document into a destination document. Creates 1
// page in the destination document for every N source pages. This class is
// intended to be used once via ExportNPagesToOne() and then destroyed.
class CPDF_NPageToOneExporter final : public CPDF_PageOrganizer {
 public:
  CPDF_NPageToOneExporter(CPDF_Document* pDestDoc, CPDF_Document* pSrcDoc);
  ~CPDF_NPageToOneExporter();

  // For the pages from the source document with |pageIndices| as their page
  // indices, insert them into the destination document, starting at page index
  // 0.
  // |pageIndices| is 0-based.
  // |destPageSize| is the destination document page dimensions, measured in
  // PDF "user space" units.
  // |nPagesOnXAxis| and |nPagesOnXAxis| together defines how many source
  // pages fit on one destination page.
  bool ExportNPagesToOne(pdfium::span<const uint32_t> pageIndices,
                         const CFX_SizeF& destPageSize,
                         size_t nPagesOnXAxis,
                         size_t nPagesOnYAxis);

  std::unique_ptr<XObjectContext> CreateXObjectContextFromPage(
      int src_page_index);

 private:
  // Map page object number to XObject object name.
  using PageXObjectMap = std::map<uint32_t, ByteString>;

  // Creates an XObject from |pSrcPage|, or find an existing XObject that
  // represents |pSrcPage|. The transformation matrix is specified in
  // |settings|.
  // Returns the XObject reference surrounded by the transformation matrix.
  ByteString AddSubPage(const RetainPtr<CPDF_Page>& pSrcPage,
                        const NupPageSettings& settings);

  // Creates an XObject from |pSrcPage|. Updates mapping as needed.
  // Returns the name of the newly created XObject.
  ByteString MakeXObjectFromPage(const RetainPtr<CPDF_Page>& pSrcPage);
  CPDF_Stream* MakeXObjectFromPageRaw(const RetainPtr<CPDF_Page>& pSrcPage);

  // Adds |bsContent| as the Contents key in |pDestPageDict|.
  // Adds the objects in |m_XObjectNameToNumberMap| to the XObject dictionary in
  // |pDestPageDict|'s Resources dictionary.
  void FinishPage(CPDF_Dictionary* pDestPageDict, const ByteString& bsContent);

  // Counter for giving new XObjects unique names.
  uint32_t m_nObjectNumber = 0;

  // Keeps track of created XObjects in the current page.
  // Map XObject's object name to it's object number.
  std::map<ByteString, uint32_t> m_XObjectNameToNumberMap;

  // Mapping of source page object number and XObject name of the entire doc.
  // If there are multiple source pages that reference the same object number,
  // they can also share the same created XObject.
  PageXObjectMap m_SrcPageXObjectMap;
};

CPDF_NPageToOneExporter::CPDF_NPageToOneExporter(CPDF_Document* pDestDoc,
                                                 CPDF_Document* pSrcDoc)
    : CPDF_PageOrganizer(pDestDoc, pSrcDoc) {}

CPDF_NPageToOneExporter::~CPDF_NPageToOneExporter() = default;

bool CPDF_NPageToOneExporter::ExportNPagesToOne(
    pdfium::span<const uint32_t> pageIndices,
    const CFX_SizeF& destPageSize,
    size_t nPagesOnXAxis,
    size_t nPagesOnYAxis) {
  if (!Init())
    return false;

  FX_SAFE_SIZE_T nSafePagesPerSheet = nPagesOnXAxis;
  nSafePagesPerSheet *= nPagesOnYAxis;
  if (!nSafePagesPerSheet.IsValid())
    return false;

  ClearObjectNumberMap();
  m_SrcPageXObjectMap.clear();
  size_t nPagesPerSheet = nSafePagesPerSheet.ValueOrDie();
  NupState nupState(destPageSize, nPagesOnXAxis, nPagesOnYAxis);

  size_t curpage = 0;
  const CFX_FloatRect destPageRect(0, 0, destPageSize.width,
                                   destPageSize.height);
  for (size_t iOuterPage = 0; iOuterPage < pageIndices.size();
       iOuterPage += nPagesPerSheet) {
    m_XObjectNameToNumberMap.clear();

    // Create a new page
    CPDF_Dictionary* pDestPageDict = dest()->CreateNewPage(curpage);
    if (!pDestPageDict)
      return false;

    pDestPageDict->SetRectFor(pdfium::page_object::kMediaBox, destPageRect);
    ByteString bsContent;
    size_t iInnerPageMax =
        std::min(iOuterPage + nPagesPerSheet, pageIndices.size());
    for (size_t i = iOuterPage; i < iInnerPageMax; ++i) {
      auto* pSrcPageDict = src()->GetPageDictionary(pageIndices[i]);
      if (!pSrcPageDict)
        return false;

      auto pSrcPage = pdfium::MakeRetain<CPDF_Page>(src(), pSrcPageDict);
      pSrcPage->SetRenderCache(
          std::make_unique<CPDF_PageRenderCache>(pSrcPage.Get()));
      NupPageSettings settings =
          nupState.CalculateNewPagePosition(pSrcPage->GetPageSize());
      bsContent += AddSubPage(pSrcPage, settings);
    }

    FinishPage(pDestPageDict, bsContent);
    ++curpage;
  }

  return true;
}

ByteString CPDF_NPageToOneExporter::AddSubPage(
    const RetainPtr<CPDF_Page>& pSrcPage,
    const NupPageSettings& settings) {
  uint32_t dwSrcPageObjnum = pSrcPage->GetDict()->GetObjNum();
  const auto it = m_SrcPageXObjectMap.find(dwSrcPageObjnum);
  ByteString bsXObjectName = it != m_SrcPageXObjectMap.end()
                                 ? it->second
                                 : MakeXObjectFromPage(pSrcPage);

  CFX_Matrix matrix;
  matrix.Scale(settings.scale, settings.scale);
  matrix.Translate(settings.subPageStartPoint.x, settings.subPageStartPoint.y);

  fxcrt::ostringstream contentStream;
  contentStream << "q\n"
                << matrix.a << " " << matrix.b << " " << matrix.c << " "
                << matrix.d << " " << matrix.e << " " << matrix.f << " cm\n"
                << "/" << bsXObjectName << " Do Q\n";
  return ByteString(contentStream);
}

CPDF_Stream* CPDF_NPageToOneExporter::MakeXObjectFromPageRaw(
    const RetainPtr<CPDF_Page>& pSrcPage) {
  const CPDF_Dictionary* pSrcPageDict = pSrcPage->GetDict();
  const CPDF_Object* pSrcContentObj =
      pSrcPageDict->GetDirectObjectFor(pdfium::page_object::kContents);

  CPDF_Stream* pNewXObject = dest()->NewIndirect<CPDF_Stream>(
      nullptr, 0, dest()->New<CPDF_Dictionary>());
  CPDF_Dictionary* pNewXObjectDict = pNewXObject->GetDict();
  static const char kResourceString[] = "Resources";
  if (!CopyInheritable(pNewXObjectDict, pSrcPageDict, kResourceString)) {
    // Use a default empty resources if it does not exist.
    pNewXObjectDict->SetNewFor<CPDF_Dictionary>(kResourceString);
  }
  uint32_t dwSrcPageObj = pSrcPageDict->GetObjNum();
  uint32_t dwNewXobjectObj = pNewXObjectDict->GetObjNum();
  AddObjectMapping(dwSrcPageObj, dwNewXobjectObj);
  UpdateReference(pNewXObjectDict);

  pNewXObjectDict->SetNewFor<CPDF_Name>("Type", "XObject");
  pNewXObjectDict->SetNewFor<CPDF_Name>("Subtype", "Form");
  pNewXObjectDict->SetNewFor<CPDF_Number>("FormType", 1);
  pNewXObjectDict->SetRectFor("BBox", pSrcPage->GetBBox());
  pNewXObjectDict->SetMatrixFor("Matrix", pSrcPage->GetPageMatrix());

  if (pSrcContentObj) {
    ByteString bsSrcContentStream;
    const CPDF_Array* pSrcContentArray = ToArray(pSrcContentObj);
    if (pSrcContentArray) {
      for (size_t i = 0; i < pSrcContentArray->size(); ++i) {
        const CPDF_Stream* pStream = pSrcContentArray->GetStreamAt(i);
        auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
        pAcc->LoadAllDataFiltered();
        bsSrcContentStream += ByteString(pAcc->GetSpan());
        bsSrcContentStream += "\n";
      }
    } else {
      const CPDF_Stream* pStream = pSrcContentObj->AsStream();
      auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
      pAcc->LoadAllDataFiltered();
      bsSrcContentStream = ByteString(pAcc->GetSpan());
    }
    pNewXObject->SetDataAndRemoveFilter(bsSrcContentStream.raw_span());
  }
  return pNewXObject;
}

ByteString CPDF_NPageToOneExporter::MakeXObjectFromPage(
    const RetainPtr<CPDF_Page>& pSrcPage) {
  CPDF_Stream* pNewXObject = MakeXObjectFromPageRaw(pSrcPage);

  // TODO(xlou): A better name schema to avoid possible object name collision.
  ByteString bsXObjectName = ByteString::Format("X%d", ++m_nObjectNumber);
  m_XObjectNameToNumberMap[bsXObjectName] = pNewXObject->GetObjNum();
  m_SrcPageXObjectMap[pSrcPage->GetDict()->GetObjNum()] = bsXObjectName;
  return bsXObjectName;
}

std::unique_ptr<XObjectContext>
CPDF_NPageToOneExporter::CreateXObjectContextFromPage(int src_page_index) {
  CPDF_Dictionary* src_page_dict = src()->GetPageDictionary(src_page_index);
  if (!src_page_dict)
    return nullptr;

  auto src_page = pdfium::MakeRetain<CPDF_Page>(src(), src_page_dict);
  auto xobject = std::make_unique<XObjectContext>();
  xobject->dest_doc = dest();
  xobject->xobject = MakeXObjectFromPageRaw(src_page);
  return xobject;
}

void CPDF_NPageToOneExporter::FinishPage(CPDF_Dictionary* pDestPageDict,
                                         const ByteString& bsContent) {
  DCHECK(pDestPageDict);

  CPDF_Dictionary* pRes =
      pDestPageDict->GetDictFor(pdfium::page_object::kResources);
  if (!pRes) {
    pRes = pDestPageDict->SetNewFor<CPDF_Dictionary>(
        pdfium::page_object::kResources);
  }

  CPDF_Dictionary* pPageXObject = pRes->GetDictFor("XObject");
  if (!pPageXObject)
    pPageXObject = pRes->SetNewFor<CPDF_Dictionary>("XObject");

  for (auto& it : m_XObjectNameToNumberMap)
    pPageXObject->SetNewFor<CPDF_Reference>(it.first, dest(), it.second);

  auto pDict = dest()->New<CPDF_Dictionary>();
  CPDF_Stream* pStream =
      dest()->NewIndirect<CPDF_Stream>(nullptr, 0, std::move(pDict));
  pStream->SetData(bsContent.raw_span());
  pDestPageDict->SetNewFor<CPDF_Reference>(pdfium::page_object::kContents,
                                           dest(), pStream->GetObjNum());
}

}  // namespace

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_ImportPagesByIndex(FPDF_DOCUMENT dest_doc,
                        FPDF_DOCUMENT src_doc,
                        const int* page_indices,
                        unsigned long length,
                        int index) {
  CPDF_Document* pDestDoc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!dest_doc)
    return false;

  CPDF_Document* pSrcDoc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!pSrcDoc)
    return false;

  CPDF_PageExporter exporter(pDestDoc, pSrcDoc);

  if (!page_indices) {
    std::vector<uint32_t> page_indices_vec(pSrcDoc->GetPageCount());
    std::iota(page_indices_vec.begin(), page_indices_vec.end(), 0);
    return exporter.ExportPage(page_indices_vec, index);
  }

  if (length == 0)
    return false;

  return exporter.ExportPage(
      pdfium::make_span(reinterpret_cast<const uint32_t*>(page_indices),
                        length),
      index);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_ImportPages(FPDF_DOCUMENT dest_doc,
                                                     FPDF_DOCUMENT src_doc,
                                                     FPDF_BYTESTRING pagerange,
                                                     int index) {
  CPDF_Document* pDestDoc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!dest_doc)
    return false;

  CPDF_Document* pSrcDoc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!pSrcDoc)
    return false;

  std::vector<uint32_t> page_indices = GetPageIndices(*pSrcDoc, pagerange);
  if (page_indices.empty())
    return false;

  CPDF_PageExporter exporter(pDestDoc, pSrcDoc);
  return exporter.ExportPage(page_indices, index);
}

FPDF_EXPORT FPDF_DOCUMENT FPDF_CALLCONV
FPDF_ImportNPagesToOne(FPDF_DOCUMENT src_doc,
                       float output_width,
                       float output_height,
                       size_t num_pages_on_x_axis,
                       size_t num_pages_on_y_axis) {
  CPDF_Document* pSrcDoc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!pSrcDoc)
    return nullptr;

  if (output_width <= 0 || output_height <= 0 || num_pages_on_x_axis <= 0 ||
      num_pages_on_y_axis <= 0) {
    return nullptr;
  }

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  if (!output_doc)
    return nullptr;

  CPDF_Document* pDestDoc = CPDFDocumentFromFPDFDocument(output_doc.get());
  DCHECK(pDestDoc);

  std::vector<uint32_t> page_indices = GetPageIndices(*pSrcDoc, ByteString());
  if (page_indices.empty())
    return nullptr;

  if (num_pages_on_x_axis == 1 && num_pages_on_y_axis == 1) {
    CPDF_PageExporter exporter(pDestDoc, pSrcDoc);
    if (!exporter.ExportPage(page_indices, 0))
      return nullptr;
    return output_doc.release();
  }

  CPDF_NPageToOneExporter exporter(pDestDoc, pSrcDoc);
  if (!exporter.ExportNPagesToOne(page_indices,
                                  CFX_SizeF(output_width, output_height),
                                  num_pages_on_x_axis, num_pages_on_y_axis)) {
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

  // If used directly with std::make_unique(), linking fails.
  // Build toolchain bug?
  constexpr int kNoContentStream = CPDF_PageObject::kNoContentStream;
  auto form = std::make_unique<CPDF_Form>(xobj->dest_doc, nullptr,
                                          xobj->xobject.Get(), nullptr);
  auto form_object = std::make_unique<CPDF_FormObject>(
      kNoContentStream, std::move(form), CFX_Matrix());
  return FPDFPageObjectFromCPDFPageObject(form_object.release());
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_CopyViewerPreferences(FPDF_DOCUMENT dest_doc, FPDF_DOCUMENT src_doc) {
  CPDF_Document* pDstDoc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!pDstDoc)
    return false;

  CPDF_Document* pSrcDoc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!pSrcDoc)
    return false;

  const CPDF_Dictionary* pSrcDict = pSrcDoc->GetRoot();
  pSrcDict = pSrcDict->GetDictFor("ViewerPreferences");
  if (!pSrcDict)
    return false;

  CPDF_Dictionary* pDstDict = pDstDoc->GetRoot();
  if (!pDstDict)
    return false;

  pDstDict->SetFor("ViewerPreferences", pSrcDict->CloneDirectObject());
  return true;
}
