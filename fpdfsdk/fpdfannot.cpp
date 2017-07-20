// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_annot.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fpdfdoc/cpdf_interform.h"
#include "core/fpdfdoc/cpvt_color.h"
#include "core/fpdfdoc/cpvt_generateap.h"
#include "fpdfsdk/fsdk_define.h"

namespace {

// These checks ensure the consistency of annotation subtype values across core/
// and public.
static_assert(static_cast<int>(CPDF_Annot::Subtype::UNKNOWN) ==
                  FPDF_ANNOT_UNKNOWN,
              "CPDF_Annot::UNKNOWN value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::TEXT) == FPDF_ANNOT_TEXT,
              "CPDF_Annot::TEXT value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::LINK) == FPDF_ANNOT_LINK,
              "CPDF_Annot::LINK value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::FREETEXT) ==
                  FPDF_ANNOT_FREETEXT,
              "CPDF_Annot::FREETEXT value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::LINE) == FPDF_ANNOT_LINE,
              "CPDF_Annot::LINE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::SQUARE) ==
                  FPDF_ANNOT_SQUARE,
              "CPDF_Annot::SQUARE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::CIRCLE) ==
                  FPDF_ANNOT_CIRCLE,
              "CPDF_Annot::CIRCLE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::POLYGON) ==
                  FPDF_ANNOT_POLYGON,
              "CPDF_Annot::POLYGON value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::POLYLINE) ==
                  FPDF_ANNOT_POLYLINE,
              "CPDF_Annot::POLYLINE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::HIGHLIGHT) ==
                  FPDF_ANNOT_HIGHLIGHT,
              "CPDF_Annot::HIGHLIGHT value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::UNDERLINE) ==
                  FPDF_ANNOT_UNDERLINE,
              "CPDF_Annot::UNDERLINE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::SQUIGGLY) ==
                  FPDF_ANNOT_SQUIGGLY,
              "CPDF_Annot::SQUIGGLY value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::STRIKEOUT) ==
                  FPDF_ANNOT_STRIKEOUT,
              "CPDF_Annot::STRIKEOUT value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::STAMP) == FPDF_ANNOT_STAMP,
              "CPDF_Annot::STAMP value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::CARET) == FPDF_ANNOT_CARET,
              "CPDF_Annot::CARET value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::INK) == FPDF_ANNOT_INK,
              "CPDF_Annot::INK value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::POPUP) == FPDF_ANNOT_POPUP,
              "CPDF_Annot::POPUP value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::FILEATTACHMENT) ==
                  FPDF_ANNOT_FILEATTACHMENT,
              "CPDF_Annot::FILEATTACHMENT value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::SOUND) == FPDF_ANNOT_SOUND,
              "CPDF_Annot::SOUND value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::MOVIE) == FPDF_ANNOT_MOVIE,
              "CPDF_Annot::MOVIE value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::WIDGET) ==
                  FPDF_ANNOT_WIDGET,
              "CPDF_Annot::WIDGET value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::SCREEN) ==
                  FPDF_ANNOT_SCREEN,
              "CPDF_Annot::SCREEN value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::PRINTERMARK) ==
                  FPDF_ANNOT_PRINTERMARK,
              "CPDF_Annot::PRINTERMARK value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::TRAPNET) ==
                  FPDF_ANNOT_TRAPNET,
              "CPDF_Annot::TRAPNET value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::WATERMARK) ==
                  FPDF_ANNOT_WATERMARK,
              "CPDF_Annot::WATERMARK value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::THREED) ==
                  FPDF_ANNOT_THREED,
              "CPDF_Annot::THREED value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::RICHMEDIA) ==
                  FPDF_ANNOT_RICHMEDIA,
              "CPDF_Annot::RICHMEDIA value mismatch");
static_assert(static_cast<int>(CPDF_Annot::Subtype::XFAWIDGET) ==
                  FPDF_ANNOT_XFAWIDGET,
              "CPDF_Annot::XFAWIDGET value mismatch");

// These checks ensure the consistency of dictionary value types across core/
// and public/.
static_assert(static_cast<int>(CPDF_Object::Type::BOOLEAN) ==
                  FPDF_OBJECT_BOOLEAN,
              "CPDF_Object::BOOLEAN value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::NUMBER) == FPDF_OBJECT_NUMBER,
              "CPDF_Object::NUMBER value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::STRING) == FPDF_OBJECT_STRING,
              "CPDF_Object::STRING value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::NAME) == FPDF_OBJECT_NAME,
              "CPDF_Object::NAME value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::ARRAY) == FPDF_OBJECT_ARRAY,
              "CPDF_Object::ARRAY value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::DICTIONARY) ==
                  FPDF_OBJECT_DICTIONARY,
              "CPDF_Object::DICTIONARY value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::STREAM) == FPDF_OBJECT_STREAM,
              "CPDF_Object::STREAM value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::NULLOBJ) ==
                  FPDF_OBJECT_NULLOBJ,
              "CPDF_Object::NULLOBJ value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::REFERENCE) ==
                  FPDF_OBJECT_REFERENCE,
              "CPDF_Object::REFERENCE value mismatch");

class CPDF_AnnotContext {
 public:
  CPDF_AnnotContext(CPDF_Dictionary* pAnnotDict,
                    CPDF_Page* pPage,
                    CPDF_Stream* pStream)
      : m_pAnnotDict(pAnnotDict), m_pPage(pPage) {
    SetForm(pStream);
  }
  ~CPDF_AnnotContext() {}

