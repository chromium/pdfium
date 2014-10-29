// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "ReaderVC.h"

#include "ChildFrm.h"
#include "FX_SplitterWnd.h"
#include "BookmarkView.h"
#include "ReaderVCView.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
	//m_nPosH = m_nPosV = 0;
//	m_pBkView = NULL;
	m_pView = NULL;
//	m_bBookmark = FALSE;
}

CChildFrame::~CChildFrame()
{
/*	if (m_pBkView != NULL)
	{
		delete m_pBkView;
		m_pBkView = NULL;
	}
	if (m_pView != NULL)
	{
		delete m_pView;
		m_pView = NULL;
	}*/
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;
	cs.style |= WS_MAXIMIZE | WS_VISIBLE;
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers





void CChildFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIChildWnd::OnSize(nType, cx, cy);

	
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	if ( !m_wndSplitter.CreateStatic( this, 1, 2 ) ||
		!m_wndSplitter.CreateView( 0,0,RUNTIME_CLASS(CBookMarkView),CSize(180,0),pContext )|| 
		 !m_wndSplitter.CreateView( 0,1,pContext->m_pNewViewClass,CSize(0,0),pContext )) 
		{
			return FALSE;
		}
	m_pBkView = (CBookMarkView *)(m_wndSplitter.GetPane(0,0));
	m_pBkView->m_pFram = this;
	m_pView = (CReaderVCView*)(m_wndSplitter.GetPane(0,1));
	m_pView->m_pFram = this;
	this->SetActiveView( (CView*)m_wndSplitter.GetPane(0,1) );
	
	return TRUE;
	
	//return CMDIChildWnd::OnCreateClient(lpcs, pContext);
}
