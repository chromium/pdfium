#if !defined(AFX_EXPORTPAGE_H__D9B06547_96CA_4749_88CB_2506D8AF61D6__INCLUDED_)
#define AFX_EXPORTPAGE_H__D9B06547_96CA_4749_88CB_2506D8AF61D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportPage dialog
class CReaderVCView;

class CExportPage : public CDialog
{
// Construction
public:
	CBitmap winbmp;
	FPDF_BITMAP m_bitmap;
	FPDF_PAGE m_page;
	CReaderVCView *m_pView;
	void SetDlgInfo();
	void InitDialogInfo(CReaderVCView *pView);
	CExportPage(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportPage)
	enum { IDD = IDD_EXPORT_PAGE };
	int		m_nHeight;
	int		m_nPageHeight;
	int		m_nRotate;
	int		m_nWidth;
	int		m_nPageWidth;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportPage)
	afx_msg void OnRanderPage();
	afx_msg void OnSave();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTPAGE_H__D9B06547_96CA_4749_88CB_2506D8AF61D6__INCLUDED_)
