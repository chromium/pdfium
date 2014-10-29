// ZoomDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReaderVC.h"
#include "ZoomDlg.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ReaderVCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg dialog


CZoomDlg::CZoomDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CZoomDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CZoomDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CZoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CZoomDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CZoomDlg, CDialog)
	//{{AFX_MSG_MAP(CZoomDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg message handlers

void CZoomDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CZoomDlg::OnOK() 
{
	// TODO: Add extra validation here
	CComboBox *pCmb = (CComboBox *)GetDlgItem(IDC_COMBO1);
	int i = pCmb->GetCurSel();
	int nScal = pCmb->GetItemData(i);
	double dbScal = nScal / 100.0f;
	m_pView->ScalPage(dbScal);
	CDialog::OnOK();
}

BOOL CZoomDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_pView = (CReaderVCView *)(((CChildFrame *)((CMainFrame *)AfxGetMainWnd())->GetActiveFrame())->GetActiveView());
	CComboBox *pCmb = (CComboBox *)GetDlgItem(IDC_COMBO1);
	pCmb->AddString("-----25%-----");
	pCmb->SetItemData(0, 25);
	pCmb->AddString("-----50%-----");
	pCmb->SetItemData(1, 50);
	pCmb->AddString("-----75%-----");
	pCmb->SetItemData(3, 75);
	pCmb->AddString("-----100%----");
	pCmb->SetItemData(3, 100);
	pCmb->AddString("-----125%----");
	pCmb->SetItemData(4, 125);
	pCmb->AddString("-----200%----");
	pCmb->SetItemData(5, 200);
	pCmb->AddString("-----300%----");
	pCmb->SetItemData(6, 300);
	pCmb->AddString("-----400%----");
	pCmb->SetItemData(7, 400);
	pCmb->AddString("-----600%----");
	pCmb->SetItemData(8, 600);
	pCmb->AddString("-----800%----");
	pCmb->SetItemData(9, 800);
	pCmb->AddString("-----1200%----");
	pCmb->SetItemData(10, 1200);
	pCmb->AddString("-----1600%---");
	pCmb->SetItemData(11, 160);
	pCmb->AddString("-----3200%---");
	pCmb->SetItemData(12, 3200);
	pCmb->AddString("-----6400%----");
	pCmb->SetItemData(13, 6400);
	this->SetDlgItemTextA(IDC_COMBO1, "Select Zoom Factor");
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
