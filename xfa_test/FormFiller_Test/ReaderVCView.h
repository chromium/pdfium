// ReaderVCView.h : interface of the CReaderVCView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_READERVCVIEW_H__9AFC449C_26D0_4906_ABAE_1298871862E2__INCLUDED_)
#define AFX_READERVCVIEW_H__9AFC449C_26D0_4906_ABAE_1298871862E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ExportPage.h"
#include "afxtempl.h"
#include <wininet.h>
#include <afxinet.h>
#include <afxctl.h>

typedef struct  
{
	CString m_strFind;
	int m_nFlag;
	int m_nDirection;
	BOOL m_bFirst;
	wchar_t *m_pCurFindBuf; //unicode code
	int m_nStartPageIndex; // the page index for starting find
	int m_nStartCharIndex; //start index
}FindInfo;

typedef struct  
{
	double m_dbLeft;
	double m_dbTop;
	double m_dbRight;
	double m_dbBottom;
}PDFRect;

#define IsALTpressed()			(GetKeyState(VK_MENU) < 0)
#define IsCTRLpressed()			(GetKeyState(VK_CONTROL) < 0)
#define IsSHIFTpressed()		(GetKeyState(VK_SHIFT)&0x8000)
#define IsINSERTpressed()		(GetKeyState(VK_INSERT) & 0x01)	
// void Sample_PageToDevice(struct _FPDF_FORMFILLINFO* pThis,FPDF_PAGE page,double page_x,double page_y, int* device_x, int* device_y);
// 
// void Sample_Invalidate(struct _FPDF_FORMFILLINFO* pThis,FPDF_PAGE page, double left, double top, double right, double bottom);
// 
// void Sample_DeviceToPage(struct _FPDF_FORMFILLINFO* pThis,FPDF_PAGE page,int device_x, int device_y, double* page_x, double* page_y);
// 
// void Sample_SetCaret(struct _FPDF_FORMFILLINFO* pThis,FPDF_PAGE page,double page_x, double page_y, int nWidth, int nHeight);
// 
// void Sample_Release(struct _FPDF_FORMFILLINFO* pThis);
// 
// 
// int Sample_SetTimer(struct _FPDF_FORMFILLINFO* pThis, int uElapse, TimerCallback lpTimerFunc);
// 
// void Sample_KillTimer(struct _FPDF_FORMFILLINFO* pThis,int nID);
// 
// void Sample_SetCursor(struct _FPDF_FORMFILLINFO* pThis,int nCursorType);
// 
// void Sample_OnChange(struct _FPDF_FORMFILLINFO* pThis);
// 
// FPDF_BOOL	Sample_IsSHIFTKeyDown(struct _FPDF_FORMFILLINFO* pThis);
// 
// FPDF_BOOL	Sample_IsCTRLKeyDown(struct _FPDF_FORMFILLINFO* pThis);
// 
// FPDF_BOOL	Sample_IsALTKeyDown(struct _FPDF_FORMFILLINFO* pThis) ;
// 
// FPDF_BOOL	Sample_IsINSERTKeyDown(struct _FPDF_FORMFILLINFO* pThis) ;
class CReaderVCDoc;

typedef unsigned long			FX_DWORD;
static int CALLBACK FontEnumProc(const LOGFONTA *plf, const TEXTMETRICA *lpntme, FX_DWORD FontType, LPARAM lParam) 
{
	FPDF_AddInstalledFont((void*)lParam, plf->lfFaceName, plf->lfCharSet);
	return 1;
}
#define FXDWORD_FROM_MSBFIRST2(i) (((unsigned char)(i) << 24) | ((unsigned char)((i) >> 8) << 16) | ((unsigned char)((i) >> 16) << 8) | (unsigned char)((i) >> 24))
class CSampleFontInfo : public FPDF_SYSFONTINFO
{
public:
	CSampleFontInfo()
	{
		m_hDC = CreateDCA("DISPLAY", NULL, NULL, NULL);
	}
	
	HDC		m_hDC;
	
	void ReleaseImpl()
	{
		DeleteDC(m_hDC);
		delete this;
	}
	
	void EnumFontsImpl(void* pMapper) 
	{
		LOGFONTA lf;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfFaceName[0] = 0;
		lf.lfPitchAndFamily = 0;
		::EnumFontFamiliesExA(m_hDC, &lf, FontEnumProc, (LPARAM)pMapper, 0);
	}
	
	void* MapFontImpl(int weight, int bItalic, int charset, int pitch_family, const char* face, int* bExact)
	{
		return ::CreateFontA(-10, 0, 0, 0, weight, bItalic, 0, 0, 
			charset, OUT_TT_ONLY_PRECIS, 0, 0, pitch_family, face);
	}
	
