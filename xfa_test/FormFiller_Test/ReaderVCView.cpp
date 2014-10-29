// ReaderVCView.cpp : implementation of the CReaderVCView class
//

#include "stdafx.h"
#include "ReaderVC.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "ReaderVCDoc.h"
#include "ReaderVCView.h"

#include "GotoPageDlg.h"
#include "ZoomDlg.h"
#include "FindDlg.h"
#include "ConvertDlg.h"
#include "JS_ResponseDlg.h"
#include "TestJsDlg.h"
//#include "../../include/pp_event.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReaderVCView

IMPLEMENT_DYNCREATE(CReaderVCView, CView)

BEGIN_MESSAGE_MAP(CReaderVCView, CView)
	//{{AFX_MSG_MAP(CReaderVCView)
	ON_COMMAND(ID_DOC_FIRSTPAGE, OnDocFirstpage)
	ON_COMMAND(ID_DOC_GOTOPAGE, OnDocGotopage)
	ON_COMMAND(ID_DOC_LASTPAGE, OnDocLastpage)
	ON_COMMAND(ID_DOC_NEXTPAGE, OnDocNextpage)
	ON_COMMAND(ID_DOC_PREPAGE, OnDocPrepage)
	ON_COMMAND(ID_VIEW_CLOCKWISE, OnClockwise)
	ON_COMMAND(ID_VIEW_COUNTERCLOCKWISE, OnCounterclockwise)
	ON_COMMAND(ID_VIEW_ACTUALSIZE, OnViewActualSize)
	ON_COMMAND(ID_VIEW_FITPAGE, OnViewFitPage)
	ON_COMMAND(ID_VIEW_FITWIDTH, OnViewFitWidth)
	ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomIn)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomOut)
	ON_COMMAND(ID_VIEW_ZOOMTO, OnViewZoomTo)
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_TOOL_SNAPSHOT, OnToolSnapshot)
	ON_COMMAND(ID_TOOL_SELECT, OnToolSelect)
	ON_COMMAND(ID_TOOL_HAND, OnToolHand)
	ON_COMMAND(ID_TOOL_PDF2TXT, OnToolPdf2txt)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_COMMAND(ID_TOOL_EXTRACTLINKS, OnToolExtractlinks)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_DOC_FIRSTPAGE, OnUpdateDocFirstpage)
	ON_UPDATE_COMMAND_UI(ID_DOC_LASTPAGE, OnUpdateDocLastpage)
	ON_UPDATE_COMMAND_UI(ID_DOC_NEXTPAGE, OnUpdateDocNextpage)
	ON_UPDATE_COMMAND_UI(ID_DOC_PREPAGE, OnUpdateDocPrepage)
	ON_UPDATE_COMMAND_UI(ID_TOOL_HAND, OnUpdateToolHand)
	ON_UPDATE_COMMAND_UI(ID_TOOL_SNAPSHOT, OnUpdateToolSnapshot)
	ON_UPDATE_COMMAND_UI(ID_TOOL_SELECT, OnUpdateToolSelect)
	ON_COMMAND(ID_VIEW_BOOKMARK, OnViewBookmark)
	ON_WM_MOUSEWHEEL()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_RENDERBITMAP, OnRenderbitmap)
	ON_COMMAND(ID_EXPORT_PDF_TO_BITMAP, OnExportPdfToBitmap)
	ON_WM_CHAR()
	ON_WM_KEYUP()
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(IDM_Test_JS, OnTestJS)
	ON_COMMAND(TEST_PRINT_METALFILE, OnPrintMetalfile)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReaderVCView construction/destruction
void Sample_PageToDevice(struct _FPDF_FORMFILLINFO* pThis,FPDF_PAGE page,double page_x,double page_y, int* device_x, int* device_y)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		pView->PageToDeviceImpl(page, page_x, page_y, device_x, device_y);
	}
	//((CReaderVCView*)pThis)->PageToDeviceImpl(page, page_x, page_y, device_x, device_y);
}

void Sample_Invalidate(struct _FPDF_FORMFILLINFO* pThis,FPDF_PAGE page, double left, double top, double right, double bottom)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		pView->InvalidateImpl(page,left, top, right, bottom);
	}
	//((CReaderVCView*)pThis)->InvalidateImpl(page,left, top, right, bottom);
}

void Sample_OutputSelectedRect(struct _FPDF_FORMFILLINFO* pThis,FPDF_PAGE page, double left, double top, double right, double bottom)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		pView->OutputSelectedRectImpl(page,left, top, right, bottom);
	}
}

void Sample_DeviceToPage(struct _FPDF_FORMFILLINFO* pThis,FPDF_PAGE page,int device_x, int device_y, double* page_x, double* page_y)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		pView->DeviceToPageImpl(page, device_x, device_y, page_x, page_y);
	}
	//((CReaderVCView*)pThis)->DeviceToPageImpl(page, device_x, device_y, page_x, page_y);
}
/* /* Remove by Amy Lin 20100913, Since we don't this the FFI_SetCaret any more.
void Sample_SetCaret(struct _FPDF_FORMFILLINFO* pThis,FPDF_PAGE page,double page_x, double page_y, int nWidth, int nHeight)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		pView->SetCaretImpl(page, page_x, page_y, nWidth,  nHeight);
	}
	//((CReaderVCView*)pThis)->SetCaretImpl(page, page_x, page_y, nWidth,  nHeight);
}
*/
void Sample_Release(struct _FPDF_FORMFILLINFO* pThis)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		pView->ReleaseImpl();
	}
	//((CReaderVCView*)pThis)->ReleaseImpl();
}


int Sample_SetTimer(struct _FPDF_FORMFILLINFO* pThis, int uElapse, TimerCallback lpTimerFunc)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		return pView->SetTimerImpl(uElapse, lpTimerFunc);
	}else{
		return -1;
	}
	//return ((CReaderVCView*)pThis)->SetTimerImpl(uElapse, lpTimerFunc);
}

void Sample_KillTimer(struct _FPDF_FORMFILLINFO* pThis,int nID)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		pView->KillTimerImpl(nID);
	}
	//((CReaderVCView*)pThis)->KillTimerImpl(nID);
}

void Sample_SetCursor(struct _FPDF_FORMFILLINFO* pThis,int nCursorType)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		pView->SetCurorImpl(nCursorType);
	}
	//((CReaderVCView*)pThis)->SetCurorImpl(nCursorType);
}

void Sample_OnChange(struct _FPDF_FORMFILLINFO* pThis)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		pView->OnChangeImpl();
	}
}
FPDF_BOOL	Sample_IsSHIFTKeyDown(struct _FPDF_FORMFILLINFO* pThis)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		return pView->IsSHIFTKeyDownImpl();
	}
	return FALSE;
}
FPDF_BOOL	Sample_IsCTRLKeyDown(struct _FPDF_FORMFILLINFO* pThis)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		return pView->IsCTRLKeyDownImpl();
	}
	return FALSE;
}
FPDF_BOOL	Sample_IsALTKeyDown(struct _FPDF_FORMFILLINFO* pThis) 
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		return pView->IsALTKeyDownImpl();
	}
	return FALSE;
}
FPDF_BOOL	Sample_IsINSERTKeyDown(struct _FPDF_FORMFILLINFO* pThis) 
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		return pView->IsINSERTKeyDownImpl();
	}
	return FALSE;
}

FPDF_PAGE Sample_GetPage(struct _FPDF_FORMFILLINFO* pThis,FPDF_DOCUMENT document, int nPageIndex)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		return pView->GetPageImpl(document,nPageIndex);
	}
	return NULL;
}

FPDF_PAGE Sample_GetCurrentPage(struct _FPDF_FORMFILLINFO* pThis, FPDF_DOCUMENT document)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		return pView->GetCurrentPageImpl(document);
	}
	return NULL;
}

int Sample_GetRotation(struct _FPDF_FORMFILLINFO* pThis, FPDF_PAGE page)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		return pView->GetRotationImpl(page);
	}
	return NULL;
}
FPDF_SYSTEMTIME Sample_GetLocalTime(struct _FPDF_FORMFILLINFO* pThis)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		return pView->GetLocalTimeImpl();
	}
	return FPDF_SYSTEMTIME();
}

void SampleRelease(struct _FPDF_SYSFONTINFO* pThis)
{
	((CSampleFontInfo*)pThis)->ReleaseImpl();
}

void SampleEnumFonts(struct _FPDF_SYSFONTINFO* pThis, void* pMapper)
{
	((CSampleFontInfo*)pThis)->EnumFontsImpl(pMapper);
}

void* SampleMapFont(struct _FPDF_SYSFONTINFO* pThis, int weight, int bItalic, int charset, int pitch_family, 
						const char* face, int* bExact)
{
	return ((CSampleFontInfo*)pThis)->MapFontImpl(weight, bItalic, charset, pitch_family, face, bExact);
}

unsigned long SampleGetFontData(struct _FPDF_SYSFONTINFO* pThis, void* hFont,
								unsigned int table, unsigned char* buffer, unsigned long buf_size)
{
	return ((CSampleFontInfo*)pThis)->GetFontDataImpl(hFont, table, buffer, buf_size);
}

unsigned long SampleGetFaceName(struct _FPDF_SYSFONTINFO* pThis, void* hFont, char* buffer, unsigned long buf_size)
{
	return ((CSampleFontInfo*)pThis)->GetFaceNameImpl(hFont, buffer, buf_size);
}

int SampleGetFontCharset(struct _FPDF_SYSFONTINFO* pThis, void* hFont)
{
	return ((CSampleFontInfo*)pThis)->GetFontCharsetImpl(hFont);
}

void SampleDeleteFont(struct _FPDF_SYSFONTINFO* pThis, void* hFont)
{
	((CSampleFontInfo*)pThis)->DeleteFontImpl(hFont);
}

void SetSampleFontInfo()
{
	CSampleFontInfo* pFontInfo = new CSampleFontInfo;
	pFontInfo->version = 1;
	pFontInfo->DeleteFont = SampleDeleteFont;
	pFontInfo->EnumFonts = SampleEnumFonts;
	pFontInfo->GetFaceName = SampleGetFaceName;
	pFontInfo->GetFont = NULL;
	pFontInfo->GetFontCharset = SampleGetFontCharset;
	pFontInfo->GetFontData = SampleGetFontData;
	pFontInfo->MapFont = SampleMapFont;
	pFontInfo->Release = SampleRelease;
	FPDF_SetSystemFontInfo(pFontInfo);
}

void 	Sample_ExecuteNamedAction(struct _FPDF_FORMFILLINFO* pThis, FPDF_BYTESTRING namedAction)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if ( pView )
	{
		 pView->ExecuteNamedActionImpl(namedAction);
	}

}
void CReaderVCView::ExecuteNamedActionImpl(FPDF_BYTESTRING namedaction)
{
	if(strcmp("Print", (LPCSTR)namedaction) == 0)
		OnFilePrint();
}
void CReaderVCView::OutputSelectedRectImpl(FPDF_PAGE page, double left, double top, double right, double bottom)
{

	if(page == m_pPage)
	{

 		int device_left, device_top, device_right, device_bottom;
		
		int nActualRangeX = 0;
		int nActualRangeY = 0;
		if ( m_nRotateFlag % 2 == 0 )
		{
			nActualRangeX = m_nActualSizeX;
			nActualRangeY = m_nActualSizeY;
		}else{
			nActualRangeX = m_nActualSizeY;
			nActualRangeY = m_nActualSizeX;
		}
		
		FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
			(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, left, top, &device_left, &device_top);
		
		FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
			(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, right, bottom, &device_right, &device_bottom);
		
		CRect rc(device_left,device_top, device_right, device_bottom);



		m_SelectArray.Add(rc);


	}
}

void		Sample_Release(FPDF_LPVOID clientData)
{
	if (!clientData) return;

	fclose(((FPDF_FILE*)clientData)->file);
	delete ((FPDF_FILE*)clientData);
}

FPDF_DWORD	Sample_GetSize(FPDF_LPVOID clientData)
{
	if (!clientData) return 0;

	long curPos = ftell(((FPDF_FILE*)clientData)->file);
	fseek(((FPDF_FILE*)clientData)->file, 0, SEEK_END);
	long size = ftell(((FPDF_FILE*)clientData)->file);
	fseek(((FPDF_FILE*)clientData)->file, curPos, SEEK_SET);

	return (FPDF_DWORD)size;
}

FPDF_RESULT	Sample_ReadBlock(FPDF_LPVOID clientData, FPDF_DWORD offset, FPDF_LPVOID buffer, FPDF_DWORD size)
{
	if (!clientData) return -1;

	fseek(((FPDF_FILE*)clientData)->file, (long)offset, SEEK_SET);
	size_t readSize = fread(buffer, 1, size, ((FPDF_FILE*)clientData)->file);
	return readSize == size ? 0 : -1;
}

FPDF_RESULT	Sample_WriteBlock(FPDF_LPVOID clientData, FPDF_DWORD offset, FPDF_LPCVOID buffer, FPDF_DWORD size)
{
	if (!clientData) return -1;

	fseek(((FPDF_FILE*)clientData)->file, (long)offset, SEEK_SET);
	//Write data
	size_t writeSize = fwrite(buffer, 1, size, ((FPDF_FILE*)clientData)->file);
	return writeSize == size ? 0 : -1;
}

FPDF_RESULT	Sample_Flush(FPDF_LPVOID clientData)
{
	if (!clientData) return -1;

	//Flush file
	fflush(((FPDF_FILE*)clientData)->file);

	return 0;
}

FPDF_RESULT	Sample_Truncate(FPDF_LPVOID clientData, FPDF_DWORD size)
{
	return 0;
}

void	Sample_DisplayCaret(struct _FPDF_FORMFILLINFO* pThis, FPDF_PAGE page, FPDF_BOOL bVisible, double left, double top, double right, double bottom)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if (!pView) return;

	HWND hWnd = pView->m_hWnd;

	if (bVisible)
	{
		CPoint ltPt;
		pView->PageToDevice(left, top, ltPt);
		CPoint rbPt;
		pView->PageToDevice(right, bottom, rbPt);
		CRect rcCaret(ltPt, rbPt);
		
		::DestroyCaret();
		::CreateCaret(hWnd, (HBITMAP)0, rcCaret.Width(), rcCaret.Height());
		::SetCaretPos (rcCaret.left, rcCaret.top);
		::ShowCaret(hWnd);
	}
	else
	{
		::DestroyCaret();
		::HideCaret(hWnd);
	}
}

int		Sample_GetCurrentPageIndex(struct _FPDF_FORMFILLINFO* pThis, FPDF_DOCUMENT document)
{
	CReaderVCView* pView =(CReaderVCView*)pThis;
	if (!pView) return -1;

	return pView->GetCurrentPageIndex();
}

void	Sample_SetCurrentPage(struct _FPDF_FORMFILLINFO* pThis, FPDF_DOCUMENT document, int iCurPage)
{
	CReaderVCView* pView = (CReaderVCView*)pThis;
	if (!pView) return;

	FPDF_DOCUMENT curDoc = pView->GetPDFDoc();
	if (curDoc != document)
		return;

	int nPageCount = FPDF_GetPageCount(curDoc);
	if (nPageCount > iCurPage)
	{
		int nCurPageInx = pView->GetCurrentPageIndex();
		if (nCurPageInx != iCurPage)
		{
			pView->GotoPage(nCurPageInx);
		}
	}
}

void	Sample_GotoURL(struct _FPDF_FORMFILLINFO* pThis, FPDF_DOCUMENT document, FPDF_WIDESTRING wsURL)
{
	CReaderVCView* pView = (CReaderVCView*)pThis;
	if (!pView) return;

	wchar_t* pURL = (wchar_t*)wsURL;
	MessageBoxW(NULL, pURL, NULL, MB_OK);
}

FPDF_WIDESTRING	Sample_GetURL(struct _FPDF_FORMFILLINFO* pThis, FPDF_DOCUMENT document)
{
	CReaderVCView* pView = (CReaderVCView*)pThis;
	if (!pView) return NULL;

	if (pView->GetPDFDoc() != document)
		return NULL;

	//not support in this demo

	return NULL;
}

