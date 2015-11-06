// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdfview.h"

#include "../include/fsdk_define.h"
#include "../include/fsdk_mgr.h"
#include "../include/fsdk_rendercontext.h"
#include "../include/javascript/IJavaScript.h"
#include "core/include/fxcodec/fx_codec.h"
#include "core/include/fxcrt/fx_safe_types.h"
#include "public/fpdf_ext.h"
#include "public/fpdf_progressive.h"
#include "third_party/base/nonstd_unique_ptr.h"
#include "third_party/base/numerics/safe_conversions_impl.h"

CPDF_Document* CPDFDocumentFromFPDFDocument(FPDF_DOCUMENT doc) {
  return static_cast<CPDF_Document*>(doc);
}

FPDF_DOCUMENT FPDFDocumentFromCPDFDocument(CPDF_Document* doc) {
  return static_cast<FPDF_DOCUMENT>(doc);
}

CPDF_Page* CPDFPageFromFPDFPage(FPDF_PAGE page) {
  return static_cast<CPDF_Page*>(page);
}

CPDF_CustomAccess::CPDF_CustomAccess(FPDF_FILEACCESS* pFileAccess) {
  if (pFileAccess)
    m_FileAccess = *pFileAccess;
}

FX_BOOL CPDF_CustomAccess::ReadBlock(void* buffer,
                                     FX_FILESIZE offset,
                                     size_t size) {
  if (offset < 0) {
    return FALSE;
  }
  FX_SAFE_FILESIZE newPos =
      pdfium::base::checked_cast<FX_FILESIZE, size_t>(size);
  newPos += offset;
  if (!newPos.IsValid() || newPos.ValueOrDie() > m_FileAccess.m_FileLen) {
    return FALSE;
  }
  return m_FileAccess.m_GetBlock(m_FileAccess.m_Param, offset, (uint8_t*)buffer,
                                 size);
}

// 0 bit: FPDF_POLICY_MACHINETIME_ACCESS
static FX_DWORD foxit_sandbox_policy = 0xFFFFFFFF;

void FSDK_SetSandBoxPolicy(FPDF_DWORD policy, FPDF_BOOL enable) {
  switch (policy) {
    case FPDF_POLICY_MACHINETIME_ACCESS: {
      if (enable)
        foxit_sandbox_policy |= 0x01;
      else
        foxit_sandbox_policy &= 0xFFFFFFFE;
    } break;
    default:
      break;
  }
}

FPDF_BOOL FSDK_IsSandBoxPolicyEnabled(FPDF_DWORD policy) {
  switch (policy) {
    case FPDF_POLICY_MACHINETIME_ACCESS:
      return !!(foxit_sandbox_policy & 0x01);
    default:
      return FALSE;
  }
}

CCodec_ModuleMgr* g_pCodecModule = nullptr;

DLLEXPORT void STDCALL FPDF_InitLibrary() {
  FPDF_InitLibraryWithConfig(nullptr);
}

DLLEXPORT void STDCALL FPDF_InitLibraryWithConfig(
    const FPDF_LIBRARY_CONFIG* cfg) {
  g_pCodecModule = new CCodec_ModuleMgr();

  CFX_GEModule::Create(cfg ? cfg->m_pUserFontPaths : nullptr);
  CFX_GEModule::Get()->SetCodecModule(g_pCodecModule);

  CPDF_ModuleMgr::Create();
  CPDF_ModuleMgr::Get()->SetCodecModule(g_pCodecModule);
  CPDF_ModuleMgr::Get()->InitPageModule();
  CPDF_ModuleMgr::Get()->InitRenderModule();
  CPDF_ModuleMgr* pModuleMgr = CPDF_ModuleMgr::Get();
  if (pModuleMgr) {
    pModuleMgr->LoadEmbeddedGB1CMaps();
    pModuleMgr->LoadEmbeddedJapan1CMaps();
    pModuleMgr->LoadEmbeddedCNS1CMaps();
    pModuleMgr->LoadEmbeddedKorea1CMaps();
  }
  if (cfg && cfg->version >= 2)
    IJS_Runtime::Initialize(cfg->m_v8EmbedderSlot, cfg->m_pIsolate);
}

