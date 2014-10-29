// ReaderVCDoc.cpp : implementation of the CReaderVCDoc class
//

#include "stdafx.h"
#include "ReaderVC.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ReaderVCDoc.h"
#include "ReaderVCView.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReaderVCDoc

IMPLEMENT_DYNCREATE(CReaderVCDoc, CDocument)

BEGIN_MESSAGE_MAP(CReaderVCDoc, CDocument)
	//{{AFX_MSG_MAP(CReaderVCDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReaderVCDoc construction/destruction

CReaderVCDoc::CReaderVCDoc()
{
	// TODO: add one-time construction code here

}

CReaderVCDoc::~CReaderVCDoc()
{
}

BOOL CReaderVCDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CReaderVCDoc serialization

void CReaderVCDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CReaderVCDoc diagnostics

#ifdef _DEBUG
void CReaderVCDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CReaderVCDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CReaderVCDoc commands

BOOL CReaderVCDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// TODO: Add your specialized creation code here
	void* pDoc = FPDF_LoadDocument(lpszPathName, NULL);

	m_strPDFName=lpszPathName;
	if(NULL == pDoc) return FALSE;	
	POSITION pos = GetFirstViewPosition();
	int nCount = FPDF_GetPageCount(pDoc);
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if(pView->IsKindOf(RUNTIME_CLASS(CReaderVCView)))
		{
			CMainFrame* pMFrm = (CMainFrame*)AfxGetMainWnd();
			CChildFrame* pChildFrm =(CChildFrame*) pMFrm->GetActiveFrame();
			pChildFrm->SetActiveView(pView, TRUE);
//			FPDFApp_SetDocument(((CReaderVCView*)pView)->GetFPDFApp(), (FPDF_DOCUMENT)pDoc);
			((CReaderVCView*)pView)->SetPDFDocument(pDoc, nCount);
		}	
	}
	return TRUE;
}
