// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../include/fsdk_define.h"
#include "../include/fpdfdoc.h"

static int this_module = 0;

static CPDF_Bookmark FindBookmark(CPDF_BookmarkTree& tree, CPDF_Bookmark This, const CFX_WideString& title)
{
	if (This != NULL) {
		// First check this item
		CFX_WideString this_title = This.GetTitle();
		if (this_title.CompareNoCase(title) == 0)
			return This;
	}
	// go into children items
	CPDF_Bookmark Child = tree.GetFirstChild(This);
	while (Child != NULL) {
		// check if this item
		CPDF_Bookmark Found = FindBookmark(tree, Child, title);
		if (Found) return Found;
		Child = tree.GetNextSibling(Child);
	}
	return NULL;
}

DLLEXPORT FPDF_BOOKMARK STDCALL FPDFBookmark_Find(FPDF_DOCUMENT document, FPDF_WIDESTRING title)
{
	if (document == NULL) return NULL;
	if (title == NULL || title[0] == 0) return NULL;

	CPDF_Document* pDoc = (CPDF_Document*)document;
	CPDF_BookmarkTree tree(pDoc);

	FX_STRSIZE len = CFX_WideString::WStringLength(title);
	CFX_WideString wstr = CFX_WideString::FromUTF16LE(title, len);
	return FindBookmark(tree, NULL, wstr);
}

DLLEXPORT FPDF_DEST STDCALL FPDFBookmark_GetDest(FPDF_DOCUMENT document, FPDF_BOOKMARK bookmark)
{
	if (document == NULL) return NULL;
	if (bookmark == NULL) return NULL;

	CPDF_Bookmark Bookmark = (CPDF_Dictionary*)bookmark;
	CPDF_Document* pDoc = (CPDF_Document*)document;
	CPDF_Dest dest = Bookmark.GetDest(pDoc);
	if (dest != NULL) return dest;

	// If this bookmark is not directly associated with a dest, we try to get action
	CPDF_Action Action = Bookmark.GetAction();
	if (Action == NULL) return NULL;
	return Action.GetDest(pDoc);
}

DLLEXPORT FPDF_ACTION STDCALL FPDFBookmark_GetAction(FPDF_BOOKMARK bookmark)
{
	if (bookmark == NULL) return NULL;

	CPDF_Bookmark Bookmark = (CPDF_Dictionary*)bookmark;
	return Bookmark.GetAction();
}

