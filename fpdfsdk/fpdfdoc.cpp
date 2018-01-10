// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_doc.h"

#include <memory>
#include <set>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfdoc/cpdf_bookmark.h"
#include "core/fpdfdoc/cpdf_bookmarktree.h"
#include "core/fpdfdoc/cpdf_dest.h"
#include "core/fpdfdoc/cpdf_pagelabel.h"
#include "fpdfsdk/fsdk_define.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

CPDF_Bookmark FindBookmark(const CPDF_BookmarkTree& tree,
                           CPDF_Bookmark bookmark,
                           const WideString& title,
                           std::set<CPDF_Dictionary*>* visited) {
  // Return if already checked to avoid circular calling.
  if (pdfium::ContainsKey(*visited, bookmark.GetDict()))
    return CPDF_Bookmark();
  visited->insert(bookmark.GetDict());

  if (bookmark.GetDict() &&
      bookmark.GetTitle().CompareNoCase(title.c_str()) == 0) {
    // First check this item.
    return bookmark;
  }

  // Go into children items.
  CPDF_Bookmark child = tree.GetFirstChild(bookmark);
  while (child.GetDict() && !pdfium::ContainsKey(*visited, child.GetDict())) {
    // Check this item and its children.
    CPDF_Bookmark found = FindBookmark(tree, child, title, visited);
    if (found.GetDict())
      return found;
    child = tree.GetNextSibling(child);
  }
  return CPDF_Bookmark();
}

CPDF_LinkList* GetLinkList(CPDF_Page* page) {
  if (!page)
    return nullptr;

  CPDF_Document* pDoc = page->m_pDocument.Get();
  std::unique_ptr<CPDF_LinkList>* pHolder = pDoc->LinksContext();
  if (!pHolder->get())
    *pHolder = pdfium::MakeUnique<CPDF_LinkList>();
  return pHolder->get();
}

}  // namespace

FPDF_EXPORT FPDF_BOOKMARK FPDF_CALLCONV
FPDFBookmark_GetFirstChild(FPDF_DOCUMENT document, FPDF_BOOKMARK pDict) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;
  CPDF_BookmarkTree tree(pDoc);
  CPDF_Bookmark bookmark =
      CPDF_Bookmark(ToDictionary(static_cast<CPDF_Object*>(pDict)));
  return tree.GetFirstChild(bookmark).GetDict();
}

