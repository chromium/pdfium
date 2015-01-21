// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../include/fsdk_define.h"
#include "../include/fpdfdoc.h"

static int THISMODULE = 0;

static CPDF_Bookmark FindBookmark(const CPDF_BookmarkTree& tree, CPDF_Bookmark bookmark, const CFX_WideString& title)
{
	if (bookmark && bookmark.GetTitle().CompareNoCase(title) == 0) {
		// First check this item
		return bookmark;
	}
	// go into children items
	CPDF_Bookmark child = tree.GetFirstChild(bookmark);
	while (child) {
		// check if this item
		CPDF_Bookmark found = FindBookmark(tree, child, title);
		if (found)
			return found;
		child = tree.GetNextSibling(child);
	}
	return CPDF_Bookmark();
}

DLLEXPORT FPDF_BOOKMARK STDCALL FPDFBookmark_GetFirstChild(FPDF_DOCUMENT document, FPDF_BOOKMARK pDict)
{
    if (!document)
        return NULL;
    CPDF_Document* pDoc = (CPDF_Document*)document;
    CPDF_BookmarkTree tree(pDoc);
    CPDF_Bookmark bookmark = CPDF_Bookmark((CPDF_Dictionary*)pDict);
    return tree.GetFirstChild(bookmark).GetDict();
}

DLLEXPORT FPDF_BOOKMARK STDCALL FPDFBookmark_GetNextSibling(FPDF_DOCUMENT document, FPDF_BOOKMARK pDict)
{
    if (!document || !pDict)
        return NULL;
    CPDF_Document* pDoc = (CPDF_Document*)document;
    CPDF_BookmarkTree tree(pDoc);
    CPDF_Bookmark bookmark = CPDF_Bookmark((CPDF_Dictionary*)pDict);
    return tree.GetNextSibling(bookmark).GetDict();
}

DLLEXPORT unsigned long STDCALL FPDFBookmark_GetTitle(FPDF_BOOKMARK pDict, void* buffer, unsigned long buflen)
{
    if (!pDict)
        return 0;
    CPDF_Bookmark bookmark((CPDF_Dictionary*)pDict);
    CFX_WideString title = bookmark.GetTitle();
    CFX_ByteString encodedTitle = title.UTF16LE_Encode();
    unsigned long len = encodedTitle.GetLength();
    if (buffer && buflen >= len) {
        FXSYS_memcpy(buffer, encodedTitle.c_str(), len);
    }
    return len;
}

DLLEXPORT FPDF_BOOKMARK STDCALL FPDFBookmark_Find(FPDF_DOCUMENT document, FPDF_WIDESTRING title)
{
	if (!document)
		return NULL;
	if (!title || title[0] == 0)
		return NULL;
	CPDF_Document* pDoc = (CPDF_Document*)document;
	CPDF_BookmarkTree tree(pDoc);
	FX_STRSIZE len = CFX_WideString::WStringLength(title);
	CFX_WideString encodedTitle = CFX_WideString::FromUTF16LE(title, len);
	return FindBookmark(tree, CPDF_Bookmark(), encodedTitle).GetDict();
}

DLLEXPORT FPDF_DEST STDCALL FPDFBookmark_GetDest(FPDF_DOCUMENT document, FPDF_BOOKMARK pDict)
{
	if (!document)
		return NULL;
	if (!pDict)
		return NULL;
	CPDF_Bookmark bookmark((CPDF_Dictionary*)pDict);
	CPDF_Document* pDoc = (CPDF_Document*)document;
	CPDF_Dest dest = bookmark.GetDest(pDoc);
	if (dest)
		return dest;
	// If this bookmark is not directly associated with a dest, we try to get action
	CPDF_Action action = bookmark.GetAction();
	if (!action)
		return NULL;
	return action.GetDest(pDoc);
}

DLLEXPORT FPDF_ACTION STDCALL FPDFBookmark_GetAction(FPDF_BOOKMARK pDict)
{
	if (!pDict)
		return NULL;
	CPDF_Bookmark bookmark((CPDF_Dictionary*)pDict);
	return bookmark.GetAction();
}

