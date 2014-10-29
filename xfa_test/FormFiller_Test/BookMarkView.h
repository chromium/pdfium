#if !defined(AFX_BOOKMARKVIEW_H__B1EB2803_BE21_4768_A54B_2DE1E0FC968A__INCLUDED_)
#define AFX_BOOKMARKVIEW_H__B1EB2803_BE21_4768_A54B_2DE1E0FC968A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BookMarkView.h : header file
//

class CChildFrame;
class CReaderVCDoc;
/////////////////////////////////////////////////////////////////////////////
// CBookMarkView view

class CBookMarkView : public CTreeView
{
protected:
	CBookMarkView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBookMarkView)

// Attributes
public:
	CChildFrame *m_pFram;
	FPDF_DOCUMENT m_pDoc;
	HTREEITEM m_hItemRoot;
	CReaderVCDoc* GetDocument();
// Operations
public:
	void InsertChildItem(FPDF_BOOKMARK bookmark, HTREEITEM hItem, CTreeCtrl &treectrl);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBookMarkView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CBookMarkView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CBookMarkView)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CMap<HTREEITEM, HTREEITEM&, CPoint, CPoint&> m_PosMap; 
};

#ifndef _DEBUG  // debug version in PDFReaderVCView.cpp
inline CReaderVCDoc* CBookMarkView::GetDocument()
{ return (CReaderVCDoc*)m_pDocument; }
#endif
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOOKMARKVIEW_H__B1EB2803_BE21_4768_A54B_2DE1E0FC968A__INCLUDED_)