void Sample_AddDoRecord(struct _FPDF_FORMFILLINFO* pThis, FPDF_DOCUMENT document, FPDF_WIDGET hWidget)
{
	//not support
}

void	Sample_PageEvent(struct _FPDF_FORMFILLINFO* pThis, FPDF_PAGE page, FPDF_DWORD eventFlag)
{
	//
}

void	Sample_GetPageViewRect(struct _FPDF_FORMFILLINFO* pThis, FPDF_PAGE page, double* left, double* top, double* right, double* bottom)
{
	CReaderVCView* pView = (CReaderVCView*)pThis;
	if (!pView) return;

	if (pView->GetPage() != page)
		return;

	CRect clientRect;
	pView->GetClientRect(&clientRect);

	*left = (double)clientRect.left;
	*right = (double)clientRect.right;
	*top = (double)clientRect.top;
	*bottom = (double)clientRect.bottom;
}

#define	 WM_XFAMENU_COPY	  10000

FPDF_BOOL Sample_PopupMenu(struct _FPDF_FORMFILLINFO* pThis, FPDF_PAGE page, FPDF_WIDGET hWidget, int menuFlag, float x, float y)
{
	CReaderVCView* pView = (CReaderVCView*)pThis;
	if (!pView)
		return FALSE;

	CMenu menu;
	menu.CreatePopupMenu();

	int nMenuIndex = 0;

	if (menuFlag & FXFA_MEMU_COPY)
		menu.InsertMenu(nMenuIndex++, MF_BYPOSITION, WM_XFAMENU_COPY, "Copy");
	//...

	CPoint pt;
	pView->PageToDevice(x, y, pt);

	UINT nID = menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, pView);
	switch(nID)
	{
	case WM_XFAMENU_COPY:
		{
			FPDF_DWORD length = 0;
			FPDF_Widget_Copy(pView->GetPDFDoc(), hWidget, NULL, &length);
			if (length > 0)
			{
				unsigned short* buffer = (unsigned short*)malloc((length+1)*sizeof(unsigned short));
				memset(buffer, 0, (length+1)*sizeof(unsigned short));
				FPDF_Widget_Copy(pView->GetPDFDoc(), hWidget, buffer, &length);
				free(buffer);
			}
		}
		break;
	}

	menu.DestroyMenu();

	return TRUE;
}

FPDF_FILEHANDLER* Sample_OpenFile(struct _FPDF_FORMFILLINFO* pThis, int fileFlag, FPDF_WIDESTRING wsURL)
{
	char* pszURL;
	CString strURL;
	if (wsURL == NULL) {
		if (fileFlag == FXFA_FILE_XDP)
			strURL = "C://temp.xdp";
		else if(fileFlag == FXFA_FILE_XML)
			strURL = "C://temp.xml";
	}
	else {
		int iSize;
		iSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsURL, -1, NULL, 0, NULL, NULL);
		pszURL = (char*)malloc((iSize+1));
		WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsURL, -1, pszURL, iSize, NULL, NULL);
		CString str(pszURL);
		strURL = str;
	}
	

	FILE* file = fopen(strURL, "r");
	FPDF_FILE* pFileHander = new FPDF_FILE;
	pFileHander->file = file;
	pFileHander->fileHandler.clientData = pFileHander;
	pFileHander->fileHandler.Flush = Sample_Flush;
	pFileHander->fileHandler.GetSize = Sample_GetSize;
	pFileHander->fileHandler.ReadBlock = Sample_ReadBlock;
	pFileHander->fileHandler.Release = Sample_Release;
	pFileHander->fileHandler.Truncate = Sample_Truncate;
	pFileHander->fileHandler.WriteBlock = Sample_WriteBlock;

	free(pszURL);
	return &pFileHander->fileHandler;
}

FPDF_BOOL Sample_GetFilePath(struct _FPDF_FORMFILLINFO* pThis, FPDF_FILEHANDLER* pFileHandler, FPDF_BSTR* path)
{
	CReaderVCView* pView = (CReaderVCView*)pThis;
	if (!pView)
		return NULL;

	CString filePath = pView->GetFilePath();
	FPDF_BStr_Set(path, filePath.GetBuffer(filePath.GetLength()), filePath.GetLength());

	return TRUE;
}

void	Sample_EmailTo(struct _FPDF_FORMFILLINFO* pThis, FPDF_FILEHANDLER* fileHandler, FPDF_WIDESTRING emailTo)
{
	MessageBoxW(NULL, (wchar_t*)emailTo, L"Sample_email", MB_OK);
}

void	Sample_UploadTo(struct _FPDF_FORMFILLINFO* pThis, FPDF_FILEHANDLER* fileHandler, FPDF_WIDESTRING uploadTo)
{
	int iSize;
	char* pszURL;
	iSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)uploadTo, -1, NULL, 0, NULL, NULL);
	pszURL = (char*)malloc((iSize+1));
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)uploadTo, -1, pszURL, iSize, NULL, NULL);
	CString strPath(pszURL);

	CString strUploadPath = "C://test";
	int pos = strPath.ReverseFind('.');
	if (pos != -1){
		CString suffix = strPath.Right(strPath.GetLength()-pos);
		strUploadPath += suffix;
	}

	FILE* file = fopen(strUploadPath, "r");
	if (file) {
		int size = fileHandler->GetSize(fileHandler->clientData);
		BYTE* buffer = (BYTE*)malloc(size);
		fileHandler->ReadBlock(fileHandler->clientData, 0, buffer, size);
		fwrite(buffer, size, 1, file);
		fflush(file);
		fclose(file);
		free(buffer);
	}

	free(pszURL);
}

int		Sample_GetAppName(struct _FPDF_FORMFILLINFO* pThis, void* appName, int length)
{
	if(appName == NULL || length <= 0)
	{
		CString name = AfxGetAppName();
		return name.GetLength();
	}	
	else
	{
		CString name = AfxGetAppName();
		int len = name.GetLength();
		if(length > len)
			length = len;
		memcpy(appName, name.GetBuffer(name.GetLength()), length);
		return length;
	}
}

int		Sample_GetPlatform(struct _FPDF_FORMFILLINFO* pThis, void* platform, int length)
{
	if(platform == NULL || length <= 0)
	{
		return 3;
	}	
	else
	{
		if(length > 3)
			length = 3;
		memcpy(platform, "win", length);
		return length;
	}
}

int		Sample_GetDocumentCount(struct _FPDF_FORMFILLINFO* pThis)
{
	return 1;
}

int		Sample_GetCurDocumentIndex(struct _FPDF_FORMFILLINFO* pThis)
{
	return 0;
}

FPDF_LPFILEHANDLER	Sample_DownloadFromURL(struct _FPDF_FORMFILLINFO* pThis, FPDF_WIDESTRING URL)
{
	int iSize;
	char* pszURL;
	iSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)URL, -1, NULL, 0, NULL, NULL);
	pszURL = (char*)malloc((iSize+1));
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)URL, -1, pszURL, iSize, NULL, NULL);
	CString strURL(pszURL);

	CString bsLocal = strURL;
	CReaderVCView::CreateLocalPath(bsLocal);

	if (strURL.Left(7) == "http://")
	{
		CInternetSession sess;
		CHttpFile *pFile = (CHttpFile*)sess.OpenURL(strURL);
		int iLength = pFile->GetLength();
		if (pFile == NULL || iLength < 1) return NULL;

		FILE *pImageFile = fopen(bsLocal, "wb");

		BYTE* pContent = new BYTE[iLength];
		memset(pContent, 0, iLength);
		int iRead = pFile->Read(pContent, iLength);

		fwrite(pContent, 1, iLength, pImageFile);
		free(pContent);
		fflush(pImageFile);
		fclose(pImageFile);

		pFile->Close();
		delete pFile;
		sess.Close();
	}
	else if (strURL.Left(6) == "ftp://")
	{
		CInternetSession sess;
		CFtpConnection* pConnect = sess.GetFtpConnection(bsLocal, "NULL", "NULL"); 
		CInternetFile* pFile = pConnect->OpenFile(bsLocal);

		int iLength = pFile->GetLength();
		if (pFile == NULL || iLength < 1) return NULL;
		FILE *pImageFile = fopen(bsLocal, "wb");

		BYTE* pContent = new BYTE[iLength];
		memset(pContent, 0, iLength);
		int iRead = pFile->Read(pContent, iLength);

		fwrite(pContent, 1, iLength, pImageFile);
		free(pContent);
		fflush(pImageFile);
		fclose(pImageFile);

		pFile->Close();
		delete pFile;
		sess.Close();
	}	
	
	free(pszURL);
	
	FPDF_FILE* fileWrap = new FPDF_FILE;
	FILE* file = fopen(bsLocal, "r");
	fileWrap->file = file;
	fileWrap->fileHandler.clientData = fileWrap;
	fileWrap->fileHandler.ReadBlock = Sample_ReadBlock;
	fileWrap->fileHandler.GetSize = Sample_GetSize;
	fileWrap->fileHandler.Flush = Sample_Flush;
	fileWrap->fileHandler.Release = Sample_Release;
	fileWrap->fileHandler.Truncate = Sample_Truncate;
	fileWrap->fileHandler.WriteBlock = Sample_WriteBlock;

	return &fileWrap->fileHandler;
}

FPDF_BOOL	Sample_PostRequestURL(struct _FPDF_FORMFILLINFO* pThis, FPDF_WIDESTRING wsURL, FPDF_WIDESTRING wsData, FPDF_WIDESTRING wsContentType, FPDF_WIDESTRING wsEncode, FPDF_WIDESTRING wsHeader, FPDF_BSTR* respone)
{
	int iSize;
	char* pszURL;
	iSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsURL, -1, NULL, 0, NULL, NULL);
	pszURL = (char*)malloc((iSize+1));
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsURL, -1, pszURL, iSize, NULL, NULL);
	CString csURL(pszURL);

	char* pszData;
	iSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsData, -1, NULL, 0, NULL, NULL);
	pszData = (char*)malloc(iSize+1);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsData, -1, pszData, iSize, NULL, NULL);
	CString csData(pszData);
	
	char* pszContentType;
	iSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsContentType, -1, NULL, 0, NULL, NULL);
	pszContentType = (char*)malloc(iSize+1);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsContentType, -1, pszContentType, iSize, NULL, NULL);
	CString csContentType(pszContentType);
	
	char* pszHeader;
	iSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsHeader, -1, NULL, 0, NULL, NULL);
	pszHeader = (char*)malloc(iSize+1);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsHeader, -1, pszHeader, iSize, NULL, NULL);
	CString csHeader(pszHeader);

	CString csApp = AfxGetAppName();
	csApp += L"/1.0";
	BOOL bRet = FALSE;
	DWORD dwServiceType = 0, dwFlags = ICU_NO_META;
	CString csServer, sObject, csUserName, csPassword;
	INTERNET_PORT nPort = 0;

	bRet = AfxParseURLEx(csURL, dwServiceType, csServer, sObject, nPort, csUserName, csPassword, dwFlags);
	if (!bRet)
		return bRet;

	if (dwServiceType != AFX_INET_SERVICE_HTTP && dwServiceType != AFX_INET_SERVICE_HTTPS)
		return bRet;

	CString csObject = sObject;
	CString csResponse;
	bRet = CReaderVCView::HttpDataPost(csData, csApp, csObject, csServer, csUserName, csPassword, nPort,
		dwServiceType == AFX_INET_SERVICE_HTTPS, csContentType, csHeader, csResponse);

	FPDF_BStr_Init(respone);
	FPDF_BStr_Set(respone, (FPDF_LPCSTR)csResponse.GetBuffer(csResponse.GetLength()), csResponse.GetLength());

	free(pszURL);
	free(pszData);
	free(pszContentType);
	free(pszHeader);

	return true;
}

FPDF_BOOL	Sample_PutRequestURL(struct _FPDF_FORMFILLINFO* pThis, FPDF_WIDESTRING wsURL, FPDF_WIDESTRING wsData, FPDF_WIDESTRING wsEncode)
{
	int iSize;
	char* pszURL;
	iSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsURL, -1, NULL, 0, NULL, NULL);
	pszURL = (char*)malloc((iSize+1));
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsURL, -1, pszURL, iSize, NULL, NULL);
	CString csURL(pszURL);

	char* pszData;
	iSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsData, -1, NULL, 0, NULL, NULL);
	pszData = (char*)malloc((iSize+1));
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsData, -1, pszData, iSize, NULL, NULL);
	CString csData(pszData);

	CString csApp = AfxGetAppName();
	csApp += L"/1.0";
	BOOL bRet = FALSE;
	DWORD dwServiceType = 0, dwFlags = ICU_NO_META;
	CString csServer, sObject, csUserName, csPassword;
	INTERNET_PORT nPort = 0;

	bRet = AfxParseURLEx(csURL, dwServiceType, csServer, sObject, nPort, csUserName, csPassword, dwFlags);
	if (!bRet)
		return bRet;

	if (dwServiceType != AFX_INET_SERVICE_HTTP && dwServiceType != AFX_INET_SERVICE_HTTPS)
		return bRet;

	CString csObject = sObject;

	bRet = CReaderVCView::HttpDataPut(csData, csApp, csObject, csServer, csUserName, csPassword, nPort, dwServiceType == AFX_INET_SERVICE_HTTPS);
	
	free(pszData);
	free(pszURL);
	return TRUE;
}

FPDF_BOOL	Sample_ShowFileDialog(struct _FPDF_FORMFILLINFO* pThis, FPDF_WIDESTRING wsTitle, FPDF_WIDESTRING wsFilter, FPDF_BOOL isOpen, FPDF_STRINGHANDLE pathArr)
{
	int iSize;
	char* pszFilter;
	iSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsFilter, -1, NULL, 0, NULL, NULL);
	pszFilter = (char*)malloc((iSize+1));
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wsFilter, -1, pszFilter, iSize, NULL, NULL);

	CFileDialog fileOpen(isOpen, NULL,NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST, pszFilter);
	//	fileOpen.m_ofn.Flags|=OFN_ENABLEHOOK|OFN_ALLOWMULTISELECT;
	if(fileOpen.DoModal()==IDCANCEL) 
	{ 
		return FALSE;	
	}
	
	POSITION pos;	
	pos=fileOpen.GetStartPosition(); 
	CString csFile;
	while(pos!=NULL) 
	{ 
		csFile=fileOpen.GetNextPathName(pos); 
		FPDF_StringHandleAddString(pathArr, csFile.GetBuffer(csFile.GetLength()), csFile.GetLength());		
	}

	free(pszFilter);
	return TRUE;
}

FPDF_SYSTEMTIME CReaderVCView::GetLocalTimeImpl()
{
	FPDF_SYSTEMTIME sys;
	time_t curTime;
	time(&curTime);
	tm* pTm = localtime(&curTime);
	if(pTm)
	{
		sys.wDay = pTm->tm_mday;
		sys.wDayOfWeek= pTm->tm_wday;
		sys.wHour = pTm->tm_hour;
		sys.wMilliseconds = 0;
		sys.wMinute = pTm->tm_min;
		sys.wMonth = pTm->tm_mon;
		sys.wSecond = pTm->tm_sec;
		sys.wYear = pTm->tm_year + 1900;
	}
	
	return sys;
}

int CReaderVCView::GetRotationImpl(FPDF_PAGE page)
{
	return m_nRotateFlag;
}

FPDF_PAGE CReaderVCView::GetPageImpl(FPDF_DOCUMENT document,int nPageIndex)
{
	FPDF_PAGE page = NULL;
	m_pageMap.Lookup(nPageIndex, page);
	if(page)
		return page;
	page = FPDF_LoadPage(document, nPageIndex);
	FORM_OnAfterLoadPage(page, m_pApp);
	m_pageMap.SetAt(nPageIndex, page);
	return page;
}

FPDF_PAGE CReaderVCView::GetCurrentPageImpl(FPDF_DOCUMENT document)
{
	return m_pPage;
}

