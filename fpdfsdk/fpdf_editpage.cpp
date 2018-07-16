// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_edit.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "constants/page_object.h"
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
#include "fpdfsdk/cpdfsdk_helpers.h"
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
  if (!pPage)
    return false;

  const CPDF_Dictionary* pFormDict = pPage->GetDict();
  if (!pFormDict || !pFormDict->KeyExist("Type"))
    return false;

  const CPDF_Object* pObject = pFormDict->GetObjectFor("Type")->GetDirect();
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

CPDF_Dictionary* GetMarkParamDict(FPDF_PAGEOBJECTMARK mark) {
  CPDF_ContentMarkItem* pMarkItem =
      CPDFContentMarkItemFromFPDFPageObjectMark(mark);
  return pMarkItem ? pMarkItem->GetParam() : nullptr;
}

CPDF_Dictionary* GetOrCreateMarkParamsDict(FPDF_DOCUMENT document,
                                           FPDF_PAGEOBJECTMARK mark) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  CPDF_ContentMarkItem* pMarkItem =
      CPDFContentMarkItemFromFPDFPageObjectMark(mark);
  if (!pMarkItem)
    return nullptr;

  CPDF_Dictionary* pParams = pMarkItem->GetParam();

  // If the Params dict does not exist, create a new one.
  if (!pParams) {
    auto new_dict =
        pdfium::MakeUnique<CPDF_Dictionary>(pDoc->GetByteStringPool());
    pParams = new_dict.get();
    pMarkItem->SetDirectDict(std::move(new_dict));
  }

  return pParams;
}

bool PageObjectContainsMark(CPDF_PageObject* pPageObj,
                            FPDF_PAGEOBJECTMARK mark) {
  const CPDF_ContentMarkItem* pMarkItem =
      CPDFContentMarkItemFromFPDFPageObjectMark(mark);
  return pMarkItem && pPageObj->m_ContentMark.ContainsItem(pMarkItem);
}

unsigned int GetUnsignedAlpha(float alpha) {
  return static_cast<unsigned int>(alpha * 255.f + 0.5f);
}

const CPDF_PageObjectList* CPDFPageObjListFromFPDFFormObject(
    FPDF_PAGEOBJECT page_object) {
  auto* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj)
    return nullptr;

  CPDF_FormObject* pFormObject = pPageObj->AsForm();
  if (!pFormObject)
    return nullptr;

  const CPDF_Form* pForm = pFormObject->form();
  if (!pForm)
    return nullptr;

  return pForm->GetPageObjectList();
}

}  // namespace

FPDF_EXPORT FPDF_DOCUMENT FPDF_CALLCONV FPDF_CreateNewDocument() {
  auto pDoc = pdfium::MakeUnique<CPDF_Document>();
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

#ifdef PDF_ENABLE_XFA
  pDoc->SetExtension(pdfium::MakeUnique<CPDFXFA_Context>(pDoc.get()));
#endif  // PDF_ENABLE_XFA

  // Caller takes ownership of pDoc.
  return FPDFDocumentFromCPDFDocument(pDoc.release());
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_Delete(FPDF_DOCUMENT document,
                                               int page_index) {
  auto* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return;

  CPDF_Document::Extension* pExtension = pDoc->GetExtension();
  if (pExtension) {
    pExtension->DeletePage(page_index);
    return;
  }

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

  pPageDict->SetRectFor(pdfium::page_object::kMediaBox,
                        CFX_FloatRect(0, 0, width, height));
  pPageDict->SetNewFor<CPDF_Number>(pdfium::page_object::kRotate, 0);
  pPageDict->SetNewFor<CPDF_Dictionary>(pdfium::page_object::kResources);

#ifdef PDF_ENABLE_XFA
  auto* pContext = static_cast<CPDFXFA_Context*>(pDoc->GetExtension());
  if (pContext) {
    auto pXFAPage = pdfium::MakeRetain<CPDFXFA_Page>(pContext, page_index);
    pXFAPage->LoadPDFPage(pPageDict);
    return FPDFPageFromIPDFPage(pXFAPage.Leak());  // Caller takes ownership.
  }
#endif  // PDF_ENABLE_XFA

  auto pPage = pdfium::MakeRetain<CPDF_Page>(pDoc, pPageDict, true);
  pPage->ParseContent();
  return FPDFPageFromIPDFPage(pPage.Leak());  // Caller takes ownership.
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

  pPage->AppendPageObject(std::move(pPageObjHolder));
  CalcBoundingBox(pPageObj);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPage_RemoveObject(FPDF_PAGE page, FPDF_PAGEOBJECT page_obj) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_obj);
  if (!pPageObj)
    return false;

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return false;

  return pPage->RemovePageObject(pPageObj);
}

