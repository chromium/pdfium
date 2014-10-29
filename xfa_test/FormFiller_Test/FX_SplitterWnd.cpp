// FX_SplitterWnd.cpp: implementation of the CFX_SplitterWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "teststatusbar.h"
#include "FX_SplitterWnd.h"

#include <afxpriv.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFX_SplitterWnd::CFX_SplitterWnd()
{
	m_cxSplitter	= m_cySplitter = 4 + 1 + 1;
	m_cxBorderShare = m_cyBorderShare = 0;
	m_cxSplitterGap = m_cySplitterGap = 4 + 1 + 1;
	m_cxBorder		= m_cyBorder = 1;
	m_nHidedCol		= -1;
	m_bBarLocked	= FALSE;
}

CFX_SplitterWnd::~CFX_SplitterWnd()
{

}

BEGIN_MESSAGE_MAP(CFX_SplitterWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(CFX_SplitterWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CFX_SplitterWnd::OnDrawSplitter(CDC *pDC,ESplitType nType,const CRect &rectArg)
{
	if((nType != splitBorder) || (pDC == NULL))
	{
		CSplitterWnd::OnDrawSplitter(pDC, nType, rectArg);
		return;
	}

	pDC->Draw3dRect(rectArg, GetSysColor(COLOR_BTNSHADOW), GetSysColor(COLOR_BTNHIGHLIGHT));
}
void CFX_SplitterWnd::RecalcLayout()
{
	ASSERT_VALID(this);
	ASSERT(m_nRows > 0 && m_nCols > 0); // must have at least one pane

	CRect rectClient;
	GetClientRect(rectClient);
	rectClient.InflateRect(-m_cxBorder, -m_cyBorder);

	CRect rectInside;
	GetInsideRect(rectInside);

	// layout columns (restrict to possible sizes)
	LayoutRowCol(m_pColInfo, m_nCols, rectInside.Width(), m_cxSplitterGap);
	LayoutRowCol(m_pRowInfo, m_nRows, rectInside.Height(), m_cySplitterGap);

	// adjust the panes (and optionally scroll bars)

	// give the hint for the maximum number of HWNDs
	AFX_SIZEPARENTPARAMS layout;
	layout.hDWP = ::BeginDeferWindowPos((m_nCols + 1) * (m_nRows + 1) + 1);

	// size of scrollbars
	int cx = rectClient.right - rectInside.right;
	int cy = rectClient.bottom - rectInside.bottom;

	// reposition size box
	if (m_bHasHScroll && m_bHasVScroll)
	{
		CWnd* pScrollBar = GetDlgItem(AFX_IDW_SIZE_BOX);
		ASSERT(pScrollBar != NULL);

		// fix style if necessary
		BOOL bSizingParent = (GetSizingParent() != NULL);
		// modifyStyle returns TRUE if style changes
		if (pScrollBar->ModifyStyle(SBS_SIZEGRIP|SBS_SIZEBOX,
				bSizingParent ? SBS_SIZEGRIP : SBS_SIZEBOX))
			pScrollBar->Invalidate();
		pScrollBar->EnableWindow(bSizingParent);

		// reposition the size box
		DeferClientPos(&layout, pScrollBar,
			rectInside.right,
			rectInside.bottom, cx, cy, TRUE);
	}

	// reposition scroll bars
	if (m_bHasHScroll)
	{
		int cxSplitterBox = m_cxSplitter;// split box bigger
		int x = rectClient.left;
		int y = rectInside.bottom;
		for (int col = 0; col < m_nCols; col++)
		{
			CWnd* pScrollBar = GetDlgItem(AFX_IDW_HSCROLL_FIRST + col);
			ASSERT(pScrollBar != NULL);
			int cx = m_pColInfo[col].nCurSize;
			if (col == 0 && m_nCols < m_nMaxCols)
				x += cxSplitterBox, cx -= cxSplitterBox;
			DeferClientPos(&layout, pScrollBar, x, y, cx, cy, TRUE);
			x += cx + m_cxSplitterGap;
		}
	}

	if (m_bHasVScroll)
	{
		int cySplitterBox = m_cySplitter;// split box bigger
		int x = rectInside.right;
		int y = rectClient.top;
		for (int row = 0; row < m_nRows; row++)
		{
			CWnd* pScrollBar = GetDlgItem(AFX_IDW_VSCROLL_FIRST + row);
			ASSERT(pScrollBar != NULL);
			int cy = m_pRowInfo[row].nCurSize;
			if (row == 0 && m_nRows < m_nMaxRows)
				y += cySplitterBox, cy -= cySplitterBox;
			DeferClientPos(&layout, pScrollBar, x, y, cx, cy, TRUE);
			y += cy + m_cySplitterGap;
		}
	}

	//BLOCK: Reposition all the panes
	{
		int x = rectClient.left;
		for (int col = 0; col < m_nCols; col++)
		{
			int cx = m_pColInfo[col].nCurSize;
			int y = rectClient.top;
			for (int row = 0; row < m_nRows; row++)
			{
				int cy = m_pRowInfo[row].nCurSize;
				CWnd* pWnd = GetPane(row, col);
				DeferClientPos(&layout, pWnd, x, y, cx, cy, FALSE);
				y += cy + m_cySplitterGap;
			}
			x += cx + m_cxSplitterGap;
		}
	}

	// move and resize all the windows at once!
	if (layout.hDWP == NULL || !::EndDeferWindowPos(layout.hDWP))
		TRACE0("Warning: DeferWindowPos failed - low system resources.\n");

	// invalidate all the splitter bars (with NULL pDC)
	DrawAllSplitBars(NULL, rectInside.right, rectInside.bottom);
}

void CFX_SplitterWnd::DeferClientPos(AFX_SIZEPARENTPARAMS *lpLayout,CWnd *pWnd,int x,int y,int cx,int cy,BOOL bScrollBar)
{
	ASSERT(pWnd != NULL);
	ASSERT(pWnd->m_hWnd != NULL);

	if (bScrollBar)
	{
		// if there is enough room, draw scroll bar without border
		// if there is not enough room, set the WS_BORDER bit so that
		//   we will at least get a proper border drawn
		BOOL bNeedBorder = (cx <= 1 || cy <= 1);
		pWnd->ModifyStyle(bNeedBorder ? 0 : 1, bNeedBorder ? 1 : 0);
	}
	CRect rect(x, y, x+cx, y+cy);

	// adjust for 3d border (splitter windows have implied border)
	if ((pWnd->GetExStyle() & WS_EX_CLIENTEDGE) || pWnd->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
	{
		rect.InflateRect(1, 1); // for proper draw CFlatSplitterWnd in view
//		rect.InflateRect(afxData.cxBorder2, afxData.cyBorder2);
	}

	// first check if the new rectangle is the same as the current
	CRect rectOld;
	pWnd->GetWindowRect(rectOld);
	pWnd->GetParent()->ScreenToClient(&rectOld);
	if (rect != rectOld)
		AfxRepositionWindow(lpLayout, pWnd->m_hWnd, rect);
}

void CFX_SplitterWnd::LayoutRowCol(CSplitterWnd::CRowColInfo *pInfoArray,int nMax,int nSize,int nSizeSplitter)
{
	ASSERT(pInfoArray != NULL);
	ASSERT(nMax > 0);
	ASSERT(nSizeSplitter > 0);

	CSplitterWnd::CRowColInfo* pInfo;
	int i;

	if (nSize < 0)
		nSize = 0;  // if really too small, layout as zero size

	// start with ideal sizes
	for (i = 0, pInfo = pInfoArray; i < nMax-1; i++, pInfo++)
	{
		if (pInfo->nIdealSize < pInfo->nMinSize)
			pInfo->nIdealSize = 0;      // too small to see
		pInfo->nCurSize = pInfo->nIdealSize;
	}
	pInfo->nCurSize = INT_MAX;  // last row/column takes the rest

	for (i = 0, pInfo = pInfoArray; i < nMax; i++, pInfo++)
	{
		ASSERT(nSize >= 0);
		if (nSize == 0)
		{
			// no more room (set pane to be invisible)
			pInfo->nCurSize = 0;
			continue;       // don't worry about splitters
		}
		else if (nSize < pInfo->nMinSize && i != 0)
		{
			// additional panes below the recommended minimum size
			//   aren't shown and the size goes to the previous pane
			pInfo->nCurSize = 0;

			// previous pane already has room for splitter + border
			//   add remaining size and remove the extra border
			(pInfo-1)->nCurSize += nSize + 2;
			nSize = 0;
		}
		else
		{
			// otherwise we can add the second pane
			ASSERT(nSize > 0);
			if (pInfo->nCurSize == 0)
			{
				// too small to see
				if (i != 0)
					pInfo->nCurSize = 0;
			}
			else if (nSize < pInfo->nCurSize)
			{
				// this row/col won't fit completely - make as small as possible
				pInfo->nCurSize = nSize;
				nSize = 0;
			}
			else
			{
				// can fit everything
				nSize -= pInfo->nCurSize;
			}
		}

		// see if we should add a splitter
		ASSERT(nSize >= 0);
		if (i != nMax - 1)
		{
			// should have a splitter
			if (nSize > nSizeSplitter)
			{
				nSize -= nSizeSplitter; // leave room for splitter + border
				ASSERT(nSize > 0);
			}
			else
			{
				// not enough room - add left over less splitter size
				pInfo->nCurSize += nSize;
				if (pInfo->nCurSize > (nSizeSplitter - 2))
					pInfo->nCurSize -= (nSizeSplitter - 2);
				nSize = 0;
			}
		}
	}
	ASSERT(nSize == 0); // all space should be allocated
}

void CFX_SplitterWnd::HideColumn(int nCol)
{
	if ( m_nHidedCol != -1 )
		return;

	ASSERT_VALID(this);
	ASSERT(m_nCols > 1);
	ASSERT(nCol < m_nCols);
	ASSERT(m_nHidedCol == -1);
	m_nHidedCol = nCol;
	
    // if the column has an active window -- change it
	int rowActive, colActive;
	if (GetActivePane(&rowActive, &colActive) != NULL &&
		colActive == nCol)
	{
		if (++colActive >= m_nCols)
			colActive = 0;
		SetActivePane(rowActive, colActive);
	}
	
    // hide all column panes
	for (int row = 0; row < m_nRows; row++)
	{
		CWnd* pPaneHide = GetPane(row, nCol);
		ASSERT(pPaneHide != NULL);
		pPaneHide->ShowWindow(SW_HIDE);
		pPaneHide->SetDlgCtrlID(
			AFX_IDW_PANE_FIRST + row * 16 + m_nCols);
		
		for (int col = nCol + 1; col < m_nCols; col++)
		{
			CWnd* pPane = GetPane(row, col);
			ASSERT(pPane != NULL);
			pPane->SetDlgCtrlID(IdFromRowCol(row, col - 1));
		}
	}
	m_nCols--;
	m_pColInfo[m_nCols].nCurSize = m_pColInfo[nCol].nCurSize;
	RecalcLayout();
}

void CFX_SplitterWnd::ShowColumn()
{
	if ( m_nHidedCol == -1 )
		return;

	ASSERT_VALID(this);
	ASSERT(m_nCols < m_nMaxCols);
	ASSERT(m_nHidedCol != -1);
	
	int colNew = m_nHidedCol;
	m_nHidedCol = -1;
	int cxNew = m_pColInfo[m_nCols].nCurSize;
	m_nCols++;  // add a column
	ASSERT(m_nCols == m_nMaxCols);
	
    // fill the hided column
	int col;
	for (int row = 0; row < m_nRows; row++)
	{
		CWnd* pPaneShow = GetDlgItem(
			AFX_IDW_PANE_FIRST + row * 16 + m_nCols);
		ASSERT(pPaneShow != NULL);
		pPaneShow->ShowWindow(SW_SHOWNA);
		
		for (col = m_nCols - 2; col >= colNew; col--)
		{
			CWnd* pPane = GetPane(row, col);
			ASSERT(pPane != NULL);
			pPane->SetDlgCtrlID(IdFromRowCol(row, col + 1));
		}
		
		pPaneShow->SetDlgCtrlID(IdFromRowCol(row, colNew));
	}
	
    // new panes have been created -- recalculate layout
	for (col = colNew + 1; col < m_nCols; col++)
		m_pColInfo[col].nIdealSize = m_pColInfo[col - 1].nCurSize;
	m_pColInfo[colNew].nIdealSize = cxNew;

	RecalcLayout();
}

void CFX_SplitterWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (!m_bBarLocked)
		CSplitterWnd::OnLButtonDown(nFlags, point);
}

void CFX_SplitterWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (!m_bBarLocked)
		CSplitterWnd::OnMouseMove(nFlags, point);
	else
		CWnd::OnMouseMove(nFlags, point);	
}