bool CReaderVCView::IsALTKeyDownImpl()
{
	return GetKeyState(VK_MENU) < 0;
}
bool CReaderVCView::IsINSERTKeyDownImpl()
{
	return GetKeyState(VK_INSERT) & 0x01;
}
bool CReaderVCView::IsSHIFTKeyDownImpl()
{
	return !((GetKeyState(VK_SHIFT)&0x8000) == 0);
}
bool CReaderVCView::IsCTRLKeyDownImpl()
{
	return GetKeyState(VK_CONTROL) < 0;
}

void CReaderVCView::OnChangeImpl()
{

}

CString CReaderVCView::GetFilePath()
{
	CReaderVCDoc* pDoc = GetDocument();
	if(pDoc)
	{
		return pDoc->m_strPDFName;
	}
	return "";
}

BOOL CReaderVCView::SubmitFormImpl(void* pBuffer, int nLength, CString strURL)
{
	CString tempFDFFile = "D://1.fdf";

	if (pBuffer == NULL || nLength <= 0)
	{
		return FALSE;
	}

	CFile file;
	if (file.Open(tempFDFFile, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		file.Write(pBuffer, nLength);
		file.Close();
	}

	return TRUE;
}

int Sample_appResponse(struct _IPDF_JsPlatform* pThis, FPDF_WIDESTRING Question, FPDF_WIDESTRING Title, 
					   FPDF_WIDESTRING Default, FPDF_WIDESTRING cLabel, FPDF_BOOL bPassword, void* response, int length)
{
	CReaderVCView* pView = (CReaderVCView*)pThis->m_pFormfillinfo;
	FPDF_WIDESTRING wsResponse;

	if (pView->m_pwsResponse && response != NULL)
	{
		wsResponse = (FPDF_WIDESTRING)pView->m_pwsResponse;
		length = wcslen((const wchar_t*)wsResponse);
		memcpy(response, wsResponse, length*sizeof(wchar_t));
		pView->m_pwsResponse = NULL;
	}	
	else
	{
		CJS_ResponseDlg dlg;
		dlg.SetTitle(Title);
		dlg.SetDefault(Default);
		dlg.SetLabel(cLabel);
		dlg.SetQuestion(Question);
		dlg.SetIsVisible(bPassword);
		int iRet = dlg.DoModal();		
		
		if (iRet == 1)
		{
			wsResponse = dlg.GetResponse();
			length = wcslen((const wchar_t*)wsResponse);
			pView->m_pwsResponse = new wchar_t[length+1];
			memset(pView->m_pwsResponse, 0, length*sizeof(wchar_t));
			memcpy(pView->m_pwsResponse, wsResponse, length*sizeof(wchar_t));
			pView->m_pwsResponse[length] = L'\0';
		}
	}
	
	return length*sizeof(wchar_t);
}

int Sample_appalert(struct _IPDF_JsPlatform* pThis, FPDF_WIDESTRING Msg, FPDF_WIDESTRING Title, int Type, int Icon)
{
	int nRet = 0;
	if(pThis && pThis->m_pFormfillinfo)
	{
		CReaderVCView* pView = (CReaderVCView*)pThis->m_pFormfillinfo;
		int msgType = MB_OK;
		switch(Type)
		{
	
		case 1:
			msgType = MB_OKCANCEL;
			break;
		case 2:
			msgType = MB_YESNO;
			break;
		case 3:
			msgType = MB_YESNOCANCEL;
			break;	
		case 0:
		default:
			break;
		}
		nRet = MessageBoxW(pView->m_hWnd, (const wchar_t*)Msg, (const wchar_t*)Title, msgType);
		switch(nRet)
		{
		case IDOK:
			return 1;
		case IDCANCEL:
			return 2;
		case IDNO:
			return 3;
		case IDYES:
			return 4;
		}
		return nRet;
	}
	return nRet;
}

void Sample_appbeep(struct _IPDF_JsPlatform* pThis, int nType)
{
	MessageBeep(nType);
	//AfxMessageBox("aaaa");
}


CString userSelFilePath;
int  Sample_fieldBrowse(struct _IPDF_JsPlatform* pThis,void* filePath, int length)
{
	if(userSelFilePath.IsEmpty())
	{
		CFileDialog fd(FALSE, "fdf");
		if(fd.DoModal() == IDOK)
		{
			userSelFilePath = fd.GetPathName();
			
			if(filePath == NULL || length == 0)
				return userSelFilePath.GetLength() + 1;
			else
				return 0;
		}
		else
			return 0;
	}	
	else
	{
		int nLen = userSelFilePath.GetLength()+1;
		if(length > nLen)
			length = nLen;
		memcpy(filePath, userSelFilePath.GetBuffer(length), length);
		userSelFilePath.ReleaseBuffer();
		userSelFilePath = "";
		return length;
	}
}
int Sample_docGetFilePath(struct _IPDF_JsPlatform* pThis, void* filePath, int length)
{
	if(pThis && pThis->m_pFormfillinfo)
	{
		CReaderVCView* pView = (CReaderVCView*)pThis->m_pFormfillinfo;
		CString csFilePath = pView->GetFilePath();

		int nbufflen = csFilePath.GetLength() + 1;
		if(filePath == NULL || length == 0)
			return nbufflen;

		if(length > nbufflen)
			length = nbufflen;
		memcpy(filePath, csFilePath.GetBuffer(length), length);
		csFilePath.ReleaseBuffer();

		return length;

	}
	return 0;
}

void Sample_docSubmitForm(struct _IPDF_JsPlatform* pThis,void* formData, int length, FPDF_WIDESTRING URL)
{
	if(pThis && pThis->m_pFormfillinfo)
	{
		CReaderVCView *pView = (CReaderVCView*)pThis->m_pFormfillinfo;
		if (pView)
		{
			pView->SubmitFormImpl(formData, length, "");
		}
	}	
}

void Sample_gotoPage(struct _IPDF_JsPlatform* pThis, int nPageNum)
{
	if(pThis && pThis->m_pFormfillinfo)
	{
		CReaderVCView *pView = (CReaderVCView*)pThis->m_pFormfillinfo;
		if (pView)
		{
			pView->GotoPage(nPageNum);
		}
	}	
}

CReaderVCView::CReaderVCView()
{
	// TODO: add construction code here
	m_pFram = NULL;
	m_pExportPageDlg = NULL;
	m_pDoc = NULL;
	m_pPage = NULL;
	m_nTotalPage = 0;
	m_nRotateFlag = 0;
	m_dbScaleFactor = 1.0f;
	m_nPageIndex = -1;
	m_dbPageWidth = 0.0f;
	m_dbPageHeight = 0.0f;
	m_nStartX = 0;
	m_nStartY = 0;
	m_nActualSizeX = 0;
	m_nActualSizeY = 0;

	//for search text
	m_pTextPage = NULL;
	m_FindInfo.m_strFind = _T("");
	m_FindInfo.m_nFlag = -1;
	m_FindInfo.m_nDirection = -1;
	m_FindInfo.m_nStartPageIndex = -1;
	m_FindInfo.m_nStartCharIndex = -1;
	m_FindInfo.m_bFirst = TRUE;
	m_FindInfo.m_pCurFindBuf = NULL;
	m_pSCHHandle = NULL;

	m_rtFind = NULL;
	m_nRectNum = 0;
	
	//for select text
	m_bSelect = FALSE;
	m_bHand = TRUE;
	m_bSnap = FALSE;
	m_bHasChar = FALSE;
	
	m_ptLBDown.x = m_ptLBDown.y = 0;
	m_ptLBUp.x = m_ptLBUp.y = 0;
	m_ptOld.x = m_ptOld.y = 0;

	m_nStartIndex = m_nEndIndex = m_nOldIndex = -1;
	m_rtArray.RemoveAll();
	m_rtOld.left = m_rtOld.right = m_rtOld.bottom = m_rtOld.top = 0;

	m_nPosH = m_nPosV = -1;

	// for links
	m_pLink = NULL;
	m_bBookmark = FALSE;

	m_bmp = NULL;
	m_pwsResponse = NULL;

	this->FFI_Invalidate = Sample_Invalidate;
	this->Release= Sample_Release;
	this->FFI_SetTimer = Sample_SetTimer;
	this->FFI_KillTimer = Sample_KillTimer;
	this->FFI_GetLocalTime = Sample_GetLocalTime;
	this->FFI_SetCursor = Sample_SetCursor;
	this->FFI_OnChange = Sample_OnChange;
	this->FFI_GetPage = Sample_GetPage;
	this->FFI_GetCurrentPage = Sample_GetCurrentPage;
	this->FFI_GetRotation = Sample_GetRotation;
	this->FFI_OutputSelectedRect = Sample_OutputSelectedRect;
	this->FFI_ExecuteNamedAction = Sample_ExecuteNamedAction;
	this->FFI_OutputSelectedRect  = NULL;
	this->FFI_SetTextFieldFocus = NULL;
	this->FFI_DoGoToAction = NULL;
	this->FFI_DoURIAction = NULL;
	this->FFI_DisplayCaret = Sample_DisplayCaret;
	this->FFI_GetCurrentPageIndex = Sample_GetCurrentPageIndex;
	this->FFI_SetCurrentPage = Sample_SetCurrentPage;
	this->FFI_GotoURL = Sample_GotoURL;
	this->FFI_GetPageViewRect = Sample_GetPageViewRect;
	this->FFI_PopupMenu = Sample_PopupMenu;
	this->FFI_OpenFile = Sample_OpenFile;
	this->FFI_GetFilePath = Sample_GetFilePath;
	this->FFI_EmailTo = Sample_EmailTo;
	this->FFI_UploadTo = Sample_UploadTo;
	this->FFI_GetPlatform = Sample_GetPlatform;
	this->FFI_GetDocumentCount = Sample_GetDocumentCount;
	this->FFI_GetCurDocumentIndex = Sample_GetCurDocumentIndex;
	this->FFI_DownloadFromURL = Sample_DownloadFromURL;
	this->FFI_PostRequestURL = Sample_PostRequestURL;
	this->FFI_PutRequestURL = Sample_PutRequestURL;
	this->FFI_ShowFileDialog = Sample_ShowFileDialog;	
	this->version = 1;
	
	this->m_pJsPlatform = NULL;
  	this->m_pJsPlatform = new IPDF_JSPLATFORM;
	memset(m_pJsPlatform, 0, sizeof(IPDF_JSPLATFORM));
  	this->m_pJsPlatform->app_alert = Sample_appalert;
	this->m_pJsPlatform->app_response = Sample_appResponse;
 	this->m_pJsPlatform->app_beep = Sample_appbeep;
  	this->m_pJsPlatform->Field_browse =Sample_fieldBrowse;
  	this->m_pJsPlatform->Doc_getFilePath = Sample_docGetFilePath;
	this->m_pJsPlatform->Doc_submitForm = Sample_docSubmitForm;
	this->m_pJsPlatform->Doc_gotoPage = Sample_gotoPage;
  	this->m_pJsPlatform->m_pFormfillinfo = this;

	m_pApp = NULL;
}


CReaderVCView::~CReaderVCView()
{

//	FPDF_DestroyApp(m_App);

	if(m_pTextPage != NULL)
	{
		FPDFText_ClosePage(m_pTextPage);
		m_pTextPage = NULL;
	}
	if (m_pLink != NULL)
	{
		FPDFLink_CloseWebLinks(m_pLink);
		m_pLink = NULL;
	}

	POSITION pos = m_pageMap.GetStartPosition();
	while(pos)
	{
		int nIndex = 0;
		FPDF_PAGE page = NULL;
		m_pageMap.GetNextAssoc(pos, nIndex, page);

		if (page)
		{
			FORM_OnBeforeClosePage(page, m_pApp);
			FPDF_ClosePage(page);
		}
	}
	m_pPage = NULL;

	if (m_pDoc != NULL)
	{
		//Should strictly follow the reverse order of initialization .
		FORM_DoDocumentAAction(m_pApp, FPDFDOC_AACTION_WC);
		if(m_pApp)
			FPDFDOC_ExitFormFillEnviroument(m_pApp);
		FPDF_CloseDocument(m_pDoc);
		m_pDoc = NULL;
	}
	if (m_FindInfo.m_pCurFindBuf != NULL)
	{
		delete []m_FindInfo.m_pCurFindBuf;
		m_FindInfo.m_pCurFindBuf = NULL;
	}
	if (m_rtFind != NULL)
	{
		delete m_rtFind;
		m_rtFind = NULL;
	}
	if(m_bmp != NULL)
	{
		FPDFBitmap_Destroy(m_bmp);
	}

	if (m_pwsResponse)
	{
		delete m_pwsResponse;
		m_pwsResponse = NULL;
	}

	if(this->m_pJsPlatform)
		delete m_pJsPlatform;

	m_mapTimerFuns.RemoveAll();
//	m_formFiledInfo.Release();
//	}
//	m_rtArray.RemoveAll();
}

BOOL CReaderVCView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style |= WS_MAXIMIZE | WS_VISIBLE | WS_VSCROLL |WS_HSCROLL;
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CReaderVCView drawing

void CReaderVCView::OnDraw(CDC* pDC)
{
	CReaderVCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
	DrawPage(m_nRotateFlag, pDC);
	DrawAllRect(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CReaderVCView printing

BOOL CReaderVCView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CReaderVCView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CReaderVCView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CReaderVCView diagnostics

#ifdef _DEBUG
void CReaderVCView::AssertValid() const
{
	CView::AssertValid();
}

void CReaderVCView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CReaderVCDoc* CReaderVCView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CReaderVCDoc)));
	return (CReaderVCDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CReaderVCView message handlers

BOOL CReaderVCView::LoadPDFPage(FPDF_DOCUMENT doc, int nIndex, CPoint pos)
{
	if(NULL == doc) return FALSE;
	if(nIndex < 0) nIndex = 0;
	if(nIndex > m_nTotalPage) nIndex = m_nTotalPage;

	FORM_DoPageAAction(m_pPage, m_pApp, FPDFPAGE_AACTION_CLOSE);

	m_pPage = NULL;
	m_pageMap.Lookup(nIndex, m_pPage);
	if(!m_pPage)
	{
		m_pPage = FPDF_LoadPage(doc, nIndex);
		FORM_OnAfterLoadPage(m_pPage, m_pApp);
		m_pageMap.SetAt(nIndex, m_pPage);	
	}
	if(NULL == m_pPage) return FALSE;

	FORM_DoPageAAction(m_pPage, m_pApp, FPDFPAGE_AACTION_OPEN);

	m_nPageIndex = nIndex;
    SetPageMetrics(m_pPage);

	if (m_pTextPage != NULL)
	{
		FPDFText_ClosePage(m_pTextPage);
		m_pTextPage = NULL;	
	}
	m_pTextPage = FPDFText_LoadPage(m_pPage);


	CChildFrame *pParent = (CChildFrame *)this->GetParentFrame();
	if (pParent != NULL)
	{
		pParent->SetActiveView(this);
		SyncScroll();	
	}
	
	if(pos.x !=0 && pos.y != 0)
	{
		FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(m_nActualSizeX * m_dbScaleFactor),
			(int)(m_nActualSizeY * m_dbScaleFactor), m_nRotateFlag, pos.x, pos.y, &m_nStartX, &m_nStartY);			
		m_nStartX = -m_nStartX + 20;
		m_nStartY = -m_nStartY + 20;
		
		int nSizeX = 0; 
		int nSizeY = 0;
		if (1 == m_nRotateFlag || 3 == m_nRotateFlag)
		{
			nSizeX = m_nActualSizeY;
			nSizeY = m_nActualSizeX;
		}
		else
		{
			nSizeX = m_nActualSizeX;
			nSizeY = m_nActualSizeY;
		}
		SCROLLINFO scrinfo;
		GetScrollInfo(SB_VERT, &scrinfo);                                         
		scrinfo.nMin = 0;                                                         
		scrinfo.nMax =(int) (nSizeY * m_dbScaleFactor + abs(m_nStartY));                                             
		SetScrollInfo(SB_VERT, &scrinfo);  
		SetScrollPos(SB_VERT, abs(m_nStartY), TRUE);

		GetScrollInfo(SB_HORZ, &scrinfo);                                         
		scrinfo.nMin = 0;                                                         
		scrinfo.nMax = (int)(nSizeX * m_dbScaleFactor + abs(m_nStartX));                                             
		SetScrollInfo(SB_HORZ, &scrinfo);  
		SetScrollPos(SB_HORZ, abs(m_nStartX), TRUE);
	}
	this->Invalidate(TRUE);
//	FPDFApp_SetPage(m_App, m_pPage);
	return TRUE;

}


