// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdfview.h"

#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_occontext.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfapi/render/cpdf_docrenderdata.h"
#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "core/fpdfapi/render/cpdf_pagerendercontext.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fpdfdoc/cpdf_nametree.h"
#include "core/fpdfdoc/cpdf_viewerpreferences.h"
#include "core/fxcrt/cfx_readonlymemorystream.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_renderdevice.h"
#include "fpdfsdk/cpdfsdk_customaccess.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_renderpage.h"
#include "fxjs/ijs_runtime.h"
#include "public/fpdf_formfill.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/span.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_page.h"
#include "fxbarcode/BC_Library.h"
#endif  // PDF_ENABLE_XFA

#if defined(OS_WIN)
#include "core/fpdfapi/render/cpdf_progressiverenderer.h"
#include "core/fpdfapi/render/cpdf_windowsrenderdevice.h"
#include "public/fpdf_edit.h"

// These checks are here because core/ and public/ cannot depend on each other.
static_assert(WindowsPrintMode::kModeEmf == FPDF_PRINTMODE_EMF,
              "WindowsPrintMode::kModeEmf value mismatch");
static_assert(WindowsPrintMode::kModeTextOnly == FPDF_PRINTMODE_TEXTONLY,
              "WindowsPrintMode::kModeTextOnly value mismatch");
static_assert(WindowsPrintMode::kModePostScript2 == FPDF_PRINTMODE_POSTSCRIPT2,
              "WindowsPrintMode::kModePostScript2 value mismatch");
static_assert(WindowsPrintMode::kModePostScript3 == FPDF_PRINTMODE_POSTSCRIPT3,
              "WindowsPrintMode::kModePostScript3 value mismatch");
static_assert(WindowsPrintMode::kModePostScript2PassThrough ==
                  FPDF_PRINTMODE_POSTSCRIPT2_PASSTHROUGH,
              "WindowsPrintMode::kModePostScript2PassThrough value mismatch");
static_assert(WindowsPrintMode::kModePostScript3PassThrough ==
                  FPDF_PRINTMODE_POSTSCRIPT3_PASSTHROUGH,
              "WindowsPrintMode::kModePostScript3PassThrough value mismatch");
#endif  // defined(OS_WIN)

namespace {

bool g_bLibraryInitialized = false;

FPDF_DOCUMENT LoadDocumentImpl(
    const RetainPtr<IFX_SeekableReadStream>& pFileAccess,
    FPDF_BYTESTRING password) {
  if (!pFileAccess) {
    ProcessParseError(CPDF_Parser::FILE_ERROR);
    return nullptr;
  }

  auto pDocument = pdfium::MakeUnique<CPDF_Document>(
      pdfium::MakeUnique<CPDF_DocRenderData>(),
      pdfium::MakeUnique<CPDF_DocPageData>());

  CPDF_Parser::Error error = pDocument->LoadDoc(pFileAccess, password);
  if (error != CPDF_Parser::SUCCESS) {
    ProcessParseError(error);
    return nullptr;
  }

  ReportUnsupportedFeatures(pDocument.get());
  return FPDFDocumentFromCPDFDocument(pDocument.release());
}

}  // namespace

FPDF_EXPORT void FPDF_CALLCONV FPDF_InitLibrary() {
  FPDF_InitLibraryWithConfig(nullptr);
}

