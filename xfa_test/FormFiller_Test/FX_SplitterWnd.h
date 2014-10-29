// FX_SplitterWnd.h: interface for the CFX_SplitterWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FX_SPLITTERWND_H__4BF45CD9_0C9D_4223_9D0E_9ECA148F49FB__INCLUDED_)
#define AFX_FX_SPLITTERWND_H__4BF45CD9_0C9D_4223_9D0E_9ECA148F49FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFX_SplitterWnd : public CSplitterWnd
{
public:
	CFX_SplitterWnd();
	virtual ~CFX_SplitterWnd();

public:
	virtual	void	OnDrawSplitter(CDC *pDC,ESplitType nType,const CRect &rectArg);
	virtual	void	RecalcLayout();
	static	void	DeferClientPos(AFX_SIZEPARENTPARAMS *lpLayout,CWnd *pWnd,int x,int y,int cx,int cy,BOOL bScrollBar);
	static	void	LayoutRowCol(CSplitterWnd::CRowColInfo *pInfoArray,int nMax,int nSize,int nSizeSplitter);
	void			LockBar(BOOL bState=TRUE){m_bBarLocked=bState;}
	void			HideColumn(int nCol);
	void			ShowColumn();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFX_SplitterWnd)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CFX_SplitterWnd)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	BOOL	m_bBarLocked;
	int		m_nHidedCol;

};

#endif // !defined(AFX_FX_SPLITTERWND_H__4BF45CD9_0C9D_4223_9D0E_9ECA148F49FB__INCLUDED_)
