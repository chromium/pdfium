// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_edit.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_formobject.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fpdfapi/page/cpdf_shadingobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fpdfdoc/cpdf_annotlist.h"
#include "fpdfsdk/fsdk_define.h"
#include "public/fpdf_formfill.h"
#include "third_party/base/logging.h"
#include "third_party/base/stl_util.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_page.h"
#endif  // PDF_ENABLE_XFA

#if _FX_OS_ == _FX_OS_ANDROID_
#include <time.h>
#else
#include <ctime>
#endif

namespace {

static_assert(FPDF_PAGEOBJ_TEXT == CPDF_PageObject::TEXT,
              "FPDF_PAGEOBJ_TEXT/CPDF_PageObject::TEXT mismatch");
static_assert(FPDF_PAGEOBJ_PATH == CPDF_PageObject::PATH,
              "FPDF_PAGEOBJ_PATH/CPDF_PageObject::PATH mismatch");
static_assert(FPDF_PAGEOBJ_IMAGE == CPDF_PageObject::IMAGE,
              "FPDF_PAGEOBJ_IMAGE/CPDF_PageObject::IMAGE mismatch");
static_assert(FPDF_PAGEOBJ_SHADING == CPDF_PageObject::SHADING,
              "FPDF_PAGEOBJ_SHADING/CPDF_PageObject::SHADING mismatch");
static_assert(FPDF_PAGEOBJ_FORM == CPDF_PageObject::FORM,
              "FPDF_PAGEOBJ_FORM/CPDF_PageObject::FORM mismatch");

bool IsPageObject(CPDF_Page* pPage) {
  if (!pPage || !pPage->m_pFormDict || !pPage->m_pFormDict->KeyExist("Type"))
    return false;

  CPDF_Object* pObject = pPage->m_pFormDict->GetObjectFor("Type")->GetDirect();
  return pObject && !pObject->GetString().Compare("Page");
}

void CalcBoundingBox(CPDF_PageObject* pPageObj) {
  switch (pPageObj->GetType()) {
    case CPDF_PageObject::TEXT: {
      break;
    }
    case CPDF_PageObject::PATH: {
      CPDF_PathObject* pPathObj = pPageObj->AsPath();
      pPathObj->CalcBoundingBox();
      break;
    }
    case CPDF_PageObject::IMAGE: {
      CPDF_ImageObject* pImageObj = pPageObj->AsImage();
      pImageObj->CalcBoundingBox();
      break;
    }
    case CPDF_PageObject::SHADING: {
      CPDF_ShadingObject* pShadingObj = pPageObj->AsShading();
      pShadingObj->CalcBoundingBox();
      break;
    }
    case CPDF_PageObject::FORM: {
      CPDF_FormObject* pFormObj = pPageObj->AsForm();
      pFormObj->CalcBoundingBox();
      break;
    }
    default: {
      NOTREACHED();
      break;
    }
  }
}

}  // namespace

