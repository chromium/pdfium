// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_doc.h"

#include <memory>
#include <set>
#include <utility>

#include "constants/form_fields.h"
#include "core/fpdfapi/page/cpdf_annotcontext.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfdoc/cpdf_aaction.h"
#include "core/fpdfdoc/cpdf_bookmark.h"
#include "core/fpdfdoc/cpdf_bookmarktree.h"
#include "core/fpdfdoc/cpdf_dest.h"
#include "core/fpdfdoc/cpdf_linklist.h"
#include "core/fpdfdoc/cpdf_pagelabel.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/fpdf_formfill.h"
#include "third_party/base/stl_util.h"

namespace {

CPDF_Bookmark FindBookmark(const CPDF_BookmarkTree& tree,
                           CPDF_Bookmark bookmark,
                           const WideString& title,
                           std::set<const CPDF_Dictionary*>* visited) {
  // Return if already checked to avoid circular calling.
  if (pdfium::Contains(*visited, bookmark.GetDict()))
    return CPDF_Bookmark();
  visited->insert(bookmark.GetDict());

  if (bookmark.GetDict() &&
      bookmark.GetTitle().CompareNoCase(title.c_str()) == 0) {
    // First check this item.
    return bookmark;
  }

  // Go into children items.
  CPDF_Bookmark child = tree.GetFirstChild(&bookmark);
  while (child.GetDict() && !pdfium::Contains(*visited, child.GetDict())) {
    // Check this item and its children.
    CPDF_Bookmark found = FindBookmark(tree, child, title, visited);
    if (found.GetDict())
      return found;
    child = tree.GetNextSibling(&child);
  }
  return CPDF_Bookmark();
}

CPDF_LinkList* GetLinkList(CPDF_Page* page) {
  CPDF_Document* pDoc = page->GetDocument();
  auto* pList = static_cast<CPDF_LinkList*>(pDoc->GetLinksContext());
  if (pList)
    return pList;

  auto pNewList = std::make_unique<CPDF_LinkList>();
  pList = pNewList.get();
  pDoc->SetLinksContext(std::move(pNewList));
  return pList;
}

}  // namespace

FPDF_EXPORT FPDF_BOOKMARK FPDF_CALLCONV
FPDFBookmark_GetFirstChild(FPDF_DOCUMENT document, FPDF_BOOKMARK bookmark) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;
  CPDF_BookmarkTree tree(pDoc);
  CPDF_Bookmark cBookmark(CPDFDictionaryFromFPDFBookmark(bookmark));
  return FPDFBookmarkFromCPDFDictionary(
      tree.GetFirstChild(&cBookmark).GetDict());
}

FPDF_EXPORT FPDF_BOOKMARK FPDF_CALLCONV
FPDFBookmark_GetNextSibling(FPDF_DOCUMENT document, FPDF_BOOKMARK bookmark) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  if (!bookmark)
    return nullptr;

  CPDF_BookmarkTree tree(pDoc);
  CPDF_Bookmark cBookmark(CPDFDictionaryFromFPDFBookmark(bookmark));
  return FPDFBookmarkFromCPDFDictionary(
      tree.GetNextSibling(&cBookmark).GetDict());
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFBookmark_GetTitle(FPDF_BOOKMARK bookmark,
                      void* buffer,
                      unsigned long buflen) {
  if (!bookmark)
    return 0;
  CPDF_Bookmark cBookmark(CPDFDictionaryFromFPDFBookmark(bookmark));
  WideString title = cBookmark.GetTitle();
  return Utf16EncodeMaybeCopyAndReturnLength(title, buffer, buflen);
}

FPDF_EXPORT FPDF_BOOKMARK FPDF_CALLCONV
FPDFBookmark_Find(FPDF_DOCUMENT document, FPDF_WIDESTRING title) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  WideString encodedTitle = WideStringFromFPDFWideString(title);
  if (encodedTitle.IsEmpty())
    return nullptr;

  CPDF_BookmarkTree tree(pDoc);
  std::set<const CPDF_Dictionary*> visited;
  return FPDFBookmarkFromCPDFDictionary(
      FindBookmark(tree, CPDF_Bookmark(), encodedTitle, &visited).GetDict());
}

FPDF_EXPORT FPDF_DEST FPDF_CALLCONV
FPDFBookmark_GetDest(FPDF_DOCUMENT document, FPDF_BOOKMARK bookmark) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  if (!bookmark)
    return nullptr;

  CPDF_Bookmark cBookmark(CPDFDictionaryFromFPDFBookmark(bookmark));
  CPDF_Dest dest = cBookmark.GetDest(pDoc);
  if (dest.GetArray())
    return FPDFDestFromCPDFArray(dest.GetArray());
  // If this bookmark is not directly associated with a dest, we try to get
  // action
  CPDF_Action action = cBookmark.GetAction();
  if (!action.GetDict())
    return nullptr;
  return FPDFDestFromCPDFArray(action.GetDest(pDoc).GetArray());
}

