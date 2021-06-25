// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_edit.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/render/cpdf_imagerenderer.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fpdfapi/render/cpdf_renderstatus.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "fpdfsdk/cpdfsdk_customaccess.h"
#include "fpdfsdk/cpdfsdk_helpers.h"

namespace {

// These checks ensure the consistency of colorspace values across core/ and
// public/.
static_assert(static_cast<int>(CPDF_ColorSpace::Family::kDeviceGray) ==
                  FPDF_COLORSPACE_DEVICEGRAY,
              "kDeviceGray value mismatch");
static_assert(static_cast<int>(CPDF_ColorSpace::Family::kDeviceRGB) ==
                  FPDF_COLORSPACE_DEVICERGB,
              "kDeviceRGB value mismatch");
static_assert(static_cast<int>(CPDF_ColorSpace::Family::kDeviceCMYK) ==
                  FPDF_COLORSPACE_DEVICECMYK,
              "kDeviceCMYK value mismatch");
static_assert(static_cast<int>(CPDF_ColorSpace::Family::kCalGray) ==
                  FPDF_COLORSPACE_CALGRAY,
              "kCalGray value mismatch");
static_assert(static_cast<int>(CPDF_ColorSpace::Family::kCalRGB) ==
                  FPDF_COLORSPACE_CALRGB,
              "kCalRGB value mismatch");
static_assert(static_cast<int>(CPDF_ColorSpace::Family::kLab) ==
                  FPDF_COLORSPACE_LAB,
              "kLab value mismatch");
static_assert(static_cast<int>(CPDF_ColorSpace::Family::kICCBased) ==
                  FPDF_COLORSPACE_ICCBASED,
              "kICCBased value mismatch");
static_assert(static_cast<int>(CPDF_ColorSpace::Family::kSeparation) ==
                  FPDF_COLORSPACE_SEPARATION,
              "kSeparation value mismatch");
static_assert(static_cast<int>(CPDF_ColorSpace::Family::kDeviceN) ==
                  FPDF_COLORSPACE_DEVICEN,
              "kDeviceN value mismatch");
static_assert(static_cast<int>(CPDF_ColorSpace::Family::kIndexed) ==
                  FPDF_COLORSPACE_INDEXED,
              "kIndexed value mismatch");
static_assert(static_cast<int>(CPDF_ColorSpace::Family::kPattern) ==
                  FPDF_COLORSPACE_PATTERN,
              "kPattern value mismatch");

RetainPtr<IFX_SeekableReadStream> MakeSeekableReadStream(
    FPDF_FILEACCESS* pFileAccess) {
  return pdfium::MakeRetain<CPDFSDK_CustomAccess>(pFileAccess);
}

CPDF_ImageObject* CPDFImageObjectFromFPDFPageObject(
    FPDF_PAGEOBJECT image_object) {
  CPDF_PageObject* pPageObject = CPDFPageObjectFromFPDFPageObject(image_object);
  return pPageObject ? pPageObject->AsImage() : nullptr;
}

bool LoadJpegHelper(FPDF_PAGE* pages,
                    int count,
                    FPDF_PAGEOBJECT image_object,
                    FPDF_FILEACCESS* file_access,
                    bool inline_jpeg) {
  CPDF_ImageObject* pImgObj = CPDFImageObjectFromFPDFPageObject(image_object);
  if (!pImgObj)
    return false;

  if (!file_access)
    return false;

  if (pages) {
    for (int index = 0; index < count; index++) {
      CPDF_Page* pPage = CPDFPageFromFPDFPage(pages[index]);
      if (pPage)
        pImgObj->GetImage()->ResetCache(pPage);
    }
  }

  RetainPtr<IFX_SeekableReadStream> pFile = MakeSeekableReadStream(file_access);
  if (inline_jpeg)
    pImgObj->GetImage()->SetJpegImageInline(pFile);
  else
    pImgObj->GetImage()->SetJpegImage(pFile);
  pImgObj->SetDirty(true);
  return true;
}

}  // namespace