DLLEXPORT unsigned long STDCALL FPDFAction_GetType(FPDF_ACTION pDict)
{
	if (!pDict)
		return 0;
	CPDF_Action action = (CPDF_Dictionary*)pDict;
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
	return PDFACTION_UNSUPPORTED;
}

DLLEXPORT FPDF_DEST STDCALL FPDFAction_GetDest(FPDF_DOCUMENT document, FPDF_ACTION pDict)
{
	if (!document)
		return NULL;
	if (!pDict)
		return NULL;
	CPDF_Document* pDoc = (CPDF_Document*)document;
	CPDF_Action action = (CPDF_Dictionary*)pDict;
	return action.GetDest(pDoc);
}

DLLEXPORT unsigned long STDCALL FPDFAction_GetURIPath(FPDF_DOCUMENT document, FPDF_ACTION pDict,
											  void* buffer, unsigned long buflen)
{
	if (!document)
		return 0;
	if (!pDict)
		return 0;
	CPDF_Document* pDoc = (CPDF_Document*)document;
	CPDF_Action action = (CPDF_Dictionary*)pDict;
	CFX_ByteString path = action.GetURI(pDoc);
	unsigned long len = path.GetLength() + 1;
	if (buffer != NULL && buflen >= len)
		FXSYS_memcpy(buffer, path.c_str(), len);
	return len;
}

DLLEXPORT unsigned long STDCALL FPDFDest_GetPageIndex(FPDF_DOCUMENT document, FPDF_DEST pDict)
{
	if (!document)
		return 0;
	if (!pDict)
		return 0;
	CPDF_Document* pDoc = (CPDF_Document*)document;
	CPDF_Dest dest = (CPDF_Array*)pDict;
	return dest.GetPageIndex(pDoc);
}

static void ReleaseLinkList(FX_LPVOID data)
{
	delete (CPDF_LinkList*)data;
}

DLLEXPORT FPDF_LINK STDCALL FPDFLink_GetLinkAtPoint(FPDF_PAGE page, double x, double y)
{
	if (!page)
		return NULL;
	CPDF_Page* pPage = (CPDF_Page*)page;
	// Link list is stored with the document
	CPDF_Document* pDoc = pPage->m_pDocument;
	CPDF_LinkList* pLinkList = (CPDF_LinkList*)pDoc->GetPrivateData(&THISMODULE);
	if (!pLinkList) {
		pLinkList = FX_NEW CPDF_LinkList(pDoc);
		pDoc->SetPrivateData(&THISMODULE, pLinkList, ReleaseLinkList);
	}
	return pLinkList->GetLinkAtPoint(pPage, (FX_FLOAT)x, (FX_FLOAT)y);
}

DLLEXPORT FPDF_DEST STDCALL FPDFLink_GetDest(FPDF_DOCUMENT document, FPDF_LINK pDict)
{
	if (!document)
		return NULL;
	CPDF_Document* pDoc = (CPDF_Document*)document;
	if (!pDict)
		return NULL;
	CPDF_Link link = (CPDF_Dictionary*)pDict;

	FPDF_DEST dest = link.GetDest(pDoc);
	if (dest)
		return dest;
	// If this link is not directly associated with a dest, we try to get action
	CPDF_Action action = link.GetAction();
	if (!action)
		return NULL;
	return action.GetDest(pDoc);
}

DLLEXPORT FPDF_ACTION STDCALL FPDFLink_GetAction(FPDF_LINK pDict)
{
	if (!pDict)
		return NULL;
	CPDF_Link link = (CPDF_Dictionary*)pDict;
	return link.GetAction();
}