void CReaderVCView::InvalidateImpl(FPDF_PAGE page, double left, double top, double right, double bottom)
{
	int device_left, device_top, device_right, device_bottom;

	int nActualRangeX = 0;
	int nActualRangeY = 0;
	if ( m_nRotateFlag % 2 == 0 )
	{
		nActualRangeX = m_nActualSizeX;
		nActualRangeY = m_nActualSizeY;
	}else{
		nActualRangeX = m_nActualSizeY;
		nActualRangeY = m_nActualSizeX;
	}

	FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
		(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, left, top, &device_left, &device_top);

	FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
		(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, right, bottom, &device_right, &device_bottom);

	CRect rc(device_left,device_top, device_right, device_bottom);
// 	TRACE("left = %d\r\n", device_left);
// 		TRACE("top = %d\r\n", device_top);
// 			TRACE("right = %d\r\n", device_right);
// 				TRACE("bottom = %d\r\n", device_bottom);
	if(device_right-device_left>5)
	TRACE("left=%d,top=%d,right=%d,bottom=%d\r\n",device_left,device_top,device_right,device_bottom);
	::InvalidateRect(m_hWnd, rc, FALSE);
}

void CReaderVCView::SetCaretImpl(FPDF_PAGE page,double page_x, double page_y, int nWidth, int nHeight)
{

}

void CReaderVCView::ReleaseImpl()
{

}
CMap<int, int,TimerCallback, TimerCallback> CReaderVCView::m_mapTimerFuns;
int CReaderVCView::SetTimerImpl(int uElapse, TimerCallback lpTimerFunc)
{
	int nTimeID =  ::SetTimer(NULL, 0, uElapse, TimerProc);	
	m_mapTimerFuns.SetAt(nTimeID, lpTimerFunc);
	return nTimeID;
}

void CReaderVCView::KillTimerImpl(int nID)
{
	::KillTimer(NULL, nID);
	m_mapTimerFuns.RemoveKey(nID);
}

void CReaderVCView::SetCurorImpl(int nCursorType)
{
	HCURSOR hcur = LoadCursor(NULL, IDC_UPARROW);
	switch(nCursorType)
	{
	 	case	FXCT_ARROW:
	 	case	FXCT_NESW:		
	 	case	FXCT_NWSE:		
	 	case	FXCT_VBEAM:		
	 	case	FXCT_HBEAM:		
	 	case    FXCT_HAND:
	 	//	::SetCursor(hcur);
	 	break;
	}
}

void CReaderVCView::PageToDeviceImpl(FPDF_PAGE page,double page_x,double page_y, int* device_x, int* device_y)
{
	int nActualRangeX = 0;
	int nActualRangeY = 0;
	if ( m_nRotateFlag % 2 == 0 )
	{
		nActualRangeX = m_nActualSizeX;
		nActualRangeY = m_nActualSizeY;
	}else{
		nActualRangeX = m_nActualSizeY;
		nActualRangeY = m_nActualSizeX;
	}
 	FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
				(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, page_x, page_y,device_x, device_y);
}

void CReaderVCView::DeviceToPageImpl(FPDF_PAGE page,int device_x, int device_y, double* page_x, double* page_y)
{
	int nActualRangeX = 0;
	int nActualRangeY = 0;
	if ( m_nRotateFlag % 2 == 0 )
	{
		nActualRangeX = m_nActualSizeX;
		nActualRangeY = m_nActualSizeY;
	}else{
		nActualRangeX = m_nActualSizeY;
		nActualRangeY = m_nActualSizeX;
	}
	FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
		(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, device_x, device_y, page_x, page_y);

}

void CReaderVCView::SetPageMetrics(FPDF_PAGE pPage)
{
	m_nStartX = 10;
	m_nStartY = 10;
	if(NULL==pPage) return;
	//get pdf page width an height;
	m_dbPageWidth = FPDF_GetPageWidth(pPage);
	m_dbPageHeight = FPDF_GetPageHeight(pPage);

	CDC *pDC = GetDC();
	int ix, iy;
	ix = pDC->GetDeviceCaps(LOGPIXELSX);
	iy = pDC->GetDeviceCaps(LOGPIXELSY);
	m_nActualSizeX = (int)(m_dbPageWidth / 72 * ix + 0.5f);//convert pdf coordinates to device
	m_nActualSizeY = (int)(m_dbPageHeight / 72 * iy + 0.5f);//convert pdf coordinates to device
	ReleaseDC(pDC);

}

void CReaderVCView::SetScalFactor(double dbScal)
{
	if(dbScal > 64 ) m_dbScaleFactor = 64;
	if( dbScal < 0) m_dbScaleFactor = 0.08f;
	m_dbScaleFactor = dbScal;
}
void CReaderVCView::DrawPage(int nRotate, CDC *pDC)
{
	int nSizeX = m_nActualSizeX;
	int nSizeY = m_nActualSizeY;

	if (1 == nRotate || 3 == nRotate)
	{
		int temp = nSizeX;
		nSizeX = nSizeY;
		nSizeY = temp;
	}
	
	int nShowSizeX = (int)(nSizeX * m_dbScaleFactor + m_nStartX);
	int nShowSizeY = (int)(nSizeY * m_dbScaleFactor + m_nStartY);
	

	CRect rc; 
	pDC->GetClipBox(&rc);
		FPDF_BITMAP bmptemp = FPDFBitmap_Create(rc.Width(), rc.Height(), 0);
		int nClientWidth = FPDFBitmap_GetWidth(bmptemp);
		int nClientHeight = FPDFBitmap_GetHeight(bmptemp);
		
		
		FPDFBitmap_FillRect(bmptemp, 0, 0, nClientWidth, nClientHeight, 255,255,255, 0);
		FPDF_RenderPageBitmap(bmptemp, m_pPage, m_nStartX-rc.left, m_nStartY-rc.top, (int)(nSizeX * m_dbScaleFactor), (int)(nSizeY * m_dbScaleFactor), nRotate, 
			FPDF_LCD_TEXT | FPDF_NO_NATIVETEXT);
		FPDF_FFLDraw(m_pApp, bmptemp, m_pPage, m_nStartX-rc.left, m_nStartY-rc.top, (int)(nSizeX * m_dbScaleFactor), (int)(nSizeY * m_dbScaleFactor), nRotate, 
			FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_NATIVETEXT);
		
		// 	m_pPage2 = FPDF_LoadPage(m_pDoc, 2);
		// 	FPDF_RenderPageBitmap(m_bmp, m_pPage2, m_nStartX+500, m_nStartY, (int)(nSizeX * m_dbScaleFactor), (int)(nSizeY * m_dbScaleFactor), nRotate, 
		// 		FPDF_LCD_TEXT | FPDF_NO_NATIVETEXT);
		// 	FPDF_FFLDraw(m_pApp, m_bmp, m_pPage2, m_nStartX+500, m_nStartY, (int)(nSizeX * m_dbScaleFactor), (int)(nSizeY * m_dbScaleFactor), nRotate, 
		//  		FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_NATIVETEXT);
		
		int t = FPDFBitmap_GetStride(bmptemp);
		int bufsize=FPDFBitmap_GetStride(bmptemp)*nClientHeight;
		void* bmpbuf=FPDFBitmap_GetBuffer(bmptemp);
		CDC MemDC;
		
		CBitmap winbmp;
		MemDC.CreateCompatibleDC(pDC);
		if((HBITMAP)winbmp != NULL)
			winbmp.DeleteObject();
		if(HBITMAP(winbmp) == NULL)
		{
			winbmp.CreateCompatibleBitmap(pDC,nClientWidth,nClientHeight);
			winbmp.SetBitmapBits(bufsize,bmpbuf);
		}
		
		MemDC.SelectObject(&winbmp); 
		
		pDC->BitBlt(rc.left , rc.top , nClientWidth, nClientHeight, &MemDC,0,0,SRCCOPY);
		MemDC.DeleteDC();    
		
		FPDFBitmap_Destroy(bmptemp);
	
	
	int size = m_SelectArray.GetSize();
	for(int i=0; i<size; i++)
	{
		
		
		CRect rc = m_SelectArray.GetAt(i);
		
		CDC memdc;
		CBitmap bmp,*pOldBitmap; 
		memdc.CreateCompatibleDC(pDC); 
		
		bmp.CreateCompatibleBitmap(pDC,rc.Width(),rc.Height());
		
		pOldBitmap = memdc.SelectObject(&bmp);
		
		memdc.FillSolidRect(0,0,rc.Width(),rc.Height(),RGB(0,100,160));
		
		BLENDFUNCTION bf; 
		
		bf.BlendOp = AC_SRC_OVER; 
		
		bf.BlendFlags = 0; 
		
		bf.SourceConstantAlpha = 0x4f; 
		
		bf.AlphaFormat = 0;
		

		
		BOOL ret=AlphaBlend(pDC->GetSafeHdc(),rc.left,rc.top,rc.Width(),rc.Height(),memdc.GetSafeHdc(),0,0,rc.Width(),rc.Height(),bf);
		
		memdc.SelectObject(pOldBitmap);
		memdc.DeleteDC();
		bmp.DeleteObject();
		
	}
	m_SelectArray.RemoveAll();




}

void CReaderVCView::GetNewPageSize(int &nsizeX, int &nsizeY)
{
	int nSizeX = m_nActualSizeX;
	int nSizeY = m_nActualSizeY;
	if (1 == m_nRotateFlag || 3 == m_nRotateFlag)
	{
		int temp = nSizeX;
		nSizeX = nSizeY;
		nSizeY = temp;
	}
	 nsizeX = (int)(nSizeX*m_dbScaleFactor);
	 nsizeY = (int)(nSizeY*m_dbScaleFactor);
}

void CReaderVCView::OnDocFirstpage() 
{

	if(m_nPageIndex == 0) return;
	this->m_nPageIndex = 0;
	this->LoadPDFPage(m_pDoc, 0);
	DeleteAllRect();
}

void CReaderVCView::OnDocGotopage() 
{
	CGotoPageDlg dlg;
	dlg.DoModal();	
}

void CReaderVCView::OnDocLastpage() 
{
// 	FPDF_DOCUMENT doc = FPDF_LoadDocument("d:\\a1.pdf", "");
//     FPDF_PAGE page = FPDF_LoadPage(doc, 0);
// 	FPDF_IMAGEOBJECT imgObject = FPDFPageObj_NewImgeObj(doc);
// 	 long ret = FPDFImageObj_LoadFromFileEx(&page, 1 ,
// 								 imgObject, "E:\\temp\\temp\\k.gif",TRUE
// 								 );
// 	 FPDFImageObj_SetMatrix(imgObject, 240, 0, 0, 160, 1*50, 0 );
// 	 FPDFPage_InsertObject(page, imgObject);
// 	 FPDFPage_GenerateContent(page);
// 	 FPDF_SaveAsFile(doc, "D:\\out.pdf", 0, NULL, 0, NULL, 0);
//  	 FPDF_ClosePage(page);
//  	 FPDF_CloseDocument(doc);
// 	return;


	if(m_nPageIndex == m_nTotalPage -1) return;
	this->m_nPageIndex = m_nTotalPage -1;
	LoadPDFPage(m_pDoc, m_nPageIndex);
	DeleteAllRect();
}

void CReaderVCView::OnDocNextpage() 
{
	m_nPageIndex ++ ;
	m_nPageIndex %= m_nTotalPage;
	LoadPDFPage(m_pDoc, m_nPageIndex);
	DeleteAllRect();
}

void CReaderVCView::OnDocPrepage() 
{
	m_nPageIndex --;
	if(m_nPageIndex < 0) m_nPageIndex = m_nTotalPage-1;
	LoadPDFPage(m_pDoc, m_nPageIndex);
	DeleteAllRect();
}

void CReaderVCView::OnClockwise() 
{
	m_nRotateFlag ++;
	m_nRotateFlag %= 4;
	LoadPDFPage(m_pDoc, m_nPageIndex);
}

void CReaderVCView::OnCounterclockwise() 
{
	m_nRotateFlag --;
	if (m_nRotateFlag < 0) m_nRotateFlag = 3;
	LoadPDFPage(m_pDoc, m_nPageIndex);
}

BOOL CReaderVCView::SetPDFDocument(FPDF_DOCUMENT pDoc, int nPageNum)
{
	if(pDoc == NULL) return FALSE;

	m_pApp = FPDFDOC_InitFormFillEnviroument(pDoc,this);
	FPDF_LoadXFA(pDoc);

	FORM_DoDocumentJSAction(m_pApp);
	FORM_DoDocumentOpenAction(m_pApp);
//	FORM_OnAfterLoadDocument(m_pApp);
	FPDF_SetFormFieldHighlightColor(m_pApp, 0, RGB(0,255, 0));
	FPDF_SetFormFieldHighlightAlpha(m_pApp, 128);

	
	m_pDoc = pDoc;
	m_nTotalPage = nPageNum;
	if(!LoadPDFPage(m_pDoc, 0)) return FALSE;

	return TRUE;
}

void CReaderVCView::GotoPage(int index)
{
	if(index < 0 || index >= m_nTotalPage){MessageBoxA("Invalidate index");}
	if(index == m_nPageIndex) return;
	if(!LoadPDFPage(m_pDoc, index)) return;
	DeleteAllRect();
}

void CReaderVCView::OnViewActualSize() 
{
	m_nStartX = m_nStartY = 10;
	ScalPage(1.0f);
}

void CReaderVCView::OnViewFitPage() 
{
	m_nStartX = m_nStartY = 10;
	CRect rect;
	GetClientRect(rect);
	double dbHeight = rect.Height();	
	double dbScal = dbHeight / m_nActualSizeY;
	ScalPage(dbScal);
}

void CReaderVCView::OnViewFitWidth() 
{
	m_nStartX = m_nStartY = 10;
	CRect rect;
	GetClientRect(rect);
	double dbWidth= rect.Width();
	double dbScal = dbWidth / m_nActualSizeX;
	ScalPage(dbScal);
}

void CReaderVCView::OnViewZoomIn() 
{
	double dbScal = m_dbScaleFactor;
	dbScal += 0.25f;
	if(dbScal > 6400.0f) return;
	ScalPage(dbScal);
	
}

void CReaderVCView::OnViewZoomOut() 
{
	double dbScal = m_dbScaleFactor;
	dbScal -= 0.25f;
	if(dbScal < 0.25f) return;
	ScalPage(dbScal);
}

void CReaderVCView::OnViewZoomTo() 
{
	CZoomDlg dlg;
	dlg.DoModal();
}

void CReaderVCView::ScalPage(double dbScal)
{
	SetScalFactor(dbScal);
	CChildFrame *pParent = (CChildFrame *)this->GetParentFrame();
	if (pParent != NULL)
	{
		pParent->SetActiveView(this);
		SyncScroll();	
	}
	Invalidate(TRUE);
}

void CReaderVCView::OnEditFind() 
{
	if(m_pTextPage == NULL)
	{
		AfxMessageBox("Sorry, the fpdftext.dll may has expired. For keeping on using the dll, please contact sales@foxitsoftware.com.");
		return;
	}
	CFindDlg dlg;
	dlg.DoModal();	
}