FPDF_EXPORT FPDF_PAGEOBJECT FPDF_CALLCONV
FPDFPageObj_NewImageObj(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  auto pImageObj = std::make_unique<CPDF_ImageObject>();
  pImageObj->SetImage(pdfium::MakeRetain<CPDF_Image>(pDoc));

  // Caller takes ownership.
  return FPDFPageObjectFromCPDFPageObject(pImageObj.release());
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFImageObj_LoadJpegFile(FPDF_PAGE* pages,
                          int count,
                          FPDF_PAGEOBJECT image_object,
                          FPDF_FILEACCESS* file_access) {
  return LoadJpegHelper(pages, count, image_object, file_access, false);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFImageObj_LoadJpegFileInline(FPDF_PAGE* pages,
                                int count,
                                FPDF_PAGEOBJECT image_object,
                                FPDF_FILEACCESS* file_access) {
  return LoadJpegHelper(pages, count, image_object, file_access, true);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFImageObj_GetMatrix(FPDF_PAGEOBJECT image_object,
                       double* a,
                       double* b,
                       double* c,
                       double* d,
                       double* e,
                       double* f) {
  CPDF_ImageObject* pImgObj = CPDFImageObjectFromFPDFPageObject(image_object);
  if (!pImgObj || !a || !b || !c || !d || !e || !f)
    return false;

  const CFX_Matrix& matrix = pImgObj->matrix();
  *a = matrix.a;
  *b = matrix.b;
  *c = matrix.c;
  *d = matrix.d;
  *e = matrix.e;
  *f = matrix.f;
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFImageObj_SetMatrix(FPDF_PAGEOBJECT image_object,
                       double a,
                       double b,
                       double c,
                       double d,
                       double e,
                       double f) {
  CPDF_ImageObject* pImgObj = CPDFImageObjectFromFPDFPageObject(image_object);
  if (!pImgObj)
    return false;

  pImgObj->SetImageMatrix(CFX_Matrix(
      static_cast<float>(a), static_cast<float>(b), static_cast<float>(c),
      static_cast<float>(d), static_cast<float>(e), static_cast<float>(f)));
  pImgObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFImageObj_SetBitmap(FPDF_PAGE* pages,
                       int count,
                       FPDF_PAGEOBJECT image_object,
                       FPDF_BITMAP bitmap) {
  CPDF_ImageObject* pImgObj = CPDFImageObjectFromFPDFPageObject(image_object);
  if (!pImgObj)
    return false;

  if (!bitmap)
    return false;

  if (pages) {
    for (int index = 0; index < count; index++) {
      CPDF_Page* pPage = CPDFPageFromFPDFPage(pages[index]);
      if (pPage)
        pImgObj->GetImage()->ResetCache(pPage);
    }
  }

  RetainPtr<CFX_DIBitmap> holder(CFXDIBitmapFromFPDFBitmap(bitmap));
  pImgObj->GetImage()->SetImage(holder);
  pImgObj->CalcBoundingBox();
  pImgObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BITMAP FPDF_CALLCONV
FPDFImageObj_GetBitmap(FPDF_PAGEOBJECT image_object) {
  CPDF_ImageObject* pImgObj = CPDFImageObjectFromFPDFPageObject(image_object);
  if (!pImgObj)
    return nullptr;

  RetainPtr<CPDF_Image> pImg = pImgObj->GetImage();
  if (!pImg)
    return nullptr;

  RetainPtr<CFX_DIBBase> pSource = pImg->LoadDIBBase();
  if (!pSource)
    return nullptr;

  RetainPtr<CFX_DIBitmap> pBitmap;
  // If the source image has a representation of 1 bit per pixel, then convert
  // it to a grayscale bitmap having 1 byte per pixel, since bitmaps have no
  // concept of bits. Otherwise, convert the source image to a bitmap directly,
  // retaining its color representation.
  if (pSource->GetBPP() == 1)
    pBitmap = pSource->CloneConvert(FXDIB_Format::k8bppRgb);
  else
    pBitmap = pSource->Clone(nullptr);

  return FPDFBitmapFromCFXDIBitmap(pBitmap.Leak());
}

FPDF_EXPORT FPDF_BITMAP FPDF_CALLCONV
FPDFImageObj_GetRenderedBitmap(FPDF_DOCUMENT document,
                               FPDF_PAGE page,
                               FPDF_PAGEOBJECT image_object) {
  CPDF_Document* doc = CPDFDocumentFromFPDFDocument(document);
  if (!doc)
    return nullptr;

  CPDF_Page* optional_page = CPDFPageFromFPDFPage(page);
  if (optional_page && optional_page->GetDocument() != doc)
    return nullptr;

  CPDF_ImageObject* image = CPDFImageObjectFromFPDFPageObject(image_object);
  if (!image)
    return nullptr;

  // Create |result_bitmap|.
  const CFX_Matrix& image_matrix = image->matrix();
  int output_width = image_matrix.a;
  int output_height = image_matrix.d;
  auto result_bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!result_bitmap->Create(output_width, output_height, FXDIB_Format::kArgb))
    return nullptr;

  // Set up all the rendering code.
  CPDF_Dictionary* page_resources =
      optional_page ? optional_page->GetPageResources() : nullptr;
  CPDF_RenderContext context(doc, page_resources, /*pPageCache=*/nullptr);
  CFX_DefaultRenderDevice device;
  device.Attach(result_bitmap, /*bRgbByteOrder=*/false,
                /*pBackdropBitmap=*/nullptr, /*bGroupKnockout=*/false);
  CPDF_RenderStatus status(&context, &device);
  CPDF_ImageRenderer renderer;

  // Need to first flip the image, as expected by |renderer|.
  CFX_Matrix render_matrix(1, 0, 0, -1, 0, output_height);

  // Then take |image_matrix|'s offset into account.
  render_matrix.Translate(-image_matrix.e, image_matrix.f);

  // Do the actual rendering.
  bool should_continue = renderer.Start(&status, image, render_matrix,
                                        /*bStdCS=*/false, BlendMode::kNormal);
  while (should_continue)
    should_continue = renderer.Continue(/*pPause=*/nullptr);

  if (!renderer.GetResult())
    return nullptr;

  // Caller takes ownership.
  return FPDFBitmapFromCFXDIBitmap(result_bitmap.Leak());
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFImageObj_GetImageDataDecoded(FPDF_PAGEOBJECT image_object,
                                 void* buffer,
                                 unsigned long buflen) {
  CPDF_ImageObject* pImgObj = CPDFImageObjectFromFPDFPageObject(image_object);
  if (!pImgObj)
    return 0;

  RetainPtr<CPDF_Image> pImg = pImgObj->GetImage();
  if (!pImg)
    return 0;

  CPDF_Stream* pImgStream = pImg->GetStream();
  if (!pImgStream)
    return 0;

  return DecodeStreamMaybeCopyAndReturnLength(pImgStream, buffer, buflen);
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFImageObj_GetImageDataRaw(FPDF_PAGEOBJECT image_object,
                             void* buffer,
                             unsigned long buflen) {
  CPDF_ImageObject* pImgObj = CPDFImageObjectFromFPDFPageObject(image_object);
  if (!pImgObj)
    return 0;

  RetainPtr<CPDF_Image> pImg = pImgObj->GetImage();
  if (!pImg)
    return 0;

  CPDF_Stream* pImgStream = pImg->GetStream();
  if (!pImgStream)
    return 0;

  return GetRawStreamMaybeCopyAndReturnLength(pImgStream, buffer, buflen);
}

FPDF_EXPORT int FPDF_CALLCONV
FPDFImageObj_GetImageFilterCount(FPDF_PAGEOBJECT image_object) {
  CPDF_ImageObject* pImgObj = CPDFImageObjectFromFPDFPageObject(image_object);
  if (!pImgObj)
    return 0;

  RetainPtr<CPDF_Image> pImg = pImgObj->GetImage();
  if (!pImg)
    return 0;

  CPDF_Dictionary* pDict = pImg->GetDict();
  CPDF_Object* pFilter = pDict ? pDict->GetDirectObjectFor("Filter") : nullptr;
  if (!pFilter)
    return 0;

  if (pFilter->IsArray())
    return pFilter->AsArray()->size();
  if (pFilter->IsName())
    return 1;

  return 0;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFImageObj_GetImageFilter(FPDF_PAGEOBJECT image_object,
                            int index,
                            void* buffer,
                            unsigned long buflen) {
  if (index < 0 || index >= FPDFImageObj_GetImageFilterCount(image_object))
    return 0;

  CPDF_PageObject* pObj = CPDFPageObjectFromFPDFPageObject(image_object);
  CPDF_Object* pFilter =
      pObj->AsImage()->GetImage()->GetDict()->GetDirectObjectFor("Filter");
  ByteString bsFilter;
  if (pFilter->IsName())
    bsFilter = pFilter->AsName()->GetString();
  else
    bsFilter = pFilter->AsArray()->GetStringAt(index);

  return NulTerminateMaybeCopyAndReturnLength(bsFilter, buffer, buflen);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFImageObj_GetImageMetadata(FPDF_PAGEOBJECT image_object,
                              FPDF_PAGE page,
                              FPDF_IMAGEOBJ_METADATA* metadata) {
  CPDF_ImageObject* pImgObj = CPDFImageObjectFromFPDFPageObject(image_object);
  if (!pImgObj || !metadata)
    return false;

  RetainPtr<CPDF_Image> pImg = pImgObj->GetImage();
  if (!pImg)
    return false;

  metadata->marked_content_id =
      pImgObj->GetContentMarks()->GetMarkedContentID();

  const int nPixelWidth = pImg->GetPixelWidth();
  const int nPixelHeight = pImg->GetPixelHeight();
  metadata->width = nPixelWidth;
  metadata->height = nPixelHeight;

  const float nWidth = pImgObj->GetRect().Width();
  const float nHeight = pImgObj->GetRect().Height();
  constexpr int nPointsPerInch = 72;
  if (nWidth != 0 && nHeight != 0) {
    metadata->horizontal_dpi = nPixelWidth / nWidth * nPointsPerInch;
    metadata->vertical_dpi = nPixelHeight / nHeight * nPointsPerInch;
  }

  metadata->bits_per_pixel = 0;
  metadata->colorspace = FPDF_COLORSPACE_UNKNOWN;

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !pPage->GetDocument() || !pImg->GetStream())
    return true;

  auto pSource = pdfium::MakeRetain<CPDF_DIB>();
  CPDF_DIB::LoadState ret =
      pSource->StartLoadDIBBase(pPage->GetDocument(), pImg->GetStream(), false,
                                nullptr, pPage->GetPageResources(), false,
                                CPDF_ColorSpace::Family::kUnknown, false);
  if (ret == CPDF_DIB::LoadState::kFail)
    return true;

  metadata->bits_per_pixel = pSource->GetBPP();
  if (pSource->GetColorSpace()) {
    metadata->colorspace =
        static_cast<int>(pSource->GetColorSpace()->GetFamily());
  }
  return true;
}