  bool HasForm() const { return !!m_pAnnotForm; }

  void SetForm(CPDF_Stream* pStream) {
    if (!pStream)
      return;

    // Reset the annotation matrix to be the identity matrix, since the
    // appearance stream already takes matrix into account.
    pStream->GetDict()->SetMatrixFor("Matrix", CFX_Matrix());

    m_pAnnotForm = pdfium::MakeUnique<CPDF_Form>(
        m_pPage->m_pDocument.Get(), m_pPage->m_pResources.Get(), pStream);
    m_pAnnotForm->ParseContent(nullptr, nullptr, nullptr);
  }

  CPDF_Form* GetForm() const { return m_pAnnotForm.get(); }
  CPDF_Dictionary* GetAnnotDict() const { return m_pAnnotDict.Get(); }
  CPDF_Page* GetPage() const { return m_pPage.Get(); }

 private:
  std::unique_ptr<CPDF_Form> m_pAnnotForm;
  CFX_UnownedPtr<CPDF_Dictionary> m_pAnnotDict;
  CFX_UnownedPtr<CPDF_Page> m_pPage;
};

CPDF_AnnotContext* CPDFAnnotContextFromFPDFAnnotation(FPDF_ANNOTATION annot) {
  return static_cast<CPDF_AnnotContext*>(annot);
}

bool HasAPStream(const CPDF_Dictionary* pAnnotDict) {
  return !!FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
}

void UpdateContentStream(CPDF_Form* pForm, CPDF_Stream* pStream) {
  ASSERT(pForm);
  ASSERT(pStream);

  CPDF_PageContentGenerator generator(pForm);
  std::ostringstream buf;
  generator.ProcessPageObjects(&buf);
  pStream->SetData(&buf);
}

}  // namespace

DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_IsSupportedSubtype(FPDF_ANNOTATION_SUBTYPE subtype) {
  // The supported subtypes must also be communicated in the user doc.
  return subtype == FPDF_ANNOT_CIRCLE || subtype == FPDF_ANNOT_HIGHLIGHT ||
         subtype == FPDF_ANNOT_INK || subtype == FPDF_ANNOT_POPUP ||
         subtype == FPDF_ANNOT_SQUARE || subtype == FPDF_ANNOT_SQUIGGLY ||
         subtype == FPDF_ANNOT_STAMP || subtype == FPDF_ANNOT_STRIKEOUT ||
         subtype == FPDF_ANNOT_TEXT || subtype == FPDF_ANNOT_UNDERLINE;
}

DLLEXPORT FPDF_ANNOTATION STDCALL
FPDFPage_CreateAnnot(FPDF_PAGE page, FPDF_ANNOTATION_SUBTYPE subtype) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !FPDFAnnot_IsSupportedSubtype(subtype))
    return nullptr;

  auto pDict = pdfium::MakeUnique<CPDF_Dictionary>(
      pPage->m_pDocument->GetByteStringPool());
  pDict->SetNewFor<CPDF_Name>("Type", "Annot");
  pDict->SetNewFor<CPDF_Name>("Subtype",
                              CPDF_Annot::AnnotSubtypeToString(
                                  static_cast<CPDF_Annot::Subtype>(subtype)));
  auto pNewAnnot =
      pdfium::MakeUnique<CPDF_AnnotContext>(pDict.get(), pPage, nullptr);

  CPDF_Array* pAnnotList = pPage->m_pFormDict->GetArrayFor("Annots");
  if (!pAnnotList)
    pAnnotList = pPage->m_pFormDict->SetNewFor<CPDF_Array>("Annots");

  pAnnotList->Add(std::move(pDict));
  return pNewAnnot.release();
}

