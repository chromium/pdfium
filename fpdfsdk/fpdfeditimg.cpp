// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_edit.h"

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/render/cpdf_dibsource.h"
#include "fpdfsdk/fsdk_define.h"
#include "third_party/base/ptr_util.h"

namespace {

// These checks ensure the consistency of colorspace values across core/ and
// public/.
static_assert(PDFCS_DEVICEGRAY == FPDF_COLORSPACE_DEVICEGRAY,
              "PDFCS_DEVICEGRAY value mismatch");
static_assert(PDFCS_DEVICERGB == FPDF_COLORSPACE_DEVICERGB,
              "PDFCS_DEVICERGB value mismatch");
static_assert(PDFCS_DEVICECMYK == FPDF_COLORSPACE_DEVICECMYK,
              "PDFCS_DEVICECMYK value mismatch");
static_assert(PDFCS_CALGRAY == FPDF_COLORSPACE_CALGRAY,
              "PDFCS_CALGRAY value mismatch");
static_assert(PDFCS_CALRGB == FPDF_COLORSPACE_CALRGB,
              "PDFCS_CALRGB value mismatch");
static_assert(PDFCS_LAB == FPDF_COLORSPACE_LAB, "PDFCS_LAB value mismatch");
static_assert(PDFCS_ICCBASED == FPDF_COLORSPACE_ICCBASED,
              "PDFCS_ICCBASED value mismatch");
static_assert(PDFCS_SEPARATION == FPDF_COLORSPACE_SEPARATION,
              "PDFCS_SEPARATION value mismatch");
static_assert(PDFCS_DEVICEN == FPDF_COLORSPACE_DEVICEN,
              "PDFCS_DEVICEN value mismatch");
static_assert(PDFCS_INDEXED == FPDF_COLORSPACE_INDEXED,
              "PDFCS_INDEXED value mismatch");
static_assert(PDFCS_PATTERN == FPDF_COLORSPACE_PATTERN,
              "PDFCS_PATTERN value mismatch");

bool LoadJpegHelper(FPDF_PAGE* pages,
                    int nCount,
                    FPDF_PAGEOBJECT image_object,
                    FPDF_FILEACCESS* fileAccess,
                    bool inlineJpeg) {
  if (!image_object || !fileAccess)
    return false;

  RetainPtr<IFX_SeekableReadStream> pFile = MakeSeekableReadStream(fileAccess);
  CPDF_ImageObject* pImgObj = static_cast<CPDF_ImageObject*>(image_object);

  if (pages) {
    for (int index = 0; index < nCount; index++) {
      CPDF_Page* pPage = CPDFPageFromFPDFPage(pages[index]);
      if (pPage)
        pImgObj->GetImage()->ResetCache(pPage, nullptr);
    }
  }

  if (inlineJpeg)
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

  auto pImageObj = pdfium::MakeUnique<CPDF_ImageObject>();
  pImageObj->SetImage(pdfium::MakeRetain<CPDF_Image>(pDoc));
  return pImageObj.release();
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFImageObj_LoadJpegFile(FPDF_PAGE* pages,
                          int nCount,
                          FPDF_PAGEOBJECT image_object,
                          FPDF_FILEACCESS* fileAccess) {
  return LoadJpegHelper(pages, nCount, image_object, fileAccess, false);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFImageObj_LoadJpegFileInline(FPDF_PAGE* pages,
                                int nCount,
                                FPDF_PAGEOBJECT image_object,
                                FPDF_FILEACCESS* fileAccess) {
  return LoadJpegHelper(pages, nCount, image_object, fileAccess, true);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFImageObj_SetMatrix(FPDF_PAGEOBJECT image_object,
                       double a,
                       double b,
                       double c,
                       double d,
                       double e,
                       double f) {
  if (!image_object)
    return false;

  CPDF_ImageObject* pImgObj = static_cast<CPDF_ImageObject*>(image_object);
  pImgObj->set_matrix(CFX_Matrix(static_cast<float>(a), static_cast<float>(b),
                                 static_cast<float>(c), static_cast<float>(d),
                                 static_cast<float>(e), static_cast<float>(f)));
  pImgObj->CalcBoundingBox();
  pImgObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFImageObj_SetBitmap(FPDF_PAGE* pages,
                       int nCount,
                       FPDF_PAGEOBJECT image_object,
                       FPDF_BITMAP bitmap) {
  if (!image_object || !bitmap || !pages)
    return false;

  CPDF_ImageObject* pImgObj = static_cast<CPDF_ImageObject*>(image_object);
  for (int index = 0; index < nCount; index++) {
    CPDF_Page* pPage = CPDFPageFromFPDFPage(pages[index]);
    if (pPage)
      pImgObj->GetImage()->ResetCache(pPage, nullptr);
  }
  RetainPtr<CFX_DIBitmap> holder(CFXBitmapFromFPDFBitmap(bitmap));
  pImgObj->GetImage()->SetImage(holder);
  pImgObj->CalcBoundingBox();
  pImgObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BITMAP FPDF_CALLCONV
FPDFImageObj_GetBitmap(FPDF_PAGEOBJECT image_object) {
  CPDF_PageObject* pObj = CPDFPageObjectFromFPDFPageObject(image_object);
  if (!pObj || !pObj->IsImage())
    return nullptr;

  RetainPtr<CPDF_Image> pImg = pObj->AsImage()->GetImage();
  if (!pImg)
    return nullptr;

  RetainPtr<CFX_DIBSource> pSource = pImg->LoadDIBSource();
  if (!pSource)
    return nullptr;

  RetainPtr<CFX_DIBitmap> pBitmap;
  // If the source image has a representation of 1 bit per pixel, then convert
  // it to a grayscale bitmap having 1 byte per pixel, since bitmaps have no
  // concept of bits. Otherwise, convert the source image to a bitmap directly,
  // retaining its color representation.
  if (pSource->GetBPP() == 1)
    pBitmap = pSource->CloneConvert(FXDIB_8bppRgb);
  else
    pBitmap = pSource->Clone(nullptr);

  return pBitmap.Leak();
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFImageObj_GetImageDataDecoded(FPDF_PAGEOBJECT image_object,
                                 void* buffer,
                                 unsigned long buflen) {
  CPDF_PageObject* pObj = CPDFPageObjectFromFPDFPageObject(image_object);
  if (!pObj || !pObj->IsImage())
    return 0;

  RetainPtr<CPDF_Image> pImg = pObj->AsImage()->GetImage();
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
  CPDF_PageObject* pObj = CPDFPageObjectFromFPDFPageObject(image_object);
  if (!pObj || !pObj->IsImage())
    return 0;

  RetainPtr<CPDF_Image> pImg = pObj->AsImage()->GetImage();
  if (!pImg)
    return 0;

  CPDF_Stream* pImgStream = pImg->GetStream();
  if (!pImgStream)
    return 0;

  uint32_t len = pImgStream->GetRawSize();
  if (buffer && buflen >= len)
    memcpy(buffer, pImgStream->GetRawData(), len);

  return len;
}

FPDF_EXPORT int FPDF_CALLCONV
FPDFImageObj_GetImageFilterCount(FPDF_PAGEOBJECT image_object) {
  CPDF_PageObject* pObj = CPDFPageObjectFromFPDFPageObject(image_object);
  if (!pObj || !pObj->IsImage())
    return 0;

  RetainPtr<CPDF_Image> pImg = pObj->AsImage()->GetImage();
  if (!pImg)
    return 0;

  CPDF_Dictionary* pDict = pImg->GetDict();
  CPDF_Object* pFilter = pDict ? pDict->GetDirectObjectFor("Filter") : nullptr;
  if (!pFilter)
    return 0;

  if (pFilter->IsArray())
    return pFilter->AsArray()->GetCount();
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

  unsigned long len = bsFilter.GetLength() + 1;
  if (buffer && len <= buflen)
    memcpy(buffer, bsFilter.c_str(), len);
  return len;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFImageObj_GetImageMetadata(FPDF_PAGEOBJECT image_object,
                              FPDF_PAGE page,
                              FPDF_IMAGEOBJ_METADATA* metadata) {
  CPDF_PageObject* pObj = CPDFPageObjectFromFPDFPageObject(image_object);
  if (!pObj || !pObj->IsImage() || !metadata)
    return false;

  RetainPtr<CPDF_Image> pImg = pObj->AsImage()->GetImage();
  if (!pImg)
    return false;

  metadata->marked_content_id = pObj->m_ContentMark.GetMarkedContentID();

  const int nPixelWidth = pImg->GetPixelWidth();
  const int nPixelHeight = pImg->GetPixelHeight();
  metadata->width = nPixelWidth;
  metadata->height = nPixelHeight;

  const float nWidth = pObj->m_Right - pObj->m_Left;
  const float nHeight = pObj->m_Top - pObj->m_Bottom;
  constexpr int nPointsPerInch = 72;
  if (nWidth != 0 && nHeight != 0) {
    metadata->horizontal_dpi = nPixelWidth / nWidth * nPointsPerInch;
    metadata->vertical_dpi = nPixelHeight / nHeight * nPointsPerInch;
  }

  metadata->bits_per_pixel = 0;
  metadata->colorspace = FPDF_COLORSPACE_UNKNOWN;

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !pPage->m_pDocument.Get() || !pImg->GetStream())
    return true;

  auto pSource = pdfium::MakeRetain<CPDF_DIBSource>();
  if (!pSource->StartLoadDIBSource(pPage->m_pDocument.Get(), pImg->GetStream(),
                                   false, nullptr,
                                   pPage->m_pPageResources.Get())) {
    return true;
  }

  metadata->bits_per_pixel = pSource->GetBPP();
  if (pSource->GetColorSpace())
    metadata->colorspace = pSource->GetColorSpace()->GetFamily();

  return true;
}