FPDF_EXPORT FPDF_BOOKMARK FPDF_CALLCONV
FPDFBookmark_GetNextSibling(FPDF_DOCUMENT document, FPDF_BOOKMARK pDict) {
  if (!pDict)
    return nullptr;
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;
  CPDF_BookmarkTree tree(pDoc);
  CPDF_Bookmark bookmark =
      CPDF_Bookmark(ToDictionary(static_cast<CPDF_Object*>(pDict)));
  return tree.GetNextSibling(bookmark).GetDict();
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFBookmark_GetTitle(FPDF_BOOKMARK pDict, void* buffer, unsigned long buflen) {
  if (!pDict)
    return 0;
  CPDF_Bookmark bookmark(ToDictionary(static_cast<CPDF_Object*>(pDict)));
  WideString title = bookmark.GetTitle();
  return Utf16EncodeMaybeCopyAndReturnLength(title, buffer, buflen);
}

FPDF_EXPORT FPDF_BOOKMARK FPDF_CALLCONV
FPDFBookmark_Find(FPDF_DOCUMENT document, FPDF_WIDESTRING title) {
  if (!title || title[0] == 0)
    return nullptr;
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;
  CPDF_BookmarkTree tree(pDoc);
  size_t len = WideString::WStringLength(title);
  WideString encodedTitle = WideString::FromUTF16LE(title, len);
  std::set<CPDF_Dictionary*> visited;
  return FindBookmark(tree, CPDF_Bookmark(), encodedTitle, &visited).GetDict();
}

FPDF_EXPORT FPDF_DEST FPDF_CALLCONV FPDFBookmark_GetDest(FPDF_DOCUMENT document,
                                                         FPDF_BOOKMARK pDict) {
  if (!pDict)
    return nullptr;
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;
  CPDF_Bookmark bookmark(ToDictionary(static_cast<CPDF_Object*>(pDict)));
  CPDF_Dest dest = bookmark.GetDest(pDoc);
  if (dest.GetObject())
    return dest.GetObject();
  // If this bookmark is not directly associated with a dest, we try to get
  // action
  CPDF_Action action = bookmark.GetAction();
  if (!action.GetDict())
    return nullptr;
  return action.GetDest(pDoc).GetObject();
}

FPDF_EXPORT FPDF_ACTION FPDF_CALLCONV
FPDFBookmark_GetAction(FPDF_BOOKMARK pDict) {
  if (!pDict)
    return nullptr;
  CPDF_Bookmark bookmark(ToDictionary(static_cast<CPDF_Object*>(pDict)));
  return bookmark.GetAction().GetDict();
}

FPDF_EXPORT unsigned long FPDF_CALLCONV FPDFAction_GetType(FPDF_ACTION pDict) {
  if (!pDict)
    return PDFACTION_UNSUPPORTED;

  CPDF_Action action(ToDictionary(static_cast<CPDF_Object*>(pDict)));
  CPDF_Action::ActionType type = action.GetType();
  switch (type) {
    case CPDF_Action::GoTo:
      return PDFACTION_GOTO;
    case CPDF_Action::GoToR:
      return PDFACTION_REMOTEGOTO;
    case CPDF_Action::URI:
      return PDFACTION_URI;
    case CPDF_Action::Launch:
      return PDFACTION_LAUNCH;
    default:
      return PDFACTION_UNSUPPORTED;
  }
}

FPDF_EXPORT FPDF_DEST FPDF_CALLCONV FPDFAction_GetDest(FPDF_DOCUMENT document,
                                                       FPDF_ACTION pDict) {
  if (!pDict)
    return nullptr;
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;
  CPDF_Action action(ToDictionary(static_cast<CPDF_Object*>(pDict)));
  return action.GetDest(pDoc).GetObject();
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFAction_GetFilePath(FPDF_ACTION pDict, void* buffer, unsigned long buflen) {
  unsigned long type = FPDFAction_GetType(pDict);
  if (type != PDFACTION_REMOTEGOTO && type != PDFACTION_LAUNCH)
    return 0;

  CPDF_Action action(ToDictionary(static_cast<CPDF_Object*>(pDict)));
  ByteString path = action.GetFilePath().UTF8Encode();
  unsigned long len = path.GetLength() + 1;
  if (buffer && len <= buflen)
    memcpy(buffer, path.c_str(), len);
  return len;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFAction_GetURIPath(FPDF_DOCUMENT document,
                      FPDF_ACTION pDict,
                      void* buffer,
                      unsigned long buflen) {
  if (!pDict)
    return 0;
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;
  CPDF_Action action(ToDictionary(static_cast<CPDF_Object*>(pDict)));
  ByteString path = action.GetURI(pDoc);
  unsigned long len = path.GetLength() + 1;
  if (buffer && len <= buflen)
    memcpy(buffer, path.c_str(), len);
  return len;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFDest_GetPageIndex(FPDF_DOCUMENT document, FPDF_DEST pDict) {
  if (!pDict)
    return 0;
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;
  CPDF_Dest dest(static_cast<CPDF_Array*>(pDict));
  return dest.GetPageIndex(pDoc);
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFDest_GetView(FPDF_DEST pDict,
                 unsigned long* pNumParams,
                 FS_FLOAT* pParams) {
  if (!pDict) {
    *pNumParams = 0;
    return 0;
  }

  CPDF_Dest dest(static_cast<CPDF_Array*>(pDict));
  unsigned long nParams = dest.GetNumParams();
  ASSERT(nParams <= 4);
  *pNumParams = nParams;
  for (unsigned long i = 0; i < nParams; ++i)
    pParams[i] = dest.GetParam(i);
  return dest.GetZoomMode();
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFDest_GetLocationInPage(FPDF_DEST pDict,
                           FPDF_BOOL* hasXVal,
                           FPDF_BOOL* hasYVal,
                           FPDF_BOOL* hasZoomVal,
                           FS_FLOAT* x,
                           FS_FLOAT* y,
                           FS_FLOAT* zoom) {
  if (!pDict)
    return false;

  auto dest = pdfium::MakeUnique<CPDF_Dest>(static_cast<CPDF_Object*>(pDict));

  // FPDF_BOOL is an int, GetXYZ expects bools.
  bool bHasX;
  bool bHasY;
  bool bHasZoom;
  if (!dest->GetXYZ(&bHasX, &bHasY, &bHasZoom, x, y, zoom))
    return false;

  *hasXVal = bHasX;
  *hasYVal = bHasY;
  *hasZoomVal = bHasZoom;
  return true;
}

FPDF_EXPORT FPDF_LINK FPDF_CALLCONV FPDFLink_GetLinkAtPoint(FPDF_PAGE page,
                                                            double x,
                                                            double y) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return nullptr;

  CPDF_LinkList* pLinkList = GetLinkList(pPage);
  if (!pLinkList)
    return nullptr;

  return pLinkList
      ->GetLinkAtPoint(pPage,
                       CFX_PointF(static_cast<float>(x), static_cast<float>(y)),
                       nullptr)
      .GetDict();
}

FPDF_EXPORT int FPDF_CALLCONV FPDFLink_GetLinkZOrderAtPoint(FPDF_PAGE page,
                                                            double x,
                                                            double y) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return -1;

  CPDF_LinkList* pLinkList = GetLinkList(pPage);
  if (!pLinkList)
    return -1;

  int z_order = -1;
  pLinkList->GetLinkAtPoint(
      pPage, CFX_PointF(static_cast<float>(x), static_cast<float>(y)),
      &z_order);
  return z_order;
}

FPDF_EXPORT FPDF_DEST FPDF_CALLCONV FPDFLink_GetDest(FPDF_DOCUMENT document,
                                                     FPDF_LINK pDict) {
  if (!pDict)
    return nullptr;
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;
  CPDF_Link link(ToDictionary(static_cast<CPDF_Object*>(pDict)));
  FPDF_DEST dest = link.GetDest(pDoc).GetObject();
  if (dest)
    return dest;
  // If this link is not directly associated with a dest, we try to get action
  CPDF_Action action = link.GetAction();
  if (!action.GetDict())
    return nullptr;
  return action.GetDest(pDoc).GetObject();
}

FPDF_EXPORT FPDF_ACTION FPDF_CALLCONV FPDFLink_GetAction(FPDF_LINK pDict) {
  if (!pDict)
    return nullptr;

  CPDF_Link link(ToDictionary(static_cast<CPDF_Object*>(pDict)));
  return link.GetAction().GetDict();
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFLink_Enumerate(FPDF_PAGE page,
                                                       int* startPos,
                                                       FPDF_LINK* linkAnnot) {
  if (!startPos || !linkAnnot)
    return false;
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !pPage->m_pFormDict)
    return false;
  CPDF_Array* pAnnots = pPage->m_pFormDict->GetArrayFor("Annots");
  if (!pAnnots)
    return false;
  for (size_t i = *startPos; i < pAnnots->GetCount(); i++) {
    CPDF_Dictionary* pDict =
        ToDictionary(static_cast<CPDF_Object*>(pAnnots->GetDirectObjectAt(i)));
    if (!pDict)
      continue;
    if (pDict->GetStringFor("Subtype") == "Link") {
      *startPos = static_cast<int>(i + 1);
      *linkAnnot = static_cast<FPDF_LINK>(pDict);
      return true;
    }
  }
  return false;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFLink_GetAnnotRect(FPDF_LINK linkAnnot,
                                                          FS_RECTF* rect) {
  if (!linkAnnot || !rect)
    return false;
  CPDF_Dictionary* pAnnotDict =
      ToDictionary(static_cast<CPDF_Object*>(linkAnnot));
  FSRECTFFromCFXFloatRect(pAnnotDict->GetRectFor("Rect"), rect);
  return true;
}

FPDF_EXPORT int FPDF_CALLCONV FPDFLink_CountQuadPoints(FPDF_LINK linkAnnot) {
  if (!linkAnnot)
    return 0;
  CPDF_Dictionary* pAnnotDict =
      ToDictionary(static_cast<CPDF_Object*>(linkAnnot));
  CPDF_Array* pArray = pAnnotDict->GetArrayFor("QuadPoints");
  if (!pArray)
    return 0;
  return static_cast<int>(pArray->GetCount() / 8);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFLink_GetQuadPoints(FPDF_LINK linkAnnot,
                       int quadIndex,
                       FS_QUADPOINTSF* quadPoints) {
  if (!linkAnnot || !quadPoints)
    return false;
  CPDF_Dictionary* pAnnotDict =
      ToDictionary(static_cast<CPDF_Object*>(linkAnnot));
  CPDF_Array* pArray = pAnnotDict->GetArrayFor("QuadPoints");
  if (!pArray)
    return false;

  if (quadIndex < 0 ||
      static_cast<size_t>(quadIndex) >= pArray->GetCount() / 8 ||
      (static_cast<size_t>(quadIndex * 8 + 7) >= pArray->GetCount())) {
    return false;
  }

  quadPoints->x1 = pArray->GetNumberAt(quadIndex * 8);
  quadPoints->y1 = pArray->GetNumberAt(quadIndex * 8 + 1);
  quadPoints->x2 = pArray->GetNumberAt(quadIndex * 8 + 2);
  quadPoints->y2 = pArray->GetNumberAt(quadIndex * 8 + 3);
  quadPoints->x3 = pArray->GetNumberAt(quadIndex * 8 + 4);
  quadPoints->y3 = pArray->GetNumberAt(quadIndex * 8 + 5);
  quadPoints->x4 = pArray->GetNumberAt(quadIndex * 8 + 6);
  quadPoints->y4 = pArray->GetNumberAt(quadIndex * 8 + 7);
  return true;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV FPDF_GetMetaText(FPDF_DOCUMENT document,
                                                         FPDF_BYTESTRING tag,
                                                         void* buffer,
                                                         unsigned long buflen) {
  if (!tag)
    return 0;
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;
  pDoc->LoadDocumentInfo();
  const CPDF_Dictionary* pInfo = pDoc->GetInfo();
  if (!pInfo)
    return 0;
  WideString text = pInfo->GetUnicodeTextFor(tag);
  return Utf16EncodeMaybeCopyAndReturnLength(text, buffer, buflen);
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_GetPageLabel(FPDF_DOCUMENT document,
                  int page_index,
                  void* buffer,
                  unsigned long buflen) {
  if (page_index < 0)
    return 0;

  // CPDF_PageLabel can deal with NULL |document|.
  CPDF_PageLabel label(CPDFDocumentFromFPDFDocument(document));
  Optional<WideString> str = label.GetLabel(page_index);
  return str.has_value()
             ? Utf16EncodeMaybeCopyAndReturnLength(str.value(), buffer, buflen)
             : 0;
}