DLLEXPORT void STDCALL FPDF_DestroyLibrary() {
  CPDF_ModuleMgr::Destroy();
  CFX_GEModule::Destroy();

  delete g_pCodecModule;
  g_pCodecModule = nullptr;
}

#ifndef _WIN32
int g_LastError;
void SetLastError(int err) {
  g_LastError = err;
}

int GetLastError() {
  return g_LastError;
}
#endif

void ProcessParseError(FX_DWORD err_code) {
  // Translate FPDFAPI error code to FPDFVIEW error code
  switch (err_code) {
    case PDFPARSE_ERROR_FILE:
      err_code = FPDF_ERR_FILE;
      break;
    case PDFPARSE_ERROR_FORMAT:
      err_code = FPDF_ERR_FORMAT;
      break;
    case PDFPARSE_ERROR_PASSWORD:
      err_code = FPDF_ERR_PASSWORD;
      break;
    case PDFPARSE_ERROR_HANDLER:
      err_code = FPDF_ERR_SECURITY;
      break;
  }
  SetLastError(err_code);
}

DLLEXPORT void STDCALL FPDF_SetSandBoxPolicy(FPDF_DWORD policy,
                                             FPDF_BOOL enable) {
  return FSDK_SetSandBoxPolicy(policy, enable);
}

DLLEXPORT FPDF_DOCUMENT STDCALL FPDF_LoadDocument(FPDF_STRING file_path,
                                                  FPDF_BYTESTRING password) {
  // NOTE: the creation of the file needs to be by the embedder on the
  // other side of this API.
  IFX_FileRead* pFileAccess = FX_CreateFileRead((const FX_CHAR*)file_path);
  if (!pFileAccess) {
    return nullptr;
  }

  CPDF_Parser* pParser = new CPDF_Parser;
  pParser->SetPassword(password);

  FX_DWORD err_code = pParser->StartParse(pFileAccess);
  if (err_code) {
    delete pParser;
    ProcessParseError(err_code);
    return NULL;
  }
  return pParser->GetDocument();
}

class CMemFile final : public IFX_FileRead {
 public:
  CMemFile(uint8_t* pBuf, FX_FILESIZE size) : m_pBuf(pBuf), m_size(size) {}

  void Release() override { delete this; }
  FX_FILESIZE GetSize() override { return m_size; }
  FX_BOOL ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override {
    if (offset < 0) {
      return FALSE;
    }
    FX_SAFE_FILESIZE newPos =
        pdfium::base::checked_cast<FX_FILESIZE, size_t>(size);
    newPos += offset;
    if (!newPos.IsValid() || newPos.ValueOrDie() > (FX_DWORD)m_size) {
      return FALSE;
    }
    FXSYS_memcpy(buffer, m_pBuf + offset, size);
    return TRUE;
  }

 private:
  ~CMemFile() override {}

  uint8_t* const m_pBuf;
  const FX_FILESIZE m_size;
};

DLLEXPORT FPDF_DOCUMENT STDCALL FPDF_LoadMemDocument(const void* data_buf,
                                                     int size,
                                                     FPDF_BYTESTRING password) {
  CPDF_Parser* pParser = new CPDF_Parser;
  pParser->SetPassword(password);
  CMemFile* pMemFile = new CMemFile((uint8_t*)data_buf, size);
  FX_DWORD err_code = pParser->StartParse(pMemFile);
  if (err_code) {
    delete pParser;
    ProcessParseError(err_code);
    return NULL;
  }
  CPDF_Document* pDoc = NULL;
  pDoc = pParser ? pParser->GetDocument() : NULL;
  CheckUnSupportError(pDoc, err_code);
  return FPDFDocumentFromCPDFDocument(pParser->GetDocument());
}

DLLEXPORT FPDF_DOCUMENT STDCALL
FPDF_LoadCustomDocument(FPDF_FILEACCESS* pFileAccess,
                        FPDF_BYTESTRING password) {
  CPDF_Parser* pParser = new CPDF_Parser;
  pParser->SetPassword(password);
  CPDF_CustomAccess* pFile = new CPDF_CustomAccess(pFileAccess);
  FX_DWORD err_code = pParser->StartParse(pFile);
  if (err_code) {
    delete pParser;
    ProcessParseError(err_code);
    return NULL;
  }
  CPDF_Document* pDoc = NULL;
  pDoc = pParser ? pParser->GetDocument() : NULL;
  CheckUnSupportError(pDoc, err_code);
  return FPDFDocumentFromCPDFDocument(pParser->GetDocument());
}