FPDF_EXPORT int FPDF_CALLCONV FPDFPage_CountObject(FPDF_PAGE page) {
  return FPDFPage_CountObjects(page);
}

FPDF_EXPORT int FPDF_CALLCONV FPDFPage_CountObjects(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return -1;

  return pPage->GetPageObjectCount();
}

FPDF_EXPORT FPDF_PAGEOBJECT FPDF_CALLCONV FPDFPage_GetObject(FPDF_PAGE page,
                                                             int index) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!IsPageObject(pPage))
    return nullptr;

  return FPDFPageObjectFromCPDFPageObject(pPage->GetPageObjectByIndex(index));
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFPage_HasTransparency(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  return pPage && pPage->BackgroundAlphaNeeded();
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPageObj_Destroy(FPDF_PAGEOBJECT page_obj) {
  delete CPDFPageObjectFromFPDFPageObject(page_obj);
}

FPDF_EXPORT int FPDF_CALLCONV
FPDFPageObj_CountMarks(FPDF_PAGEOBJECT page_object) {
  if (!page_object)
    return -1;

  const auto& mark =
      CPDFPageObjectFromFPDFPageObject(page_object)->m_ContentMark;
  return mark.CountItems();
}

FPDF_EXPORT FPDF_PAGEOBJECTMARK FPDF_CALLCONV
FPDFPageObj_GetMark(FPDF_PAGEOBJECT page_object, unsigned long index) {
  if (!page_object)
    return nullptr;

  auto* mark = &CPDFPageObjectFromFPDFPageObject(page_object)->m_ContentMark;
  if (index >= mark->CountItems())
    return nullptr;

  return FPDFPageObjectMarkFromCPDFContentMarkItem(mark->GetItem(index));
}

FPDF_EXPORT FPDF_PAGEOBJECTMARK FPDF_CALLCONV
FPDFPageObj_AddMark(FPDF_PAGEOBJECT page_object, FPDF_BYTESTRING name) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj)
    return nullptr;

  auto* mark = &pPageObj->m_ContentMark;
  mark->AddMark(name);
  unsigned long index = mark->CountItems() - 1;

  pPageObj->SetDirty(true);

  return FPDFPageObjectMarkFromCPDFContentMarkItem(mark->GetItem(index));
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObj_RemoveMark(FPDF_PAGEOBJECT page_object, FPDF_PAGEOBJECTMARK mark) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  CPDF_ContentMarkItem* pMarkItem =
      CPDFContentMarkItemFromFPDFPageObjectMark(mark);
  if (!pPageObj || !pMarkItem)
    return false;

  bool result = pPageObj->m_ContentMark.RemoveMark(pMarkItem);
  if (result)
    pPageObj->SetDirty(true);

  return result;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFPageObjMark_GetName(FPDF_PAGEOBJECTMARK mark,
                        void* buffer,
                        unsigned long buflen) {
  if (!mark)
    return 0;

  const CPDF_ContentMarkItem* pMarkItem =
      CPDFContentMarkItemFromFPDFPageObjectMark(mark);

  return Utf16EncodeMaybeCopyAndReturnLength(
      WideString::FromUTF8(pMarkItem->GetName().AsStringView()), buffer,
      buflen);
}