FPDF_EXPORT FPDF_ACTION FPDF_CALLCONV
FPDFBookmark_GetAction(FPDF_BOOKMARK bookmark) {
  if (!bookmark)
    return nullptr;

  CPDF_Bookmark cBookmark(CPDFDictionaryFromFPDFBookmark(bookmark));
  return FPDFActionFromCPDFDictionary(cBookmark.GetAction().GetDict());
}

FPDF_EXPORT unsigned long FPDF_CALLCONV FPDFAction_GetType(FPDF_ACTION action) {
  if (!action)
    return PDFACTION_UNSUPPORTED;

  CPDF_Action cAction(CPDFDictionaryFromFPDFAction(action));
  CPDF_Action::ActionType type = cAction.GetType();
  switch (type) {
    case CPDF_Action::GoTo:
      return PDFACTION_GOTO;
    case CPDF_Action::GoToR:
      return PDFACTION_REMOTEGOTO;
    case CPDF_Action::GoToE:
      return PDFACTION_EMBEDDEDGOTO;
    case CPDF_Action::URI:
      return PDFACTION_URI;
    case CPDF_Action::Launch:
      return PDFACTION_LAUNCH;
    default:
      return PDFACTION_UNSUPPORTED;
  }
}

FPDF_EXPORT FPDF_DEST FPDF_CALLCONV FPDFAction_GetDest(FPDF_DOCUMENT document,
                                                       FPDF_ACTION action) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  unsigned long type = FPDFAction_GetType(action);
  if (type != PDFACTION_GOTO && type != PDFACTION_REMOTEGOTO &&
      type != PDFACTION_EMBEDDEDGOTO) {
    return nullptr;
  }
  CPDF_Action cAction(CPDFDictionaryFromFPDFAction(action));
  return FPDFDestFromCPDFArray(cAction.GetDest(pDoc).GetArray());
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFAction_GetFilePath(FPDF_ACTION action, void* buffer, unsigned long buflen) {
  unsigned long type = FPDFAction_GetType(action);
  if (type != PDFACTION_REMOTEGOTO && type != PDFACTION_EMBEDDEDGOTO &&
      type != PDFACTION_LAUNCH) {
    return 0;
  }

  CPDF_Action cAction(CPDFDictionaryFromFPDFAction(action));
  ByteString path = cAction.GetFilePath().ToUTF8();
  return NulTerminateMaybeCopyAndReturnLength(path, buffer, buflen);
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFAction_GetURIPath(FPDF_DOCUMENT document,
                      FPDF_ACTION action,
                      void* buffer,
                      unsigned long buflen) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;

  unsigned long type = FPDFAction_GetType(action);
  if (type != PDFACTION_URI)
    return 0;

  CPDF_Action cAction(CPDFDictionaryFromFPDFAction(action));
  ByteString path = cAction.GetURI(pDoc);
  unsigned long len = path.GetLength() + 1;
  if (buffer && len <= buflen)
    memcpy(buffer, path.c_str(), len);
  return len;
}