DLLEXPORT FPDF_BOOL STDCALL FPDF_GetFileVersion(FPDF_DOCUMENT doc,
                                                int* fileVersion) {
  if (!fileVersion)
    return FALSE;

  *fileVersion = 0;
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(doc);
  if (!pDoc)
    return FALSE;

  CPDF_Parser* pParser = (CPDF_Parser*)pDoc->GetParser();
  if (!pParser)
    return FALSE;

  *fileVersion = pParser->GetFileVersion();
  return TRUE;
}

// jabdelmalek: changed return type from FX_DWORD to build on Linux (and match
// header).
DLLEXPORT unsigned long STDCALL FPDF_GetDocPermissions(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;

  CPDF_Parser* pParser = (CPDF_Parser*)pDoc->GetParser();
  CPDF_Dictionary* pDict = pParser->GetEncryptDict();
  return pDict ? pDict->GetInteger("P") : (FX_DWORD)-1;
}

DLLEXPORT int STDCALL FPDF_GetSecurityHandlerRevision(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return -1;

  CPDF_Parser* pParser = (CPDF_Parser*)pDoc->GetParser();
  CPDF_Dictionary* pDict = pParser->GetEncryptDict();
  return pDict ? pDict->GetInteger("R") : -1;
}

DLLEXPORT int STDCALL FPDF_GetPageCount(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  return pDoc ? pDoc->GetPageCount() : 0;
}

DLLEXPORT FPDF_PAGE STDCALL FPDF_LoadPage(FPDF_DOCUMENT document,
                                          int page_index) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  if (page_index < 0 || page_index >= FPDF_GetPageCount(document))
    return nullptr;

  CPDF_Dictionary* pDict = pDoc->GetPage(page_index);
  if (pDict == NULL)
    return NULL;
  CPDF_Page* pPage = new CPDF_Page;
  pPage->Load(pDoc, pDict);
  pPage->ParseContent();
  return pPage;
}

DLLEXPORT double STDCALL FPDF_GetPageWidth(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  return pPage ? pPage->GetPageWidth() : 0.0;
}

DLLEXPORT double STDCALL FPDF_GetPageHeight(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  return pPage ? pPage->GetPageHeight() : 0.0;
}

void DropContext(void* data) {
  delete (CRenderContext*)data;
}

#if defined(_DEBUG) || defined(DEBUG)
#define DEBUG_TRACE
#endif

#if defined(_WIN32)
DLLEXPORT void STDCALL FPDF_RenderPage(HDC dc,
                                       FPDF_PAGE page,
                                       int start_x,
                                       int start_y,
                                       int size_x,
                                       int size_y,
                                       int rotate,
                                       int flags) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return;

  CRenderContext* pContext = new CRenderContext;
  pPage->SetPrivateData((void*)1, pContext, DropContext);

#ifndef _WIN32_WCE
  CFX_DIBitmap* pBitmap = NULL;
  FX_BOOL bBackgroundAlphaNeeded = FALSE;
  bBackgroundAlphaNeeded = pPage->BackgroundAlphaNeeded();
  if (bBackgroundAlphaNeeded) {
    pBitmap = new CFX_DIBitmap;
    pBitmap->Create(size_x, size_y, FXDIB_Argb);
    pBitmap->Clear(0x00ffffff);
#ifdef _SKIA_SUPPORT_
    pContext->m_pDevice = new CFX_SkiaDevice;
    ((CFX_SkiaDevice*)pContext->m_pDevice)->Attach((CFX_DIBitmap*)pBitmap);
#else
    pContext->m_pDevice = new CFX_FxgeDevice;
    ((CFX_FxgeDevice*)pContext->m_pDevice)->Attach((CFX_DIBitmap*)pBitmap);
#endif
  } else
    pContext->m_pDevice = new CFX_WindowsDevice(dc);

  FPDF_RenderPage_Retail(pContext, page, start_x, start_y, size_x, size_y,
                         rotate, flags, TRUE, NULL);

  if (bBackgroundAlphaNeeded) {
    if (pBitmap) {
      CFX_WindowsDevice WinDC(dc);

      if (WinDC.GetDeviceCaps(FXDC_DEVICE_CLASS) == FXDC_PRINTER) {
        CFX_DIBitmap* pDst = new CFX_DIBitmap;
        int pitch = pBitmap->GetPitch();
        pDst->Create(size_x, size_y, FXDIB_Rgb32);
        FXSYS_memset(pDst->GetBuffer(), -1, pitch * size_y);
        pDst->CompositeBitmap(0, 0, size_x, size_y, pBitmap, 0, 0,
                              FXDIB_BLEND_NORMAL, NULL, FALSE, NULL);
        WinDC.StretchDIBits(pDst, 0, 0, size_x, size_y);
        delete pDst;
      } else
        WinDC.SetDIBits(pBitmap, 0, 0);
    }
  }
