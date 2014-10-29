// ChildFrm.h : interface of the CChildFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDFRM_H__452F838E_A3F7_4BBF_B4BB_4765F4D394E7__INCLUDED_)
#define AFX_CHILDFRM_H__452F838E_A3F7_4BBF_B4BB_4765F4D394E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//class CFX_SplitterWnd;
#include "FX_SplitterWnd.h"
class CReaderVCView;
class CBookMarkView;
//class CFX_SplitterWnd;
class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

// Attributes
public:
	CFX_SplitterWnd m_wndSplitter;
	CReaderVCView* m_pView;
	CBookMarkView* m_pBkView;
//	BOOL m_bBookmark;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CChildFrame)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	//int m_nPosH, m_nPosV;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__452F838E_A3F7_4BBF_B4BB_4765F4D394E7__INCLUDED_)
