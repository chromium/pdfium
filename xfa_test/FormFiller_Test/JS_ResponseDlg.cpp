// JS_ResponseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "readervc.h"
#include "JS_ResponseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJS_ResponseDlg dialog


CJS_ResponseDlg::CJS_ResponseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CJS_ResponseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CJS_ResponseDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_swTitle =(FPDF_WIDESTRING) L"";
	m_swQuestion =(FPDF_WIDESTRING) L"";
	m_swLabel =(FPDF_WIDESTRING) L"";
	m_swDefault =(FPDF_WIDESTRING) L"";
	m_swResponse =L"";
	m_bIsVisible = false;
	m_pResponseEdit = NULL;
}


void CJS_ResponseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJS_ResponseDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CJS_ResponseDlg, CDialog)
	//{{AFX_MSG_MAP(CJS_ResponseDlg)
		// NOTE: the ClassWizard will add message map macros here
		ON_BN_CLICKED(ID_JS_OK, OnResOk)
		ON_BN_CLICKED(ID_JS_CANCEL, OnResCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJS_ResponseDlg message handlers
BOOL CJS_ResponseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	::SetWindowTextW(this->GetSafeHwnd(), (LPCWSTR)m_swTitle);

	CStatic* pQuestion = (CStatic*)GetDlgItem(IDC_JS_QUESTION);
	::SetWindowTextW(pQuestion->GetSafeHwnd(), (LPCWSTR)m_swQuestion);

	CStatic* pLabel = (CStatic*)GetDlgItem(IDC_JS_ANSWER);

	CRect DialogRect;
	CRect LabelRect;
	RECT rect;
	GetWindowRect(&DialogRect);
	pLabel->GetWindowRect(&LabelRect);	

	if(m_swLabel == (FPDF_WIDESTRING)L"")
	{	
		rect.left = DialogRect.left + 20;
		pLabel->ShowWindow(SW_HIDE);
	}
	else
	{
		rect.left = LabelRect.right + 1;
		::SetWindowTextW(pLabel->GetSafeHwnd(), (LPCWSTR)m_swLabel);
	}

	rect.top = LabelRect.top - 3;
	rect.right = DialogRect.right - 20;
	rect.bottom = LabelRect.bottom + 2;
	ScreenToClient(&rect);
	m_pResponseEdit = new CEdit();

	if(m_bIsVisible)
		m_pResponseEdit->Create(ES_AUTOVSCROLL | ES_NOHIDESEL | ES_PASSWORD | WS_BORDER, rect, this, IDC_JS_EDIT);
	else
		m_pResponseEdit->Create(ES_AUTOVSCROLL | ES_NOHIDESEL | WS_BORDER, rect, this, IDC_JS_EDIT);
	
	::SetWindowTextW(m_pResponseEdit->GetSafeHwnd(), (LPCWSTR)m_swDefault);
	m_pResponseEdit->ShowWindow(SW_SHOW);

	return TRUE;
}

void CJS_ResponseDlg::OnResOk()
{
	m_swResponse = (wchar_t*)malloc(sizeof(wchar_t) * 250);
	::GetWindowTextW(m_pResponseEdit->GetSafeHwnd(), m_swResponse, 250);

	if(m_pResponseEdit)
	{
		m_pResponseEdit->DestroyWindow();
		delete m_pResponseEdit;
		m_pResponseEdit = NULL;
	}
	CDialog::OnOK();
}

void CJS_ResponseDlg::OnResCancel()
{
	if(m_pResponseEdit)
	{
		m_pResponseEdit->DestroyWindow();
		delete m_pResponseEdit;
		m_pResponseEdit = NULL;
	}
	
	CDialog::OnCancel();
}

void CJS_ResponseDlg::SetTitle(FPDF_WIDESTRING swTitle)
{
	m_swTitle = swTitle;
}

void CJS_ResponseDlg::SetQuestion(FPDF_WIDESTRING swQuestion)
{
	m_swQuestion = swQuestion;
}

void CJS_ResponseDlg::SetLabel(FPDF_WIDESTRING swLabel)
{
	m_swLabel = swLabel;
}

void CJS_ResponseDlg::SetDefault(FPDF_WIDESTRING swDefault)
{
	m_swDefault = swDefault;
}

FPDF_WIDESTRING CJS_ResponseDlg::GetResponse()
{
	return (FPDF_WIDESTRING)m_swResponse;
}

void CJS_ResponseDlg::SetIsVisible(FPDF_BOOL bPassword)
{
	m_bIsVisible = bPassword;
}