FPDF_EXPORT FPDF_DOCUMENT FPDF_CALLCONV FPDF_CreateNewDocument() {
  auto pDoc = pdfium::MakeUnique<CPDF_Document>(nullptr);
  pDoc->CreateNewDoc();

  time_t currentTime;
  ByteString DateStr;
  if (FSDK_IsSandBoxPolicyEnabled(FPDF_POLICY_MACHINETIME_ACCESS)) {
    if (time(&currentTime) != -1) {
      tm* pTM = localtime(&currentTime);
      if (pTM) {
        DateStr = ByteString::Format(
            "D:%04d%02d%02d%02d%02d%02d", pTM->tm_year + 1900, pTM->tm_mon + 1,
            pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
      }
    }
  }

  CPDF_Dictionary* pInfoDict = pDoc->GetInfo();
  if (pInfoDict) {
    if (FSDK_IsSandBoxPolicyEnabled(FPDF_POLICY_MACHINETIME_ACCESS))
      pInfoDict->SetNewFor<CPDF_String>("CreationDate", DateStr, false);
    pInfoDict->SetNewFor<CPDF_String>("Creator", L"PDFium");
  }

  // Caller takes ownership of pDoc.
  return FPDFDocumentFromCPDFDocument(pDoc.release());
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_Delete(FPDF_DOCUMENT document,
                                               int page_index) {
  if (UnderlyingDocumentType* pDoc = UnderlyingFromFPDFDocument(document))
    pDoc->DeletePage(page_index);
}

FPDF_EXPORT FPDF_PAGE FPDF_CALLCONV FPDFPage_New(FPDF_DOCUMENT document,
                                                 int page_index,
                                                 double width,
                                                 double height) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  page_index = pdfium::clamp(page_index, 0, pDoc->GetPageCount());
  CPDF_Dictionary* pPageDict = pDoc->CreateNewPage(page_index);
  if (!pPageDict)
    return nullptr;

  CPDF_Array* pMediaBoxArray = pPageDict->SetNewFor<CPDF_Array>("MediaBox");
  pMediaBoxArray->AddNew<CPDF_Number>(0);
  pMediaBoxArray->AddNew<CPDF_Number>(0);
  pMediaBoxArray->AddNew<CPDF_Number>(static_cast<float>(width));
  pMediaBoxArray->AddNew<CPDF_Number>(static_cast<float>(height));
  pPageDict->SetNewFor<CPDF_Number>("Rotate", 0);
  pPageDict->SetNewFor<CPDF_Dictionary>("Resources");

#ifdef PDF_ENABLE_XFA
  auto pXFAPage = pdfium::MakeRetain<CPDFXFA_Page>(
      static_cast<CPDFXFA_Context*>(document), page_index);
  pXFAPage->LoadPDFPage(pPageDict);
  return pXFAPage.Leak();  // Caller takes ownership.
#else   // PDF_ENABLE_XFA
  auto pPage = pdfium::MakeUnique<CPDF_Page>(pDoc, pPageDict, true);
  pPage->ParseContent();
  return pPage.release();  // Caller takes ownership.
#endif  // PDF_ENABLE_XFA
}

FPDF_EXPORT int FPDF_CALLCONV FPDFPage_GetRotation(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  return IsPageObject(pPage) ? pPage->GetPageRotation() : -1;
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_InsertObject(FPDF_PAGE page,
                                                     FPDF_PAGEOBJECT page_obj) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_obj);
  if (!pPageObj)
    return;

  std::unique_ptr<CPDF_PageObject> pPageObjHolder(pPageObj);
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return;
  pPageObj->SetDirty(true);
  pPage->GetPageObjectList()->push_back(std::move(pPageObjHolder));
  CalcBoundingBox(pPageObj);
}

FPDF_EXPORT int FPDF_CALLCONV FPDFPage_CountObject(FPDF_PAGE page) {
  return FPDFPage_CountObjects(page);
}

FPDF_EXPORT int FPDF_CALLCONV FPDFPage_CountObjects(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return -1;
  return pdfium::CollectionSize<int>(*pPage->GetPageObjectList());
}