#else
  // get clip region
  RECT rect, cliprect;
  rect.left = start_x;
  rect.top = start_y;
  rect.right = start_x + size_x;
  rect.bottom = start_y + size_y;
  GetClipBox(dc, &cliprect);
  IntersectRect(&rect, &rect, &cliprect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

#ifdef DEBUG_TRACE
  {
    char str[128];
    memset(str, 0, sizeof(str));
    FXSYS_snprintf(str, sizeof(str) - 1, "Rendering DIB %d x %d", width,
                   height);
    CPDF_ModuleMgr::Get()->ReportError(999, str);
  }
#endif

  // Create a DIB section
  LPVOID pBuffer;
  BITMAPINFOHEADER bmih;
  FXSYS_memset(&bmih, 0, sizeof bmih);
  bmih.biSize = sizeof bmih;
  bmih.biBitCount = 24;
  bmih.biHeight = -height;
  bmih.biPlanes = 1;
  bmih.biWidth = width;
  pContext->m_hBitmap = CreateDIBSection(dc, (BITMAPINFO*)&bmih, DIB_RGB_COLORS,
                                         &pBuffer, NULL, 0);
  if (pContext->m_hBitmap == NULL) {
#if defined(DEBUG) || defined(_DEBUG)
    char str[128];
    memset(str, 0, sizeof(str));
    FXSYS_snprintf(str, sizeof(str) - 1,
                   "Error CreateDIBSection: %d x %d, error code = %d", width,
                   height, GetLastError());
    CPDF_ModuleMgr::Get()->ReportError(FPDFERR_OUT_OF_MEMORY, str);
#else
    CPDF_ModuleMgr::Get()->ReportError(FPDFERR_OUT_OF_MEMORY, NULL);
#endif
  }
  FXSYS_memset(pBuffer, 0xff, height * ((width * 3 + 3) / 4 * 4));

#ifdef DEBUG_TRACE
  { CPDF_ModuleMgr::Get()->ReportError(999, "DIBSection created"); }
#endif

  // Create a device with this external buffer
  pContext->m_pBitmap = new CFX_DIBitmap;
  pContext->m_pBitmap->Create(width, height, FXDIB_Rgb, (uint8_t*)pBuffer);
  pContext->m_pDevice = new CPDF_FxgeDevice;
  ((CPDF_FxgeDevice*)pContext->m_pDevice)->Attach(pContext->m_pBitmap);

#ifdef DEBUG_TRACE
  CPDF_ModuleMgr::Get()->ReportError(999, "Ready for PDF rendering");
#endif

  // output to bitmap device
  FPDF_RenderPage_Retail(pContext, page, start_x - rect.left,
                         start_y - rect.top, size_x, size_y, rotate, flags);

#ifdef DEBUG_TRACE
  CPDF_ModuleMgr::Get()->ReportError(999, "Finished PDF rendering");
#endif

  // Now output to real device
  HDC hMemDC = CreateCompatibleDC(dc);
  if (hMemDC == NULL) {
#if defined(DEBUG) || defined(_DEBUG)
    char str[128];
    memset(str, 0, sizeof(str));
    FXSYS_snprintf(str, sizeof(str) - 1,
                   "Error CreateCompatibleDC. Error code = %d", GetLastError());
    CPDF_ModuleMgr::Get()->ReportError(FPDFERR_OUT_OF_MEMORY, str);
#else
    CPDF_ModuleMgr::Get()->ReportError(FPDFERR_OUT_OF_MEMORY, NULL);
#endif
  }

  HGDIOBJ hOldBitmap = SelectObject(hMemDC, pContext->m_hBitmap);

#ifdef DEBUG_TRACE
  CPDF_ModuleMgr::Get()->ReportError(999, "Ready for screen rendering");
#endif

  BitBlt(dc, rect.left, rect.top, width, height, hMemDC, 0, 0, SRCCOPY);
  SelectObject(hMemDC, hOldBitmap);
  DeleteDC(hMemDC);

#ifdef DEBUG_TRACE
  CPDF_ModuleMgr::Get()->ReportError(999, "Finished screen rendering");
#endif

#endif
  if (bBackgroundAlphaNeeded) {
    delete pBitmap;
    pBitmap = NULL;
  }
  delete pContext;
  pPage->RemovePrivateData((void*)1);
}
#endif