FPDF_EXPORT int FPDF_CALLCONV
FPDFPageObjMark_CountParams(FPDF_PAGEOBJECTMARK mark) {
  if (!mark)
    return -1;

  const CPDF_ContentMarkItem* pMarkItem =
      CPDFContentMarkItemFromFPDFPageObjectMark(mark);

  const CPDF_Dictionary* pParams = pMarkItem->GetParam();
  return pParams ? pParams->GetCount() : 0;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFPageObjMark_GetParamKey(FPDF_PAGEOBJECTMARK mark,
                            unsigned long index,
                            void* buffer,
                            unsigned long buflen) {
  const CPDF_Dictionary* pParams = GetMarkParamDict(mark);
  if (!pParams)
    return 0;

  for (auto& it : *pParams) {
    if (index == 0) {
      return Utf16EncodeMaybeCopyAndReturnLength(
          WideString::FromUTF8(it.first.AsStringView()), buffer, buflen);
    }
    --index;
  }

  return 0;
}

FPDF_EXPORT FPDF_OBJECT_TYPE FPDF_CALLCONV
FPDFPageObjMark_GetParamValueType(FPDF_PAGEOBJECTMARK mark,
                                  FPDF_BYTESTRING key) {
  const CPDF_Dictionary* pParams = GetMarkParamDict(mark);
  if (!pParams)
    return FPDF_OBJECT_UNKNOWN;

  const CPDF_Object* pObject = pParams->GetObjectFor(key);
  return pObject ? pObject->GetType() : FPDF_OBJECT_UNKNOWN;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObjMark_GetParamIntValue(FPDF_PAGEOBJECTMARK mark,
                                 FPDF_BYTESTRING key,
                                 int* out_value) {
  const CPDF_Dictionary* pParams = GetMarkParamDict(mark);
  if (!pParams)
    return false;

  const CPDF_Object* pObj = pParams->GetObjectFor(key);
  if (!pObj || !pObj->IsNumber())
    return false;

  *out_value = pObj->GetInteger();
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObjMark_GetParamStringValue(FPDF_PAGEOBJECTMARK mark,
                                    FPDF_BYTESTRING key,
                                    void* buffer,
                                    unsigned long buflen,
                                    unsigned long* out_buflen) {
  if (!out_buflen)
    return false;

  const CPDF_Dictionary* pParams = GetMarkParamDict(mark);
  if (!pParams)
    return false;

  const CPDF_Object* pObj = pParams->GetObjectFor(key);
  if (!pObj || !pObj->IsString())
    return false;

  *out_buflen = Utf16EncodeMaybeCopyAndReturnLength(
      WideString::FromUTF8(pObj->GetString().AsStringView()), buffer, buflen);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObjMark_GetParamBlobValue(FPDF_PAGEOBJECTMARK mark,
                                  FPDF_BYTESTRING key,
                                  void* buffer,
                                  unsigned long buflen,
                                  unsigned long* out_buflen) {
  if (!out_buflen)
    return false;

  const CPDF_Dictionary* pParams = GetMarkParamDict(mark);
  if (!pParams)
    return false;

  const CPDF_Object* pObj = pParams->GetObjectFor(key);
  if (!pObj || !pObj->IsString())
    return false;

  ByteString result = pObj->GetString();
  unsigned long len = result.GetLength();

  if (buffer && len <= buflen)
    memcpy(buffer, result.c_str(), len);

  *out_buflen = len;
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObj_HasTransparency(FPDF_PAGEOBJECT pageObject) {
  if (!pageObject)
    return false;

  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(pageObject);
  int blend_type = pPageObj->m_GeneralState.GetBlendType();
  if (blend_type != FXDIB_BLEND_NORMAL)
    return true;

  const CPDF_Dictionary* pSMaskDict =
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
      const CPDF_Transparency& trans = pForm->GetTransparency();
      if (trans.IsGroup() || trans.IsIsolated())
        return true;
    }
  }

  return false;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObjMark_SetIntParam(FPDF_DOCUMENT document,
                            FPDF_PAGEOBJECT page_object,
                            FPDF_PAGEOBJECTMARK mark,
                            FPDF_BYTESTRING key,
                            int value) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj || !PageObjectContainsMark(pPageObj, mark))
    return false;

  CPDF_Dictionary* pParams = GetOrCreateMarkParamsDict(document, mark);
  if (!pParams)
    return false;

  pParams->SetNewFor<CPDF_Number>(key, value);
  pPageObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObjMark_SetStringParam(FPDF_DOCUMENT document,
                               FPDF_PAGEOBJECT page_object,
                               FPDF_PAGEOBJECTMARK mark,
                               FPDF_BYTESTRING key,
                               FPDF_BYTESTRING value) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj || !PageObjectContainsMark(pPageObj, mark))
    return false;

  CPDF_Dictionary* pParams = GetOrCreateMarkParamsDict(document, mark);
  if (!pParams)
    return false;

  pParams->SetNewFor<CPDF_String>(key, value, false);
  pPageObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObjMark_SetBlobParam(FPDF_DOCUMENT document,
                             FPDF_PAGEOBJECT page_object,
                             FPDF_PAGEOBJECTMARK mark,
                             FPDF_BYTESTRING key,
                             void* value,
                             unsigned long value_len) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj || !PageObjectContainsMark(pPageObj, mark))
    return false;

  CPDF_Dictionary* pParams = GetOrCreateMarkParamsDict(document, mark);
  if (!pParams)
    return false;

  if (!value && value_len > 0)
    return false;

  pParams->SetNewFor<CPDF_String>(
      key, ByteString(static_cast<const char*>(value), value_len), true);
  pPageObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObjMark_RemoveParam(FPDF_PAGEOBJECT page_object,
                            FPDF_PAGEOBJECTMARK mark,
                            FPDF_BYTESTRING key) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj)
    return false;

  CPDF_Dictionary* pParams = GetMarkParamDict(mark);
  if (!pParams)
    return false;

  auto removed = pParams->RemoveFor(key);
  if (!removed)
    return false;

  pPageObj->SetDirty(true);
  return true;
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
  pPage->GetDict()->SetNewFor<CPDF_Number>(pdfium::page_object::kRotate,
                                           rotate * 90);
}

