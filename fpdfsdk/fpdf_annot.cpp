// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_annot.h"

#include <memory>
#include <utility>

#include "constants/annotation_common.h"
#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"
#include "core/fpdfapi/page/cpdf_annotcontext.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fpdfdoc/cpdf_color_utils.h"
#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "core/fpdfdoc/cpvt_generateap.h"
#include "core/fxge/cfx_color.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_interactiveform.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

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

// These checks ensure the consistency of annotation appearance mode values
// across core/ and public.
static_assert(static_cast<int>(CPDF_Annot::AppearanceMode::Normal) ==
                  FPDF_ANNOT_APPEARANCEMODE_NORMAL,
              "CPDF_Annot::AppearanceMode::Normal value mismatch");
static_assert(static_cast<int>(CPDF_Annot::AppearanceMode::Rollover) ==
                  FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
              "CPDF_Annot::AppearanceMode::Rollover value mismatch");
static_assert(static_cast<int>(CPDF_Annot::AppearanceMode::Down) ==
                  FPDF_ANNOT_APPEARANCEMODE_DOWN,
              "CPDF_Annot::AppearanceMode::Down value mismatch");

// These checks ensure the consistency of dictionary value types across core/
// and public/.
static_assert(static_cast<int>(CPDF_Object::Type::kBoolean) ==
                  FPDF_OBJECT_BOOLEAN,
              "CPDF_Object::kBoolean value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::kNumber) ==
                  FPDF_OBJECT_NUMBER,
              "CPDF_Object::kNumber value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::kString) ==
                  FPDF_OBJECT_STRING,
              "CPDF_Object::kString value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::kName) == FPDF_OBJECT_NAME,
              "CPDF_Object::kName value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::kArray) == FPDF_OBJECT_ARRAY,
              "CPDF_Object::kArray value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::kDictionary) ==
                  FPDF_OBJECT_DICTIONARY,
              "CPDF_Object::kDictionary value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::kStream) ==
                  FPDF_OBJECT_STREAM,
              "CPDF_Object::kStream value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::kNullobj) ==
                  FPDF_OBJECT_NULLOBJ,
              "CPDF_Object::kNullobj value mismatch");
static_assert(static_cast<int>(CPDF_Object::Type::kReference) ==
                  FPDF_OBJECT_REFERENCE,
              "CPDF_Object::kReference value mismatch");

bool HasAPStream(CPDF_Dictionary* pAnnotDict) {
  return !!GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
}

void UpdateContentStream(CPDF_Form* pForm, CPDF_Stream* pStream) {
  ASSERT(pForm);
  ASSERT(pStream);

  CPDF_PageContentGenerator generator(pForm);
  std::ostringstream buf;
  generator.ProcessPageObjects(&buf);
  pStream->SetDataFromStringstreamAndRemoveFilter(&buf);
}

void SetQuadPointsAtIndex(CPDF_Array* array,
                          size_t quad_index,
                          const FS_QUADPOINTSF* quad_points) {
  ASSERT(array);
  ASSERT(quad_points);
  ASSERT(IsValidQuadPointsIndex(array, quad_index));

  size_t nIndex = quad_index * 8;
  array->SetNewAt<CPDF_Number>(nIndex, quad_points->x1);
  array->SetNewAt<CPDF_Number>(++nIndex, quad_points->y1);
  array->SetNewAt<CPDF_Number>(++nIndex, quad_points->x2);
  array->SetNewAt<CPDF_Number>(++nIndex, quad_points->y2);
  array->SetNewAt<CPDF_Number>(++nIndex, quad_points->x3);
  array->SetNewAt<CPDF_Number>(++nIndex, quad_points->y3);
  array->SetNewAt<CPDF_Number>(++nIndex, quad_points->x4);
  array->SetNewAt<CPDF_Number>(++nIndex, quad_points->y4);
}