DLLEXPORT void STDCALL FPDF_RenderPageBitmap(FPDF_BITMAP bitmap,
                                             FPDF_PAGE page,
                                             int start_x,
                                             int start_y,
                                             int size_x,
                                             int size_y,
                                             int rotate,
                                             int flags) {
  if (!bitmap)
    return;
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return;
  CRenderContext* pContext = new CRenderContext;
  pPage->SetPrivateData((void*)1, pContext, DropContext);
#ifdef _SKIA_SUPPORT_
  pContext->m_pDevice = new CFX_SkiaDevice;

  if (flags & FPDF_REVERSE_BYTE_ORDER)
    ((CFX_SkiaDevice*)pContext->m_pDevice)
        ->Attach((CFX_DIBitmap*)bitmap, 0, TRUE);
  else
    ((CFX_SkiaDevice*)pContext->m_pDevice)->Attach((CFX_DIBitmap*)bitmap);
#else
  pContext->m_pDevice = new CFX_FxgeDevice;

  if (flags & FPDF_REVERSE_BYTE_ORDER)
    ((CFX_FxgeDevice*)pContext->m_pDevice)
        ->Attach((CFX_DIBitmap*)bitmap, 0, TRUE);
  else
    ((CFX_FxgeDevice*)pContext->m_pDevice)->Attach((CFX_DIBitmap*)bitmap);
#endif

  FPDF_RenderPage_Retail(pContext, page, start_x, start_y, size_x, size_y,
                         rotate, flags, TRUE, NULL);

  delete pContext;
  pPage->RemovePrivateData((void*)1);
}

DLLEXPORT void STDCALL FPDF_ClosePage(FPDF_PAGE page) {
  if (!page)
    return;
  CPDFSDK_PageView* pPageView =
      (CPDFSDK_PageView*)(((CPDF_Page*)page))->GetPrivateData((void*)page);
  if (pPageView && pPageView->IsLocked()) {
    pPageView->TakeOverPage();
    return;
  }
  delete (CPDF_Page*)page;
}

DLLEXPORT void STDCALL FPDF_CloseDocument(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return;

  CPDF_Parser* pParser = (CPDF_Parser*)pDoc->GetParser();
  if (!pParser) {
    delete pDoc;
    return;
  }
  delete pParser;
}

DLLEXPORT unsigned long STDCALL FPDF_GetLastError() {
  return GetLastError();
}

DLLEXPORT void STDCALL FPDF_DeviceToPage(FPDF_PAGE page,
                                         int start_x,
                                         int start_y,
                                         int size_x,
                                         int size_y,
                                         int rotate,
                                         int device_x,
                                         int device_y,
                                         double* page_x,
                                         double* page_y) {
  if (page == NULL || page_x == NULL || page_y == NULL)
    return;
  CPDF_Page* pPage = (CPDF_Page*)page;

  CPDF_Matrix page2device;
  pPage->GetDisplayMatrix(page2device, start_x, start_y, size_x, size_y,
                          rotate);
  CPDF_Matrix device2page;
  device2page.SetReverse(page2device);

  FX_FLOAT page_x_f, page_y_f;
  device2page.Transform((FX_FLOAT)(device_x), (FX_FLOAT)(device_y), page_x_f,
                        page_y_f);

  *page_x = (page_x_f);
  *page_y = (page_y_f);
}

