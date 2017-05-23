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

#if _FX_OS_ == _FX_ANDROID_
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

}  // namespace

DLLEXPORT FPDF_DOCUMENT STDCALL FPDF_CreateNewDocument() {
  auto pDoc = pdfium::MakeUnique<CPDF_Document>(nullptr);
  pDoc->CreateNewDoc();

  time_t currentTime;
  CFX_ByteString DateStr;
  if (FSDK_IsSandBoxPolicyEnabled(FPDF_POLICY_MACHINETIME_ACCESS)) {
    if (time(&currentTime) != -1) {
      tm* pTM = localtime(&currentTime);
      if (pTM) {
        DateStr.Format("D:%04d%02d%02d%02d%02d%02d", pTM->tm_year + 1900,
                       pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min,
                       pTM->tm_sec);
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

DLLEXPORT void STDCALL FPDFPage_Delete(FPDF_DOCUMENT document, int page_index) {
  if (UnderlyingDocumentType* pDoc = UnderlyingFromFPDFDocument(document))
    pDoc->DeletePage(page_index);
}

DLLEXPORT FPDF_PAGE STDCALL FPDFPage_New(FPDF_DOCUMENT document,
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

DLLEXPORT int STDCALL FPDFPage_GetRotation(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  return IsPageObject(pPage) ? pPage->GetPageRotation() : -1;
}

DLLEXPORT void STDCALL FPDFPage_InsertObject(FPDF_PAGE page,
                                             FPDF_PAGEOBJECT page_obj) {
  CPDF_PageObject* pPageObj = static_cast<CPDF_PageObject*>(page_obj);
  if (!pPageObj)
    return;

  std::unique_ptr<CPDF_PageObject> pPageObjHolder(pPageObj);
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return;

  pPage->GetPageObjectList()->push_back(std::move(pPageObjHolder));
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

DLLEXPORT int STDCALL FPDFPage_CountObject(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return -1;
  return pdfium::CollectionSize<int>(*pPage->GetPageObjectList());
}

DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPage_GetObject(FPDF_PAGE page,
                                                     int index) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return nullptr;
  return pPage->GetPageObjectList()->GetPageObjectByIndex(index);
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPage_HasTransparency(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  return pPage && pPage->BackgroundAlphaNeeded();
}

DLLEXPORT FPDF_BOOL STDCALL
FPDFPageObj_HasTransparency(FPDF_PAGEOBJECT pageObject) {
  if (!pageObject)
    return false;

  CPDF_PageObject* pPageObj = static_cast<CPDF_PageObject*>(pageObject);
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
      int trans = pForm->m_Transparency;
      if ((trans & PDFTRANS_ISOLATED) || (trans & PDFTRANS_GROUP))
        return true;
    }
  }

  return false;
}

DLLEXPORT int STDCALL FPDFPageObj_GetType(FPDF_PAGEOBJECT pageObject) {
  if (!pageObject)
    return FPDF_PAGEOBJ_UNKNOWN;

  CPDF_PageObject* pPageObj = static_cast<CPDF_PageObject*>(pageObject);
  return pPageObj->GetType();
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GenerateContent(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return false;

  CPDF_PageContentGenerator CG(pPage);
  CG.GenerateContent();
  return true;
}

DLLEXPORT void STDCALL FPDFPageObj_Transform(FPDF_PAGEOBJECT page_object,
                                             double a,
                                             double b,
                                             double c,
                                             double d,
                                             double e,
                                             double f) {
  CPDF_PageObject* pPageObj = static_cast<CPDF_PageObject*>(page_object);
  if (!pPageObj)
    return;

  CFX_Matrix matrix((float)a, (float)b, (float)c, (float)d, (float)e, (float)f);
  pPageObj->Transform(matrix);
}

DLLEXPORT void STDCALL FPDFPage_TransformAnnots(FPDF_PAGE page,
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
    CFX_FloatRect rect = pAnnot->GetRect();  // transformAnnots Rectangle
    CFX_Matrix matrix((float)a, (float)b, (float)c, (float)d, (float)e,
                      (float)f);
    matrix.TransformRect(rect);

    CPDF_Array* pRectArray = pAnnot->GetAnnotDict()->GetArrayFor("Rect");
    if (!pRectArray)
      pRectArray = pAnnot->GetAnnotDict()->SetNewFor<CPDF_Array>("Rect");

    pRectArray->SetNewAt<CPDF_Number>(0, rect.left);
    pRectArray->SetNewAt<CPDF_Number>(1, rect.bottom);
    pRectArray->SetNewAt<CPDF_Number>(2, rect.right);
    pRectArray->SetNewAt<CPDF_Number>(3, rect.top);

    // TODO(unknown): Transform AP's rectangle
  }
}

DLLEXPORT void STDCALL FPDFPage_SetRotation(FPDF_PAGE page, int rotate) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return;

  rotate %= 4;
  pPage->m_pFormDict->SetNewFor<CPDF_Number>("Rotate", rotate * 90);
}
