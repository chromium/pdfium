// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "ReaderVC.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable

	m_wndToolBar.ModifyStyle(0,TBSTYLE_FLAT | CBRS_TOOLTIPS | TBSTYLE_TRANSPARENT | TBBS_CHECKBOX);
	m_wndToolBar.GetToolBarCtrl().SetButtonWidth(40,40);

	CImageList ImgList,ImgList1;
	CBitmap bm;
	ImgList.Create(22,22,ILC_COLOR8|ILC_MASK, 16, 16);
	ImgList.SetBkColor(::GetSysColor(15));
	bm.LoadBitmap(IDB_BITMAP23);//OPEN
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP35);//Print
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP17);//first page
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP24);//prev page
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP22);//next page
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP21);//last page
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP16);//count clockwise
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP15);//clockwise
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP14);//zoom in
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP26);//zoom out
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP13);//actual size
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP18);//fit page
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP19);//fit width
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP36);//search
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP7);//Bookmark
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP25);//snap shot
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP2);//select text
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP20);//hand tool
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP12);//about
	ImgList.Add(&bm,RGB(0,255,0));
	bm.Detach();
	m_wndToolBar.GetToolBarCtrl().SetImageList(&ImgList);
	//m_wndToolBar.GetToolBarCtrl().SetHotImageList(&ImgList);
	ImgList.Detach();

	ImgList1.Create(22,22,ILC_COLOR8|ILC_MASK, 16, 16);
	ImgList1.SetBkColor(::GetSysColor(15));
	bm.LoadBitmap(IDB_BITMAP23);//open
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP30);//printer
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP8);//first page
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP29);//prev page
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP28);//next page
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP27);//last page
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP1);//count clockwise
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP6);//clockwise
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP33);//zoom in
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP34);//zoom out
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP4);//actual size
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP9);//fit page
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP10);//fit width
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP31);//search
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP5);//bookmark
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP32);//snap
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP3);//select text
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP11);//hand tool
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	bm.LoadBitmap(IDB_BITMAP12);//about
	ImgList1.Add(&bm,RGB(0,255,0));
	bm.Detach();
	m_wndToolBar.GetToolBarCtrl().SetDisabledImageList(&ImgList1);
	ImgList1.Detach();
    
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