DLLEXPORT FPDF_BOOL STDCALL FPDFLink_Enumerate(FPDF_PAGE page, int* startPos, FPDF_LINK* linkAnnot)
{
	if(!page || !startPos || !linkAnnot)
		return FALSE;
	CPDF_Page* pPage = (CPDF_Page*)page;
	if(!pPage->m_pFormDict)
		return FALSE;
	CPDF_Array* pAnnots = pPage->m_pFormDict->GetArray("Annots");
	if(!pAnnots)
		return FALSE;
	for (int i = *startPos; i < (int)pAnnots->GetCount(); i++) {
		CPDF_Dictionary* pDict = (CPDF_Dictionary*)pAnnots->GetElementValue(i);
		if (!pDict || pDict->GetType() != PDFOBJ_DICTIONARY)
			continue;
		if(pDict->GetString(FX_BSTRC("Subtype")).Equal(FX_BSTRC("Link"))) {
			*startPos = i + 1;
			*linkAnnot = (FPDF_LINK)pDict; 
			return TRUE;
		}
	}
	return FALSE;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFLink_GetAnnotRect(FPDF_LINK linkAnnot, FS_RECTF* rect)
{
	if(!linkAnnot || !rect)
		return FALSE;
	CPDF_Dictionary* pAnnotDict = (CPDF_Dictionary*)linkAnnot;
	CPDF_Rect rt = pAnnotDict->GetRect(FX_BSTRC("Rect"));
	rect->left = rt.left;
	rect->bottom = rt.bottom;
	rect->right = rt.right;
	rect->top = rt.top;
	return TRUE;
}

DLLEXPORT int STDCALL FPDFLink_CountQuadPoints(FPDF_LINK linkAnnot)
{
	if(!linkAnnot)
		return 0;
	CPDF_Dictionary* pAnnotDict = (CPDF_Dictionary*)linkAnnot;
	CPDF_Array* pArray = pAnnotDict->GetArray(FX_BSTRC("QuadPoints"));
	if (!pArray)
		return 0;
	else
		return pArray->GetCount() / 8;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFLink_GetQuadPoints(FPDF_LINK linkAnnot, int quadIndex, FS_QUADPOINTSF* quadPoints)
{
	if(!linkAnnot || !quadPoints)
		return FALSE;
	CPDF_Dictionary* pAnnotDict = (CPDF_Dictionary*)linkAnnot;
	CPDF_Array* pArray = pAnnotDict->GetArray(FX_BSTRC("QuadPoints"));
	if (pArray) {
		if (quadIndex < 0 || quadIndex >= (int)pArray->GetCount()/8 || ((quadIndex*8+7) >= (int)pArray->GetCount()))
			return FALSE;
		quadPoints->x1 = pArray->GetNumber(quadIndex*8);
		quadPoints->y1 = pArray->GetNumber(quadIndex*8+1);
		quadPoints->x2 = pArray->GetNumber(quadIndex*8+2);
		quadPoints->y2 = pArray->GetNumber(quadIndex*8+3);
		quadPoints->x3 = pArray->GetNumber(quadIndex*8+4);
		quadPoints->y3 = pArray->GetNumber(quadIndex*8+5);
		quadPoints->x4 = pArray->GetNumber(quadIndex*8+6);
		quadPoints->y4 = pArray->GetNumber(quadIndex*8+7);
		return TRUE;
	} 
	return FALSE;
}

DLLEXPORT unsigned long STDCALL FPDF_GetMetaText(FPDF_DOCUMENT doc, FPDF_BYTESTRING tag,
												 void* buffer, unsigned long buflen)
{
	if (!doc || !tag)
		return 0;
	CPDF_Document* pDoc = (CPDF_Document*)doc;
	// Get info dictionary
	CPDF_Dictionary* pInfo = pDoc->GetInfo();
	if (!pInfo)
		return 0;
	CFX_WideString text = pInfo->GetUnicodeText(tag);
	// Use UTF-16LE encoding
	CFX_ByteString encodedText = text.UTF16LE_Encode();
	unsigned long len = encodedText.GetLength();
	if (buffer && buflen >= len) {
		FXSYS_memcpy(buffer, encodedText.c_str(), len);
	}
	return len;
}