DLLEXPORT void STDCALL FPDF_PageToDevice(FPDF_PAGE page,
                                         int start_x,
                                         int start_y,
                                         int size_x,
                                         int size_y,
                                         int rotate,
                                         double page_x,
                                         double page_y,
                                         int* device_x,
                                         int* device_y) {
  if (!device_x || !device_y)
    return;
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return;
  CPDF_Matrix page2device;
  pPage->GetDisplayMatrix(page2device, start_x, start_y, size_x, size_y,
                          rotate);

  FX_FLOAT device_x_f, device_y_f;
  page2device.Transform(((FX_FLOAT)page_x), ((FX_FLOAT)page_y), device_x_f,
                        device_y_f);

  *device_x = FXSYS_round(device_x_f);
  *device_y = FXSYS_round(device_y_f);
}

DLLEXPORT FPDF_BITMAP STDCALL FPDFBitmap_Create(int width,
                                                int height,
                                                int alpha) {
  nonstd::unique_ptr<CFX_DIBitmap> pBitmap(new CFX_DIBitmap);
  if (!pBitmap->Create(width, height, alpha ? FXDIB_Argb : FXDIB_Rgb32)) {
    return NULL;
  }
  return pBitmap.release();
}

DLLEXPORT FPDF_BITMAP STDCALL FPDFBitmap_CreateEx(int width,
                                                  int height,
                                                  int format,
                                                  void* first_scan,
                                                  int stride) {
  FXDIB_Format fx_format;
  switch (format) {
    case FPDFBitmap_Gray:
      fx_format = FXDIB_8bppRgb;
      break;
    case FPDFBitmap_BGR:
      fx_format = FXDIB_Rgb;
      break;
    case FPDFBitmap_BGRx:
      fx_format = FXDIB_Rgb32;
      break;
    case FPDFBitmap_BGRA:
      fx_format = FXDIB_Argb;
      break;
    default:
      return NULL;
  }
  CFX_DIBitmap* pBitmap = new CFX_DIBitmap;
  pBitmap->Create(width, height, fx_format, (uint8_t*)first_scan, stride);
  return pBitmap;
}

DLLEXPORT void STDCALL FPDFBitmap_FillRect(FPDF_BITMAP bitmap,
                                           int left,
                                           int top,
                                           int width,
                                           int height,
                                           FPDF_DWORD color) {
  if (bitmap == NULL)
    return;
#ifdef _SKIA_SUPPORT_
  CFX_SkiaDevice device;
#else
  CFX_FxgeDevice device;
#endif
  device.Attach((CFX_DIBitmap*)bitmap);
  if (!((CFX_DIBitmap*)bitmap)->HasAlpha())
    color |= 0xFF000000;
  FX_RECT rect(left, top, left + width, top + height);
  device.FillRect(&rect, color);
}

DLLEXPORT void* STDCALL FPDFBitmap_GetBuffer(FPDF_BITMAP bitmap) {
  if (bitmap == NULL)
    return NULL;
  return ((CFX_DIBitmap*)bitmap)->GetBuffer();
}

DLLEXPORT int STDCALL FPDFBitmap_GetWidth(FPDF_BITMAP bitmap) {
  if (bitmap == NULL)
    return 0;
  return ((CFX_DIBitmap*)bitmap)->GetWidth();
}

DLLEXPORT int STDCALL FPDFBitmap_GetHeight(FPDF_BITMAP bitmap) {
  if (bitmap == NULL)
    return 0;
  return ((CFX_DIBitmap*)bitmap)->GetHeight();
}

DLLEXPORT int STDCALL FPDFBitmap_GetStride(FPDF_BITMAP bitmap) {
  if (bitmap == NULL)
    return 0;
  return ((CFX_DIBitmap*)bitmap)->GetPitch();
}

DLLEXPORT void STDCALL FPDFBitmap_Destroy(FPDF_BITMAP bitmap) {
  delete (CFX_DIBitmap*)bitmap;
}

