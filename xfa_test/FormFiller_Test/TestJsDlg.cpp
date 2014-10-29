// TestJsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "readervc.h"
#include "TestJsDlg.h"
#include "../../include/fpdfformfill.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestJsDlg dialog


CTestJsDlg::CTestJsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestJsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestJsDlg)
	m_js = _T("");
	//}}AFX_DATA_INIT
}


void CTestJsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestJsDlg)
	DDX_Text(pDX, IDC_EDIT1, m_js);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTestJsDlg, CDialog)
	//{{AFX_MSG_MAP(CTestJsDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestJsDlg message handlers

void CTestJsDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	LPCTSTR lpStr = m_js.GetBuffer(m_js.GetLength());
	int nLen = MultiByteToWideChar(CP_ACP, 0, lpStr, m_js.GetLength(), NULL, 0);
	wchar_t* pbuf = new wchar_t[nLen+1];
	MultiByteToWideChar(CP_ACP, 0, lpStr, m_js.GetLength(), pbuf, nLen);
	pbuf[nLen] = 0;
	m_js.ReleaseBuffer();
//	RunJS(m_handle,pbuf);
	delete[] pbuf;
}
