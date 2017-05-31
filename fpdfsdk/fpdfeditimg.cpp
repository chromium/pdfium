// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_edit.h"

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "fpdfsdk/fsdk_define.h"
#include "third_party/base/ptr_util.h"

namespace {

bool LoadJpegHelper(FPDF_PAGE* pages,
                    int nCount,
                    FPDF_PAGEOBJECT image_object,
                    FPDF_FILEACCESS* fileAccess,
                    bool inlineJpeg) {
  if (!image_object || !fileAccess)
    return false;

  CFX_RetainPtr<IFX_SeekableReadStream> pFile =
      MakeSeekableReadStream(fileAccess);
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

DLLEXPORT FPDF_PAGEOBJECT STDCALL
FPDFPageObj_NewImageObj(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  auto pImageObj = pdfium::MakeUnique<CPDF_ImageObject>();
  pImageObj->SetImage(pdfium::MakeRetain<CPDF_Image>(pDoc));
  return pImageObj.release();
}

DLLEXPORT FPDF_BOOL STDCALL
FPDFImageObj_LoadJpegFile(FPDF_PAGE* pages,
                          int nCount,
                          FPDF_PAGEOBJECT image_object,
                          FPDF_FILEACCESS* fileAccess) {
  return LoadJpegHelper(pages, nCount, image_object, fileAccess, false);
}

DLLEXPORT FPDF_BOOL STDCALL
FPDFImageObj_LoadJpegFileInline(FPDF_PAGE* pages,
                                int nCount,
                                FPDF_PAGEOBJECT image_object,
                                FPDF_FILEACCESS* fileAccess) {
  return LoadJpegHelper(pages, nCount, image_object, fileAccess, true);
}

DLLEXPORT FPDF_BOOL STDCALL FPDFImageObj_SetMatrix(FPDF_PAGEOBJECT image_object,
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

DLLEXPORT FPDF_BOOL STDCALL FPDFImageObj_SetBitmap(FPDF_PAGE* pages,
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
  CFX_RetainPtr<CFX_DIBitmap> holder(CFXBitmapFromFPDFBitmap(bitmap));
  pImgObj->GetImage()->SetImage(holder);
  pImgObj->CalcBoundingBox();
  pImgObj->SetDirty(true);
  return true;
}