	unsigned long GetFontDataImpl(void* hFont, unsigned int table, unsigned char* buffer, unsigned long buf_size)
	{
		HFONT hOldFont = (HFONT)::SelectObject(m_hDC, (HFONT)hFont);
		buf_size = ::GetFontData(m_hDC, FXDWORD_FROM_MSBFIRST2(table), 0, buffer, buf_size);
		::SelectObject(m_hDC, hOldFont);
		if (buf_size == GDI_ERROR) return 0;
		return buf_size;
	}
	
	unsigned long GetFaceNameImpl(void* hFont, char* buffer, unsigned long buf_size)
	{
		HFONT hOldFont = (HFONT)::SelectObject(m_hDC, (HFONT)hFont);
		unsigned long ret = ::GetTextFaceA(m_hDC, buf_size, buffer);
		::SelectObject(m_hDC, hOldFont);
		if (buf_size == GDI_ERROR) return 0;
		return ret;
	}
	
	int GetFontCharsetImpl(void* hFont)
	{
		TEXTMETRIC tm;
		HFONT hOldFont = (HFONT)::SelectObject(m_hDC, (HFONT)hFont);
		::GetTextMetrics(m_hDC, &tm);
		::SelectObject(m_hDC, hOldFont);
		return tm.tmCharSet;
	}
	
	void DeleteFontImpl(void* hFont)
	{
		::DeleteObject(hFont);
	}
};

typedef struct _FPDF_FILE
{
	FPDF_FILEHANDLER fileHandler;
	FILE* file;
}FPDF_FILE;

class CReaderVCView : public CView, public FPDF_FORMFILLINFO
{
protected: // create from serialization only
	CReaderVCView();
	DECLARE_DYNCREATE(CReaderVCView)

// Attributes
public:
	CReaderVCDoc* GetDocument();
	CChildFrame *m_pFram;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReaderVCView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
public:
	CExportPage* m_pExportPageDlg; 
	FPDF_PAGE GetPage(){ return m_pPage;}
	void SyncScroll();
	CRect SelectSegment(CPoint pt_lt, CPoint pt_rb);
	void GetRects(int nStart, int nEnd);
	void CreateCaret(CPoint pt);
	BOOL GetCharIndexByPoint(CPoint pt, int &nIndex);
	void LoadMyCursor(int nflag = 0);
	void DeleteAllRect();
	void DrawAllRect(CDC *pDC);
	void DrawReverse(CDC *pDC, CRect rect);
	void FindNext(int nDirection);
	void FindText(CString strFind, BOOL bCase, BOOL bWholeword,  int  Direction);
	void ScalPage(double dbScal);
	void GotoPage(int index);
	BOOL SetPDFDocument(FPDF_DOCUMENT pDoc, int nPageNum);
	void DrawPage(int nRotate, CDC *pDC);
	void SetPageMetrics(FPDF_PAGE pPage);

	void SetScalFactor(double dbScal);
	void GetNewPageSize(int &nsizeX, int &nsizeY);
	BOOL LoadPDFPage(FPDF_DOCUMENT doc, int nIndex, CPoint pos = CPoint(0,0));

	FPDF_DOCUMENT GetPDFDoc(){ return m_pDoc;}
	int GetTotalPages(){ return m_nTotalPage;}
	BOOL  DeviceToPage(CPoint pt, double& page_x, double& page_y);
	BOOL  PageToDevice(double page_x, double page_y, CPoint& pt);
	int  GetCurrentPageIndex() { return m_nPageIndex; }
	static BOOL HttpDataPut(CString csData, CString csAppName, CString csObject, CString csServer, 
		CString csUserName, CString csPassword, INTERNET_PORT nPort, BOOL IsHTTPS);
	static BOOL NeedAuth(CHttpFile *pHttpFile);
	static BOOL HttpDataPost(CString csData, CString csAppName, CString csObject, CString csServer, CString csUserName, 
		CString csPassword, INTERNET_PORT nPort , BOOL IsHTTPS, CString csContentType, CString csAddHeader, CString &csResponse);
	static void	CreateLocalPath(CString &csPath);

