#if !defined(AFX_FINDDLG_H__355AF96B_B487_4503_8658_7F37B397D38A__INCLUDED_)
#define AFX_FINDDLG_H__355AF96B_B487_4503_8658_7F37B397D38A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog
class CReaderVCView;
class CFindDlg : public CDialog
{
// Construction
public:
	CFindDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindDlg)
	enum { IDD = IDD_DLG_FIND };
	BOOL	m_bMatchCase;
	BOOL	m_bWholeWord;
	CString	m_strFindWhat;
	int		m_nDirection;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CReaderVCView *m_pView;
	// Generated message map functions
	//{{AFX_MSG(CFindDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDDLG_H__355AF96B_B487_4503_8658_7F37B397D38A__INCLUDED_)