void FPDF_RenderPage_Retail(CRenderContext* pContext,
                            FPDF_PAGE page,
                            int start_x,
                            int start_y,
                            int size_x,
                            int size_y,
                            int rotate,
                            int flags,
                            FX_BOOL bNeedToRestore,
                            IFSDK_PAUSE_Adapter* pause) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return;

  if (!pContext->m_pOptions)
    pContext->m_pOptions = new CPDF_RenderOptions;

  if (flags & FPDF_LCD_TEXT)
    pContext->m_pOptions->m_Flags |= RENDER_CLEARTYPE;
  else
    pContext->m_pOptions->m_Flags &= ~RENDER_CLEARTYPE;
  if (flags & FPDF_NO_NATIVETEXT)
    pContext->m_pOptions->m_Flags |= RENDER_NO_NATIVETEXT;
  if (flags & FPDF_RENDER_LIMITEDIMAGECACHE)
    pContext->m_pOptions->m_Flags |= RENDER_LIMITEDIMAGECACHE;
  if (flags & FPDF_RENDER_FORCEHALFTONE)
    pContext->m_pOptions->m_Flags |= RENDER_FORCE_HALFTONE;
  if (flags & FPDF_RENDER_NO_SMOOTHTEXT)
    pContext->m_pOptions->m_Flags |= RENDER_NOTEXTSMOOTH;
  if (flags & FPDF_RENDER_NO_SMOOTHIMAGE)
    pContext->m_pOptions->m_Flags |= RENDER_NOIMAGESMOOTH;
  if (flags & FPDF_RENDER_NO_SMOOTHPATH)
    pContext->m_pOptions->m_Flags |= RENDER_NOPATHSMOOTH;
  // Grayscale output
  if (flags & FPDF_GRAYSCALE) {
    pContext->m_pOptions->m_ColorMode = RENDER_COLOR_GRAY;
    pContext->m_pOptions->m_ForeColor = 0;
    pContext->m_pOptions->m_BackColor = 0xffffff;
  }
  const CPDF_OCContext::UsageType usage =
      (flags & FPDF_PRINTING) ? CPDF_OCContext::Print : CPDF_OCContext::View;
  pContext->m_pOptions->m_AddFlags = flags >> 8;
  pContext->m_pOptions->m_pOCContext =
      new CPDF_OCContext(pPage->m_pDocument, usage);

  CFX_AffineMatrix matrix;
  pPage->GetDisplayMatrix(matrix, start_x, start_y, size_x, size_y, rotate);

  FX_RECT clip;
  clip.left = start_x;
  clip.right = start_x + size_x;
  clip.top = start_y;
  clip.bottom = start_y + size_y;
  pContext->m_pDevice->SaveState();
  pContext->m_pDevice->SetClip_Rect(&clip);

  pContext->m_pContext = new CPDF_RenderContext;
  pContext->m_pContext->Create(pPage);
  pContext->m_pContext->AppendObjectList(pPage, &matrix);

  if (flags & FPDF_ANNOT) {
    pContext->m_pAnnots = new CPDF_AnnotList(pPage);
    FX_BOOL bPrinting = pContext->m_pDevice->GetDeviceClass() != FXDC_DISPLAY;
    pContext->m_pAnnots->DisplayAnnots(pPage, pContext->m_pContext, bPrinting,
                                       &matrix, TRUE, NULL);
  }

  pContext->m_pRenderer = new CPDF_ProgressiveRenderer(
      pContext->m_pContext, pContext->m_pDevice, pContext->m_pOptions);
  pContext->m_pRenderer->Start(pause);
  if (bNeedToRestore)
    pContext->m_pDevice->RestoreState();
}

DLLEXPORT int STDCALL FPDF_GetPageSizeByIndex(FPDF_DOCUMENT document,
                                              int page_index,
                                              double* width,
                                              double* height) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return FALSE;

  CPDF_Dictionary* pDict = pDoc->GetPage(page_index);
  if (!pDict)
    return FALSE;

  CPDF_Page page;
  page.Load(pDoc, pDict);
  *width = page.GetPageWidth();
  *height = page.GetPageHeight();

  return TRUE;
}