FPDF_EXPORT FPDF_PAGEOBJECT FPDF_CALLCONV FPDFPage_GetObject(FPDF_PAGE page,
                                                             int index) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return nullptr;
  return pPage->GetPageObjectList()->GetPageObjectByIndex(index);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFPage_HasTransparency(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  return pPage && pPage->BackgroundAlphaNeeded();
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPageObj_Destroy(FPDF_PAGEOBJECT page_obj) {
  delete CPDFPageObjectFromFPDFPageObject(page_obj);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObj_HasTransparency(FPDF_PAGEOBJECT pageObject) {
  if (!pageObject)
    return false;

  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(pageObject);
  int blend_type = pPageObj->m_GeneralState.GetBlendType();
  if (blend_type != FXDIB_BLEND_NORMAL)
    return true;

  CPDF_Dictionary* pSMaskDict =
      ToDictionary(pPageObj->m_GeneralState.GetSoftMask());
  if (pSMaskDict)
    return true;

  if (pPageObj->m_GeneralState.GetFillAlpha() != 1.0f)
    return true;

  if (pPageObj->IsPath() && pPageObj->m_GeneralState.GetStrokeAlpha() != 1.0f) {
    return true;
  }

  if (pPageObj->IsForm()) {
    const CPDF_Form* pForm = pPageObj->AsForm()->form();
    if (pForm) {
      int trans = pForm->m_iTransparency;
      if ((trans & PDFTRANS_ISOLATED) || (trans & PDFTRANS_GROUP))
        return true;
    }
  }

  return false;
}

FPDF_EXPORT int FPDF_CALLCONV FPDFPageObj_GetType(FPDF_PAGEOBJECT pageObject) {
  if (!pageObject)
    return FPDF_PAGEOBJ_UNKNOWN;

  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(pageObject);
  return pPageObj->GetType();
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFPage_GenerateContent(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return false;

  CPDF_PageContentGenerator CG(pPage);
  CG.GenerateContent();
  return true;
}

FPDF_EXPORT void FPDF_CALLCONV
FPDFPageObj_Transform(FPDF_PAGEOBJECT page_object,
                      double a,
                      double b,
                      double c,
                      double d,
                      double e,
                      double f) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj)
    return;

  CFX_Matrix matrix((float)a, (float)b, (float)c, (float)d, (float)e, (float)f);
  pPageObj->Transform(matrix);
}

FPDF_EXPORT void FPDF_CALLCONV
FPDFPageObj_SetBlendMode(FPDF_PAGEOBJECT page_object,
                         FPDF_BYTESTRING blend_mode) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj)
    return;

  pPageObj->m_GeneralState.SetBlendMode(blend_mode);
  pPageObj->SetDirty(true);
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_TransformAnnots(FPDF_PAGE page,
                                                        double a,
                                                        double b,
                                                        double c,
                                                        double d,
                                                        double e,
                                                        double f) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return;

  CPDF_AnnotList AnnotList(pPage);
  for (size_t i = 0; i < AnnotList.Count(); ++i) {
    CPDF_Annot* pAnnot = AnnotList.GetAt(i);
    CFX_Matrix matrix((float)a, (float)b, (float)c, (float)d, (float)e,
                      (float)f);
    CFX_FloatRect rect = matrix.TransformRect(pAnnot->GetRect());

    CPDF_Dictionary* pAnnotDict = pAnnot->GetAnnotDict();
    CPDF_Array* pRectArray = pAnnotDict->GetArrayFor("Rect");
    if (pRectArray)
      pRectArray->Clear();
    else
      pRectArray = pAnnotDict->SetNewFor<CPDF_Array>("Rect");

    pRectArray->AddNew<CPDF_Number>(rect.left);
    pRectArray->AddNew<CPDF_Number>(rect.bottom);
    pRectArray->AddNew<CPDF_Number>(rect.right);
    pRectArray->AddNew<CPDF_Number>(rect.top);

    // TODO(unknown): Transform AP's rectangle
  }
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_SetRotation(FPDF_PAGE page,
                                                    int rotate) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return;

  rotate %= 4;
  pPage->m_pFormDict->SetNewFor<CPDF_Number>("Rotate", rotate * 90);
}

FPDF_BOOL FPDFPageObj_SetFillColor(FPDF_PAGEOBJECT page_object,
                                   unsigned int R,
                                   unsigned int G,
                                   unsigned int B,
                                   unsigned int A) {
  if (!page_object || R > 255 || G > 255 || B > 255 || A > 255)
    return false;

  float rgb[3] = {R / 255.f, G / 255.f, B / 255.f};
  auto* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  pPageObj->m_GeneralState.SetFillAlpha(A / 255.f);
  pPageObj->m_ColorState.SetFillColor(
      CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB), rgb, 3);
  pPageObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObj_GetBounds(FPDF_PAGEOBJECT pageObject,
                      float* left,
                      float* bottom,
                      float* right,
                      float* top) {
  if (!pageObject)
    return false;

  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(pageObject);
  CFX_FloatRect bbox = pPageObj->GetRect();
  *left = bbox.left;
  *bottom = bbox.bottom;
  *right = bbox.right;
  *top = bbox.top;
  return true;
}
