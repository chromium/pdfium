// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_edit.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fpdfapi/fpdf_edit/include/cpdf_pagecontentgenerator.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_form.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_formobject.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_imageobject.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_pageobject.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_pathobject.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_shadingobject.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_number.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_string.h"
#include "core/fpdfdoc/include/cpdf_annot.h"
#include "core/fpdfdoc/include/cpdf_annotlist.h"
#include "fpdfsdk/include/fsdk_define.h"
#include "public/fpdf_formfill.h"
#include "third_party/base/stl_util.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_app.h"
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_doc.h"
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_page.h"
#endif  // PDF_ENABLE_XFA

#if _FX_OS_ == _FX_ANDROID_
#include "time.h"
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
  CPDF_Document* pDoc = new CPDF_Document(nullptr);
  pDoc->CreateNewDoc();
  time_t currentTime;

  CFX_ByteString DateStr;

  if (FSDK_IsSandBoxPolicyEnabled(FPDF_POLICY_MACHINETIME_ACCESS)) {
    if (-1 != time(&currentTime)) {
      tm* pTM = localtime(&currentTime);
      if (pTM) {
        DateStr.Format("D:%04d%02d%02d%02d%02d%02d", pTM->tm_year + 1900,
                       pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min,
                       pTM->tm_sec);
      }
    }
  }

  CPDF_Dictionary* pInfoDict = nullptr;
  pInfoDict = pDoc->GetInfo();
  if (pInfoDict) {
    if (FSDK_IsSandBoxPolicyEnabled(FPDF_POLICY_MACHINETIME_ACCESS))
      pInfoDict->SetFor("CreationDate", new CPDF_String(DateStr, FALSE));
    pInfoDict->SetFor("Creator", new CPDF_String(L"PDFium"));
  }

  return FPDFDocumentFromCPDFDocument(pDoc);
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

  page_index = std::min(std::max(page_index, 0), pDoc->GetPageCount());
  CPDF_Dictionary* pPageDict = pDoc->CreateNewPage(page_index);
  if (!pPageDict)
    return nullptr;

  CPDF_Array* pMediaBoxArray = new CPDF_Array;
  pMediaBoxArray->Add(new CPDF_Number(0));
  pMediaBoxArray->Add(new CPDF_Number(0));
  pMediaBoxArray->Add(new CPDF_Number(FX_FLOAT(width)));
  pMediaBoxArray->Add(new CPDF_Number(FX_FLOAT(height)));

  pPageDict->SetFor("MediaBox", pMediaBoxArray);
  pPageDict->SetFor("Rotate", new CPDF_Number(0));
  pPageDict->SetFor("Resources", new CPDF_Dictionary);

#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pPage =
      new CPDFXFA_Page((CPDFXFA_Document*)document, page_index);
  pPage->LoadPDFPage(pPageDict);
#else   // PDF_ENABLE_XFA
  CPDF_Page* pPage = new CPDF_Page(pDoc, pPageDict, true);
  pPage->ParseContent();
#endif  // PDF_ENABLE_XFA

  return pPage;
}

DLLEXPORT int STDCALL FPDFPage_GetRotation(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return -1;

  CPDF_Dictionary* pDict = pPage->m_pFormDict;
  while (pDict) {
    if (pDict->KeyExist("Rotate")) {
      CPDF_Object* pRotateObj = pDict->GetObjectFor("Rotate")->GetDirect();
      return pRotateObj ? pRotateObj->GetInteger() / 90 : 0;
    }
    if (!pDict->KeyExist("Parent"))
      break;

    pDict = ToDictionary(pDict->GetObjectFor("Parent")->GetDirect());
  }

  return 0;
}

DLLEXPORT void STDCALL FPDFPage_InsertObject(FPDF_PAGE page,
                                             FPDF_PAGEOBJECT page_obj) {
  CPDF_PageObject* pPageObj = reinterpret_cast<CPDF_PageObject*>(page_obj);
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
      ASSERT(false);
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
    return FALSE;

  CPDF_PageObject* pPageObj = reinterpret_cast<CPDF_PageObject*>(pageObject);
  int blend_type = pPageObj->m_GeneralState.GetBlendType();
  if (blend_type != FXDIB_BLEND_NORMAL)
    return TRUE;

  CPDF_Dictionary* pSMaskDict =
      ToDictionary(pPageObj->m_GeneralState.GetSoftMask());
  if (pSMaskDict)
    return TRUE;

  if (pPageObj->m_GeneralState.GetFillAlpha() != 1.0f)
    return TRUE;

  if (pPageObj->IsPath() && pPageObj->m_GeneralState.GetStrokeAlpha() != 1.0f) {
    return TRUE;
  }

  if (pPageObj->IsForm()) {
    const CPDF_Form* pForm = pPageObj->AsForm()->form();
    if (pForm) {
      int trans = pForm->m_Transparency;
      if ((trans & PDFTRANS_ISOLATED) || (trans & PDFTRANS_GROUP))
        return TRUE;
    }
  }

  return FALSE;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GenerateContent(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return FALSE;

  CPDF_PageContentGenerator CG(pPage);
  CG.GenerateContent();
  return TRUE;
}

DLLEXPORT void STDCALL FPDFPageObj_Transform(FPDF_PAGEOBJECT page_object,
                                             double a,
                                             double b,
                                             double c,
                                             double d,
                                             double e,
                                             double f) {
  CPDF_PageObject* pPageObj = reinterpret_cast<CPDF_PageObject*>(page_object);
  if (!pPageObj)
    return;

  CFX_Matrix matrix((FX_FLOAT)a, (FX_FLOAT)b, (FX_FLOAT)c, (FX_FLOAT)d,
                    (FX_FLOAT)e, (FX_FLOAT)f);
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
    // transformAnnots Rectangle
    CFX_FloatRect rect = pAnnot->GetRect();
    CFX_Matrix matrix((FX_FLOAT)a, (FX_FLOAT)b, (FX_FLOAT)c, (FX_FLOAT)d,
                      (FX_FLOAT)e, (FX_FLOAT)f);
    rect.Transform(&matrix);
    CPDF_Array* pRectArray = pAnnot->GetAnnotDict()->GetArrayFor("Rect");
    if (!pRectArray)
      pRectArray = new CPDF_Array;
    pRectArray->SetAt(0, new CPDF_Number(rect.left));
    pRectArray->SetAt(1, new CPDF_Number(rect.bottom));
    pRectArray->SetAt(2, new CPDF_Number(rect.right));
    pRectArray->SetAt(3, new CPDF_Number(rect.top));
    pAnnot->GetAnnotDict()->SetFor("Rect", pRectArray);

    // Transform AP's rectangle
    // To Do
  }
}

DLLEXPORT void STDCALL FPDFPage_SetRotation(FPDF_PAGE page, int rotate) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return;

  CPDF_Dictionary* pDict = pPage->m_pFormDict;
  rotate %= 4;
  pDict->SetFor("Rotate", new CPDF_Number(rotate * 90));
}
