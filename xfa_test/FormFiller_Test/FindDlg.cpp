// FindDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReaderVC.h"
#include "FindDlg.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ReaderVCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog


CFindDlg::CFindDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFindDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFindDlg)
	m_bMatchCase = FALSE;
	m_bWholeWord = FALSE;
	m_strFindWhat = _T("");
	m_nDirection = -1;
	//}}AFX_DATA_INIT
}


void CFindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindDlg)
	DDX_Check(pDX, IDC_CHECK_MATCHCASE, m_bMatchCase);
	DDX_Check(pDX, IDC_CHECK_MATCHWHOLE, m_bWholeWord);
	DDX_Text(pDX, IDC_EDIT1, m_strFindWhat);
	DDX_Radio(pDX, IDC_RADIO_Down, m_nDirection);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindDlg, CDialog)
	//{{AFX_MSG_MAP(CFindDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindDlg message handlers

void CFindDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE);
	m_pView->FindText(m_strFindWhat, m_bMatchCase, m_bWholeWord, m_nDirection);
	//CDialog::OnOK();
}

BOOL CFindDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_pView = (CReaderVCView *)(((CChildFrame *)((CMainFrame *)AfxGetMainWnd())->GetActiveFrame())->GetActiveView());
	ASSERT(m_pView);
	((CButton *)GetDlgItem(IDC_RADIO_Down))->SetCheck(1);
	((CButton *)GetDlgItem(IDC_CHECK_MATCHWHOLE))->SetCheck(1);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