void AppendQuadPoints(CPDF_Array* array, const FS_QUADPOINTSF* quad_points) {
  ASSERT(quad_points);
  ASSERT(array);

  array->AddNew<CPDF_Number>(quad_points->x1);
  array->AddNew<CPDF_Number>(quad_points->y1);
  array->AddNew<CPDF_Number>(quad_points->x2);
  array->AddNew<CPDF_Number>(quad_points->y2);
  array->AddNew<CPDF_Number>(quad_points->x3);
  array->AddNew<CPDF_Number>(quad_points->y3);
  array->AddNew<CPDF_Number>(quad_points->x4);
  array->AddNew<CPDF_Number>(quad_points->y4);
}

void UpdateBBox(CPDF_Dictionary* annot_dict) {
  ASSERT(annot_dict);
  // Update BBox entry in appearance stream based on the bounding rectangle
  // of the annotation's quadpoints.
  CPDF_Stream* pStream =
      GetAnnotAP(annot_dict, CPDF_Annot::AppearanceMode::Normal);
  if (pStream) {
    CFX_FloatRect boundingRect =
        CPDF_Annot::BoundingRectFromQuadPoints(annot_dict);
    if (boundingRect.Contains(pStream->GetDict()->GetRectFor("BBox")))
      pStream->GetDict()->SetRectFor("BBox", boundingRect);
  }
}

CPDF_Dictionary* GetAnnotDictFromFPDFAnnotation(FPDF_ANNOTATION annot) {
  CPDF_AnnotContext* context = CPDFAnnotContextFromFPDFAnnotation(annot);
  return context ? context->GetAnnotDict() : nullptr;
}

}  // namespace

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_IsSupportedSubtype(FPDF_ANNOTATION_SUBTYPE subtype) {
  // The supported subtypes must also be communicated in the user doc.
  return subtype == FPDF_ANNOT_CIRCLE || subtype == FPDF_ANNOT_FREETEXT ||
         subtype == FPDF_ANNOT_HIGHLIGHT || subtype == FPDF_ANNOT_INK ||
         subtype == FPDF_ANNOT_POPUP || subtype == FPDF_ANNOT_SQUARE ||
         subtype == FPDF_ANNOT_SQUIGGLY || subtype == FPDF_ANNOT_STAMP ||
         subtype == FPDF_ANNOT_STRIKEOUT || subtype == FPDF_ANNOT_TEXT ||
         subtype == FPDF_ANNOT_UNDERLINE;
}

FPDF_EXPORT FPDF_ANNOTATION FPDF_CALLCONV
FPDFPage_CreateAnnot(FPDF_PAGE page, FPDF_ANNOTATION_SUBTYPE subtype) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || !FPDFAnnot_IsSupportedSubtype(subtype))
    return nullptr;

  auto pDict = pPage->GetDocument()->New<CPDF_Dictionary>();
  pDict->SetNewFor<CPDF_Name>(pdfium::annotation::kType, "Annot");
  pDict->SetNewFor<CPDF_Name>(pdfium::annotation::kSubtype,
                              CPDF_Annot::AnnotSubtypeToString(
                                  static_cast<CPDF_Annot::Subtype>(subtype)));
  auto pNewAnnot = pdfium::MakeUnique<CPDF_AnnotContext>(pDict.Get(), pPage);

  CPDF_Array* pAnnotList = pPage->GetDict()->GetArrayFor("Annots");
  if (!pAnnotList)
    pAnnotList = pPage->GetDict()->SetNewFor<CPDF_Array>("Annots");
  pAnnotList->Add(pDict);

  // Caller takes ownership.
  return FPDFAnnotationFromCPDFAnnotContext(pNewAnnot.release());
}

FPDF_EXPORT int FPDF_CALLCONV FPDFPage_GetAnnotCount(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return 0;

  CPDF_Array* pAnnots = pPage->GetDict()->GetArrayFor("Annots");
  return pAnnots ? pAnnots->size() : 0;
}

