#if !defined(AFX_ZOOMDLG_H__B244E9E1_5278_4244_9D19_7307E1B76C15__INCLUDED_)
#define AFX_ZOOMDLG_H__B244E9E1_5278_4244_9D19_7307E1B76C15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ZoomDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg dialog
class CReaderVCView;
class CZoomDlg : public CDialog
{
// Construction
public:
	CZoomDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CZoomDlg)
	enum { IDD = IDD_DLG_ZOOMTO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZoomDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CReaderVCView *m_pView;
	// Generated message map functions
	//{{AFX_MSG(CZoomDlg)
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZOOMDLG_H__B244E9E1_5278_4244_9D19_7307E1B76C15__INCLUDED_)