void CReaderVCView::FindText(CString strFind, BOOL bCase, BOOL bWholeword, int Direction)
{
	CString str;
	str = m_FindInfo.m_strFind;
	int nFlag = 0;
	if(bCase) { nFlag |= FPDF_MATCHCASE; }
	if(bWholeword) { nFlag |= FPDF_MATCHWHOLEWORD; }

	if(NULL == m_pTextPage) return;

	if (strFind.Compare(str) != 0 || nFlag != m_FindInfo.m_nFlag)//new search
	{
		if (NULL == m_pTextPage) return;
		if (NULL != m_pSCHHandle)
		{
			FPDFText_FindClose(m_pSCHHandle);
			m_pSCHHandle  = NULL;
		}

		int len = MultiByteToWideChar(CP_ACP, 0, strFind.GetBuffer(0), -1, NULL, NULL);
		wchar_t *pBuf = new wchar_t[len];
		memset(pBuf, 0, len*sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, strFind.GetBuffer(0), strFind.GetLength(), pBuf, len);
		pBuf[len-1] = L'\0';
		m_pSCHHandle = FPDFText_FindStart(m_pTextPage, (FPDF_WIDESTRING)pBuf, nFlag, 0);
		if(NULL == m_pSCHHandle) return;
		
        if (m_FindInfo.m_pCurFindBuf != NULL)
		{
			delete []m_FindInfo.m_pCurFindBuf;
			m_FindInfo.m_pCurFindBuf = NULL;
		}
		m_FindInfo.m_pCurFindBuf = new wchar_t[len];
		memset(m_FindInfo.m_pCurFindBuf, 0, len*sizeof(wchar_t));
		memcpy(m_FindInfo.m_pCurFindBuf, pBuf, len*sizeof(wchar_t));

		delete []pBuf;
		

		//save the find info
		m_FindInfo.m_strFind = strFind;
		m_FindInfo.m_nFlag = nFlag;
		m_FindInfo.m_nDirection = Direction;
		m_FindInfo.m_nStartPageIndex = m_nPageIndex;
		m_FindInfo.m_bFirst = TRUE;
		
		
	}
	FindNext(Direction);
}

void CReaderVCView::FindNext(int nDirection)
{
	if(NULL == m_pSCHHandle) return;
	BOOL bResult = FALSE;
	
	if (nDirection != m_FindInfo.m_nDirection)
	{
		m_FindInfo.m_nDirection = nDirection;
		m_FindInfo.m_bFirst = TRUE;
	}

	if (0 == nDirection)// find down
	{
		bResult = FPDFText_FindNext(m_pSCHHandle);
	}
	if (1 == nDirection)
	{
		bResult = FPDFText_FindPrev(m_pSCHHandle);
	}
	
	while(!bResult){

			if (m_rtFind != NULL)
			{
				delete [] m_rtFind;
				m_rtFind = NULL;
			}

			if (0 == nDirection)
			{
				m_nPageIndex ++;
				m_nPageIndex %= m_nTotalPage;
				if(!LoadPDFPage(m_pDoc, m_nPageIndex)) return;
				if (NULL == m_pTextPage) return;
				if (NULL != m_pSCHHandle)
				{
					FPDFText_FindClose(m_pSCHHandle);
					m_pSCHHandle  = NULL;
				}
				m_pSCHHandle = FPDFText_FindStart(m_pTextPage, (FPDF_WIDESTRING)m_FindInfo.m_pCurFindBuf, m_FindInfo.m_nFlag, 0);
				if(NULL == m_pSCHHandle) break;
				bResult = FPDFText_FindNext(m_pSCHHandle);
				if(!bResult && m_nPageIndex == m_FindInfo.m_nStartPageIndex) break;
			} 
			else
			{
				m_nPageIndex --;
				if (m_nPageIndex < 0) {m_nPageIndex = m_nTotalPage - 1;}
				if(!LoadPDFPage(m_pDoc, m_nPageIndex)) return;
				if (NULL == m_pTextPage) return;
				if (NULL != m_pSCHHandle)
				{
					FPDFText_FindClose(m_pSCHHandle);
					m_pSCHHandle  = NULL;
				}
				m_pSCHHandle = FPDFText_FindStart(m_pTextPage, (FPDF_WIDESTRING)m_FindInfo.m_pCurFindBuf, m_FindInfo.m_nFlag, 0);
				if(NULL == m_pSCHHandle) break;
				bResult = FPDFText_FindPrev(m_pSCHHandle);
				if(!bResult && m_nPageIndex == m_FindInfo.m_nStartPageIndex) break;
			}
	}//end while
	
	if(!bResult)//find over
	{
		FPDFText_FindClose(m_pSCHHandle);
		m_pSCHHandle = NULL;

		if (m_rtFind != NULL)
		{
			delete [] m_rtFind;
			m_rtFind = NULL;
		}
		
		m_FindInfo.m_bFirst = TRUE;
		m_FindInfo.m_nDirection = -1;
		m_FindInfo.m_nFlag = -1;
		m_FindInfo.m_nStartCharIndex = -1;
		m_FindInfo.m_nStartPageIndex = -1;
		m_FindInfo.m_pCurFindBuf = NULL;
		m_FindInfo.m_strFind = _T("");

		MessageBox("Find complete!", "Find Infomation", MB_OK | MB_ICONINFORMATION);
		return;
	}
	
	
	
	int index = FPDFText_GetSchResultIndex(m_pSCHHandle);
	if (m_nPageIndex == m_FindInfo.m_nStartPageIndex && index == m_FindInfo.m_nStartCharIndex && !m_FindInfo.m_bFirst )
	{
		if (NULL != m_pSCHHandle)
		{
			FPDFText_FindClose(m_pSCHHandle);
			m_pSCHHandle  = NULL;
		}
		MessageBox("Find complete!", "Find Infomation", MB_OK | MB_ICONINFORMATION);
		return;
	}else{
		CDC *pDC = GetDC();
		DrawAllRect(pDC);//update

		int nCount = FPDFText_GetSchCount(m_pSCHHandle);
		int nRects = FPDFText_CountRects(m_pTextPage, index, nCount);
		if (m_rtFind != NULL)
		{
			delete [] m_rtFind;
			m_rtFind = NULL;
		}
		m_rtFind = new PDFRect[nRects];
		m_nRectNum = nRects;
		for (int i=0; i<nRects; i++)
		{	
			double left, top, right, bottom;	
			FPDFText_GetRect(m_pTextPage, i, &left, &top, &right, &bottom);
			m_rtFind[i].m_dbLeft = left;
			m_rtFind[i].m_dbTop = top;
			m_rtFind[i].m_dbRight = right;
			m_rtFind[i].m_dbBottom = bottom;
		}
		DrawAllRect(pDC);//draw new rect
		ReleaseDC(pDC);
	}

	if (m_FindInfo.m_bFirst)
	{//find first string, store info;
		m_FindInfo.m_bFirst = FALSE;
		m_FindInfo.m_nStartCharIndex = index;
		m_FindInfo.m_nStartPageIndex = m_nPageIndex; 
	}
	
}

void CReaderVCView::DrawReverse(CDC *pDC, CRect rect)
{
	CRect rt = rect;
	rect.left = __min(rt.left, rt.right);
	rect.right = __max(rt.left, rt.right);
	rect.top = __min(rt.top, rt.bottom);
	rect.bottom = __max(rt.top, rt.bottom);

	ASSERT(pDC);
	int bmp_width=abs(rect.Width());
	int bmp_height=abs(rect.Height());	
	HBITMAP hbmp = CreateCompatibleBitmap(pDC->m_hDC, bmp_width, bmp_height);
	CDC MemDC;
	MemDC.CreateCompatibleDC(pDC);
	HBITMAP holdbmp  = (HBITMAP)MemDC.SelectObject(hbmp);
	// copy screen DC to memory DC
	BitBlt(MemDC, 0, 0, bmp_width, bmp_height,	pDC->m_hDC, rect.left,rect.top, SRCCOPY);
	MemDC.SelectObject(holdbmp);
	
	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof bmi);
	bmi.bmiHeader.biSize = sizeof bmi.bmiHeader;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biClrImportant = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biHeight = bmp_height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biWidth = bmp_width;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	
	// get bitmap stream
	int ret = GetDIBits(MemDC, hbmp, 0,bmp_height, NULL, &bmi, DIB_RGB_COLORS);
	
	int size = bmi.bmiHeader.biSizeImage;
	
	BYTE* pBits = new BYTE[size];
	memset(pBits, 0, size);
	ret = GetDIBits(MemDC, hbmp, 0,bmp_height, pBits, &bmi, DIB_RGB_COLORS);
	ret = GetLastError();
	DeleteObject(hbmp);
	MemDC.DeleteDC();
	for (int row = 0; row < bmp_height; row ++) {
		int pitch = (bmp_width * 3 + 3) / 4 * 4;
		int rowpos = row * pitch;
		for(int col = 0; col < bmp_width; col ++){
			int i = rowpos + col * 3;	
			pBits[i]		= 255-pBits[i] ;		
			pBits[i + 1]	= 255-pBits[i + 1];
			pBits[i + 2]	= 255-pBits[i + 2];	
		}
	}
	ret = SetDIBitsToDevice(pDC->m_hDC,rect.left,rect.top,bmp_width, bmp_height,0, 0, 0, bmp_height, pBits, &bmi, DIB_RGB_COLORS);
	delete []pBits;	

}

void CReaderVCView::DrawAllRect(CDC *pDC)
{
	int i;
	int left, top, right, bottom;

	int nSizeX = 0; 
	int nSizeY = 0;
//	int temp = 0;	
	if (1 == m_nRotateFlag || 3 == m_nRotateFlag)
	{
		nSizeX = m_nActualSizeY;
		nSizeY = m_nActualSizeX;
	}
	else
	{
		nSizeX = m_nActualSizeX;
		nSizeY = m_nActualSizeY;
	}
	ASSERT(pDC);
	if (m_rtFind != NULL)
	{
		for (i=0; i<m_nRectNum; i++)
		{
			
			PDFRect rect = m_rtFind[i];
			FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nSizeX * m_dbScaleFactor),
				(int)(nSizeY * m_dbScaleFactor), m_nRotateFlag, rect.m_dbLeft, rect.m_dbTop, &left, &top);
			FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nSizeX * m_dbScaleFactor),
				(int)(nSizeY * m_dbScaleFactor), m_nRotateFlag, rect.m_dbRight, rect.m_dbBottom, &right, &bottom);
			CRect rt(left, top, right, bottom);
			DrawReverse(pDC, rt);

		}
	}

	if (m_rtArray.GetSize() != 0)
	{
		for (i=0; i<m_rtArray.GetSize(); i++)
		{
			PDFRect rect = m_rtArray.GetAt(i);
			FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nSizeX * m_dbScaleFactor),
				(int)(nSizeY * m_dbScaleFactor), m_nRotateFlag, rect.m_dbLeft, rect.m_dbTop, &left, &top);
			FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nSizeX * m_dbScaleFactor),
				(int)(nSizeY * m_dbScaleFactor), m_nRotateFlag, rect.m_dbRight, rect.m_dbBottom, &right, &bottom);
			CRect rt(left, top, right, bottom);
			DrawReverse(pDC, rt);
		}
	}
}

void CReaderVCView::DeleteAllRect()
{
	CDC *pDC = GetDC();
	DrawAllRect(pDC);
	ReleaseDC(pDC);

	if (m_rtFind != NULL)
	{
		delete []m_rtFind;
		m_rtFind = NULL;
	}
	
	if (m_rtArray.GetSize() != 0)
	{
		m_rtArray.RemoveAll();
	}
}

void CReaderVCView::OnFilePrint() 
{
	CString strDoc = GetDocument()->GetTitle(); 
	CPrintDialog dlg(FALSE, PD_PAGENUMS | PD_USEDEVMODECOPIES);
	dlg.m_pd.nMinPage = dlg.m_pd.nFromPage =1;
	dlg.m_pd.nMaxPage = dlg.m_pd.nToPage = m_nTotalPage;
	if (dlg.DoModal() == IDOK)
	{
		int from_page, to_page;
		if (dlg.PrintAll())              
		{
			from_page = dlg.m_pd.nMinPage;
			to_page = dlg.m_pd.nMaxPage;
		}
		else if (dlg.PrintRange())       
		{                                
			from_page = dlg.GetFromPage();
			to_page = dlg.GetToPage();
		}
		else if (dlg.PrintSelection())   
		{
			from_page = to_page = m_nPageIndex + 1;     
		}
		
		HDC printDC;
		DOCINFO docInfo;

		printDC = dlg.CreatePrinterDC();
		if(NULL == printDC) return;
		docInfo.cbSize = sizeof(DOCINFO);
		docInfo.fwType = 0;
		docInfo.lpszDatatype = NULL;
		docInfo.lpszOutput = NULL;
		docInfo.lpszDocName = strDoc;

    	if(StartDoc(printDC, &docInfo) <= 0) return;
		//FPDF_DOCUMENT pDoc = NULL;
		FPDF_PAGE pPage = NULL;
		CString str;
		for (int i=from_page-1; i<to_page; i++)
		{
			if(pPage != NULL)
			{
				FPDF_ClosePage(pPage);
				pPage = NULL;
			}
			pPage = FPDF_LoadPage(m_pDoc, i);
			double npagewidth = FPDF_GetPageWidth(m_pPage);
			double npageheight = FPDF_GetPageHeight(m_pPage);
			
			int logpixelsx,logpixelsy;
			//calculate the page size
			logpixelsx = GetDeviceCaps(printDC,LOGPIXELSX);
			logpixelsy = GetDeviceCaps(printDC,LOGPIXELSY);
			int nsizeX = (int)(npagewidth / 72 *logpixelsx + 0.5f);
			int nsizeY = (int)(npageheight / 72 *logpixelsy + 0.5f);
			
			if(StartPage(printDC) <= 0)
			{	
				str.Format("one error occured when start the page %d", i);
				MessageBox(str);
				return;
			}

			//render to print device
			FPDF_RenderPage(printDC, pPage, m_nStartX, m_nStartY, nsizeX, nsizeY, m_nRotateFlag, 0);
			
			if(EndPage(printDC) <= 0)
			{
				str.Format("one error occured when close the page %d", i);
				MessageBox(str);
				return;
			}

		}//end for
		EndDoc(printDC);
		if(!DeleteDC(printDC))// delete printDC
			MessageBox("can not delete the printer");
	}	
}

void CReaderVCView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	double page_x = 0;
	double page_y = 0;
	DeviceToPage(point, page_x, page_y);

	if(m_pApp)
	{
		FPDF_BOOL b=  FORM_OnLButtonDown(m_pApp, m_pPage,ComposeFlag(),page_x, page_y);
		if(b)
			return;
	}
	DeleteAllRect();
	LoadMyCursor(1);
	m_ptLBDown = point;


	if (m_bSelect)
	{
		m_bHasChar = GetCharIndexByPoint(point, m_nStartIndex);
		CreateCaret(point);
	}
	if (m_bHand)
	{	
		m_nPosH = GetScrollPos(SB_HORZ);//SetScrollPos(SB_VERT, point.y, TRUE);
		m_nPosV = GetScrollPos(SB_VERT);
	}
	CView::OnLButtonDown(nFlags, point);
}