	virtual ~CReaderVCView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
public:
	void	PageToDeviceImpl(FPDF_PAGE page,double page_x,double page_y, int* device_x, int* device_y);
	void	DeviceToPageImpl(FPDF_PAGE page,int device_x, int device_y, double* page_x, double* page_y);
	void	InvalidateImpl(FPDF_PAGE page, double left, double top, double right, double bottom);
	void	OutputSelectedRectImpl(FPDF_PAGE page, double left, double top, double right, double bottom);
	void	SetCaretImpl(FPDF_PAGE page,double page_x, double page_y, int nWidth, int nHeight);
	void	ReleaseImpl();
	int     SetTimerImpl(int uElapse, TimerCallback lpTimerFunc);
	void	KillTimerImpl(int nID);
	FPDF_SYSTEMTIME GetLocalTimeImpl();
	void    SetCurorImpl(int nCursorType);	
	void    ExecuteNamedActionImpl(FPDF_BYTESTRING namedaction);
	void    OnChangeImpl();
	bool    IsSHIFTKeyDownImpl();
	bool    IsCTRLKeyDownImpl();
	bool    IsALTKeyDownImpl();
	bool    IsINSERTKeyDownImpl();
	BOOL	SubmitFormImpl(void* pBuffer, int nLength, CString strURL);
	FPDF_PAGE GetPageImpl(FPDF_DOCUMENT document,int nPageIndex);
	int		GetRotationImpl(FPDF_PAGE page);
	CString GetFilePath();
	FPDF_PAGE	GetCurrentPageImpl(FPDF_DOCUMENT document);
// 	friend FPDF_DOCUMENT _FPDF_GetCurDocument(struct _FPDF_FORMFILLINFO* pThis);
// 	friend FPDF_PAGE _FPDF_GetCurPage(struct _FPDF_FORMFILLINFO* pThis);
private:
//	FPDF_APP	m_App;
//	FPDF_FORMFILLINFO m_formFiledInfo;
private:
//	FPDF_APP GetFPDFApp() {return m_App;}
	unsigned int ComposeFlag();
private:
	CMap<int, int, FPDF_PAGE, FPDF_PAGE> m_pageMap;
	FPDF_FORMHANDLE	m_pApp;
	FPDF_BITMAP			m_bmp;
	//for render pdf
	FPDF_DOCUMENT		m_pDoc;
	FPDF_PAGE			m_pPage;
	int					m_nTotalPage;
	int					m_nRotateFlag;
	double				m_dbScaleFactor;
	int					m_nPageIndex;
	double				m_dbPageWidth;
	double				m_dbPageHeight;
	int					m_nStartX;
	int					m_nStartY;
	int					m_nActualSizeX;
	int					m_nActualSizeY;

	//for search text
	FPDF_TEXTPAGE		m_pTextPage;
	FindInfo			m_FindInfo;
	FPDF_SCHHANDLE		m_pSCHHandle;
	PDFRect *			m_rtFind;
	int					m_nRectNum;
	
	//for select text
	BOOL m_bSelect;
	BOOL m_bHand;
	BOOL m_bSnap;
	BOOL m_bHasChar; //whether can get a char when left button be clicked
	CPoint m_ptLBDown;
	CPoint m_ptLBUp;
	CPoint m_ptOld;

	int m_nStartIndex; //char index
	int m_nOldIndex;
	int m_nEndIndex;
	CArray <PDFRect, PDFRect> m_rtArray;
	CRect m_rtOld;
	
	//
	int m_nPosH;
	int m_nPosV;

	// for links
	FPDF_PAGELINK m_pLink;

	BOOL m_bBookmark;

public:
	wchar_t* m_pwsResponse;

private:

	CArray<CRect, CRect&> m_SelectArray;
	static CMap<int, int,TimerCallback, TimerCallback> m_mapTimerFuns;
public:
	static void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
	{
		TimerCallback callback =  NULL;
		m_mapTimerFuns.Lookup(idEvent, callback);
		if(callback)
			(*callback)(idEvent);
	}
// Generated message map functions
protected:
	//{{AFX_MSG(CReaderVCView)
	afx_msg void OnDocFirstpage();
	afx_msg void OnDocGotopage();
	afx_msg void OnDocLastpage();
	afx_msg void OnDocNextpage();
	afx_msg void OnDocPrepage();
	afx_msg void OnClockwise();
	afx_msg void OnCounterclockwise();
	afx_msg void OnViewActualSize();
	afx_msg void OnViewFitPage();
	afx_msg void OnViewFitWidth();
	afx_msg void OnViewZoomIn();
	afx_msg void OnViewZoomOut();
	afx_msg void OnViewZoomTo();
	afx_msg void OnEditFind();
	afx_msg void OnFilePrint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnToolSnapshot();
	afx_msg void OnToolSelect();
	afx_msg void OnToolHand();
	afx_msg void OnToolPdf2txt();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnToolExtractlinks();
	afx_msg void OnDestroy();
	afx_msg void OnUpdateDocFirstpage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDocLastpage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDocNextpage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDocPrepage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolHand(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolSnapshot(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolSelect(CCmdUI* pCmdUI);
	afx_msg void OnViewBookmark();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnRenderbitmap();
	afx_msg void OnExportPdfToBitmap();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnFileSave();
	afx_msg void OnTestJS();
	afx_msg void OnPrintMetalfile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ReaderVCView.cpp
inline CReaderVCDoc* CReaderVCView::GetDocument()
   { return (CReaderVCDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_READERVCVIEW_H__9AFC449C_26D0_4906_ABAE_1298871862E2__INCLUDED_)