FPDF_EXPORT int FPDF_CALLCONV FPDFDest_GetDestPageIndex(FPDF_DOCUMENT document,
                                                        FPDF_DEST dest) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return -1;

  if (!dest)
    return -1;

  CPDF_Dest destination(CPDFArrayFromFPDFDest(dest));
  return destination.GetDestPageIndex(pDoc);
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFDest_GetView(FPDF_DEST dest, unsigned long* pNumParams, FS_FLOAT* pParams) {
  if (!dest) {
    *pNumParams = 0;
    return 0;
  }

  CPDF_Dest destination(CPDFArrayFromFPDFDest(dest));
  unsigned long nParams = destination.GetNumParams();
  ASSERT(nParams <= 4);
  *pNumParams = nParams;
  for (unsigned long i = 0; i < nParams; ++i)
    pParams[i] = destination.GetParam(i);
  return destination.GetZoomMode();
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFDest_GetLocationInPage(FPDF_DEST dest,
                           FPDF_BOOL* hasXVal,
                           FPDF_BOOL* hasYVal,
                           FPDF_BOOL* hasZoomVal,
                           FS_FLOAT* x,
                           FS_FLOAT* y,
                           FS_FLOAT* zoom) {
  if (!dest)
    return false;

  auto destination = std::make_unique<CPDF_Dest>(CPDFArrayFromFPDFDest(dest));

  // FPDF_BOOL is an int, GetXYZ expects bools.
  bool bHasX;
  bool bHasY;
  bool bHasZoom;
  if (!destination->GetXYZ(&bHasX, &bHasY, &bHasZoom, x, y, zoom))
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

  CPDF_Link link = pLinkList->GetLinkAtPoint(
      pPage, CFX_PointF(static_cast<float>(x), static_cast<float>(y)), nullptr);

  return FPDFLinkFromCPDFDictionary(link.GetDict());
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
                                                     FPDF_LINK link) {
  if (!link)
    return nullptr;
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;
  CPDF_Link cLink(CPDFDictionaryFromFPDFLink(link));
  FPDF_DEST dest = FPDFDestFromCPDFArray(cLink.GetDest(pDoc).GetArray());
  if (dest)
    return dest;
  // If this link is not directly associated with a dest, we try to get action
  CPDF_Action action = cLink.GetAction();
  if (!action.GetDict())
    return nullptr;
  return FPDFDestFromCPDFArray(action.GetDest(pDoc).GetArray());
}

FPDF_EXPORT FPDF_ACTION FPDF_CALLCONV FPDFLink_GetAction(FPDF_LINK link) {
  if (!link)
    return nullptr;

  CPDF_Link cLink(CPDFDictionaryFromFPDFLink(link));
  return FPDFActionFromCPDFDictionary(cLink.GetAction().GetDict());
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFLink_Enumerate(FPDF_PAGE page,
                                                       int* start_pos,
                                                       FPDF_LINK* link_annot) {
  if (!start_pos || !link_annot)
    return false;
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return false;
  CPDF_Array* pAnnots = pPage->GetDict()->GetArrayFor("Annots");
  if (!pAnnots)
    return false;
  for (size_t i = *start_pos; i < pAnnots->size(); i++) {
    CPDF_Dictionary* pDict = ToDictionary(pAnnots->GetDirectObjectAt(i));
    if (!pDict)
      continue;
    if (pDict->GetStringFor("Subtype") == "Link") {
      *start_pos = static_cast<int>(i + 1);
      *link_annot = FPDFLinkFromCPDFDictionary(pDict);
      return true;
    }
  }
  return false;
}

FPDF_EXPORT FPDF_ANNOTATION FPDF_CALLCONV
FPDFLink_GetAnnot(FPDF_PAGE page, FPDF_LINK link_annot) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  CPDF_Dictionary* pAnnotDict = CPDFDictionaryFromFPDFLink(link_annot);
  if (!pPage || !pAnnotDict)
    return nullptr;

  auto pAnnotContext = std::make_unique<CPDF_AnnotContext>(
      pAnnotDict, IPDFPageFromFPDFPage(page));

  // Caller takes the ownership of the object.
  return FPDFAnnotationFromCPDFAnnotContext(pAnnotContext.release());
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFLink_GetAnnotRect(FPDF_LINK link_annot,
                                                          FS_RECTF* rect) {
  if (!link_annot || !rect)
    return false;

  CPDF_Dictionary* pAnnotDict = CPDFDictionaryFromFPDFLink(link_annot);
  *rect = FSRectFFromCFXFloatRect(pAnnotDict->GetRectFor("Rect"));
  return true;
}

FPDF_EXPORT int FPDF_CALLCONV FPDFLink_CountQuadPoints(FPDF_LINK link_annot) {
  const CPDF_Array* pArray =
      GetQuadPointsArrayFromDictionary(CPDFDictionaryFromFPDFLink(link_annot));
  return pArray ? static_cast<int>(pArray->size() / 8) : 0;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFLink_GetQuadPoints(FPDF_LINK link_annot,
                       int quad_index,
                       FS_QUADPOINTSF* quad_points) {
  if (!quad_points || quad_index < 0)
    return false;

  const CPDF_Dictionary* pLinkDict = CPDFDictionaryFromFPDFLink(link_annot);
  if (!pLinkDict)
    return false;

  const CPDF_Array* pArray = GetQuadPointsArrayFromDictionary(pLinkDict);
  if (!pArray)
    return false;

  return GetQuadPointsAtIndex(pArray, static_cast<size_t>(quad_index),
                              quad_points);
}

FPDF_EXPORT FPDF_ACTION FPDF_CALLCONV FPDF_GetPageAAction(FPDF_PAGE page,
                                                          int aa_type) {
  CPDF_Page* pdf_page = CPDFPageFromFPDFPage(page);
  if (!pdf_page)
    return nullptr;

  CPDF_Dictionary* page_dict = pdf_page->GetDict();
  CPDF_AAction aa(page_dict->GetDictFor(pdfium::form_fields::kAA));

  CPDF_AAction::AActionType type;
  if (aa_type == FPDFPAGE_AACTION_OPEN)
    type = CPDF_AAction::kOpenPage;
  else if (aa_type == FPDFPAGE_AACTION_CLOSE)
    type = CPDF_AAction::kClosePage;
  else
    return nullptr;

  if (!aa.ActionExist(type))
    return nullptr;

  CPDF_Action action = aa.GetAction(type);
  return FPDFActionFromCPDFDictionary(action.GetDict());
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_GetFileIdentifier(FPDF_DOCUMENT document,
                       FPDF_FILEIDTYPE id_type,
                       void* buffer,
                       unsigned long buflen) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;

  // Check if |id_type| is valid.
  if (id_type != FILEIDTYPE_PERMANENT && id_type != FILEIDTYPE_CHANGING)
    return 0;

  const CPDF_Array* pFileId = pDoc->GetFileIdentifier();
  if (!pFileId)
    return 0;

  size_t nIndex = id_type == FILEIDTYPE_PERMANENT ? 0 : 1;
  const CPDF_String* pValue = ToString(pFileId->GetDirectObjectAt(nIndex));
  if (!pValue)
    return 0;

  return NulTerminateMaybeCopyAndReturnLength(pValue->GetString(), buffer,
                                              buflen);
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