void CReaderVCView::OnLButtonUp(UINT nFlags, CPoint point) 
{

	// TODO: Add your message handler code here and/or call default
	double page_x = 0;
	double page_y = 0;
	DeviceToPage(point, page_x, page_y);

	if(m_pApp)
	{
		if(FORM_OnLButtonUp(m_pApp, m_pPage,ComposeFlag(), page_x, page_y))
			return;
	}
	LoadMyCursor();
	m_ptLBUp = point;
	if (m_bSelect || m_bSnap)
	{
		CDC *pDC  = GetDC();
		CPen *pOldPen;
		CPen pen;
		CBrush *pOldBr;
		CRect rect;
		pen.CreatePen(PS_DASH, 2, RGB(255,0,0));
		pOldPen = pDC->SelectObject(&pen);
		pOldBr = (CBrush*) (pDC->SelectStockObject(NULL_BRUSH));
		int nOldRop = pDC->SetROP2(R2_XORPEN);
		pDC->Rectangle(m_rtOld);
		pDC->SetROP2(nOldRop);
		m_rtOld.left = m_rtOld.top = m_rtOld.right = m_rtOld.bottom = 0;
		ReleaseDC(pDC);
	}
	
	if (m_bSnap)
	{	
		BOOL open = OpenClipboard();
		if (open)
		{
			::EmptyClipboard();
			int bmpWidth = abs(point.x - m_ptLBDown.x);
			int bmpHeight = abs(point.y - m_ptLBDown.y);
			if (bmpHeight == 0 || bmpWidth == 0)return;
			CRect bmpRect(m_ptLBDown, point);
			CClientDC dc(this);
			CBitmap *pBitmap = new CBitmap();
			pBitmap->CreateCompatibleBitmap(&dc,bmpWidth,bmpHeight);
			CDC memDC;
			memDC.CreateCompatibleDC(&dc);
			memDC.SelectObject(pBitmap);
			CBrush whiteBrush(RGB(255,255,255));
			//memDC.FillRect(bmpRect,&whiteBrush);
			memDC.BitBlt(0, 0, bmpRect.Width(), bmpRect.Height(),&dc, bmpRect.left, bmpRect.top,SRCCOPY);
			HBITMAP hBitmap = (HBITMAP) *pBitmap;
			:: SetClipboardData(CF_BITMAP, hBitmap);
			::CloseClipboard();
			MessageBox("The selected area has been copied to clipboard!");
			delete pBitmap;
		}
	}
	
	FPDF_LINK hlink=NULL;
	double pdfx,pdfy;

	int nActualRangeX = 0;
	int nActualRangeY = 0;
	if ( m_nRotateFlag % 2 == 0 )
	{
		nActualRangeX = m_nActualSizeX;
		nActualRangeY = m_nActualSizeY;
	}else{
		nActualRangeX = m_nActualSizeY;
		nActualRangeY = m_nActualSizeX;
	}

	FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
		(int)(nActualRangeY * m_dbScaleFactor),m_nRotateFlag,point.x,point.y,&pdfx,&pdfy);
	hlink=FPDFLink_GetLinkAtPoint(m_pPage,pdfx,pdfy);
	if (hlink)
	{
		FPDF_ACTION haction=NULL;
		FPDF_DEST hLinkDest=NULL;
		haction=FPDFLink_GetAction(hlink);
		hLinkDest=FPDFLink_GetDest(m_pDoc,hlink);
		if (haction)
		{
			int nActionType=FPDFAction_GetType(haction);
			switch(nActionType)
			{
			case PDFACTION_UNSUPPORTED: // Unsupported action type
				break;
			case PDFACTION_GOTO:		// Go to a destination within current document	
				{
					FPDF_LINK hActLinkDest =  FPDFAction_GetDest(m_pDoc,haction);
					if (hActLinkDest)
					{
						int nPageIndex=FPDFDest_GetPageIndex(m_pDoc,hActLinkDest);
						GotoPage(nPageIndex);
					}
				}
				break;
			case PDFACTION_REMOTEGOTO:	// Go to a destination within another document
				
				//	FPDFAction_GetFilePath(...);
				//	.....	
				break;
			case PDFACTION_URI:		// Universal Resource Identifier, including web pages and 
				// other Internet based resources
				//	int nsize= FPDFAction_GetURIPath(m_pDoc,haction);
				//	...
				break;
			case PDFACTION_LAUNCH:			// Launch an application or open a file
				//	FPDFAction_GetFilePath(...)
				break;
			default :
				break;
			}
		}
		else if (hLinkDest)
		{
			int nPageIndex=FPDFDest_GetPageIndex(m_pDoc,hLinkDest);
			GotoPage(nPageIndex);
		}
	}
	CView::OnLButtonUp(nFlags, point);
}

void CReaderVCView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	double page_x = 0;
	double page_y = 0;
	DeviceToPage(point, page_x, page_y);

	if(m_pApp)
	{
		if(FORM_OnMouseMove(m_pApp, m_pPage,ComposeFlag(), page_x, page_y))
			return;
	}
	if (nFlags == MK_LBUTTON)
	{
		LoadMyCursor(1);
	}else{LoadMyCursor();}
	
	CDC *pDC = GetDC();

	if (m_bSelect && nFlags == MK_LBUTTON)
	{
		if (m_bHasChar)
		{
			if (m_ptOld.x == 0 && m_ptOld.y == 0){m_ptOld = m_ptLBDown;}
	
			int nIndex = 0;
			if (!GetCharIndexByPoint(point, nIndex)) return;
			if(nIndex == m_nEndIndex) return;
			
			DrawAllRect(pDC);//update all rect
			
			m_nEndIndex = nIndex;
			GetRects(m_nStartIndex, m_nEndIndex);	//get the rect and store into m_rtArray
			DrawAllRect(pDC);//draw new rect	
		}
		else 
		{
			CPen *pOldPen;
			CPen pen;
			CBrush *pOldBr;
			CRect rect;
			pen.CreatePen(PS_DASH, 2, RGB(255,0,0));
			pOldPen = pDC->SelectObject(&pen);
			pOldBr = (CBrush*) (pDC->SelectStockObject(NULL_BRUSH));
			int nOldRop = pDC->SetROP2(R2_XORPEN);
			int nModle = pDC->Rectangle(m_rtOld);
		
			
			DrawAllRect(pDC);//update all rect
			rect = SelectSegment(m_ptLBDown, point);
			DrawAllRect(pDC);

			pDC->Rectangle(rect);
			pDC->SelectObject(pOldBr);
			pDC->SelectObject(pOldPen);
			pDC->SetROP2(nModle);
			m_rtOld = rect;	
		}
	}

	if (m_bHand && nFlags == MK_LBUTTON)
	{
		int curPos, prevPos;
		//CChildFrame *pFrame = (CChildFrame *) GetParentFrame();
		
		int dy = m_ptLBDown.y - point.y;
		prevPos = m_nPosV;
		curPos = prevPos + dy;
		prevPos = SetScrollPos(SB_VERT, curPos, TRUE);
		curPos = GetScrollPos(SB_VERT);
		int distance;
		distance = prevPos - curPos;
		m_nStartY += distance;
		ScrollWindow(0, distance);
	
		dy = m_ptLBDown.x - point.x;
		prevPos = m_nPosH;
		curPos = prevPos + dy;
		prevPos = SetScrollPos(SB_HORZ, curPos, TRUE);
		curPos = GetScrollPos(SB_HORZ);
		distance = prevPos - curPos;
		m_nStartX += distance;
		ScrollWindow(distance, 0);

	}
	if (m_bSnap && nFlags == MK_LBUTTON)
	{
		CPen *pOldPen;
		CPen pen;
		CBrush *pOldBr;
		CRect rect(m_ptLBDown, point);
		pen.CreatePen(PS_DASH, 2, RGB(255,0,0));
		pOldPen = pDC->SelectObject(&pen);
		pOldBr = (CBrush*) (pDC->SelectStockObject(NULL_BRUSH));
		int nOldRop = pDC->SetROP2(R2_XORPEN);
		int nModle = pDC->Rectangle(m_rtOld);
		
		pDC->Rectangle(rect);
		pDC->SelectObject(pOldBr);
		pDC->SelectObject(pOldPen);
		pDC->SetROP2(nModle);
		m_rtOld = rect;
		
	}
	
	FPDF_LINK hlink=NULL;
	double pdfx,pdfy;

	int nActualRangeX = 0;
	int nActualRangeY = 0;
	if ( m_nRotateFlag % 2 == 0 )
	{
		nActualRangeX = m_nActualSizeX;
		nActualRangeY = m_nActualSizeY;
	}else{
		nActualRangeX = m_nActualSizeY;
		nActualRangeY = m_nActualSizeX;
	}

	FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
		(int)(nActualRangeY * m_dbScaleFactor),m_nRotateFlag,point.x,point.y,&pdfx,&pdfy);
	hlink=FPDFLink_GetLinkAtPoint(m_pPage,pdfx,pdfy);
	if (hlink)
	{
		HCURSOR hCur = AfxGetApp()->LoadCursor(IDC_CURSOR4);
	    ::SetCursor(hCur);
	}
	ReleaseDC(pDC);
// 		
 	CView::OnMouseMove(nFlags, point);
}

void CReaderVCView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
// 	double page_x = 0;
// 	double page_y = 0;
// 	DeviceToPage(point, page_x, page_y);

	if(m_pApp)
	{
		if(FORM_OnKeyDown(m_pApp, m_pPage, nChar, ComposeFlag()))
			return;
	}
	switch(nChar)
	{
	case 35:
		OnDocLastpage();	
		break;
	case 36:
		OnDocFirstpage();
		break;
	case 37:
		this->OnDocPrepage();
		break;
	case 39:
		this->OnDocNextpage();
		break;
	case 38:
		
		break;
	case 40:
		
		break;
	default:
		break;
	}
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CReaderVCView::OnToolSnapshot() 
{
	m_bSnap = TRUE;
	m_bHand = FALSE;
	m_bSelect = FALSE;
	this->HideCaret();
}

void CReaderVCView::OnToolSelect() 
{
	if(m_pTextPage == NULL)
	{
		AfxMessageBox("Sorry, the fpdftext.dll may has expired. For keeping on using the dll, please contact sales@foxitsoftware.com.");
		return;
	}
	m_bSnap = FALSE;
	m_bHand = FALSE;
	m_bSelect = TRUE;
}

void CReaderVCView::OnToolHand() 
{
	m_bSnap = FALSE;
	m_bHand = TRUE;
	m_bSelect = FALSE;
	this->HideCaret();
}

void CReaderVCView::LoadMyCursor(int nflag)
{
	HCURSOR hCur;
	if (nflag == 1)
	{
		if (m_bSelect)
		{
			hCur = AfxGetApp()->LoadCursor(IDC_CURSOR3);
		}
		else if (m_bSnap)
		{
			hCur = AfxGetApp()->LoadStandardCursor(IDC_CROSS);
		}
		else
		{
			hCur = AfxGetApp()->LoadCursor(IDC_CURSOR1);
		}
	}
	else if (m_bHand)
	{
		hCur = AfxGetApp()->LoadCursor(IDC_CURSOR2);
	}
	else if (m_bSelect)
	{
		hCur = AfxGetApp()->LoadCursor(IDC_CURSOR3);
	}
	else if (m_bSnap)
	{
		hCur = AfxGetApp()->LoadStandardCursor(IDC_CROSS);
	}else 
	{
		hCur = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	}
	::SetCursor(hCur);
}

BOOL CReaderVCView::DeviceToPage(CPoint pt, double& page_x, double& page_y)
{
//	if(NULL == m_pTextPage) return FALSE;
	int nActualRangeX = 0;
	int nActualRangeY = 0;
	if ( m_nRotateFlag % 2 == 0 )
	{
		nActualRangeX = m_nActualSizeX;
		nActualRangeY = m_nActualSizeY;
	}else{
		nActualRangeX = m_nActualSizeY;
		nActualRangeY = m_nActualSizeX;
	}
// 	switch ( m_nRotateFlag % 4 )
// 	{
// 	case 0:
// 	case 2:
// 		{
// 			FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, m_nActualSizeX * m_dbScaleFactor,
// 			m_nActualSizeY * m_dbScaleFactor, m_nRotateFlag, pt.x, pt.y, &page_x, &page_y);
// 			break;
// 		}
// 	case 1:
// 	case 3:
// 		{
// 			FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, m_nActualSizeY * m_dbScaleFactor,
// 				m_nActualSizeX * m_dbScaleFactor, m_nRotateFlag, pt.x, pt.y, &page_x, &page_y);
// 			break;
// 		}
// 	}

 	FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
 		(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, pt.x, pt.y, &page_x, &page_y);
	return TRUE;
}

BOOL CReaderVCView::PageToDevice(double page_x, double page_y, CPoint& pt)
{
	int nActualRangeX = 0;
	int nActualRangeY = 0;
	if ( m_nRotateFlag % 2 == 0 )
	{
		nActualRangeX = m_nActualSizeX;
		nActualRangeY = m_nActualSizeY;
	}else{
		nActualRangeX = m_nActualSizeY;
		nActualRangeY = m_nActualSizeX;
	}

	FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
		(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, page_x, page_y, (int*)&pt.x, (int*)&pt.y);
	return TRUE;
}

BOOL CReaderVCView::GetCharIndexByPoint(CPoint pt, int &nIndex)
{
	double page_x = 0.0f;
	double page_y = 0.0f;

	if(NULL == m_pTextPage) return FALSE;
	int nActualRangeX = 0;
	int nActualRangeY = 0;
	if ( m_nRotateFlag % 2 == 0 )
	{
		nActualRangeX = m_nActualSizeX;
		nActualRangeY = m_nActualSizeY;
	}else{
		nActualRangeX = m_nActualSizeY;
		nActualRangeY = m_nActualSizeX;
	}
	FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
 		(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, pt.x, pt.y, &page_x, &page_y);
// 	FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, m_nActualSizeX * m_dbScaleFactor,
// 		m_nActualSizeY * m_dbScaleFactor, m_nRotateFlag, pt.x, pt.y, &page_x, &page_y);
	
	nIndex = FPDFText_GetCharIndexAtPos(m_pTextPage, page_x, page_y, 10, 10);
	if (-3 == nIndex || -1 == nIndex) return FALSE;
	return TRUE;
}

void CReaderVCView::CreateCaret(CPoint pt)
{
	double a, b, c, d;
	int nIndex = 0;
	int nAcent = 0, nDecent = 0;
	double orgx = 0.0f, orgy = 0.0f;
	double fontsize = 0.0f;
	int left = 0, right = 0;
	int top = 0, bottom = 0;
	//double left_char, right_char, top_char, bottom_char;//char bounding box	
	FPDF_FONT pfont = NULL;

	if(!GetCharIndexByPoint(pt, nIndex)) return;
	fontsize = FPDFText_GetFontSize(m_pTextPage, nIndex);
// 	FPDFText_GetOrigin(m_pTextPage, nIndex, &orgx, &orgy);
// 	pfont = FPDFText_GetFont(m_pTextPage, nIndex);
// 	if(NULL == pfont) return;
// 	nAcent = FPDFFont_GetAscent(pfont);
// 	nDecent = FPDFFont_GetDescent(pfont);
// 	FPDFText_GetMatrix(m_pTextPage, nIndex, &a, &b, &c, &d);
// 	nAcent =(int)((nAcent * fontsize / 1000.0f) * d + orgy + 0.5f);
// 	nDecent = (int)((nDecent * fontsize / 1000.0f) * d + orgy + 0.5f);
// 
// 	int nActualRangeX = 0;
// 	int nActualRangeY = 0;
// 	if ( m_nRotateFlag % 2 == 0 )
// 	{
// 		nActualRangeX = m_nActualSizeX;
// 		nActualRangeY = m_nActualSizeY;
// 	}else{
// 		nActualRangeX = m_nActualSizeY;
// 		nActualRangeY = m_nActualSizeX;
// 	}
// 	FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
//  		(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, orgx, nAcent, &left, &top);
// 
// 	FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
//  		(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, orgx, nDecent, &right, &bottom);
// 
// // 	FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, m_nActualSizeX * m_dbScaleFactor,
// // 		m_nActualSizeY * m_dbScaleFactor, m_nRotateFlag, orgx, nAcent, &left, &top);
// // 
// // 	FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, m_nActualSizeX * m_dbScaleFactor,
// // 		m_nActualSizeY * m_dbScaleFactor, m_nRotateFlag, orgx, nDecent, &right, &bottom);
// 
// 	this->CreateSolidCaret(2, abs(top - bottom));
// 	
// /*	FPDFText_GetCharBox(m_pTextPage, nIndex, &left_char, &right_char, &bottom_char, &top_char);
// 
// 	FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, m_nActualSizeX * m_dbScaleFactor,
// 		m_nActualSizeY * m_dbScaleFactor, m_nRotateFlag, left_char, top_char, &left, &top);
// 	FPDF_PageToDevice(m_pPage, m_nStartX, m_nStartY, m_nActualSizeX * m_dbScaleFactor,
// 		m_nActualSizeY * m_dbScaleFactor, m_nRotateFlag, right_char, bottom_char, &right, &bottom);*/
// 	this->SetCaretPos(CPoint(left, top-2));
// 	this->ShowCaret();
	

}