DLLEXPORT unsigned long STDCALL FPDFAction_GetType(FPDF_ACTION action)
{
	if (action == NULL) return 0;

	CPDF_Action Action = (CPDF_Dictionary*)action;
	CPDF_Action::ActionType type = Action.GetType();
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

DLLEXPORT FPDF_DEST STDCALL FPDFAction_GetDest(FPDF_DOCUMENT document, FPDF_ACTION action)
{
	if (document == NULL) return NULL;
	if (action == NULL) return NULL;
	CPDF_Document* pDoc = (CPDF_Document*)document;
	CPDF_Action Action = (CPDF_Dictionary*)action;

	return Action.GetDest(pDoc);
}

DLLEXPORT unsigned long STDCALL FPDFAction_GetURIPath(FPDF_DOCUMENT document, FPDF_ACTION action, 
											  void* buffer, unsigned long buflen)
{
	if (document == NULL) return 0;
	if (action == NULL) return 0;
	CPDF_Document* pDoc = (CPDF_Document*)document;
	CPDF_Action Action = (CPDF_Dictionary*)action;

	CFX_ByteString path = Action.GetURI(pDoc);
	unsigned long len = path.GetLength() + 1;
	if (buffer != NULL && buflen >= len)
		FXSYS_memcpy(buffer, (FX_LPCSTR)path, len);
	return len;
}

DLLEXPORT unsigned long STDCALL FPDFDest_GetPageIndex(FPDF_DOCUMENT document, FPDF_DEST dest)
{
	if (document == NULL) return 0;
	if (dest == NULL) return 0;
	CPDF_Document* pDoc = (CPDF_Document*)document;
	CPDF_Dest Dest = (CPDF_Array*)dest;

	return Dest.GetPageIndex(pDoc);
}

static void ReleaseLinkList(FX_LPVOID data)
{
	delete (CPDF_LinkList*)data;
}

DLLEXPORT FPDF_LINK STDCALL FPDFLink_GetLinkAtPoint(FPDF_PAGE page, double x, double y)
{
	if (page == NULL) return NULL;
	CPDF_Page* pPage = (CPDF_Page*)page;

	// Link list is stored with the document
	CPDF_Document* pDoc = pPage->m_pDocument;
	CPDF_LinkList* pLinkList = (CPDF_LinkList*)pDoc->GetPrivateData(&this_module);
	if (pLinkList == NULL) {
		pLinkList = FX_NEW CPDF_LinkList(pDoc);
		pDoc->SetPrivateData(&this_module, pLinkList, ReleaseLinkList);
	}

	return pLinkList->GetLinkAtPoint(pPage, (FX_FLOAT)x, (FX_FLOAT)y);
}

DLLEXPORT FPDF_DEST STDCALL FPDFLink_GetDest(FPDF_DOCUMENT document, FPDF_LINK link)
{
	if (document == NULL) return NULL;
	CPDF_Document* pDoc = (CPDF_Document*)document;
	if (link == NULL) return NULL;
	CPDF_Link Link = (CPDF_Dictionary*)link;

	FPDF_DEST dest = Link.GetDest(pDoc);
	if (dest) return dest;

	// If this link is not directly associated with a dest, we try to get action
	CPDF_Action Action = Link.GetAction();
	if (Action == NULL) return NULL;
	return Action.GetDest(pDoc);
}

DLLEXPORT FPDF_ACTION STDCALL FPDFLink_GetAction(FPDF_LINK link)
{
	if (link == NULL) return NULL;
	CPDF_Link Link = (CPDF_Dictionary*)link;

	return Link.GetAction();
}

DLLEXPORT FPDF_BOOL STDCALL FPDFLink_Enumerate(FPDF_PAGE page, int* startPos, FPDF_LINK* linkAnnot)
{
	if(!page || !startPos || !linkAnnot)
		return FALSE;
	CPDF_Page* pPage = (CPDF_Page*)page;
	if(!pPage->m_pFormDict) return FALSE;
	CPDF_Array* pAnnots = pPage->m_pFormDict->GetArray("Annots");
	if(!pAnnots) return FALSE;
	for (int i = *startPos; i < (int)pAnnots->GetCount(); i ++) {
		CPDF_Dictionary* pDict = (CPDF_Dictionary*)pAnnots->GetElementValue(i);
		if (pDict == NULL || pDict->GetType() != PDFOBJ_DICTIONARY) continue;
		if(pDict->GetString(FX_BSTRC("Subtype")).Equal(FX_BSTRC("Link")))
		{
			*startPos = i+1;
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
	if (pArray == NULL)
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
		if (0 > quadIndex || quadIndex >= (int)pArray->GetCount()/8 ||
			((quadIndex*8+7) >= (int)pArray->GetCount())) return FALSE;
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
	if (doc == NULL || tag == NULL) return 0;

	CPDF_Document* pDoc = (CPDF_Document*)doc;
	// Get info dictionary
	CPDF_Dictionary* pInfo = pDoc->GetInfo();
	if (pInfo == NULL) return 0;

	CFX_WideString text = pInfo->GetUnicodeText(tag);

	// Use UTF-16LE encoding
	CFX_ByteString bstr = text.UTF16LE_Encode();
	unsigned long len = bstr.GetLength();
	if (buffer != NULL && buflen >= len+2) {
		FXSYS_memcpy(buffer, (FX_LPCSTR)bstr, len);
		// use double zero as trailer
		((FX_BYTE*)buffer)[len] = ((FX_BYTE*)buffer)[len+1] = 0;
	}
	return len+2;
}