DLLEXPORT FPDF_BOOL STDCALL
FPDF_VIEWERREF_GetPrintScaling(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return TRUE;
  CPDF_ViewerPreferences viewRef(pDoc);
  return viewRef.PrintScaling();
}

DLLEXPORT int STDCALL FPDF_VIEWERREF_GetNumCopies(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 1;
  CPDF_ViewerPreferences viewRef(pDoc);
  return viewRef.NumCopies();
}

DLLEXPORT FPDF_PAGERANGE STDCALL
FPDF_VIEWERREF_GetPrintPageRange(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return NULL;
  CPDF_ViewerPreferences viewRef(pDoc);
  return viewRef.PrintPageRange();
}

DLLEXPORT FPDF_DUPLEXTYPE STDCALL
FPDF_VIEWERREF_GetDuplex(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return DuplexUndefined;
  CPDF_ViewerPreferences viewRef(pDoc);
  CFX_ByteString duplex = viewRef.Duplex();
  if (FX_BSTRC("Simplex") == duplex)
    return Simplex;
  if (FX_BSTRC("DuplexFlipShortEdge") == duplex)
    return DuplexFlipShortEdge;
  if (FX_BSTRC("DuplexFlipLongEdge") == duplex)
    return DuplexFlipLongEdge;
  return DuplexUndefined;
}

DLLEXPORT FPDF_DWORD STDCALL FPDF_CountNamedDests(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;

  CPDF_Dictionary* pRoot = pDoc->GetRoot();
  if (!pRoot)
    return 0;

  CPDF_NameTree nameTree(pDoc, FX_BSTRC("Dests"));
  int count = nameTree.GetCount();
  CPDF_Dictionary* pDest = pRoot->GetDict(FX_BSTRC("Dests"));
  if (pDest)
    count += pDest->GetCount();
  return count;
}

DLLEXPORT FPDF_DEST STDCALL FPDF_GetNamedDestByName(FPDF_DOCUMENT document,
                                                    FPDF_BYTESTRING name) {
  if (!name || name[0] == 0)
    return nullptr;

  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  CPDF_NameTree name_tree(pDoc, FX_BSTRC("Dests"));
  return name_tree.LookupNamedDest(pDoc, name);
}

DLLEXPORT FPDF_DEST STDCALL FPDF_GetNamedDest(FPDF_DOCUMENT document,
                                              int index,
                                              void* buffer,
                                              long* buflen) {
  if (!buffer)
    *buflen = 0;
  if (!document || index < 0)
    return NULL;
  CPDF_Document* pDoc = (CPDF_Document*)document;

  CPDF_Dictionary* pRoot = pDoc->GetRoot();
  if (!pRoot)
    return NULL;

  CPDF_Object* pDestObj = NULL;
  CFX_ByteString bsName;
  CPDF_NameTree nameTree(pDoc, FX_BSTRC("Dests"));
  int count = nameTree.GetCount();
  if (index >= count) {
    CPDF_Dictionary* pDest = pRoot->GetDict(FX_BSTRC("Dests"));
    if (!pDest)
      return NULL;
    if (index >= count + pDest->GetCount())
      return NULL;
    index -= count;
    FX_POSITION pos = pDest->GetStartPos();
    int i = 0;
    while (pos) {
      pDestObj = pDest->GetNextElement(pos, bsName);
      if (!pDestObj)
        continue;
      if (i == index)
        break;
      i++;
    }
  } else {
    pDestObj = nameTree.LookupValue(index, bsName);
  }
  if (!pDestObj)
    return nullptr;
  if (CPDF_Dictionary* pDict = pDestObj->AsDictionary()) {
    pDestObj = pDict->GetArray(FX_BSTRC("D"));
    if (!pDestObj)
      return nullptr;
  }
  if (!pDestObj->IsArray())
    return nullptr;

  CFX_WideString wsName = PDF_DecodeText(bsName);
  CFX_ByteString utf16Name = wsName.UTF16LE_Encode();
  unsigned int len = utf16Name.GetLength();
  if (!buffer) {
    *buflen = len;
  } else if (*buflen >= len) {
    memcpy(buffer, utf16Name.c_str(), len);
    *buflen = len;
  } else {
    *buflen = -1;
  }
  return (FPDF_DEST)pDestObj;
}