void CReaderVCView::GetRects(int nStart, int nEnd)
{
	int temp, nCount;
	nCount = nEnd - nStart;
	if (nCount < 0)
	{
		temp = nEnd;
		nEnd = nStart;
		nStart = temp;

		nCount = abs(nCount);
	}
	 nCount ++;

	 int num = FPDFText_CountRects(m_pTextPage, nStart, nCount);
	 if(num == 0) return;
	 
	 if (m_rtArray.GetSize() > 0)
	 {
		 m_rtArray.RemoveAll();
	 }
	 PDFRect rect;
	 for (int i=0; i<num; i++)
	 {
		 FPDFText_GetRect(m_pTextPage, i, &rect.m_dbLeft, &rect.m_dbTop, &rect.m_dbRight, &rect.m_dbBottom);
		 m_rtArray.Add(rect);
	 }
	return;
}

CRect CReaderVCView::SelectSegment(CPoint pt_lt, CPoint pt_rb)
{
	CRect rect(pt_lt, pt_rb);
// 	double left, top, right, bottom;
// 	int start_index = 0, nCount = 0;
// 	int nRect = 0;
// 
// 	int nActualRangeX = 0;
// 	int nActualRangeY = 0;
// 	if ( m_nRotateFlag % 2 == 0 )
// 	{
// 		nActualRangeX = m_nActualSizeX;
// 		nActualRangeY = m_nActualSizeY;
// 	}else{
// 		nActualRangeX = m_nActualSizeY;
// 		nActualRangeY = m_nActualSizeX;
// 	}
// 	FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
// 		(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, rect.left, rect.top, &left, &top);
// 	FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, (int)(nActualRangeX * m_dbScaleFactor),
// 		(int)(nActualRangeY * m_dbScaleFactor), m_nRotateFlag, rect.right, rect.bottom, &right, &bottom);
// 
// // 	FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, m_nActualSizeX * m_dbScaleFactor,
// // 		m_nActualSizeY * m_dbScaleFactor, m_nRotateFlag, rect.left, rect.top, &left, &top);
// // 	FPDF_DeviceToPage(m_pPage, m_nStartX, m_nStartY, m_nActualSizeX * m_dbScaleFactor,
// // 		m_nActualSizeY * m_dbScaleFactor, m_nRotateFlag, rect.right, rect.bottom, &right, &bottom);
// 
// 	int num = FPDFText_CountBoundedSegments(m_pTextPage, left, top, right, bottom);
// 
// 	if (m_rtArray.GetSize() > 0)
// 	{
// 		m_rtArray.RemoveAll();
// 	}
// 
// 	CDC *pDC = GetDC();
// 	for (int i=0; i<num; i++)
// 	{
// 		FPDFText_GetBoundedSegment(m_pTextPage, i, &start_index, &nCount);
// 		nRect = FPDFText_CountRects(m_pTextPage, start_index, nCount);
// 
// 		PDFRect rect_select;
// 		for (int j=0; j<num; j++)
// 		{
// 			FPDFText_GetRect(m_pTextPage, j, &rect_select.m_dbLeft, &rect_select.m_dbTop, &rect_select.m_dbRight, &rect_select.m_dbBottom);
// 			m_rtArray.Add(rect_select);
// 		}
// 	}
// 	ReleaseDC(pDC);
	return rect;
}

void CReaderVCView::OnToolPdf2txt() 
{
	// TODO: Add your command handler code here
	if(m_pTextPage == NULL)
	{
		AfxMessageBox("Sorry, the fpdftext.dll may has expired. For keeping on using the dll, please contact sales@foxitsoftware.com.");
		return;
	}
	CConvertDlg condlg;
	if(condlg.DoModal() == IDOK)
	{
		int nFlag=condlg.m_nFlag;
		CReaderVCDoc *pDoc = this->GetDocument();
		CString pdfname=pDoc->m_strPDFName;
		CString strname=pdfname,stem;
		if (strname.Find(".pdf") != -1 || strname.Find(".PDF") != -1)
		{
			int startatr=strname.ReverseFind('\\');
			stem=strname.Mid(startatr+1,strname.GetLength()-startatr-5);
		}
		CString defaultname=stem+".txt";

		char szFilter[] = "Text File(*.txt)|*.txt";
		CFileDialog dlg(FALSE, ".txt", defaultname, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL);

		if (dlg.DoModal() == IDOK)
		{
// 			CString txtname=dlg.GetPathName();
// 			BOOL bFlag=FPDFText_PDFToText(pdfname,txtname,nFlag,NULL);
// 			if(! bFlag) 
// 				MessageBox("Convert Failure!");
		}
	}	
}

void CReaderVCView::SyncScroll()
{
	CRect rect;                                                               
	GetClientRect(rect);                                                      
	
	int nSizeX = 0; 
	int nSizeY = 0;
	//int temp = 0;

	if (1 == m_nRotateFlag || 3 == m_nRotateFlag)
	{
		nSizeX = m_nActualSizeY;
		nSizeY = m_nActualSizeX;
	}
	else
	{
		nSizeX = m_nActualSizeX;
		nSizeY = m_nActualSizeY;
	}
	
	SCROLLINFO scrinfo;                                                       
	scrinfo.cbSize = sizeof(SCROLLINFO);                                      
	GetScrollInfo(SB_HORZ, &scrinfo);                                         
	scrinfo.nMin =0;                                                          
	scrinfo.nMax = (int)(nSizeX * m_dbScaleFactor);                                                 
	scrinfo.nPage = (unsigned int)(__min(nSizeX * m_dbScaleFactor, rect.Width()));                              
	if (nSizeX * m_dbScaleFactor < rect.Width()){scrinfo.fMask |= SIF_DISABLENOSCROLL;}         
	SetScrollInfo(SB_HORZ, &scrinfo);                                         
	SetScrollPos(SB_HORZ, 0);                                           
	//m_nPosH = nPosH;                                                          
	
	GetScrollInfo(SB_VERT, &scrinfo);                                         
	scrinfo.nMin = 0;                                                         
	scrinfo.nMax = (int)(nSizeY * m_dbScaleFactor);                                             
	scrinfo.nPage =(unsigned int)( __min(nSizeY * m_dbScaleFactor, rect.Height()));                             
	if (nSizeY * m_dbScaleFactor < rect.Height()){scrinfo.fMask |= SIF_DISABLENOSCROLL;}        
	SetScrollInfo(SB_VERT, &scrinfo);                                         
	SetScrollPos(SB_VERT, 0);                                             
	                                                     
}

void CReaderVCView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	SyncScroll();
	if(m_bmp)
	{
		FPDFBitmap_Destroy(m_bmp);
		m_bmp = NULL;
	}

	m_bmp = FPDFBitmap_Create(cx, cy, 0);

	// TODO: Add your message handler code here	
}

void CReaderVCView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	int prevPos = GetScrollPos(SB_HORZ);                               
	int curPos = prevPos;                                              
	int nMin, nMax, nPage;                                             
	GetScrollRange(SB_HORZ, &nMin, &nMax);                             
	SCROLLINFO si;                                                     
	GetScrollInfo(SB_HORZ, &si);                                       
	nPage = si.nPage;                                                  
	
	switch(nSBCode)                                                    
	{                                                                  
	case SB_TOP:                                                       
		curPos = nMin;                                                   
		break;                                                           
	case SB_BOTTOM:                                                    
		curPos = nMax;                                                   
		break;                                                           
	case SB_LINEUP:                                                    
		curPos --;                                                       
		break;                                                         
	case SB_LINEDOWN:                                                  
		curPos ++;                                                       
		break;                                                         
	case SB_ENDSCROLL:                                                 
		return;                                                          
	case SB_PAGEDOWN:                                                  
		curPos += nPage;                                                 
		break;                                                           
	case SB_PAGEUP:                                                    
		curPos -= nPage;                                                 
		break;                                                         
	case SB_THUMBPOSITION:                                             
		curPos = si.nTrackPos;                                           
		break;                                                         
	case SB_THUMBTRACK:                                                
		curPos = si.nTrackPos;                                           
		break;                                                         
	}                                                                  
	
	if (curPos < nMin) { curPos = nMin;}                               
	if(curPos > nMax){ curPos = nMax;}                                 
	SetScrollPos(SB_HORZ, curPos);                                     

	int distance;                                                      
	distance = prevPos - curPos;                                                                     
	m_nStartX += distance;                                                                                
	ScrollWindow(distance, 0);                                  
	CRect rect;
	GetClientRect(rect);
	CRect rtnew(rect.left, rect.top, rect.left+distance, rect.bottom);
	InvalidateRect(&rtnew);
	
	CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CReaderVCView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default                                   
	int prevPos = GetScrollPos(SB_VERT);                                        
	int curPos = prevPos;                                                       
	int nMin, nMax, nPage;                                                      
	GetScrollRange(SB_VERT, &nMin, &nMax);                                      
	SCROLLINFO si;                                                              
	GetScrollInfo(SB_VERT, &si);                                                
	nPage = si.nPage;                                                           
                                                                            
	switch(nSBCode)                                                             
	{                                                                           
	case SB_TOP:                                                                
		curPos = nMin;                                                            
		break;                                                                    
	case SB_BOTTOM:                                                             
		curPos = nMax;                                                            
		break;                                                                    
	case SB_LINEUP:                                                             
		curPos --;                                                                
		break;                                                                  
	case SB_LINEDOWN:                                                           
		curPos ++;                                                                
		break;                                                                  
	case SB_ENDSCROLL:                                                          
		return;                                                                   
	case SB_PAGEDOWN:                                                           
		curPos += nPage;                                                          
		break;                                                                    
	case SB_PAGEUP:                                                             
		curPos -= nPage;                                                          
		break;                                                                  
	case SB_THUMBPOSITION:                                                      
		curPos = si.nTrackPos;                                                    
		break;                                                                  
	case SB_THUMBTRACK:                                                         
		curPos = si.nTrackPos;                                                    
		break;                                                                  
	}                                                                           
                                                                            
	if (curPos < nMin) { curPos = nMin;}                                        
	if(curPos > nMax){ curPos = nMax;}                                          
	SetScrollPos(SB_VERT, curPos);                                                                                                      
                                                                            
	int distance;                                                                                                        
	distance = prevPos - curPos;                                                
	m_nStartY += distance;                                                                                                  
	ScrollWindow(0, distance);                                           

	CRect rect;
	GetClientRect(rect);
	CRect rtnew(rect.left, rect.bottom + distance, rect.right, rect.bottom);
	CRect rtnew2(rect.left, rect.top, rect.right, rect.top + distance);
	InvalidateRect(&rtnew);
	InvalidateRect(&rtnew2);

	CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CReaderVCView::OnToolExtractlinks() 
{
	if(m_pTextPage == NULL)
	{
		AfxMessageBox("Sorry, the fpdftext.dll may has expired. For keeping on using the dll, please contact sales@foxitsoftware.com.");
		return;
	}
	CString strLink = _T("");
	wchar_t *pBuf = NULL;
	if(m_pLink != NULL)
	{
		FPDFLink_CloseWebLinks(m_pLink);
		m_pLink = NULL;
	}
	m_pLink = FPDFLink_LoadWebLinks(m_pTextPage);
	if(m_pLink == NULL) return;

	int nCount = FPDFLink_CountWebLinks(m_pLink);
	if (nCount == 0) return;
	if (m_rtArray.GetSize()!=0)
	{
		m_rtArray.RemoveAll();
	}
	for (int i=0; i<nCount; i++)
	{
		int nlen = FPDFLink_GetURL(m_pLink, i, NULL, 0)+1;
		pBuf = new wchar_t[nlen];
		memset(pBuf, 0, nlen*sizeof(wchar_t));
		FPDFLink_GetURL(m_pLink, i, (unsigned short*)pBuf, nlen);
		pBuf[nlen-1] = L'\0';
		int n = WideCharToMultiByte(CP_ACP, 0, pBuf, -1, NULL, NULL, NULL, NULL);
		char *p = new char[n];
		memset(p, 0, n);
		WideCharToMultiByte(CP_ACP, 0, pBuf, nlen, p, n, NULL, NULL);
		p[n-1] = '\0';
		strLink += p;
		strLink += "\r\n";
		delete []pBuf;
		delete []p;
		int nRects = FPDFLink_CountRects(m_pLink, i);
		for (int j=0; j<nRects; j++)
		{
			PDFRect rect;
			FPDFLink_GetRect(m_pLink, i, j, 
				&rect.m_dbLeft, &rect.m_dbTop, &rect.m_dbRight, &rect.m_dbBottom);
			m_rtArray.Add(rect);
		}
	}
	CDC *pDC = GetDC();
	DrawAllRect(pDC);
	ReleaseDC(pDC);

	MessageBox(strLink, "Web Links in this page");
	
}

void CReaderVCView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
}

void CReaderVCView::OnDestroy() 
{
	CView::OnDestroy();
	// TODO: Add your message handler code here
}

void CReaderVCView::OnUpdateDocFirstpage(CCmdUI* pCmdUI) 
{
	if (0 == m_nPageIndex)
	{
		pCmdUI->Enable(FALSE);
	} 
	else
	{
		pCmdUI->Enable(TRUE);
	}
	
}

void CReaderVCView::OnUpdateDocLastpage(CCmdUI* pCmdUI) 
{
	if (m_nPageIndex == m_nTotalPage - 1)
	{
		pCmdUI->Enable(FALSE);
	} 
	else
	{
		pCmdUI->Enable(TRUE);
	}
	
}

void CReaderVCView::OnUpdateDocNextpage(CCmdUI* pCmdUI) 
{
	if (m_nPageIndex == m_nTotalPage - 1)
	{
		pCmdUI->Enable(FALSE);
	} 
	else
	{
		pCmdUI->Enable(TRUE);
	}
	
}

void CReaderVCView::OnUpdateDocPrepage(CCmdUI* pCmdUI) 
{
	if (0 == m_nPageIndex)
	{
		pCmdUI->Enable(FALSE);
	} 
	else
	{
		pCmdUI->Enable(TRUE);
	}
	
}

void CReaderVCView::OnUpdateToolHand(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (m_bHand)
	{
		pCmdUI->SetCheck(1);
	} 
	else
	{
		pCmdUI->SetCheck(0);
	}
	
	
}

void CReaderVCView::OnUpdateToolSnapshot(CCmdUI* pCmdUI) 
{
	if (m_bSnap)
	{
		pCmdUI->SetCheck(1);
	} 
	else
	{
		pCmdUI->SetCheck(0);
	}
	
}

void CReaderVCView::OnUpdateToolSelect(CCmdUI* pCmdUI) 
{
	if (m_bSelect)
	{
		pCmdUI->SetCheck(1);
	} 
	else
	{
		pCmdUI->SetCheck(0);
	}
	
}

void CReaderVCView::OnViewBookmark() 
{
	// TODO: Add your command handler code here
	CChildFrame* pParent = (CChildFrame*)GetParentFrame();
	if(pParent == NULL) return;
	if (m_bBookmark) {
		pParent->m_wndSplitter.ShowColumn();
		
	}else{
		pParent->m_wndSplitter.HideColumn(0);
	}
	m_bBookmark = !m_bBookmark;
}



BOOL CReaderVCView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	ScreenToClient(&pt);
	// TODO: Add your message handler code here and/or call default
	double page_x = 0;
	double page_y = 0;
	DeviceToPage(pt, page_x, page_y);
	
	if(m_pApp)
	{
// 		FPDF_BOOL b= FORM_OnMouseWheel(m_pApp, m_pPage,ComposeFlag(),0,zDelta,page_x, page_y);
// 		if(b)
// 			return TRUE;
	}
	
	int curPosY = 0;
	int prevPosY = 0;
	int distanceY = 25;
	if(zDelta > 0)
	{
		curPosY = GetScrollPos(SB_VERT);
		prevPosY = SetScrollPos(SB_VERT, curPosY - distanceY, TRUE);
		curPosY = GetScrollPos(SB_VERT);
		distanceY = prevPosY - curPosY;
		m_nStartY = m_nStartY + distanceY;
		ScrollWindow(0, distanceY);		
	}
	else
	{
		curPosY = GetScrollPos(SB_VERT);
		prevPosY = SetScrollPos(SB_VERT, curPosY + distanceY, TRUE);
		curPosY = GetScrollPos(SB_VERT);
		distanceY = curPosY - prevPosY;
		m_nStartY = m_nStartY - distanceY;
		ScrollWindow(0, -distanceY);
	}	
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CReaderVCView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	
}