FPDF_EXPORT void FPDF_CALLCONV
FPDF_InitLibraryWithConfig(const FPDF_LIBRARY_CONFIG* config) {
  if (g_bLibraryInitialized)
    return;

  FXMEM_InitializePartitionAlloc();
  CFX_GEModule::Create(config ? config->m_pUserFontPaths : nullptr);
  CPDF_PageModule::Create();

#ifdef PDF_ENABLE_XFA
  BC_Library_Init();
#endif  // PDF_ENABLE_XFA
  if (config && config->version >= 2)
    IJS_Runtime::Initialize(config->m_v8EmbedderSlot, config->m_pIsolate);

  g_bLibraryInitialized = true;
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_DestroyLibrary() {
  if (!g_bLibraryInitialized)
    return;

#ifdef PDF_ENABLE_XFA
  BC_Library_Destroy();
#endif  // PDF_ENABLE_XFA

  CPDF_PageModule::Destroy();
  CFX_GEModule::Destroy();
  IJS_Runtime::Destroy();

  g_bLibraryInitialized = false;
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_SetSandBoxPolicy(FPDF_DWORD policy,
                                                     FPDF_BOOL enable) {
  return SetPDFSandboxPolicy(policy, enable);
}

#if defined(OS_WIN)
#if defined(PDFIUM_PRINT_TEXT_WITH_GDI)
FPDF_EXPORT void FPDF_CALLCONV
FPDF_SetTypefaceAccessibleFunc(PDFiumEnsureTypefaceCharactersAccessible func) {
  g_pdfium_typeface_accessible_func = func;
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_SetPrintTextWithGDI(FPDF_BOOL use_gdi) {
  g_pdfium_print_text_with_gdi = !!use_gdi;
}
#endif  // PDFIUM_PRINT_TEXT_WITH_GDI

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_SetPrintMode(int mode) {
  if (mode < FPDF_PRINTMODE_EMF ||
      mode > FPDF_PRINTMODE_POSTSCRIPT3_PASSTHROUGH) {
    return FALSE;
  }
  g_pdfium_print_mode = static_cast<WindowsPrintMode>(mode);
  return TRUE;
}
#endif  // defined(OS_WIN)

FPDF_EXPORT FPDF_DOCUMENT FPDF_CALLCONV
FPDF_LoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password) {
  // NOTE: the creation of the file needs to be by the embedder on the
  // other side of this API.
  return LoadDocumentImpl(IFX_SeekableReadStream::CreateFromFilename(file_path),
                          password);
}

FPDF_EXPORT int FPDF_CALLCONV FPDF_GetFormType(FPDF_DOCUMENT document) {
  const CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return FORMTYPE_NONE;

  const CPDF_Dictionary* pRoot = pDoc->GetRoot();
  if (!pRoot)
    return FORMTYPE_NONE;

  const CPDF_Dictionary* pAcroForm = pRoot->GetDictFor("AcroForm");
  if (!pAcroForm)
    return FORMTYPE_NONE;

  const CPDF_Object* pXFA = pAcroForm->GetObjectFor("XFA");
  if (!pXFA)
    return FORMTYPE_ACRO_FORM;

  bool bNeedsRendering = pRoot->GetBooleanFor("NeedsRendering", false);
  return bNeedsRendering ? FORMTYPE_XFA_FULL : FORMTYPE_XFA_FOREGROUND;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_LoadXFA(FPDF_DOCUMENT document) {
#ifdef PDF_ENABLE_XFA
  auto* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return false;

  auto* pContext = static_cast<CPDFXFA_Context*>(pDoc->GetExtension());
  if (pContext)
    return pContext->LoadXFADoc();
#endif  // PDF_ENABLE_XFA
  return false;
}

FPDF_EXPORT FPDF_DOCUMENT FPDF_CALLCONV
FPDF_LoadMemDocument(const void* data_buf, int size, FPDF_BYTESTRING password) {
  return LoadDocumentImpl(
      pdfium::MakeRetain<CFX_ReadOnlyMemoryStream>(
          pdfium::make_span(static_cast<const uint8_t*>(data_buf), size)),
      password);
}

FPDF_EXPORT FPDF_DOCUMENT FPDF_CALLCONV
FPDF_LoadCustomDocument(FPDF_FILEACCESS* pFileAccess,
                        FPDF_BYTESTRING password) {
  if (!pFileAccess)
    return nullptr;
  return LoadDocumentImpl(pdfium::MakeRetain<CPDFSDK_CustomAccess>(pFileAccess),
                          password);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_GetFileVersion(FPDF_DOCUMENT doc,
                                                        int* fileVersion) {
  if (!fileVersion)
    return false;

  *fileVersion = 0;
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(doc);
  if (!pDoc)
    return false;

  const CPDF_Parser* pParser = pDoc->GetParser();
  if (!pParser)
    return false;

  *fileVersion = pParser->GetFileVersion();
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_DocumentHasValidCrossReferenceTable(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  return pDoc && pDoc->has_valid_cross_reference_table();
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_GetDocPermissions(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  return pDoc ? pDoc->GetUserPermissions() : 0;
}

FPDF_EXPORT int FPDF_CALLCONV
FPDF_GetSecurityHandlerRevision(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc || !pDoc->GetParser())
    return -1;

  const CPDF_Dictionary* pDict = pDoc->GetParser()->GetEncryptDict();
  return pDict ? pDict->GetIntegerFor("R") : -1;
}

FPDF_EXPORT int FPDF_CALLCONV FPDF_GetPageCount(FPDF_DOCUMENT document) {
  auto* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;

  auto* pExtension = pDoc->GetExtension();
  return pExtension ? pExtension->GetPageCount() : pDoc->GetPageCount();
}

FPDF_EXPORT FPDF_PAGE FPDF_CALLCONV FPDF_LoadPage(FPDF_DOCUMENT document,
                                                  int page_index) {
  auto* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  if (page_index < 0 || page_index >= FPDF_GetPageCount(document))
    return nullptr;

#ifdef PDF_ENABLE_XFA
  auto* pContext = static_cast<CPDFXFA_Context*>(pDoc->GetExtension());
  if (pContext)
    return FPDFPageFromIPDFPage(pContext->GetXFAPage(page_index).Leak());
#endif  // PDF_ENABLE_XFA

  CPDF_Dictionary* pDict = pDoc->GetPageDictionary(page_index);
  if (!pDict)
    return nullptr;

  auto pPage = pdfium::MakeRetain<CPDF_Page>(pDoc, pDict);
  pPage->SetRenderCache(pdfium::MakeUnique<CPDF_PageRenderCache>(pPage.Get()));
  pPage->ParseContent();
  return FPDFPageFromIPDFPage(pPage.Leak());
}

FPDF_EXPORT float FPDF_CALLCONV FPDF_GetPageWidthF(FPDF_PAGE page) {
  IPDF_Page* pPage = IPDFPageFromFPDFPage(page);
  return pPage ? pPage->GetPageWidth() : 0.0f;
}

FPDF_EXPORT double FPDF_CALLCONV FPDF_GetPageWidth(FPDF_PAGE page) {
  return FPDF_GetPageWidthF(page);
}

FPDF_EXPORT float FPDF_CALLCONV FPDF_GetPageHeightF(FPDF_PAGE page) {
  IPDF_Page* pPage = IPDFPageFromFPDFPage(page);
  return pPage ? pPage->GetPageHeight() : 0.0f;
}

FPDF_EXPORT double FPDF_CALLCONV FPDF_GetPageHeight(FPDF_PAGE page) {
  return FPDF_GetPageHeightF(page);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_GetPageBoundingBox(FPDF_PAGE page,
                                                            FS_RECTF* rect) {
  if (!rect)
    return false;

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return false;

  *rect = FSRectFFromCFXFloatRect(pPage->GetBBox());
  return true;
}

#if defined(OS_WIN)
namespace {

const double kEpsilonSize = 0.01f;

void GetScaling(CPDF_Page* pPage,
                int size_x,
                int size_y,
                int rotate,
                double* scale_x,
                double* scale_y) {
  ASSERT(pPage);
  ASSERT(scale_x);
  ASSERT(scale_y);
  double page_width = pPage->GetPageWidth();
  double page_height = pPage->GetPageHeight();
  if (page_width < kEpsilonSize || page_height < kEpsilonSize)
    return;

  if (rotate % 2 == 0) {
    *scale_x = size_x / page_width;
    *scale_y = size_y / page_height;
  } else {
    *scale_x = size_y / page_width;
    *scale_y = size_x / page_height;
  }
}

FX_RECT GetMaskDimensionsAndOffsets(CPDF_Page* pPage,
                                    int start_x,
                                    int start_y,
                                    int size_x,
                                    int size_y,
                                    int rotate,
                                    const CFX_FloatRect& mask_box) {
  double scale_x = 0.0f;
  double scale_y = 0.0f;
  GetScaling(pPage, size_x, size_y, rotate, &scale_x, &scale_y);
  if (scale_x < kEpsilonSize || scale_y < kEpsilonSize)
    return FX_RECT();

  // Compute sizes in page points. Round down to catch the entire bitmap.
  int start_x_bm = static_cast<int>(mask_box.left * scale_x);
  int start_y_bm = static_cast<int>(mask_box.bottom * scale_y);
  int size_x_bm = static_cast<int>(mask_box.right * scale_x + 1.0f) -
                  static_cast<int>(mask_box.left * scale_x);
  int size_y_bm = static_cast<int>(mask_box.top * scale_y + 1.0f) -
                  static_cast<int>(mask_box.bottom * scale_y);

  // Get page rotation
  int page_rotation = pPage->GetPageRotation();

  // Compute offsets
  int offset_x = 0;
  int offset_y = 0;
  if (size_x > size_y)
    std::swap(size_x_bm, size_y_bm);

  switch ((rotate + page_rotation) % 4) {
    case 0:
      offset_x = start_x_bm + start_x;
      offset_y = start_y + size_y - size_y_bm - start_y_bm;
      break;
    case 1:
      offset_x = start_y_bm + start_x;
      offset_y = start_x_bm + start_y;
      break;
    case 2:
      offset_x = start_x + size_x - size_x_bm - start_x_bm;
      offset_y = start_y_bm + start_y;
      break;
    case 3:
      offset_x = start_x + size_x - size_x_bm - start_y_bm;
      offset_y = start_y + size_y - size_y_bm - start_x_bm;
      break;
  }
  return FX_RECT(offset_x, offset_y, offset_x + size_x_bm,
                 offset_y + size_y_bm);
}

// Get a bitmap of just the mask section defined by |mask_box| from a full page
// bitmap |pBitmap|.
RetainPtr<CFX_DIBitmap> GetMaskBitmap(CPDF_Page* pPage,
                                      int start_x,
                                      int start_y,
                                      int size_x,
                                      int size_y,
                                      int rotate,
                                      const RetainPtr<CFX_DIBitmap>& pSrc,
                                      const CFX_FloatRect& mask_box,
                                      FX_RECT* bitmap_area) {
  ASSERT(bitmap_area);
  *bitmap_area = GetMaskDimensionsAndOffsets(pPage, start_x, start_y, size_x,
                                             size_y, rotate, mask_box);
  if (bitmap_area->IsEmpty())
    return nullptr;

  // Create a new bitmap to transfer part of the page bitmap to.
  RetainPtr<CFX_DIBitmap> pDst = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pDst->Create(bitmap_area->Width(), bitmap_area->Height(), FXDIB_Argb))
    return nullptr;

  pDst->Clear(0x00ffffff);
  pDst->TransferBitmap(0, 0, bitmap_area->Width(), bitmap_area->Height(), pSrc,
                       bitmap_area->left, bitmap_area->top);
  return pDst;
}

void RenderBitmap(CFX_RenderDevice* device,
                  const RetainPtr<CFX_DIBitmap>& pSrc,
                  const FX_RECT& mask_area) {
  int size_x_bm = mask_area.Width();
  int size_y_bm = mask_area.Height();
  if (size_x_bm == 0 || size_y_bm == 0)
    return;

  // Create a new bitmap from the old one
  RetainPtr<CFX_DIBitmap> pDst = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pDst->Create(size_x_bm, size_y_bm, FXDIB_Rgb32))
    return;

  pDst->Clear(0xffffffff);
  pDst->CompositeBitmap(0, 0, size_x_bm, size_y_bm, pSrc, 0, 0,
                        BlendMode::kNormal, nullptr, false);

  if (device->GetDeviceType() == DeviceType::kPrinter) {
    device->StretchDIBits(pDst, mask_area.left, mask_area.top, size_x_bm,
                          size_y_bm);
  } else {
    device->SetDIBits(pDst, mask_area.left, mask_area.top);
  }
}

}  // namespace

FPDF_EXPORT void FPDF_CALLCONV FPDF_RenderPage(HDC dc,
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

  auto pOwnedContext = pdfium::MakeUnique<CPDF_PageRenderContext>();
  CPDF_PageRenderContext* pContext = pOwnedContext.get();
  CPDF_Page::RenderContextClearer clearer(pPage);
  pPage->SetRenderContext(std::move(pOwnedContext));

  // Don't render the full page to bitmap for a mask unless there are a lot
  // of masks. Full page bitmaps result in large spool sizes, so they should
  // only be used when necessary. For large numbers of masks, rendering each
  // individually is inefficient and unlikely to significantly improve spool
  // size. TODO(rbpotter): Find out why this still breaks printing for some
  // PDFs (see crbug.com/777837).
  const bool bEnableImageMasks = false;
  const bool bNewBitmap = pPage->BackgroundAlphaNeeded() ||
                          (pPage->HasImageMask() && !bEnableImageMasks) ||
                          pPage->GetMaskBoundingBoxes().size() > 100;
  const bool bHasMask = pPage->HasImageMask() && !bNewBitmap;
  if (!bNewBitmap && !bHasMask) {
    pContext->m_pDevice = pdfium::MakeUnique<CPDF_WindowsRenderDevice>(dc);
    CPDFSDK_RenderPageWithContext(pContext, pPage, start_x, start_y, size_x,
                                  size_y, rotate, flags,
                                  /*need_to_restore=*/true, /*pause=*/nullptr);
    return;
  }

  RetainPtr<CFX_DIBitmap> pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  // Create will probably work fine even if it fails here: we will just attach
  // a zero-sized bitmap to |pDevice|.
  pBitmap->Create(size_x, size_y, FXDIB_Argb);
  pBitmap->Clear(0x00ffffff);
  CFX_DefaultRenderDevice* pDevice = new CFX_DefaultRenderDevice;
  pContext->m_pDevice = pdfium::WrapUnique(pDevice);
  pDevice->Attach(pBitmap, false, nullptr, false);
  if (bHasMask) {
    pContext->m_pOptions = pdfium::MakeUnique<CPDF_RenderOptions>();
    pContext->m_pOptions->GetOptions().bBreakForMasks = true;
  }

  CPDFSDK_RenderPageWithContext(pContext, pPage, start_x, start_y, size_x,
                                size_y, rotate, flags, /*need_to_restore=*/true,
                                /*pause=*/nullptr);

  if (!bHasMask) {
    CPDF_WindowsRenderDevice WinDC(dc);
    bool bitsStretched = false;
    if (WinDC.GetDeviceType() == DeviceType::kPrinter) {
      auto pDst = pdfium::MakeRetain<CFX_DIBitmap>();
      if (pDst->Create(size_x, size_y, FXDIB_Rgb32)) {
        memset(pDst->GetBuffer(), -1, pBitmap->GetPitch() * size_y);
        pDst->CompositeBitmap(0, 0, size_x, size_y, pBitmap, 0, 0,
                              BlendMode::kNormal, nullptr, false);
        WinDC.StretchDIBits(pDst, 0, 0, size_x, size_y);
        bitsStretched = true;
      }
    }
    if (!bitsStretched)
      WinDC.SetDIBits(pBitmap, 0, 0);
    return;
  }

  // Finish rendering the page to bitmap and copy the correct segments
  // of the page to individual image mask bitmaps.
  const std::vector<CFX_FloatRect>& mask_boxes = pPage->GetMaskBoundingBoxes();
  std::vector<FX_RECT> bitmap_areas(mask_boxes.size());
  std::vector<RetainPtr<CFX_DIBitmap>> bitmaps(mask_boxes.size());
  for (size_t i = 0; i < mask_boxes.size(); i++) {
    bitmaps[i] = GetMaskBitmap(pPage, start_x, start_y, size_x, size_y, rotate,
                               pBitmap, mask_boxes[i], &bitmap_areas[i]);
    pContext->m_pRenderer->Continue(nullptr);
  }

  // Begin rendering to the printer. Add flag to indicate the renderer should
  // pause after each image mask.
  pOwnedContext = pdfium::MakeUnique<CPDF_PageRenderContext>();
  pContext = pOwnedContext.get();
  pPage->SetRenderContext(std::move(pOwnedContext));
  pContext->m_pDevice = pdfium::MakeUnique<CPDF_WindowsRenderDevice>(dc);
  pContext->m_pOptions = pdfium::MakeUnique<CPDF_RenderOptions>();
  pContext->m_pOptions->GetOptions().bBreakForMasks = true;

  CPDFSDK_RenderPageWithContext(pContext, pPage, start_x, start_y, size_x,
                                size_y, rotate, flags, /*need_to_restore=*/true,
                                /*pause=*/nullptr);

  // Render masks
  for (size_t i = 0; i < mask_boxes.size(); i++) {
    // Render the bitmap for the mask and free the bitmap.
    if (bitmaps[i]) {  // will be null if mask has zero area
      RenderBitmap(pContext->m_pDevice.get(), bitmaps[i], bitmap_areas[i]);
    }
    // Render the next portion of page.
    pContext->m_pRenderer->Continue(nullptr);
  }
}
#endif  // defined(OS_WIN)

FPDF_EXPORT void FPDF_CALLCONV FPDF_RenderPageBitmap(FPDF_BITMAP bitmap,
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

  auto pOwnedContext = pdfium::MakeUnique<CPDF_PageRenderContext>();
  CPDF_PageRenderContext* pContext = pOwnedContext.get();
  CPDF_Page::RenderContextClearer clearer(pPage);
  pPage->SetRenderContext(std::move(pOwnedContext));

  auto pOwnedDevice = pdfium::MakeUnique<CFX_DefaultRenderDevice>();
  CFX_DefaultRenderDevice* pDevice = pOwnedDevice.get();
  pContext->m_pDevice = std::move(pOwnedDevice);

  RetainPtr<CFX_DIBitmap> pBitmap(CFXDIBitmapFromFPDFBitmap(bitmap));
  pDevice->Attach(pBitmap, !!(flags & FPDF_REVERSE_BYTE_ORDER), nullptr, false);
  CPDFSDK_RenderPageWithContext(pContext, pPage, start_x, start_y, size_x,
                                size_y, rotate, flags, /*need_to_restore=*/true,
                                /*pause=*/nullptr);

#ifdef _SKIA_SUPPORT_PATHS_
  pDevice->Flush(true);
  pBitmap->UnPreMultiply();
#endif
}

FPDF_EXPORT void FPDF_CALLCONV
FPDF_RenderPageBitmapWithMatrix(FPDF_BITMAP bitmap,
                                FPDF_PAGE page,
                                const FS_MATRIX* matrix,
                                const FS_RECTF* clipping,
                                int flags) {
  if (!bitmap)
    return;

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return;

  auto pOwnedContext = pdfium::MakeUnique<CPDF_PageRenderContext>();
  CPDF_PageRenderContext* pContext = pOwnedContext.get();
  CPDF_Page::RenderContextClearer clearer(pPage);
  pPage->SetRenderContext(std::move(pOwnedContext));

  auto pOwnedDevice = pdfium::MakeUnique<CFX_DefaultRenderDevice>();
  CFX_DefaultRenderDevice* pDevice = pOwnedDevice.get();
  pContext->m_pDevice = std::move(pOwnedDevice);

  RetainPtr<CFX_DIBitmap> pBitmap(CFXDIBitmapFromFPDFBitmap(bitmap));
  pDevice->Attach(pBitmap, !!(flags & FPDF_REVERSE_BYTE_ORDER), nullptr, false);

  CFX_FloatRect clipping_rect;
  if (clipping)
    clipping_rect = CFXFloatRectFromFSRectF(*clipping);
  FX_RECT clip_rect = clipping_rect.ToFxRect();

  const FX_RECT rect(0, 0, pPage->GetPageWidth(), pPage->GetPageHeight());
  CFX_Matrix transform_matrix = pPage->GetDisplayMatrix(rect, 0);
  if (matrix)
    transform_matrix *= CFXMatrixFromFSMatrix(*matrix);
  CPDFSDK_RenderPage(pContext, pPage, transform_matrix, clip_rect, flags);
}

#ifdef _SKIA_SUPPORT_
FPDF_EXPORT FPDF_RECORDER FPDF_CALLCONV FPDF_RenderPageSkp(FPDF_PAGE page,
                                                           int size_x,
                                                           int size_y) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return nullptr;

  auto pOwnedContext = pdfium::MakeUnique<CPDF_PageRenderContext>();
  CPDF_PageRenderContext* pContext = pOwnedContext.get();
  CPDF_Page::RenderContextClearer clearer(pPage);
  pPage->SetRenderContext(std::move(pOwnedContext));

  auto skDevice = pdfium::MakeUnique<CFX_DefaultRenderDevice>();
  FPDF_RECORDER recorder = skDevice->CreateRecorder(size_x, size_y);
  pContext->m_pDevice = std::move(skDevice);

  CPDFSDK_RenderPageWithContext(pContext, pPage, 0, 0, size_x, size_y, 0, 0,
                                /*need_to_restore=*/true, /*pause=*/nullptr);
  return recorder;
}
#endif  // _SKIA_SUPPORT_

FPDF_EXPORT void FPDF_CALLCONV FPDF_ClosePage(FPDF_PAGE page) {
  if (!page)
    return;

  // Take it back across the API and hold for duration of this function.
  RetainPtr<IPDF_Page> pPage;
  pPage.Unleak(IPDFPageFromFPDFPage(page));

  if (pPage->AsXFAPage())
    return;

  CPDFSDK_PageView* pPageView =
      static_cast<CPDFSDK_PageView*>(pPage->AsPDFPage()->GetView());
  if (!pPageView || pPageView->IsBeingDestroyed())
    return;

  if (pPageView->IsLocked()) {
    pPageView->TakePageOwnership();
    return;
  }

  // This will delete the |pPageView| object. We must cleanup the PageView
  // first because it will attempt to reset the View on the |pPage| during
  // destruction.
  pPageView->GetFormFillEnv()->RemovePageView(pPage.Get());
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_CloseDocument(FPDF_DOCUMENT document) {
  // Take it back across the API and throw it away,
  std::unique_ptr<CPDF_Document>(CPDFDocumentFromFPDFDocument(document));
}

FPDF_EXPORT unsigned long FPDF_CALLCONV FPDF_GetLastError() {
  return FXSYS_GetLastError();
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_DeviceToPage(FPDF_PAGE page,
                                                      int start_x,
                                                      int start_y,
                                                      int size_x,
                                                      int size_y,
                                                      int rotate,
                                                      int device_x,
                                                      int device_y,
                                                      double* page_x,
                                                      double* page_y) {
  if (!page || !page_x || !page_y)
    return false;

  IPDF_Page* pPage = IPDFPageFromFPDFPage(page);
  const FX_RECT rect(start_x, start_y, start_x + size_x, start_y + size_y);
  Optional<CFX_PointF> pos =
      pPage->DeviceToPage(rect, rotate, CFX_PointF(device_x, device_y));
  if (!pos)
    return false;

  *page_x = pos->x;
  *page_y = pos->y;
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_PageToDevice(FPDF_PAGE page,
                                                      int start_x,
                                                      int start_y,
                                                      int size_x,
                                                      int size_y,
                                                      int rotate,
                                                      double page_x,
                                                      double page_y,
                                                      int* device_x,
                                                      int* device_y) {
  if (!page || !device_x || !device_y)
    return false;

  IPDF_Page* pPage = IPDFPageFromFPDFPage(page);
  const FX_RECT rect(start_x, start_y, start_x + size_x, start_y + size_y);
  CFX_PointF page_point(static_cast<float>(page_x), static_cast<float>(page_y));
  Optional<CFX_PointF> pos = pPage->PageToDevice(rect, rotate, page_point);
  if (!pos)
    return false;

  *device_x = FXSYS_roundf(pos->x);
  *device_y = FXSYS_roundf(pos->y);
  return true;
}

FPDF_EXPORT FPDF_BITMAP FPDF_CALLCONV FPDFBitmap_Create(int width,
                                                        int height,
                                                        int alpha) {
  auto pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pBitmap->Create(width, height, alpha ? FXDIB_Argb : FXDIB_Rgb32))
    return nullptr;

  return FPDFBitmapFromCFXDIBitmap(pBitmap.Leak());
}

FPDF_EXPORT FPDF_BITMAP FPDF_CALLCONV FPDFBitmap_CreateEx(int width,
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
      return nullptr;
  }

  // Ensure external memory is good at least for the duration of this call.
  UnownedPtr<uint8_t> pChecker(static_cast<uint8_t*>(first_scan));
  auto pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pBitmap->Create(width, height, fx_format, pChecker.Get(), stride))
    return nullptr;

  return FPDFBitmapFromCFXDIBitmap(pBitmap.Leak());
}

FPDF_EXPORT int FPDF_CALLCONV FPDFBitmap_GetFormat(FPDF_BITMAP bitmap) {
  if (!bitmap)
    return FPDFBitmap_Unknown;

  FXDIB_Format format = CFXDIBitmapFromFPDFBitmap(bitmap)->GetFormat();
  switch (format) {
    case FXDIB_8bppRgb:
    case FXDIB_8bppMask:
      return FPDFBitmap_Gray;
    case FXDIB_Rgb:
      return FPDFBitmap_BGR;
    case FXDIB_Rgb32:
      return FPDFBitmap_BGRx;
    case FXDIB_Argb:
      return FPDFBitmap_BGRA;
    default:
      return FPDFBitmap_Unknown;
  }
}

FPDF_EXPORT void FPDF_CALLCONV FPDFBitmap_FillRect(FPDF_BITMAP bitmap,
                                                   int left,
                                                   int top,
                                                   int width,
                                                   int height,
                                                   FPDF_DWORD color) {
  if (!bitmap)
    return;

  CFX_DefaultRenderDevice device;
  RetainPtr<CFX_DIBitmap> pBitmap(CFXDIBitmapFromFPDFBitmap(bitmap));
  device.Attach(pBitmap, false, nullptr, false);
  if (!pBitmap->HasAlpha())
    color |= 0xFF000000;
  device.FillRect(FX_RECT(left, top, left + width, top + height), color);
}

FPDF_EXPORT void* FPDF_CALLCONV FPDFBitmap_GetBuffer(FPDF_BITMAP bitmap) {
  return bitmap ? CFXDIBitmapFromFPDFBitmap(bitmap)->GetBuffer() : nullptr;
}

FPDF_EXPORT int FPDF_CALLCONV FPDFBitmap_GetWidth(FPDF_BITMAP bitmap) {
  return bitmap ? CFXDIBitmapFromFPDFBitmap(bitmap)->GetWidth() : 0;
}

FPDF_EXPORT int FPDF_CALLCONV FPDFBitmap_GetHeight(FPDF_BITMAP bitmap) {
  return bitmap ? CFXDIBitmapFromFPDFBitmap(bitmap)->GetHeight() : 0;
}

FPDF_EXPORT int FPDF_CALLCONV FPDFBitmap_GetStride(FPDF_BITMAP bitmap) {
  return bitmap ? CFXDIBitmapFromFPDFBitmap(bitmap)->GetPitch() : 0;
}

FPDF_EXPORT void FPDF_CALLCONV FPDFBitmap_Destroy(FPDF_BITMAP bitmap) {
  RetainPtr<CFX_DIBitmap> destroyer;
  destroyer.Unleak(CFXDIBitmapFromFPDFBitmap(bitmap));
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_GetPageSizeByIndexF(FPDF_DOCUMENT document,
                         int page_index,
                         FS_SIZEF* size) {
  if (!size)
    return false;

  auto* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return false;

#ifdef PDF_ENABLE_XFA
  if (page_index < 0 || page_index >= FPDF_GetPageCount(document))
    return false;

  auto* pContext = static_cast<CPDFXFA_Context*>(pDoc->GetExtension());
  if (pContext) {
    RetainPtr<CPDFXFA_Page> pPage = pContext->GetXFAPage(page_index);
    if (!pPage)
      return false;

    size->width = pPage->GetPageWidth();
    size->height = pPage->GetPageHeight();
    return true;
  }
#endif  // PDF_ENABLE_XFA

  CPDF_Dictionary* pDict = pDoc->GetPageDictionary(page_index);
  if (!pDict)
    return false;

  auto page = pdfium::MakeRetain<CPDF_Page>(pDoc, pDict);
  page->SetRenderCache(pdfium::MakeUnique<CPDF_PageRenderCache>(page.Get()));
  size->width = page->GetPageWidth();
  size->height = page->GetPageHeight();
  return true;
}

FPDF_EXPORT int FPDF_CALLCONV FPDF_GetPageSizeByIndex(FPDF_DOCUMENT document,
                                                      int page_index,
                                                      double* width,
                                                      double* height) {
  if (!width || !height)
    return false;

  FS_SIZEF size;
  if (!FPDF_GetPageSizeByIndexF(document, page_index, &size))
    return false;

  *width = size.width;
  *height = size.height;
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_VIEWERREF_GetPrintScaling(FPDF_DOCUMENT document) {
  const CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return true;
  CPDF_ViewerPreferences viewRef(pDoc);
  return viewRef.PrintScaling();
}

FPDF_EXPORT int FPDF_CALLCONV
FPDF_VIEWERREF_GetNumCopies(FPDF_DOCUMENT document) {
  const CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 1;
  CPDF_ViewerPreferences viewRef(pDoc);
  return viewRef.NumCopies();
}

FPDF_EXPORT FPDF_PAGERANGE FPDF_CALLCONV
FPDF_VIEWERREF_GetPrintPageRange(FPDF_DOCUMENT document) {
  const CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;
  CPDF_ViewerPreferences viewRef(pDoc);
  return FPDFPageRangeFromCPDFArray(viewRef.PrintPageRange());
}

FPDF_EXPORT size_t FPDF_CALLCONV
FPDF_VIEWERREF_GetPrintPageRangeCount(FPDF_PAGERANGE pagerange) {
  const CPDF_Array* pArray = CPDFArrayFromFPDFPageRange(pagerange);
  return pArray ? pArray->size() : 0;
}

FPDF_EXPORT int FPDF_CALLCONV
FPDF_VIEWERREF_GetPrintPageRangeElement(FPDF_PAGERANGE pagerange,
                                        size_t index) {
  const CPDF_Array* pArray = CPDFArrayFromFPDFPageRange(pagerange);
  if (!pArray || index >= pArray->size())
    return -1;
  return pArray->GetIntegerAt(index);
}

FPDF_EXPORT FPDF_DUPLEXTYPE FPDF_CALLCONV
FPDF_VIEWERREF_GetDuplex(FPDF_DOCUMENT document) {
  const CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return DuplexUndefined;
  CPDF_ViewerPreferences viewRef(pDoc);
  ByteString duplex = viewRef.Duplex();
  if ("Simplex" == duplex)
    return Simplex;
  if ("DuplexFlipShortEdge" == duplex)
    return DuplexFlipShortEdge;
  if ("DuplexFlipLongEdge" == duplex)
    return DuplexFlipLongEdge;
  return DuplexUndefined;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_VIEWERREF_GetName(FPDF_DOCUMENT document,
                       FPDF_BYTESTRING key,
                       char* buffer,
                       unsigned long length) {
  const CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;

  CPDF_ViewerPreferences viewRef(pDoc);
  Optional<ByteString> bsVal = viewRef.GenericName(key);
  if (!bsVal)
    return 0;

  unsigned long dwStringLen = bsVal->GetLength() + 1;
  if (buffer && length >= dwStringLen)
    memcpy(buffer, bsVal->c_str(), dwStringLen);
  return dwStringLen;
}

FPDF_EXPORT FPDF_DWORD FPDF_CALLCONV
FPDF_CountNamedDests(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;

  const CPDF_Dictionary* pRoot = pDoc->GetRoot();
  if (!pRoot)
    return 0;

  CPDF_NameTree name_tree(pDoc, "Dests");
  pdfium::base::CheckedNumeric<FPDF_DWORD> count = name_tree.GetCount();
  const CPDF_Dictionary* pDest = pRoot->GetDictFor("Dests");
  if (pDest)
    count += pDest->size();

  if (!count.IsValid())
    return 0;

  return count.ValueOrDie();
}

FPDF_EXPORT FPDF_DEST FPDF_CALLCONV
FPDF_GetNamedDestByName(FPDF_DOCUMENT document, FPDF_BYTESTRING name) {
  if (!name || name[0] == 0)
    return nullptr;

  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  CPDF_NameTree name_tree(pDoc, "Dests");
  ByteStringView name_view(name);
  return FPDFDestFromCPDFArray(
      name_tree.LookupNamedDest(pDoc, PDF_DecodeText(name_view.raw_span())));
}

#ifdef PDF_ENABLE_V8
FPDF_EXPORT const char* FPDF_CALLCONV FPDF_GetRecommendedV8Flags() {
  // Reduce exposure since no PDF should contain web assembly.
  // Use interpreted JS only to avoid RWX pages in our address space.
  return "--no-expose-wasm --jitless";
}
#endif  // PDF_ENABLE_V8

#ifdef PDF_ENABLE_XFA
FPDF_EXPORT FPDF_RESULT FPDF_CALLCONV FPDF_BStr_Init(FPDF_BSTR* bstr) {
  if (!bstr)
    return -1;

  bstr->str = nullptr;
  bstr->len = 0;
  return 0;
}

FPDF_EXPORT FPDF_RESULT FPDF_CALLCONV FPDF_BStr_Set(FPDF_BSTR* bstr,
                                                    const char* cstr,
                                                    int length) {
  if (!bstr || !cstr)
    return -1;

  if (length == -1)
    length = strlen(cstr);

  if (length == 0) {
    FPDF_BStr_Clear(bstr);
    return 0;
  }

  if (bstr->str && bstr->len < length)
    bstr->str = FX_Realloc(char, bstr->str, length + 1);
  else if (!bstr->str)
    bstr->str = FX_Alloc(char, length + 1);

  bstr->str[length] = 0;
  memcpy(bstr->str, cstr, length);
  bstr->len = length;
  return 0;
}

FPDF_EXPORT FPDF_RESULT FPDF_CALLCONV FPDF_BStr_Clear(FPDF_BSTR* bstr) {
  if (!bstr)
    return -1;

  if (bstr->str) {
    FX_Free(bstr->str);
    bstr->str = nullptr;
  }
  bstr->len = 0;
  return 0;
}
#endif  // PDF_ENABLE_XFA

FPDF_EXPORT FPDF_DEST FPDF_CALLCONV FPDF_GetNamedDest(FPDF_DOCUMENT document,
                                                      int index,
                                                      void* buffer,
                                                      long* buflen) {
  if (!buffer)
    *buflen = 0;

  if (index < 0)
    return nullptr;

  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  const CPDF_Dictionary* pRoot = pDoc->GetRoot();
  if (!pRoot)
    return nullptr;

  CPDF_Object* pDestObj = nullptr;
  WideString wsName;
  CPDF_NameTree name_tree(pDoc, "Dests");
  int count = name_tree.GetCount();
  if (index >= count) {
    const CPDF_Dictionary* pDest = pRoot->GetDictFor("Dests");
    if (!pDest)
      return nullptr;

    pdfium::base::CheckedNumeric<int> checked_count = count;
    checked_count += pDest->size();
    if (!checked_count.IsValid() || index >= checked_count.ValueOrDie())
      return nullptr;

    index -= count;
    int i = 0;
    ByteStringView bsName;
    CPDF_DictionaryLocker locker(pDest);
    for (const auto& it : locker) {
      bsName = it.first.AsStringView();
      pDestObj = it.second.Get();
      if (!pDestObj)
        continue;
      if (i == index)
        break;
      i++;
    }
    wsName = PDF_DecodeText(bsName.raw_span());
  } else {
    pDestObj = name_tree.LookupValueAndName(index, &wsName);
  }
  if (!pDestObj)
    return nullptr;
  if (CPDF_Dictionary* pDict = pDestObj->AsDictionary()) {
    pDestObj = pDict->GetArrayFor("D");
    if (!pDestObj)
      return nullptr;
  }
  if (!pDestObj->IsArray())
    return nullptr;

  ByteString utf16Name = wsName.ToUTF16LE();
  int len = utf16Name.GetLength();
  if (!buffer) {
    *buflen = len;
  } else if (len <= *buflen) {
    memcpy(buffer, utf16Name.c_str(), len);
    *buflen = len;
  } else {
    *buflen = -1;
  }
  return FPDFDestFromCPDFArray(pDestObj->AsArray());
}
