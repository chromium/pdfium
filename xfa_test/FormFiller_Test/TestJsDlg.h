#if !defined(AFX_TESTJSDLG_H__084897F0_A03A_4D55_BDA2_98D40FB98A4F__INCLUDED_)
#define AFX_TESTJSDLG_H__084897F0_A03A_4D55_BDA2_98D40FB98A4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TestJsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTestJsDlg dialog

class CTestJsDlg : public CDialog
{
// Construction
public:
	CTestJsDlg(CWnd* pParent = NULL);   // standard constructor
	void init(void* handle) {m_handle = handle;}
// Dialog Data
	//{{AFX_DATA(CTestJsDlg)
	enum { IDD = IDD_TEST_JS };
	CString	m_js;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestJsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void * m_handle;
	// Generated message map functions
	//{{AFX_MSG(CTestJsDlg)
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTJSDLG_H__084897F0_A03A_4D55_BDA2_98D40FB98A4F__INCLUDED_)