void CReaderVCView::OnEditCopy() 
{

	if(m_pTextPage == NULL)
	{
		AfxMessageBox("Sorry, the fpdftext.dll may has expired. For keeping on using the dll, please contact sales@foxitsoftware.com.");
		return;
	}
	long left = 0;
	long top = 0;
	long right = 0;
	long bottom = 0;
	LPWSTR pBuff = NULL;
	int buflen = 0;
	CString csCopyText;

	if(m_rtArray.GetSize() != 0)
	{
		for (int i=0; i<m_rtArray.GetSize(); i++)
		{
			PDFRect rect = m_rtArray.GetAt(i);
			buflen = FPDFText_GetBoundedText(m_pTextPage, rect.m_dbLeft, rect.m_dbTop, rect.m_dbRight,
				rect.m_dbBottom, (unsigned short*)pBuff, 0) + 1;
			if(buflen == 0)
				return;
			pBuff = new wchar_t[2*buflen];
			memset(pBuff, 0, 2*buflen);

			FPDFText_GetBoundedText(m_pTextPage, rect.m_dbLeft, rect.m_dbTop, rect.m_dbRight, 
				rect.m_dbBottom, (unsigned short*)pBuff, buflen);

			int n = WideCharToMultiByte(CP_ACP, 0, pBuff, -1, NULL, NULL, NULL, NULL);
			char *p = new char[n];
			memset(p, 0, n);
			WideCharToMultiByte(CP_ACP, 0, pBuff, buflen, p, n, NULL, NULL);
			csCopyText = csCopyText + CString(p);
			delete[] p;
		}
		
		::OpenClipboard(NULL);
		::EmptyClipboard();
		HANDLE hClip=GlobalAlloc(GMEM_MOVEABLE,csCopyText.GetLength()+1);
		char* pBuf=(char*)GlobalLock(hClip); 
		strcpy(pBuf,csCopyText);
		GlobalUnlock(hClip);
		HANDLE hSuccess = SetClipboardData(CF_TEXT, hClip);
		if(NULL != hSuccess)
			AfxMessageBox("copy success!");
		::CloseClipboard();
		

	}
	
}

void CReaderVCView::OnRenderbitmap() 
{

}

void CReaderVCView::OnExportPdfToBitmap() 
{
	if (!m_pExportPageDlg)
	{
		m_pExportPageDlg=new CExportPage;
		m_pExportPageDlg->Create(IDD_EXPORT_PAGE,this);
		m_pExportPageDlg->InitDialogInfo(this);
		m_pExportPageDlg->ShowWindow(SW_SHOW);
	}
	else
		m_pExportPageDlg->ShowWindow(SW_SHOW);
}

BOOL CReaderVCView::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
// 	PP_Event event;
// 	if(pMsg->message == WM_LBUTTONDOWN)
// 	{
// 		event.type = PP_Event_Type_MouseDown;
// 		
// 		int xPos = (int)(WORD)(pMsg->lParam); 
// 		int yPos = (int)(WORD)((pMsg->lParam >> 16) & 0xFFFF); 
// 		
// 		event.u.mouse.x = xPos;
// 		event.u.mouse.y = yPos;
// 		FPDF_FormFillEventMsg(m_pPage, event);
// 	}
// 	if(pMsg->message == WM_LBUTTONUP)
// 	{
// 		event.type = PP_Event_Type_MouseUp;
// 		
// 		int xPos = (int)(WORD)(pMsg->lParam); 
// 		int yPos = (int)(WORD)((pMsg->lParam >> 16) & 0xFFFF); 
// 		
// 		event.u.mouse.x = xPos;
// 		event.u.mouse.y = yPos;
// 		FPDF_FormFillEventMsg(m_pPage, event);
// 	}
	return CView::PreTranslateMessage(pMsg);
}

void CReaderVCView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_pApp)
	{
		if(FORM_OnChar(m_pApp, m_pPage, nChar, ComposeFlag()))
			return ;
	}
	CView::OnChar(nChar, nRepCnt, nFlags);
}

void CReaderVCView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class
	((CReaderVCApp*)AfxGetApp())->m_pActiveView = (CReaderVCView*)pActivateView;
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CReaderVCView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_pApp)
		FORM_OnKeyUp(m_pApp, m_pPage, nChar, ComposeFlag());
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}


int		G_WriteBlock( FPDF_FILEWRITE* pThis, const void* pData, unsigned long size);
class CSDK_FileWrite:public FPDF_FILEWRITE
{
public:
	CSDK_FileWrite()
	{
		m_fp = NULL;
		version = 0;
		WriteBlock = G_WriteBlock;
	}
public:
	FILE* m_fp;
};
int		G_WriteBlock( FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)
{
	CSDK_FileWrite* pFW = (CSDK_FileWrite*)pThis;
	return fwrite(pData, sizeof(char), size, pFW->m_fp);
}
void CReaderVCView::OnFileSave() 
{
	// TODO: Add your command handler code here
	if(m_pDoc != NULL)
	{
		CFileDialog dlg(FALSE,"",NULL,NULL,"PDF(*.PDF)|*.PDF||All Files(*.*)|*.*");
		if (dlg.DoModal() == IDOK)
		{
			CString strPDFName = dlg.GetPathName();

			CSDK_FileWrite fw;
			fw.m_fp = fopen(dlg.GetPathName(), "wb");

			FPDF_SaveAsCopy(m_pDoc, &fw, 0);
			fclose(fw.m_fp);
//			FPDF_SaveAsFile(m_pDoc, strPDFName.GetBuffer(strPDFName.GetLength()), 0, NULL, 0, NULL, 0);
		}		
	}
}



void CReaderVCView::OnTestJS() 
{
	// TODO: Add your command handler code here
	//FPDF_WIDESTRING js = L"run=app.setInterval(\"app.alert(\\\"ok\\\")\");app.setTimeOut(\"app.clearInterval(run)\", 6000);";
//	FPDF_WIDESTRING js = L"app.alert(AFNumber_Keystroke(\'aaaaaaaaa\'))";
//	FPDF_WIDESTRING js = L"app.mailMsg(1)";
	//FPDF_WIDESTRING js = L"app.setTimeOut(\"app.clearInterval(1)\", 6000);";
	//FPDF_WIDESTRING js = L"t.1";
//	RunJS(m_pApp, js);
	CTestJsDlg dlg;
	dlg.init(m_pApp);
	dlg.DoModal();
}

//This function is to simulate the pp event.
#define PP_EVENT_MODIFIER_SHIFTKEY       1<<0
#define PP_EVENT_MODIFIER_CONTROLKEY       1<<1
#define PP_EVENT_MODIFIER_ALTKEY           1<<2
unsigned int CReaderVCView::ComposeFlag()
{
	unsigned int nFlag = 0;
	if(IsALTpressed())
		nFlag = nFlag|PP_EVENT_MODIFIER_ALTKEY;
	if(IsCTRLpressed())
		nFlag = nFlag|PP_EVENT_MODIFIER_CONTROLKEY;
	if(IsSHIFTpressed())
		nFlag = nFlag|PP_EVENT_MODIFIER_SHIFTKEY;
	return nFlag;
}

void CReaderVCView::OnPrintMetalfile() 
{
	FPDF_PAGE page = FPDF_LoadPage(m_pDoc, 0);
	if (!page) {
		return;
	}
	
	HDC printer_dc = CreateDC("WINSPOOL", "Microsoft XPS Document Writer", NULL,
		NULL);
	if (!printer_dc) {
		printf("Could not create printer DC\n");
		return;
	}
	DOCINFO di = {0};
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = "Foxit print test";
	int job_id = StartDoc(printer_dc, &di);
	
	StartPage(printer_dc);
	
	SetGraphicsMode(printer_dc, GM_ADVANCED);
	XFORM xform = {0, 0, 0, 0, 0, 0};
	xform.eM11 = xform.eM22 = 2;
	ModifyWorldTransform(printer_dc, &xform, MWT_LEFTMULTIPLY);
	
	int dc_width = GetDeviceCaps(printer_dc, PHYSICALWIDTH);
	int dc_height = GetDeviceCaps(printer_dc, PHYSICALHEIGHT);
	HDC metafile_dc = CreateEnhMetaFile(printer_dc, NULL, NULL, NULL);
	XFORM xform1 = {0, 0, 0, 0, 0, 0};
	xform1.eM11 = xform1.eM22 = 0.5;
	ModifyWorldTransform(metafile_dc, &xform1, MWT_LEFTMULTIPLY);
	FPDF_RenderPage(metafile_dc, page, 0, 0, dc_width, dc_height, 0, 0);
	
	HENHMETAFILE emf = CloseEnhMetaFile(metafile_dc);
	
	ENHMETAHEADER header = {0};
	GetEnhMetaFileHeader(emf, sizeof(header), &header);
	
	PlayEnhMetaFile(printer_dc, emf, (RECT *)&header.rclBounds);
	
	EndPage(printer_dc);
	EndDoc(printer_dc);
	
	DeleteEnhMetaFile(emf);
	DeleteDC(printer_dc);
	
}

void CReaderVCView::CreateLocalPath(CString &csPath)
{
	csPath = "c://test.pdf";
}

BOOL CReaderVCView::HttpDataPost(CString csData, CString csAppName, CString csObject, CString csServer, CString csUserName, CString csPassword, INTERNET_PORT nPort , BOOL IsHTTPS, CString csContentType, CString csAddHeader, CString &csResponse)
{
	DWORD retCode = 0;
	BOOL bRet = FALSE;
	TRY
	{
		CInternetSession sess(csAppName);
		CHttpConnection* pConnect = sess.GetHttpConnection(csServer, nPort, csUserName, csPassword);
		if (pConnect == NULL)
			return FALSE;

		DWORD dwRequestFlags = INTERNET_FLAG_EXISTING_CONNECT;
		if ( IsHTTPS == TRUE) 
			dwRequestFlags |= INTERNET_FLAG_SECURE;
		CHttpFile* pHttpFile = pConnect->OpenRequest(CHttpConnection::HTTP_VERB_POST, csObject, NULL, 1, NULL, NULL, dwRequestFlags);
		if (pHttpFile != NULL)
		{
			CString strData = csData;
			DWORD dwLength = strData.GetLength();

			CString strHeaders;
			strHeaders.Format("Content-Type: %s\r\nContent-Length: %d\r\n", csContentType, dwLength);
			strHeaders += csAddHeader;
			pHttpFile->AddRequestHeaders(strHeaders);
			DWORD dwTotalLength = dwLength + strHeaders.GetLength();

resend:
			bRet = pHttpFile->SendRequestEx(dwTotalLength);
			pHttpFile->Write(csData, dwLength);

			pHttpFile->QueryInfoStatusCode( retCode );
			if (HTTP_STATUS_OK == retCode) //succ
			{
				char buf[4096] = {0};
				UINT bytesRead = 0;
				while( ( bytesRead = pHttpFile->Read( buf, 4095 ) ) > 0 )
				{
					buf[bytesRead] = '\0';
					size_t aLen = strlen( buf ) + 1; 
					int wLen = MultiByteToWideChar( 936, 0, buf, aLen, NULL, 0 );

					LPOLESTR lpw = new WCHAR [wLen]; 
					MultiByteToWideChar( 936, 0, buf, aLen, lpw, wLen ); 
					csResponse += lpw;
					delete [] lpw; 

					memset( buf, 0, 4096 );
				} 
			}
			bRet = pHttpFile->EndRequest();
			if (bRet)
			{
				// Handle any authentication dialogs.
				if (NeedAuth(pHttpFile))
				{
					DWORD dwErr;
					dwErr = pHttpFile->ErrorDlg(GetDesktopWindow(),                                                  
						bRet ? ERROR_SUCCESS : GetLastError(),                         
						FLAGS_ERROR_UI_FILTER_FOR_ERRORS  |     
						FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS |
						FLAGS_ERROR_UI_FLAGS_GENERATE_DATA, 
						NULL);
					if (dwErr == ERROR_INTERNET_FORCE_RETRY)
					{
						goto resend;
					}
					else if(dwErr == 0)
					{
						bRet = FALSE;
					}
				}
			}
			pHttpFile->Close();
			delete pHttpFile;
		}
		pConnect->Close();
		delete pConnect;
		sess.Close();
	}
	CATCH_ALL(e)
	{
		return FALSE;
	}
	END_CATCH_ALL
		return bRet;
}

BOOL CReaderVCView::HttpDataPut(CString csData, CString csAppName, CString csObject, CString csServer, CString csUserName, CString csPassword, INTERNET_PORT nPort, BOOL IsHTTPS)
{
	DWORD retCode = 0;
	BOOL bRet = FALSE;
	TRY
	{
		CInternetSession sess(csAppName);
		CHttpConnection* pConnect = sess.GetHttpConnection(csServer, nPort, csUserName, csPassword);
		if (pConnect == NULL)
			return FALSE;

		DWORD dwRequestFlags=INTERNET_FLAG_EXISTING_CONNECT;
		if ( IsHTTPS== TRUE) 
			dwRequestFlags |= INTERNET_FLAG_SECURE;
		CHttpFile* pHttpFile = pConnect->OpenRequest(CHttpConnection::HTTP_VERB_PUT, csObject,NULL,1,NULL,NULL,dwRequestFlags);
		if (pHttpFile != NULL)
		{
resend:
			CString strData = csData;
			DWORD dwTotalLength = strData.GetLength();

			bRet = pHttpFile->SendRequestEx(dwTotalLength);
			pHttpFile->Write(csData, dwTotalLength);
			bRet = pHttpFile->EndRequest();
			if (bRet)
			{
				// Handle any authentication dialogs.
				if (NeedAuth(pHttpFile))
				{
					DWORD dwErr;
					dwErr = pHttpFile->ErrorDlg(GetDesktopWindow(),
						bRet ? ERROR_SUCCESS : GetLastError(),                         
						FLAGS_ERROR_UI_FILTER_FOR_ERRORS  |     
						FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS |
						FLAGS_ERROR_UI_FLAGS_GENERATE_DATA, 
						NULL
						);
					if (dwErr == ERROR_INTERNET_FORCE_RETRY)
					{
						goto resend;
					}
					else if(dwErr == 0)
					{
						bRet = FALSE;
					}
				}
			}
			pHttpFile->QueryInfoStatusCode( retCode );
			if (retCode != HTTP_STATUS_OK)
			{
				bRet = FALSE;
			}
			else
				bRet = TRUE;

			pHttpFile->Close();
			delete pHttpFile;
		}
		pConnect->Close();
		delete pConnect;
		sess.Close();
	}
	CATCH_ALL(e)
	{
		return FALSE;
	}
	END_CATCH_ALL
		return bRet;
}

BOOL CReaderVCView::NeedAuth(CHttpFile *pHttpFile)
{
	// Get status code.
	DWORD dwStatus;
	DWORD cbStatus = sizeof(dwStatus);
	pHttpFile->QueryInfo
		(
		HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE,
		&dwStatus,
		&cbStatus,
		NULL
		);
	//    fprintf (stderr, "Status: %d\n", dwStatus);
	// Look for 401 or 407.
	DWORD dwFlags;
	switch (dwStatus)
	{
	case HTTP_STATUS_DENIED:
		dwFlags = HTTP_QUERY_WWW_AUTHENTICATE;
		break;
	case HTTP_STATUS_PROXY_AUTH_REQ:
		dwFlags = HTTP_QUERY_PROXY_AUTHENTICATE;
		break;            
	default:
		return FALSE;
	}
	// Enumerate the authentication types.
	BOOL fRet;
	char szScheme[64];
	DWORD dwIndex = 0;
	do
	{
		DWORD cbScheme = sizeof(szScheme);
		fRet = pHttpFile->QueryInfo
			(dwFlags, szScheme, &cbScheme, &dwIndex);

		//if (fRet)
		//fprintf (stderr, "Found auth scheme: %s\n", szScheme);
	}
	while (fRet);
	return TRUE;
}