// BookMarkView.cpp : implementation file
//

#include "stdafx.h"
#include "ReaderVC.h"
#include "ChildFrm.h"
#include "BookMarkView.h"
#include "ReaderVCDoc.h"
#include "ReaderVCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBookMarkView

IMPLEMENT_DYNCREATE(CBookMarkView, CTreeView)

CBookMarkView::CBookMarkView()
{
	m_pFram = NULL;
	m_pDoc = NULL;
}

CBookMarkView::~CBookMarkView()
{
}


BEGIN_MESSAGE_MAP(CBookMarkView, CTreeView)
	//{{AFX_MSG_MAP(CBookMarkView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBookMarkView drawing

void CBookMarkView::OnDraw(CDC* pDC)
{
	CReaderVCDoc* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CBookMarkView diagnostics

#ifdef _DEBUG
void CBookMarkView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CBookMarkView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CReaderVCDoc* CBookMarkView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CReaderVCDoc)));
	return (CReaderVCDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBookMarkView message handlers

void CBookMarkView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	CReaderVCDoc* pDoc = GetDocument();
	if (! m_pFram || !m_pFram->m_pView)
	return;
	m_pDoc = m_pFram->m_pView->GetPDFDoc();
	if(m_pDoc == NULL) return;
	int num = m_pFram->m_pView->GetTotalPages();
    CString strName = pDoc->GetTitle();
	FPDF_BOOKMARK bookmark1 = NULL;

	//////////////////////////////////////////////////////////////////////////
	//insert items
	
// 	CTreeCtrl &treeCtrl = this->GetTreeCtrl();
// 	m_hItemRoot = treeCtrl.InsertItem(strName,0,0,TVI_ROOT,TVI_FIRST);
// 	treeCtrl.SetItemData(m_hItemRoot,0);
// 	bookmark1 = FPDFBookmark_GetFirstChild(m_pDoc, NULL);
// 	if (bookmark1 == NULL)//insert the page index to tree  
// 	{
// 		for (int i=0; i<num; i++)
// 		{
// 			CString str;
// 			str.Format(_T("Page%d"), i+1);
// 			HTREEITEM hItem = treeCtrl.InsertItem(str);
// 			treeCtrl.SetItemData(hItem, i);
// 		}
// 	}else{
// 		while(bookmark1 != NULL) {
// 			this->InsertChildItem(bookmark1, m_hItemRoot, treeCtrl);
// 			bookmark1 = FPDFBookmark_GetNextSibling(m_pDoc,bookmark1);
// 		} 
// 	}
// 	treeCtrl.Expand(m_hItemRoot,TVE_EXPAND);
// 	
// 	LONG nStyle = ::GetWindowLong(this->m_hWnd, GWL_STYLE);
// 	nStyle |= TVS_LINESATROOT;
// 	nStyle |= TVS_HASLINES;
// 	nStyle |= TVS_HASBUTTONS;
// 	::SetWindowLong(this->m_hWnd, GWL_STYLE, nStyle);
	
}

void CBookMarkView::InsertChildItem(FPDF_BOOKMARK bookmark, HTREEITEM hItem, CTreeCtrl &treectrl)
{

// 	CString strTitle;
// 	DWORD dwItemData = 0;
// 	WCHAR buffer[1024];
// 	CString str;
// 	int strlenth = 0;
// 	unsigned long pdf_actType = 0;
// 	//FPDF_BOOKMARK 
// 	FPDF_DEST dest = NULL;
// 	FPDF_ACTION action = NULL;
// 	
// 	memset(buffer,0,1024*sizeof(WCHAR));
// 	strlenth = FPDFBookmark_GetTitle(bookmark, buffer, 0);
// 	int nlen = WideCharToMultiByte(CP_ACP,0,buffer,-1,NULL,NULL,NULL,NULL);
// 	char *buffer1 = new char[nlen];
// 	memset(buffer1,0,nlen);
// 	WideCharToMultiByte(CP_ACP,0,buffer,strlenth,buffer1,nlen,NULL,NULL);
// 	buffer1[nlen -1] = '\0';
// /*	int strl = strlen(buffer1);
// 	strTitle = buffer;//
// 	strTitle = strTitle.Left(strl-1);*/
// 	hItem = treectrl.InsertItem(buffer1, 0, 0, hItem, TVI_LAST);
// 	action = FPDFBookmark_GetAction(bookmark);
// 	if (action != NULL)
// 	{
// 		pdf_actType = FPDFAction_GetType(action);
// 		if (pdf_actType == 1)
// 		{
// 			dest = FPDFAction_GetDest(m_pDoc, action);
// 			dwItemData = FPDFDest_GetPageIndex(m_pDoc, dest);
// 			int nZoomMode = FPDFDest_GetZoomMode(dest);
// 			if(nZoomMode == 1)
// 			{
// 				double nStartX = FPDFDest_GetZoomParam(dest, 0);
// 				double nStartY = FPDFDest_GetZoomParam(dest, 1);
// 				CPoint pos((int)nStartX, (int)nStartY);
// 				m_PosMap.SetAt(hItem, pos);
// 			}
// 			treectrl.SetItemData(hItem, dwItemData);
// 			
// 		}else{
// 			dwItemData = 0;
// 			treectrl.SetItemData(hItem, dwItemData);
// 		}
// 		
// 	}else{
// 		dest = FPDFBookmark_GetDest(m_pDoc, bookmark);
// 		dwItemData = FPDFDest_GetPageIndex(m_pDoc, dest);
// 		treectrl.SetItemData(hItem, dwItemData);
// 	}
// 	
// 	bookmark = FPDFBookmark_GetFirstChild(m_pDoc, bookmark);
// 	while(bookmark != NULL)
// 	{
// 		this->InsertChildItem(bookmark, hItem, treectrl);
// 		bookmark = FPDFBookmark_GetNextSibling(m_pDoc, bookmark);
// 	} 
// 	delete buffer1;
}

void CBookMarkView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	HTREEITEM hItem;
	DWORD dwPageIdex= 0;
	if(m_pDoc == NULL) return;
	hItem = GetTreeCtrl().GetSelectedItem();
	dwPageIdex = GetTreeCtrl().GetItemData(hItem);
	CPoint p;
	if(0 == m_PosMap.Lookup(hItem, p))
	{
		p.x = 0; 
		p.y = 0;
	}
	m_pFram->m_pView->LoadPDFPage(m_pDoc, dwPageIdex, p);
	m_pFram->m_pView->Invalidate();
	*pResult = 0;
}

