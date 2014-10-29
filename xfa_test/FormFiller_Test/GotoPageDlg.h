#if !defined(AFX_GOTOPAGEDLG_H__674C0708_4B25_4465_A73F_310610F9B991__INCLUDED_)
#define AFX_GOTOPAGEDLG_H__674C0708_4B25_4465_A73F_310610F9B991__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GotoPageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGotoPageDlg dialog
class CReaderVCView;
class CGotoPageDlg : public CDialog
{
// Construction
public:
	CGotoPageDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGotoPageDlg)
	enum { IDD = IDD_DLG_GOTOPAGE };
	int		m_nPageIndex;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGotoPageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
private:
		CReaderVCView *m_pView;
// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CGotoPageDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOTOPAGEDLG_H__674C0708_4B25_4465_A73F_310610F9B991__INCLUDED_)