FPDF_EXPORT FPDF_ANNOTATION FPDF_CALLCONV FPDFPage_GetAnnot(FPDF_PAGE page,
                                                            int index) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || index < 0)
    return nullptr;

  CPDF_Array* pAnnots = pPage->GetDict()->GetArrayFor("Annots");
  if (!pAnnots || static_cast<size_t>(index) >= pAnnots->size())
    return nullptr;

  CPDF_Dictionary* pDict = ToDictionary(pAnnots->GetDirectObjectAt(index));
  if (!pDict)
    return nullptr;

  auto pNewAnnot = pdfium::MakeUnique<CPDF_AnnotContext>(pDict, pPage);

  // Caller takes ownership.
  return FPDFAnnotationFromCPDFAnnotContext(pNewAnnot.release());
}

FPDF_EXPORT int FPDF_CALLCONV FPDFPage_GetAnnotIndex(FPDF_PAGE page,
                                                     FPDF_ANNOTATION annot) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return -1;

  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return -1;

  CPDF_Array* pAnnots = pPage->GetDict()->GetArrayFor("Annots");
  if (!pAnnots)
    return -1;

  CPDF_ArrayLocker locker(pAnnots);
  auto it = std::find_if(locker.begin(), locker.end(),
                         [pAnnotDict](const RetainPtr<CPDF_Object>& candidate) {
                           return candidate->GetDirect() == pAnnotDict;
                         });

  if (it == locker.end())
    return -1;

  return it - locker.begin();
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_CloseAnnot(FPDF_ANNOTATION annot) {
  delete CPDFAnnotContextFromFPDFAnnotation(annot);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFPage_RemoveAnnot(FPDF_PAGE page,
                                                         int index) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage || index < 0)
    return false;

  CPDF_Array* pAnnots = pPage->GetDict()->GetArrayFor("Annots");
  if (!pAnnots || static_cast<size_t>(index) >= pAnnots->size())
    return false;

  pAnnots->RemoveAt(index);
  return true;
}

FPDF_EXPORT FPDF_ANNOTATION_SUBTYPE FPDF_CALLCONV
FPDFAnnot_GetSubtype(FPDF_ANNOTATION annot) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return FPDF_ANNOT_UNKNOWN;

  return static_cast<FPDF_ANNOTATION_SUBTYPE>(CPDF_Annot::StringToAnnotSubtype(
      pAnnotDict->GetStringFor(pdfium::annotation::kSubtype)));
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_IsObjectSupportedSubtype(FPDF_ANNOTATION_SUBTYPE subtype) {
  // The supported subtypes must also be communicated in the user doc.
  return subtype == FPDF_ANNOT_INK || subtype == FPDF_ANNOT_STAMP;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_UpdateObject(FPDF_ANNOTATION annot, FPDF_PAGEOBJECT obj) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  CPDF_PageObject* pObj = CPDFPageObjectFromFPDFPageObject(obj);
  if (!pAnnot || !pAnnot->HasForm() || !pObj)
    return false;

  // Check that the annotation type is supported by this method.
  if (!FPDFAnnot_IsObjectSupportedSubtype(FPDFAnnot_GetSubtype(annot)))
    return false;

  // Check that the annotation already has an appearance stream, since an
  // existing object is to be updated.
  CPDF_Stream* pStream =
      GetAnnotAP(pAnnot->GetAnnotDict(), CPDF_Annot::AppearanceMode::Normal);
  if (!pStream)
    return false;

  // Check that the object is already in this annotation's object list.
  CPDF_Form* pForm = pAnnot->GetForm();
  auto it =
      std::find_if(pForm->begin(), pForm->end(),
                   [pObj](const std::unique_ptr<CPDF_PageObject>& candidate) {
                     return candidate.get() == pObj;
                   });
  if (it == pForm->end())
    return false;

  // Update the content stream data in the annotation's AP stream.
  UpdateContentStream(pForm, pStream);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_AppendObject(FPDF_ANNOTATION annot, FPDF_PAGEOBJECT obj) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  CPDF_PageObject* pObj = CPDFPageObjectFromFPDFPageObject(obj);
  if (!pAnnot || !pObj)
    return false;

  // Check that the annotation type is supported by this method.
  if (!FPDFAnnot_IsObjectSupportedSubtype(FPDFAnnot_GetSubtype(annot)))
    return false;

  // If the annotation does not have an AP stream yet, generate and set it.
  CPDF_Dictionary* pAnnotDict = pAnnot->GetAnnotDict();
  CPDF_Stream* pStream =
      GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
  if (!pStream) {
    CPVT_GenerateAP::GenerateEmptyAP(pAnnot->GetPage()->GetDocument(),
                                     pAnnotDict);
    pStream = GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
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
  auto it =
      std::find_if(pForm->begin(), pForm->end(),
                   [pObj](const std::unique_ptr<CPDF_PageObject>& candidate) {
                     return candidate.get() == pObj;
                   });
  if (it != pForm->end())
    return false;

  // Append the object to the object list.
  pForm->AppendPageObject(pdfium::WrapUnique(pObj));

  // Set the content stream data in the annotation's AP stream.
  UpdateContentStream(pForm, pStream);
  return true;
}

FPDF_EXPORT int FPDF_CALLCONV FPDFAnnot_GetObjectCount(FPDF_ANNOTATION annot) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  if (!pAnnot)
    return 0;

  if (!pAnnot->HasForm()) {
    CPDF_Stream* pStream =
        GetAnnotAP(pAnnot->GetAnnotDict(), CPDF_Annot::AppearanceMode::Normal);
    if (!pStream)
      return 0;

    pAnnot->SetForm(pStream);
  }
  return pAnnot->GetForm()->GetPageObjectCount();
}