DLLEXPORT int STDCALL FPDFPage_GetAnnotCount(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !pPage->m_pFormDict)
    return 0;

  CPDF_Array* pAnnots = pPage->m_pFormDict->GetArrayFor("Annots");
  return pAnnots ? pAnnots->GetCount() : 0;
}

DLLEXPORT FPDF_ANNOTATION STDCALL FPDFPage_GetAnnot(FPDF_PAGE page, int index) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !pPage->m_pFormDict || index < 0)
    return nullptr;

  CPDF_Array* pAnnots = pPage->m_pFormDict->GetArrayFor("Annots");
  if (!pAnnots || static_cast<size_t>(index) >= pAnnots->GetCount())
    return nullptr;

  CPDF_Dictionary* pDict = ToDictionary(pAnnots->GetDirectObjectAt(index));
  auto pNewAnnot = pdfium::MakeUnique<CPDF_AnnotContext>(pDict, pPage, nullptr);
  return pNewAnnot.release();
}

DLLEXPORT void STDCALL FPDFPage_CloseAnnot(FPDF_ANNOTATION annot) {
  delete CPDFAnnotContextFromFPDFAnnotation(annot);
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPage_RemoveAnnot(FPDF_PAGE page, int index) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !pPage->m_pFormDict || index < 0)
    return false;

  CPDF_Array* pAnnots = pPage->m_pFormDict->GetArrayFor("Annots");
  if (!pAnnots || static_cast<size_t>(index) >= pAnnots->GetCount())
    return false;

  pAnnots->RemoveAt(index);
  return true;
}

DLLEXPORT FPDF_ANNOTATION_SUBTYPE STDCALL
FPDFAnnot_GetSubtype(FPDF_ANNOTATION annot) {
  if (!annot)
    return FPDF_ANNOT_UNKNOWN;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return FPDF_ANNOT_UNKNOWN;

  return static_cast<FPDF_ANNOTATION_SUBTYPE>(
      CPDF_Annot::StringToAnnotSubtype(pAnnotDict->GetStringFor("Subtype")));
}

DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_IsObjectSupportedSubtype(FPDF_ANNOTATION_SUBTYPE subtype) {
  // The supported subtypes must also be communicated in the user doc.
  return subtype == FPDF_ANNOT_INK || subtype == FPDF_ANNOT_STAMP;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_UpdateObject(FPDF_ANNOTATION annot,
                                                   FPDF_PAGEOBJECT obj) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  CPDF_PageObject* pObj = CPDFPageObjectFromFPDFPageObject(obj);
  if (!pAnnot || !pAnnot->GetAnnotDict() || !pAnnot->HasForm() || !pObj)
    return false;

  // Check that the annotation type is supported by this method.
  if (!FPDFAnnot_IsObjectSupportedSubtype(FPDFAnnot_GetSubtype(annot)))
    return false;

  // Check that the annotation already has an appearance stream, since an
  // existing object is to be updated.
  CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(pAnnot->GetAnnotDict(),
                                            CPDF_Annot::AppearanceMode::Normal);
  if (!pStream)
    return false;

  // Check that the object is already in this annotation's object list.
  CPDF_Form* pForm = pAnnot->GetForm();
  CPDF_PageObjectList* pObjList = pForm->GetPageObjectList();
  auto it =
      std::find_if(pObjList->begin(), pObjList->end(),
                   [pObj](const std::unique_ptr<CPDF_PageObject>& candidate) {
                     return candidate.get() == pObj;
                   });
  if (it == pObjList->end())
    return false;

  // Update the content stream data in the annotation's AP stream.
  UpdateContentStream(pForm, pStream);
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_AppendObject(FPDF_ANNOTATION annot,
                                                   FPDF_PAGEOBJECT obj) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  CPDF_PageObject* pObj = CPDFPageObjectFromFPDFPageObject(obj);
  if (!pAnnot || !pObj)
    return false;

  CPDF_Dictionary* pAnnotDict = pAnnot->GetAnnotDict();
  CPDF_Page* pPage = pAnnot->GetPage();
  if (!pAnnotDict || !pPage)
    return false;

  // Check that the annotation type is supported by this method.
  if (!FPDFAnnot_IsObjectSupportedSubtype(FPDFAnnot_GetSubtype(annot)))
    return false;

  // If the annotation does not have an AP stream yet, generate and set it.
  CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(pAnnot->GetAnnotDict(),
                                            CPDF_Annot::AppearanceMode::Normal);
  if (!pStream) {
    auto pExtGStateDict =
        CPVT_GenerateAP::GenerateExtGStateDict(*pAnnotDict, "GS", "Normal");
    auto pResourceDict = CPVT_GenerateAP::GenerateResourceDict(
        pPage->m_pDocument.Get(), std::move(pExtGStateDict), nullptr);
    std::ostringstream sStream;
    CPVT_GenerateAP::GenerateAndSetAPDict(pPage->m_pDocument.Get(), pAnnotDict,
                                          &sStream, std::move(pResourceDict),
                                          false);
    pStream =
        FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
    if (!pStream)
      return false;
  }

  // Get the annotation's corresponding form object for parsing its AP stream.
  if (!pAnnot->HasForm())
    pAnnot->SetForm(pStream);

  // Check that the object did not come from the same annotation. If this check
  // succeeds, then it is assumed that the object came from
  // FPDFPageObj_CreateNew{Path|Rect}() or FPDFPageObj_New{Text|Image}Obj().
  // Note that an object that came from a different annotation must not be
  // passed here, since an object cannot belong to more than one annotation.
  CPDF_Form* pForm = pAnnot->GetForm();
  CPDF_PageObjectList* pObjList = pForm->GetPageObjectList();
  auto it =
      std::find_if(pObjList->begin(), pObjList->end(),
                   [pObj](const std::unique_ptr<CPDF_PageObject>& candidate) {
                     return candidate.get() == pObj;
                   });
  if (it != pObjList->end())
    return false;

  // Append the object to the object list.
  std::unique_ptr<CPDF_PageObject> pPageObjHolder(pObj);
  pObjList->push_back(std::move(pPageObjHolder));

  // Set the content stream data in the annotation's AP stream.
  UpdateContentStream(pForm, pStream);
  return true;
}

DLLEXPORT int STDCALL FPDFAnnot_GetObjectCount(FPDF_ANNOTATION annot) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  if (!pAnnot || !pAnnot->GetAnnotDict())
    return 0;

  if (!pAnnot->HasForm()) {
    CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(
        pAnnot->GetAnnotDict(), CPDF_Annot::AppearanceMode::Normal);
    if (!pStream)
      return 0;

    pAnnot->SetForm(pStream);
  }
  return pdfium::CollectionSize<int>(*pAnnot->GetForm()->GetPageObjectList());
}

DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFAnnot_GetObject(FPDF_ANNOTATION annot,
                                                      int index) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  if (!pAnnot || !pAnnot->GetAnnotDict() || index < 0)
    return nullptr;

  if (!pAnnot->HasForm()) {
    CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(
        pAnnot->GetAnnotDict(), CPDF_Annot::AppearanceMode::Normal);
    if (!pStream)
      return nullptr;

    pAnnot->SetForm(pStream);
  }

  return pAnnot->GetForm()->GetPageObjectList()->GetPageObjectByIndex(index);
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_RemoveObject(FPDF_ANNOTATION annot,
                                                   int index) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  if (!pAnnot || !pAnnot->GetAnnotDict() || !pAnnot->HasForm() || index < 0)
    return false;

  // Check that the annotation type is supported by this method.
  if (!FPDFAnnot_IsObjectSupportedSubtype(FPDFAnnot_GetSubtype(annot)))
    return false;

  // Check that the annotation already has an appearance stream, since an
  // existing object is to be deleted.
  CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(pAnnot->GetAnnotDict(),
                                            CPDF_Annot::AppearanceMode::Normal);
  if (!pStream)
    return false;

  CPDF_PageObjectList* pObjList = pAnnot->GetForm()->GetPageObjectList();
  if (static_cast<size_t>(index) >= pObjList->size())
    return false;

  pObjList->erase(pObjList->begin() + index);
  UpdateContentStream(pAnnot->GetForm(), pStream);
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetColor(FPDF_ANNOTATION annot,
                                               FPDFANNOT_COLORTYPE type,
                                               unsigned int R,
                                               unsigned int G,
                                               unsigned int B,
                                               unsigned int A) {
  if (!annot || R > 255 || G > 255 || B > 255 || A > 255)
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  // For annotations with their appearance streams already defined, the path
  // stream's own color definitions take priority over the annotation color
  // definitions set by this method, hence this method will simply fail.
  if (HasAPStream(pAnnotDict))
    return false;

  // Set the opacity of the annotation.
  pAnnotDict->SetNewFor<CPDF_Number>("CA", A / 255.f);

  // Set the color of the annotation.
  CFX_ByteString key = type == FPDFANNOT_COLORTYPE_InteriorColor ? "IC" : "C";
  CPDF_Array* pColor = pAnnotDict->GetArrayFor(key);
  if (pColor)
    pColor->Clear();
  else
    pColor = pAnnotDict->SetNewFor<CPDF_Array>(key);

  pColor->AddNew<CPDF_Number>(R / 255.f);
  pColor->AddNew<CPDF_Number>(G / 255.f);
  pColor->AddNew<CPDF_Number>(B / 255.f);

  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_GetColor(FPDF_ANNOTATION annot,
                                               FPDFANNOT_COLORTYPE type,
                                               unsigned int* R,
                                               unsigned int* G,
                                               unsigned int* B,
                                               unsigned int* A) {
  if (!annot || !R || !G || !B || !A)
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  // For annotations with their appearance streams already defined, the path
  // stream's own color definitions take priority over the annotation color
  // definitions retrieved by this method, hence this method will simply fail.
  if (HasAPStream(pAnnotDict))
    return false;

  CPDF_Array* pColor = pAnnotDict->GetArrayFor(
      type == FPDFANNOT_COLORTYPE_InteriorColor ? "IC" : "C");
  *A =
      (pAnnotDict->KeyExist("CA") ? pAnnotDict->GetNumberFor("CA") : 1) * 255.f;
  if (!pColor) {
    // Use default color. The default colors must be consistent with the ones
    // used to generate AP. See calls to GetColorStringWithDefault() in
    // CPVT_GenerateAP::Generate*AP().
    if (pAnnotDict->GetStringFor("Subtype") == "Highlight") {
      *R = 255;
      *G = 255;
      *B = 0;
    } else {
      *R = 0;
      *G = 0;
      *B = 0;
    }
    return true;
  }
  CPVT_Color color = CPVT_Color::ParseColor(*pColor);
  switch (color.nColorType) {
    case CPVT_Color::kRGB:
      *R = color.fColor1 * 255.f;
      *G = color.fColor2 * 255.f;
      *B = color.fColor3 * 255.f;
      break;
    case CPVT_Color::kGray:
      *R = 255.f * color.fColor1;
      *G = 255.f * color.fColor1;
      *B = 255.f * color.fColor1;
      break;
    case CPVT_Color::kCMYK:
      *R = 255.f * (1 - color.fColor1) * (1 - color.fColor4);
      *G = 255.f * (1 - color.fColor2) * (1 - color.fColor4);
      *B = 255.f * (1 - color.fColor3) * (1 - color.fColor4);
      break;
    case CPVT_Color::kTransparent:
      *R = 0;
      *G = 0;
      *B = 0;
      break;
  }
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_HasAttachmentPoints(FPDF_ANNOTATION annot) {
  if (!annot)
    return false;

  FPDF_ANNOTATION_SUBTYPE subtype = FPDFAnnot_GetSubtype(annot);
  return subtype == FPDF_ANNOT_LINK || subtype == FPDF_ANNOT_HIGHLIGHT ||
         subtype == FPDF_ANNOT_UNDERLINE || subtype == FPDF_ANNOT_SQUIGGLY ||
         subtype == FPDF_ANNOT_STRIKEOUT;
}

DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_SetAttachmentPoints(FPDF_ANNOTATION annot,
                              const FS_QUADPOINTSF* quadPoints) {
  if (!annot || !quadPoints || !FPDFAnnot_HasAttachmentPoints(annot))
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  // Update the "QuadPoints" entry in the annotation dictionary.
  CPDF_Array* pQuadPoints = pAnnotDict->GetArrayFor("QuadPoints");
  if (pQuadPoints)
    pQuadPoints->Clear();
  else
    pQuadPoints = pAnnotDict->SetNewFor<CPDF_Array>("QuadPoints");

  pQuadPoints->AddNew<CPDF_Number>(quadPoints->x1);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints->y1);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints->x2);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints->y2);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints->x3);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints->y3);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints->x4);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints->y4);

  // If the annotation's appearance stream is defined, and the new quadpoints
  // defines a bigger bounding box than the appearance stream currently
  // specifies, then update the "BBox" entry in the AP dictionary too, since it
  // comes from annotation dictionary's "QuadPoints" entry.
  CPDF_Stream* pStream =
      FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
  if (pStream) {
    CFX_FloatRect newRect = CPDF_Annot::RectFromQuadPoints(pAnnotDict);
    if (newRect.Contains(pStream->GetDict()->GetRectFor("BBox")))
      pStream->GetDict()->SetRectFor("BBox", newRect);
  }
  return true;
}

