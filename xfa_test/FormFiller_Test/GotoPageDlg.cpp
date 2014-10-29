// GotoPageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReaderVC.h"
#include "GotoPageDlg.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ReaderVCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGotoPageDlg dialog


CGotoPageDlg::CGotoPageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGotoPageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGotoPageDlg)
	m_nPageIndex = 0;
	//}}AFX_DATA_INIT
}


void CGotoPageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGotoPageDlg)
	DDX_Text(pDX, IDC_EDIT1, m_nPageIndex);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGotoPageDlg, CDialog)
	//{{AFX_MSG_MAP(CGotoPageDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGotoPageDlg message handlers

void CGotoPageDlg::OnOK() 
{
	// TODO: Add extra validation here
	if (NULL == m_pView) return;
	UpdateData(TRUE);
	m_pView->GotoPage(m_nPageIndex);
	CDialog::OnOK();
}

BOOL CGotoPageDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_pView = (CReaderVCView *)(((CChildFrame *)((CMainFrame *)AfxGetMainWnd())->GetActiveFrame())->GetActiveView());
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
