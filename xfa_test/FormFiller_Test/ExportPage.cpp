// ExportPage.cpp : implementation file
//

#include "stdafx.h"
#include "ReaderVC.h"
#include "ExportPage.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ReaderVCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportPage dialog


CExportPage::CExportPage(CWnd* pParent /*=NULL*/)
	: CDialog(CExportPage::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportPage)
	m_nHeight = 0;
	m_nPageHeight = 0;
	m_nRotate = 0;
	m_nWidth = 0;
	m_nPageWidth = 0;
	//}}AFX_DATA_INIT
	m_bitmap = NULL;
}


void CExportPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportPage)
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_nHeight);
	DDX_Text(pDX, IDC_EDIT_PAGE_HEIGHT, m_nPageHeight);
	DDX_Text(pDX, IDC_EDIT_ROTATE, m_nRotate);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_nWidth);
	DDX_Text(pDX, IDC_EDIT_PAGE_WIDTH, m_nPageWidth);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportPage, CDialog)
	//{{AFX_MSG_MAP(CExportPage)
	ON_BN_CLICKED(IDC_Rander_Page, OnRanderPage)
	ON_BN_CLICKED(IDC_Save, OnSave)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportPage message handlers

void CExportPage::OnRanderPage() 
{
	if (m_pView)
	{
		UpdateData(TRUE);
		m_page=m_pView->GetPage();
		if (!m_page) return;
		if (m_nRotate==1 || m_nRotate==3)
		{
			int temp;
			temp=m_nHeight;
			m_nHeight=m_nWidth;
			m_nWidth=temp;
		}
		m_bitmap=FPDFBitmap_Create(m_nWidth,m_nHeight,FPDFBitmap_BGRx);
		FPDFBitmap_FillRect(m_bitmap,0,0,m_nWidth,m_nHeight,0xff,0xff,0xff,0xff);
		FPDF_RenderPageBitmap(m_bitmap,m_page,0,0,200,200,m_nRotate,FPDF_ANNOT);
		Invalidate();
		if (m_nRotate==1 || m_nRotate==3)
		{
			int temp;
			temp=m_nHeight;
			m_nHeight=m_nWidth;
			m_nWidth=temp;
		}
		//FPDFBitmap_Destroy(bitmap);
		UpdateData(FALSE);
	}
	
}

void CExportPage::InitDialogInfo(CReaderVCView *pView)
{
	m_pView=pView;
	if (pView)	m_page=pView->GetPage();
	SetDlgInfo();
}

void CExportPage::SetDlgInfo()
{
	if (!m_page) return;
	double height,width;
	width=FPDF_GetPageWidth(m_page);
	height=FPDF_GetPageHeight(m_page);
	
	UpdateData(TRUE);	
	m_nPageHeight = (int)height;
	m_nPageWidth = (int)width;
	m_nWidth=(int)width/3;
	m_nHeight=(int)height/3;
	UpdateData(FALSE);
}

void CExportPage::OnSave() 
{
	CFileDialog SavDlg(FALSE,"","",OFN_FILEMUSTEXIST |OFN_HIDEREADONLY,"bmp(*.bmp)|*.bmp||All Files(*.*)|*.*");
	if (SavDlg.DoModal()==IDOK)
	{
		CString strFileName = SavDlg.GetPathName();
		BITMAPFILEHEADER bitmapFileHeader;
		BITMAPINFOHEADER bitmapInfoHeader;
		BITMAP strBitmap;
		int wBitCount = 32;
		winbmp.GetBitmap(&strBitmap);
		DWORD dwBmBitsSize = ((strBitmap.bmWidth * wBitCount+31)/32) * 4 * strBitmap.bmHeight;
		
		bitmapFileHeader.bfType = 0x4D42;
		bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmBitsSize;
		bitmapFileHeader.bfReserved1 = bitmapFileHeader.bfReserved2 = 0;
		bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		
		bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapInfoHeader.biWidth = strBitmap.bmWidth;
		bitmapInfoHeader.biHeight = strBitmap.bmHeight;
		bitmapInfoHeader.biPlanes = 1;
		bitmapInfoHeader.biBitCount = wBitCount;
		bitmapInfoHeader.biClrImportant = BI_RGB;
		bitmapInfoHeader.biSizeImage = 0; //strBitmap.bmWidth * strBitmap.bmHeight;
		bitmapInfoHeader.biXPelsPerMeter = 0;
		bitmapInfoHeader.biYPelsPerMeter = 0;
		bitmapInfoHeader.biClrUsed = 0;
		bitmapInfoHeader.biCompression = 0;
		
		char* context = new char[dwBmBitsSize];
		CWindowDC dc(NULL);
		GetDIBits(dc.GetSafeHdc(), (HBITMAP)winbmp.m_hObject, 0, bitmapInfoHeader.biHeight, (LPVOID)context,(BITMAPINFO*)&bitmapInfoHeader, DIB_RGB_COLORS);
		
		CFile file;
		file.Open(strFileName, CFile::modeCreate|CFile::modeWrite);
		file.Write(&bitmapFileHeader, sizeof(BITMAPFILEHEADER));
		file.Write(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER));
		file.Write(context, dwBmBitsSize);
		
		file.Close();
		delete context;
	}
}

void CExportPage::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	if(m_bitmap)
	{
		//	CDC* pDc=GetDC();
		int bufsize=FPDFBitmap_GetStride(m_bitmap)*m_nHeight;
		void* bmpbuf=FPDFBitmap_GetBuffer(m_bitmap);
		CDC MemDC;
		CDC   *pDc   =   GetDlgItem(IDC_STATIC_BITMAP)->GetDC();     //ID:   picture 
		CRect   rect;
		((CWnd *)GetDlgItem(IDC_STATIC_BITMAP))->GetWindowRect(rect);
		
		MemDC.CreateCompatibleDC(pDc);
		if((HBITMAP)winbmp != NULL)
			winbmp.DeleteObject();
		if(HBITMAP(winbmp) == NULL)
		{
			winbmp.CreateCompatibleBitmap(pDc,m_nWidth,m_nHeight);
			winbmp.SetBitmapBits(bufsize,bmpbuf);
		}

		MemDC.SelectObject(&winbmp); 
		
		pDc->BitBlt(0 , 0 , rect.Width(), rect.Height(), &MemDC,0,0,SRCCOPY);
		//pDc->StretchBlt(0,0,rect.right-rect.left,rect.bottom-rect.top,&MemDC,0,0,m_nWidth,m_nHeight,SRCCOPY);   
		MemDC.DeleteDC();     
	}
	// Do not call CDialog::OnPaint() for painting messages
}