DLLEXPORT FS_QUADPOINTSF STDCALL
FPDFAnnot_GetAttachmentPoints(FPDF_ANNOTATION annot) {
  if (!annot || !FPDFAnnot_HasAttachmentPoints(annot))
    return FS_QUADPOINTSF();

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return FS_QUADPOINTSF();

  // If the annotation's appearance stream is defined, then retrieve the
  // quadpoints defined by the "BBox" entry in the AP dictionary, since its
  // "BBox" entry comes from annotation dictionary's "QuadPoints" entry, but
  // takes priority over "QuadPoints" when rendering. Otherwise, retrieve
  // the "Quadpoints" entry from the annotation dictionary.
  CPDF_Array* pArray;
  FS_QUADPOINTSF quadPoints;
  CPDF_Stream* pStream =
      FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
  if (pStream) {
    pArray = pStream->GetDict()->GetArrayFor("BBox");
    if (!pArray)
      return FS_QUADPOINTSF();

    // Convert the BBox array into quadpoint coordinates. BBox array follows the
    // order of a rectangle array: (left, bottom, right, up); and quadpoints
    // follows the following order: (top-left vertex, top-right vertex, bottom-
    // left vertex, bottom-right vertex).
    quadPoints.x1 = pArray->GetNumberAt(0);
    quadPoints.y1 = pArray->GetNumberAt(3);
    quadPoints.x2 = pArray->GetNumberAt(2);
    quadPoints.y2 = pArray->GetNumberAt(3);
    quadPoints.x3 = pArray->GetNumberAt(0);
    quadPoints.y3 = pArray->GetNumberAt(1);
    quadPoints.x4 = pArray->GetNumberAt(2);
    quadPoints.y4 = pArray->GetNumberAt(1);
  } else {
    pArray = pAnnotDict->GetArrayFor("QuadPoints");
    if (!pArray)
      return FS_QUADPOINTSF();

    quadPoints.x1 = pArray->GetNumberAt(0);
    quadPoints.y1 = pArray->GetNumberAt(1);
    quadPoints.x2 = pArray->GetNumberAt(2);
    quadPoints.y2 = pArray->GetNumberAt(3);
    quadPoints.x3 = pArray->GetNumberAt(4);
    quadPoints.y3 = pArray->GetNumberAt(5);
    quadPoints.x4 = pArray->GetNumberAt(6);
    quadPoints.y4 = pArray->GetNumberAt(7);
  }
  return quadPoints;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetRect(FPDF_ANNOTATION annot,
                                              const FS_RECTF* rect) {
  if (!annot || !rect)
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  CFX_FloatRect newRect(rect->left, rect->bottom, rect->right, rect->top);

  // Update the "Rect" entry in the annotation dictionary.
  pAnnotDict->SetRectFor("Rect", newRect);

  // If the annotation's appearance stream is defined, the annotation is of a
  // type that does not have quadpoints, and the new rectangle is bigger than
  // the current bounding box, then update the "BBox" entry in the AP
  // dictionary too, since its "BBox" entry comes from annotation dictionary's
  // "Rect" entry.
  if (FPDFAnnot_HasAttachmentPoints(annot))
    return true;

  CPDF_Stream* pStream =
      FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
  if (pStream && newRect.Contains(pStream->GetDict()->GetRectFor("BBox")))
    pStream->GetDict()->SetRectFor("BBox", newRect);
  return true;
}

DLLEXPORT FS_RECTF STDCALL FPDFAnnot_GetRect(FPDF_ANNOTATION annot) {
  if (!annot)
    return FS_RECTF();

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return FS_RECTF();

  // If the annotation's appearance stream is defined and the annotation is of
  // a type that does not have quadpoints, then retrieve the rectangle defined
  // by the "BBox" entry in the AP dictionary, since its "BBox" entry comes
  // from annotation dictionary's "Rect" entry, but takes priority over "Rect"
  // when rendering. Otherwise, retrieve the "Rect" entry from the annotation
  // dictionary.
  CFX_FloatRect rt;
  CPDF_Stream* pStream =
      FPDFDOC_GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
  if (!pStream || FPDFAnnot_HasAttachmentPoints(annot))
    rt = pAnnotDict->GetRectFor("Rect");
  else
    rt = pStream->GetDict()->GetRectFor("BBox");

  FS_RECTF rect;
  rect.left = rt.left;
  rect.bottom = rt.bottom;
  rect.right = rt.right;
  rect.top = rt.top;
  return rect;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_HasKey(FPDF_ANNOTATION annot,
                                             FPDF_WIDESTRING key) {
  if (!annot)
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  return pAnnotDict->KeyExist(CFXByteStringFromFPDFWideString(key));
}

DLLEXPORT FPDF_OBJECT_TYPE STDCALL FPDFAnnot_GetValueType(FPDF_ANNOTATION annot,
                                                          FPDF_WIDESTRING key) {
  if (!FPDFAnnot_HasKey(annot, key))
    return FPDF_OBJECT_UNKNOWN;

  CPDF_Object* pObj =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict()->GetObjectFor(
          CFXByteStringFromFPDFWideString(key));
  if (!pObj)
    return FPDF_OBJECT_UNKNOWN;

  return pObj->GetType();
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetStringValue(FPDF_ANNOTATION annot,
                                                     FPDF_WIDESTRING key,
                                                     FPDF_WIDESTRING value) {
  if (!annot)
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  pAnnotDict->SetNewFor<CPDF_String>(CFXByteStringFromFPDFWideString(key),
                                     CFXByteStringFromFPDFWideString(value),
                                     false);
  return true;
}

DLLEXPORT unsigned long STDCALL FPDFAnnot_GetStringValue(FPDF_ANNOTATION annot,
                                                         FPDF_WIDESTRING key,
                                                         void* buffer,
                                                         unsigned long buflen) {
  if (!annot)
    return 0;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return 0;

  return Utf16EncodeMaybeCopyAndReturnLength(
      pAnnotDict->GetUnicodeTextFor(CFXByteStringFromFPDFWideString(key)),
      buffer, buflen);
}

DLLEXPORT int STDCALL FPDFAnnot_GetFlags(FPDF_ANNOTATION annot) {
  if (!annot)
    return FPDF_ANNOT_FLAG_NONE;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return FPDF_ANNOT_FLAG_NONE;

  return pAnnotDict->GetIntegerFor("F");
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetFlags(FPDF_ANNOTATION annot,
                                               int flags) {
  if (!annot)
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  pAnnotDict->SetNewFor<CPDF_Number>("F", flags);
  return true;
}

DLLEXPORT int STDCALL FPDFAnnot_GetFormFieldFlags(FPDF_PAGE page,
                                                  FPDF_ANNOTATION annot) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !annot)
    return FPDF_FORMFLAG_NONE;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return FPDF_FORMFLAG_NONE;

  CPDF_InterForm interform(pPage->m_pDocument.Get());
  CPDF_FormField* pFormField = interform.GetFieldByDict(pAnnotDict);
  return pFormField ? pFormField->GetFieldFlags() : FPDF_FORMFLAG_NONE;
}

DLLEXPORT FPDF_ANNOTATION STDCALL
FPDFAnnot_GetFormFieldAtPoint(FPDF_FORMHANDLE hHandle,
                              FPDF_PAGE page,
                              double page_x,
                              double page_y) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!hHandle || !pPage)
    return nullptr;

  CPDF_InterForm interform(pPage->m_pDocument.Get());
  int annot_index = -1;
  CPDF_FormControl* pFormCtrl = interform.GetControlAtPoint(
      pPage, CFX_PointF(static_cast<float>(page_x), static_cast<float>(page_y)),
      &annot_index);
  if (!pFormCtrl || annot_index == -1)
    return nullptr;
  return FPDFPage_GetAnnot(page, annot_index);
}