FPDF_EXPORT FPDF_PAGEOBJECT FPDF_CALLCONV
FPDFAnnot_GetObject(FPDF_ANNOTATION annot, int index) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  if (!pAnnot || index < 0)
    return nullptr;

  if (!pAnnot->HasForm()) {
    CPDF_Stream* pStream =
        GetAnnotAP(pAnnot->GetAnnotDict(), CPDF_Annot::AppearanceMode::Normal);
    if (!pStream)
      return nullptr;

    pAnnot->SetForm(pStream);
  }

  return FPDFPageObjectFromCPDFPageObject(
      pAnnot->GetForm()->GetPageObjectByIndex(index));
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_RemoveObject(FPDF_ANNOTATION annot, int index) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  if (!pAnnot || !pAnnot->HasForm() || index < 0)
    return false;

  // Check that the annotation type is supported by this method.
  if (!FPDFAnnot_IsObjectSupportedSubtype(FPDFAnnot_GetSubtype(annot)))
    return false;

  // Check that the annotation already has an appearance stream, since an
  // existing object is to be deleted.
  CPDF_Stream* pStream =
      GetAnnotAP(pAnnot->GetAnnotDict(), CPDF_Annot::AppearanceMode::Normal);
  if (!pStream)
    return false;

  if (!pAnnot->GetForm()->ErasePageObjectAtIndex(index))
    return false;

  UpdateContentStream(pAnnot->GetForm(), pStream);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFAnnot_SetColor(FPDF_ANNOTATION annot,
                                                       FPDFANNOT_COLORTYPE type,
                                                       unsigned int R,
                                                       unsigned int G,
                                                       unsigned int B,
                                                       unsigned int A) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict || R > 255 || G > 255 || B > 255 || A > 255)
    return false;

  // For annotations with their appearance streams already defined, the path
  // stream's own color definitions take priority over the annotation color
  // definitions set by this method, hence this method will simply fail.
  if (HasAPStream(pAnnotDict))
    return false;

  // Set the opacity of the annotation.
  pAnnotDict->SetNewFor<CPDF_Number>("CA", A / 255.f);

  // Set the color of the annotation.
  ByteString key = type == FPDFANNOT_COLORTYPE_InteriorColor ? "IC" : "C";
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

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFAnnot_GetColor(FPDF_ANNOTATION annot,
                                                       FPDFANNOT_COLORTYPE type,
                                                       unsigned int* R,
                                                       unsigned int* G,
                                                       unsigned int* B,
                                                       unsigned int* A) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict || !R || !G || !B || !A)
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
    if (pAnnotDict->GetStringFor(pdfium::annotation::kSubtype) == "Highlight") {
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

  CFX_Color color = fpdfdoc::CFXColorFromArray(*pColor);
  switch (color.nColorType) {
    case CFX_Color::kRGB:
      *R = color.fColor1 * 255.f;
      *G = color.fColor2 * 255.f;
      *B = color.fColor3 * 255.f;
      break;
    case CFX_Color::kGray:
      *R = 255.f * color.fColor1;
      *G = 255.f * color.fColor1;
      *B = 255.f * color.fColor1;
      break;
    case CFX_Color::kCMYK:
      *R = 255.f * (1 - color.fColor1) * (1 - color.fColor4);
      *G = 255.f * (1 - color.fColor2) * (1 - color.fColor4);
      *B = 255.f * (1 - color.fColor3) * (1 - color.fColor4);
      break;
    case CFX_Color::kTransparent:
      *R = 0;
      *G = 0;
      *B = 0;
      break;
  }
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_HasAttachmentPoints(FPDF_ANNOTATION annot) {
  if (!annot)
    return false;

  FPDF_ANNOTATION_SUBTYPE subtype = FPDFAnnot_GetSubtype(annot);
  return subtype == FPDF_ANNOT_LINK || subtype == FPDF_ANNOT_HIGHLIGHT ||
         subtype == FPDF_ANNOT_UNDERLINE || subtype == FPDF_ANNOT_SQUIGGLY ||
         subtype == FPDF_ANNOT_STRIKEOUT;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_SetAttachmentPoints(FPDF_ANNOTATION annot,
                              size_t quad_index,
                              const FS_QUADPOINTSF* quad_points) {
  if (!FPDFAnnot_HasAttachmentPoints(annot) || !quad_points)
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  CPDF_Array* pQuadPointsArray = GetQuadPointsArrayFromDictionary(pAnnotDict);
  if (!IsValidQuadPointsIndex(pQuadPointsArray, quad_index))
    return false;

  SetQuadPointsAtIndex(pQuadPointsArray, quad_index, quad_points);
  UpdateBBox(pAnnotDict);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_AppendAttachmentPoints(FPDF_ANNOTATION annot,
                                 const FS_QUADPOINTSF* quad_points) {
  if (!FPDFAnnot_HasAttachmentPoints(annot) || !quad_points)
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  CPDF_Array* pQuadPointsArray = GetQuadPointsArrayFromDictionary(pAnnotDict);
  if (!pQuadPointsArray)
    pQuadPointsArray = AddQuadPointsArrayToDictionary(pAnnotDict);
  AppendQuadPoints(pQuadPointsArray, quad_points);
  UpdateBBox(pAnnotDict);
  return true;
}

FPDF_EXPORT size_t FPDF_CALLCONV
FPDFAnnot_CountAttachmentPoints(FPDF_ANNOTATION annot) {
  if (!FPDFAnnot_HasAttachmentPoints(annot))
    return 0;

  const CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  const CPDF_Array* pArray = GetQuadPointsArrayFromDictionary(pAnnotDict);
  return pArray ? pArray->size() / 8 : 0;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_GetAttachmentPoints(FPDF_ANNOTATION annot,
                              size_t quad_index,
                              FS_QUADPOINTSF* quad_points) {
  if (!FPDFAnnot_HasAttachmentPoints(annot) || !quad_points)
    return false;

  const CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  const CPDF_Array* pArray = GetQuadPointsArrayFromDictionary(pAnnotDict);
  if (!pArray)
    return false;

  return GetQuadPointsAtIndex(pArray, quad_index, quad_points);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFAnnot_SetRect(FPDF_ANNOTATION annot,
                                                      const FS_RECTF* rect) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict || !rect)
    return false;

  CFX_FloatRect newRect = CFXFloatRectFromFSRECTF(*rect);

  // Update the "Rect" entry in the annotation dictionary.
  pAnnotDict->SetRectFor(pdfium::annotation::kRect, newRect);

  // If the annotation's appearance stream is defined, the annotation is of a
  // type that does not have quadpoints, and the new rectangle is bigger than
  // the current bounding box, then update the "BBox" entry in the AP
  // dictionary too, since its "BBox" entry comes from annotation dictionary's
  // "Rect" entry.
  if (FPDFAnnot_HasAttachmentPoints(annot))
    return true;

  CPDF_Stream* pStream =
      GetAnnotAP(pAnnotDict, CPDF_Annot::AppearanceMode::Normal);
  if (pStream && newRect.Contains(pStream->GetDict()->GetRectFor("BBox")))
    pStream->GetDict()->SetRectFor("BBox", newRect);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFAnnot_GetRect(FPDF_ANNOTATION annot,
                                                      FS_RECTF* rect) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict || !rect)
    return false;

  FSRECTFFromCFXFloatRect(pAnnotDict->GetRectFor(pdfium::annotation::kRect),
                          rect);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFAnnot_HasKey(FPDF_ANNOTATION annot,
                                                     FPDF_BYTESTRING key) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  return pAnnotDict && pAnnotDict->KeyExist(key);
}

FPDF_EXPORT FPDF_OBJECT_TYPE FPDF_CALLCONV
FPDFAnnot_GetValueType(FPDF_ANNOTATION annot, FPDF_BYTESTRING key) {
  if (!FPDFAnnot_HasKey(annot, key))
    return FPDF_OBJECT_UNKNOWN;

  auto* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  CPDF_Object* pObj = pAnnot->GetAnnotDict()->GetObjectFor(key);
  return pObj ? pObj->GetType() : FPDF_OBJECT_UNKNOWN;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_SetStringValue(FPDF_ANNOTATION annot,
                         FPDF_BYTESTRING key,
                         FPDF_WIDESTRING value) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return false;

  pAnnotDict->SetNewFor<CPDF_String>(key, WideStringFromFPDFWideString(value));
  return true;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFAnnot_GetStringValue(FPDF_ANNOTATION annot,
                         FPDF_BYTESTRING key,
                         FPDF_WCHAR* buffer,
                         unsigned long buflen) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return 0;

  return Utf16EncodeMaybeCopyAndReturnLength(pAnnotDict->GetUnicodeTextFor(key),
                                             buffer, buflen);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_GetNumberValue(FPDF_ANNOTATION annot,
                         FPDF_BYTESTRING key,
                         float* value) {
  if (!value)
    return false;

  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return false;

  const CPDF_Object* p = pAnnotDict->GetObjectFor(key);
  if (!p || p->GetType() != FPDF_OBJECT_NUMBER)
    return false;

  *value = p->GetNumber();
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_SetAP(FPDF_ANNOTATION annot,
                FPDF_ANNOT_APPEARANCEMODE appearanceMode,
                FPDF_WIDESTRING value) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return false;

  if (appearanceMode < 0 || appearanceMode >= FPDF_ANNOT_APPEARANCEMODE_COUNT)
    return false;

  static constexpr const char* kModeKeyForMode[] = {"N", "R", "D"};
  static_assert(
      FX_ArraySize(kModeKeyForMode) == FPDF_ANNOT_APPEARANCEMODE_COUNT,
      "length of kModeKeyForMode should be equal to "
      "FPDF_ANNOT_APPEARANCEMODE_COUNT");
  const char* modeKey = kModeKeyForMode[appearanceMode];

  CPDF_Dictionary* pApDict = pAnnotDict->GetDictFor(pdfium::annotation::kAP);

  // If value is null, we're in remove mode. Otherwise, we're in add/update
  // mode.
  if (value) {
    if (!pApDict)
      pApDict = pAnnotDict->SetNewFor<CPDF_Dictionary>(pdfium::annotation::kAP);

    ByteString newValue = PDF_EncodeText(WideStringFromFPDFWideString(value));
    auto* pNewApStream = pApDict->SetNewFor<CPDF_Stream>(modeKey);
    pNewApStream->SetData(newValue.raw_span());
  } else {
    if (pApDict) {
      if (appearanceMode == FPDF_ANNOT_APPEARANCEMODE_NORMAL)
        pAnnotDict->RemoveFor(pdfium::annotation::kAP);
      else
        pApDict->RemoveFor(modeKey);
    }
  }

  return true;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFAnnot_GetAP(FPDF_ANNOTATION annot,
                FPDF_ANNOT_APPEARANCEMODE appearanceMode,
                FPDF_WCHAR* buffer,
                unsigned long buflen) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return 0;

  if (appearanceMode < 0 || appearanceMode >= FPDF_ANNOT_APPEARANCEMODE_COUNT)
    return 0;

  CPDF_Annot::AppearanceMode mode =
      static_cast<CPDF_Annot::AppearanceMode>(appearanceMode);

  CPDF_Stream* pStream = GetAnnotAPNoFallback(pAnnotDict, mode);
  return Utf16EncodeMaybeCopyAndReturnLength(
      pStream ? pStream->GetUnicodeText() : WideString(), buffer, buflen);
}

FPDF_EXPORT FPDF_ANNOTATION FPDF_CALLCONV
FPDFAnnot_GetLinkedAnnot(FPDF_ANNOTATION annot, FPDF_BYTESTRING key) {
  CPDF_AnnotContext* pAnnot = CPDFAnnotContextFromFPDFAnnotation(annot);
  if (!pAnnot)
    return nullptr;

  CPDF_Dictionary* pLinkedDict = pAnnot->GetAnnotDict()->GetDictFor(key);
  if (!pLinkedDict || pLinkedDict->GetStringFor("Type") != "Annot")
    return nullptr;

  auto pLinkedAnnot =
      pdfium::MakeUnique<CPDF_AnnotContext>(pLinkedDict, pAnnot->GetPage());

  // Caller takes ownership.
  return FPDFAnnotationFromCPDFAnnotContext(pLinkedAnnot.release());
}

FPDF_EXPORT int FPDF_CALLCONV FPDFAnnot_GetFlags(FPDF_ANNOTATION annot) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  return pAnnotDict ? pAnnotDict->GetIntegerFor(pdfium::annotation::kF)
                    : FPDF_ANNOT_FLAG_NONE;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFAnnot_SetFlags(FPDF_ANNOTATION annot,
                                                       int flags) {
  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return false;

  pAnnotDict->SetNewFor<CPDF_Number>(pdfium::annotation::kF, flags);
  return true;
}

FPDF_EXPORT int FPDF_CALLCONV
FPDFAnnot_GetFormFieldFlags(FPDF_FORMHANDLE hHandle, FPDF_ANNOTATION annot) {
  CPDFSDK_InteractiveForm* pForm = FormHandleToInteractiveForm(hHandle);
  if (!pForm)
    return FPDF_FORMFLAG_NONE;

  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return FPDF_FORMFLAG_NONE;

  CPDF_InteractiveForm* pPDFForm = pForm->GetInteractiveForm();
  CPDF_FormField* pFormField = pPDFForm->GetFieldByDict(pAnnotDict);
  return pFormField ? pFormField->GetFieldFlags() : FPDF_FORMFLAG_NONE;
}

FPDF_EXPORT FPDF_ANNOTATION FPDF_CALLCONV
FPDFAnnot_GetFormFieldAtPoint(FPDF_FORMHANDLE hHandle,
                              FPDF_PAGE page,
                              double page_x,
                              double page_y) {
  CPDFSDK_InteractiveForm* pForm = FormHandleToInteractiveForm(hHandle);
  if (!pForm)
    return nullptr;

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return nullptr;

  CPDF_InteractiveForm* pPDFForm = pForm->GetInteractiveForm();
  int annot_index = -1;
  CPDF_FormControl* pFormCtrl = pPDFForm->GetControlAtPoint(
      pPage, CFX_PointF(static_cast<float>(page_x), static_cast<float>(page_y)),
      &annot_index);
  if (!pFormCtrl || annot_index == -1)
    return nullptr;
  return FPDFPage_GetAnnot(page, annot_index);
}

FPDF_EXPORT int FPDF_CALLCONV FPDFAnnot_GetOptionCount(FPDF_FORMHANDLE hHandle,
                                                       FPDF_ANNOTATION annot) {
  CPDFSDK_InteractiveForm* pForm = FormHandleToInteractiveForm(hHandle);
  if (!pForm)
    return -1;

  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return -1;

  CPDF_InteractiveForm* pPDFForm = pForm->GetInteractiveForm();
  CPDF_FormField* pFormField = pPDFForm->GetFieldByDict(pAnnotDict);
  return pFormField ? pFormField->CountOptions() : -1;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFAnnot_GetOptionLabel(FPDF_FORMHANDLE hHandle,
                         FPDF_ANNOTATION annot,
                         int index,
                         FPDF_WCHAR* buffer,
                         unsigned long buflen) {
  CPDFSDK_InteractiveForm* pForm = FormHandleToInteractiveForm(hHandle);
  if (!pForm)
    return 0;

  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict || index < 0)
    return 0;

  CPDF_InteractiveForm* pPDFForm = pForm->GetInteractiveForm();
  CPDF_FormField* pFormField = pPDFForm->GetFieldByDict(pAnnotDict);
  if (!pFormField || index >= pFormField->CountOptions())
    return 0;

  WideString ws = pFormField->GetOptionLabel(index);
  return Utf16EncodeMaybeCopyAndReturnLength(ws, buffer, buflen);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFAnnot_GetFontSize(FPDF_FORMHANDLE hHandle,
                      FPDF_ANNOTATION annot,
                      float* value) {
  if (!value)
    return false;

  CPDFSDK_InteractiveForm* pForm = FormHandleToInteractiveForm(hHandle);
  if (!pForm)
    return false;

  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return false;

  CPDF_InteractiveForm* pPDFForm = pForm->GetInteractiveForm();
  CPDF_FormControl* pFormControl = pPDFForm->GetControlByDict(pAnnotDict);
  if (!pFormControl)
    return false;

  CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormControl);
  if (!pWidget)
    return false;

  *value = pWidget->GetFontSize();
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFAnnot_IsChecked(FPDF_FORMHANDLE hHandle,
                                                        FPDF_ANNOTATION annot) {
  CPDFSDK_InteractiveForm* pForm = FormHandleToInteractiveForm(hHandle);
  if (!pForm)
    return false;

  CPDF_Dictionary* pAnnotDict = GetAnnotDictFromFPDFAnnotation(annot);
  if (!pAnnotDict)
    return false;

  CPDF_InteractiveForm* pPDFForm = pForm->GetInteractiveForm();
  CPDF_FormField* pFormField = pPDFForm->GetFieldByDict(pAnnotDict);
  if (!pFormField)
    return false;

  if (pFormField->GetType() != CPDF_FormField::kCheckBox &&
      pFormField->GetType() != CPDF_FormField::kRadioButton) {
    return false;
  }

  CPDF_FormControl* pFormControl = pPDFForm->GetControlByDict(pAnnotDict);
  if (!pFormControl)
    return false;

  CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormControl);
  return pWidget && pWidget->IsChecked();
}
