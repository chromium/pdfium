#if !defined(AFX_JS_RESPONSEDLG_H__AE1575EB_3F0A_46E7_B278_089C74F2B34C__INCLUDED_)
#define AFX_JS_RESPONSEDLG_H__AE1575EB_3F0A_46E7_B278_089C74F2B34C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// JS_ResponseDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CJS_ResponseDlg dialog

class CJS_ResponseDlg : public CDialog
{
// Construction
public:
	CJS_ResponseDlg(CWnd* pParent = NULL);   // standard constructor
	~CJS_ResponseDlg(){ if(m_swResponse) free(m_swResponse); }

// Dialog Data
	//{{AFX_DATA(CJS_ResponseDlg)
	enum { IDD = IDD_DLG_RESPONSE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
public:
	void				SetTitle(FPDF_WIDESTRING swTitle);
	void				SetQuestion(FPDF_WIDESTRING swQuestion);
	void				SetDefault(FPDF_WIDESTRING swDefault);
	void				SetLabel(FPDF_WIDESTRING swLabel);
	void				SetIsVisible(FPDF_BOOL bPassword);
	FPDF_WIDESTRING		GetResponse();

private:
	FPDF_WIDESTRING		m_swTitle;
	FPDF_WIDESTRING		m_swQuestion;
	FPDF_WIDESTRING		m_swDefault;
	FPDF_WIDESTRING		m_swLabel;
	wchar_t*		 	m_swResponse;
	FPDF_BOOL			m_bIsVisible;
	CEdit*				m_pResponseEdit;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJS_ResponseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CJS_ResponseDlg)
		// NOTE: the ClassWizard will add member functions here
	virtual BOOL OnInitDialog();
	afx_msg void OnResOk();
	afx_msg void OnResCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JS_RESPONSEDLG_H__AE1575EB_3F0A_46E7_B278_089C74F2B34C__INCLUDED_)
