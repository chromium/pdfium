#if !defined(AFX_CONVERTDLG_H__9BB8EADD_CCDD_4C68_A8A0_C1656653A947__INCLUDED_)
#define AFX_CONVERTDLG_H__9BB8EADD_CCDD_4C68_A8A0_C1656653A947__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConvertDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConvertDlg dialog
class CReaderVCView;
class CConvertDlg : public CDialog
{
// Construction
public:
	CConvertDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConvertDlg)
	enum { IDD = IDD_DLG_CONVERT };
	int		m_nFlag;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConvertDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CReaderVCView *m_pView;
	// Generated message map functions
	//{{AFX_MSG(CConvertDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONVERTDLG_H__9BB8EADD_CCDD_4C68_A8A0_C1656653A947__INCLUDED_)
