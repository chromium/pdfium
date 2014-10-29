// ConvertDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReaderVC.h"
#include "ConvertDlg.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ReaderVCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConvertDlg dialog


CConvertDlg::CConvertDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConvertDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConvertDlg)
	m_nFlag = -1;
	//}}AFX_DATA_INIT
}


void CConvertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConvertDlg)
	DDX_Radio(pDX, IDC_RADIO_Stream, m_nFlag);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConvertDlg, CDialog)
	//{{AFX_MSG_MAP(CConvertDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConvertDlg message handlers

BOOL CConvertDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_pView = (CReaderVCView *)(((CChildFrame *)((CMainFrame *)AfxGetMainWnd())->GetActiveFrame())->GetActiveView());
	ASSERT(m_pView);
	((CButton *)GetDlgItem(IDC_RADIO_Appearance))->SetCheck(1);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConvertDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE);
	CDialog::OnOK();
}