FPDF_BOOL FPDFPageObj_SetFillColor(FPDF_PAGEOBJECT page_object,
                                   unsigned int R,
                                   unsigned int G,
                                   unsigned int B,
                                   unsigned int A) {
  if (!page_object || R > 255 || G > 255 || B > 255 || A > 255)
    return false;

  std::vector<float> rgb = {R / 255.f, G / 255.f, B / 255.f};
  auto* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  pPageObj->m_GeneralState.SetFillAlpha(A / 255.f);
  pPageObj->m_ColorState.SetFillColor(
      CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB), rgb);
  pPageObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObj_GetFillColor(FPDF_PAGEOBJECT page_object,
                         unsigned int* R,
                         unsigned int* G,
                         unsigned int* B,
                         unsigned int* A) {
  auto* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj || !R || !G || !B || !A)
    return false;

  FX_COLORREF fillColor = pPageObj->m_ColorState.GetFillColorRef();
  *R = FXSYS_GetRValue(fillColor);
  *G = FXSYS_GetGValue(fillColor);
  *B = FXSYS_GetBValue(fillColor);
  *A = GetUnsignedAlpha(pPageObj->m_GeneralState.GetFillAlpha());
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

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObj_SetStrokeColor(FPDF_PAGEOBJECT page_object,
                           unsigned int R,
                           unsigned int G,
                           unsigned int B,
                           unsigned int A) {
  auto* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj || R > 255 || G > 255 || B > 255 || A > 255)
    return false;

  std::vector<float> rgb = {R / 255.f, G / 255.f, B / 255.f};
  pPageObj->m_GeneralState.SetStrokeAlpha(A / 255.f);
  pPageObj->m_ColorState.SetStrokeColor(
      CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB), rgb);
  pPageObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObj_GetStrokeColor(FPDF_PAGEOBJECT page_object,
                           unsigned int* R,
                           unsigned int* G,
                           unsigned int* B,
                           unsigned int* A) {
  auto* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj || !R || !G || !B || !A)
    return false;

  FX_COLORREF strokeColor = pPageObj->m_ColorState.GetStrokeColorRef();
  *R = FXSYS_GetRValue(strokeColor);
  *G = FXSYS_GetGValue(strokeColor);
  *B = FXSYS_GetBValue(strokeColor);
  *A = GetUnsignedAlpha(pPageObj->m_GeneralState.GetStrokeAlpha());
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObj_SetStrokeWidth(FPDF_PAGEOBJECT page_object, float width) {
  auto* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj || width < 0.0f)
    return false;

  pPageObj->m_GraphState.SetLineWidth(width);
  pPageObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObj_GetStrokeWidth(FPDF_PAGEOBJECT page_object, float* width) {
  auto* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj || !width)
    return false;

  *width = pPageObj->m_GraphState.GetLineWidth();
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObj_SetLineJoin(FPDF_PAGEOBJECT page_object, int line_join) {
  if (!page_object)
    return false;
  if (line_join <
          static_cast<int>(CFX_GraphStateData::LineJoin::LineJoinMiter) ||
      line_join >
          static_cast<int>(CFX_GraphStateData::LineJoin::LineJoinBevel)) {
    return false;
  }
  auto* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  CFX_GraphStateData::LineJoin lineJoin =
      static_cast<CFX_GraphStateData::LineJoin>(line_join);
  pPageObj->m_GraphState.SetLineJoin(lineJoin);
  pPageObj->SetDirty(true);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPageObj_SetLineCap(FPDF_PAGEOBJECT page_object, int line_cap) {
  if (!page_object)
    return false;
  if (line_cap < static_cast<int>(CFX_GraphStateData::LineCap::LineCapButt) ||
      line_cap > static_cast<int>(CFX_GraphStateData::LineCap::LineCapSquare)) {
    return false;
  }
  auto* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  CFX_GraphStateData::LineCap lineCap =
      static_cast<CFX_GraphStateData::LineCap>(line_cap);
  pPageObj->m_GraphState.SetLineCap(lineCap);
  pPageObj->SetDirty(true);
  return true;
}

FPDF_EXPORT int FPDF_CALLCONV
FPDFFormObj_CountObjects(FPDF_PAGEOBJECT page_object) {
  const CPDF_PageObjectList* pObjectList =
      CPDFPageObjListFromFPDFFormObject(page_object);
  if (!pObjectList)
    return -1;

  return pObjectList->size();
}

FPDF_EXPORT FPDF_PAGEOBJECT FPDF_CALLCONV
FPDFFormObj_GetObject(FPDF_PAGEOBJECT form_object, unsigned long index) {
  const CPDF_PageObjectList* pObjectList =
      CPDFPageObjListFromFPDFFormObject(form_object);
  if (!pObjectList)
    return nullptr;

  return FPDFPageObjectFromCPDFPageObject(
      pObjectList->GetPageObjectByIndex(index));
}
