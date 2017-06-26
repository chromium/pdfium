// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_annot.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfdoc/cpdf_annot.h"
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

    m_pAnnotForm = pdfium::MakeUnique<CPDF_Form>(
        m_pPage->m_pDocument.Get(), m_pPage->m_pResources.Get(), pStream);
    m_pAnnotForm->ParseContent(nullptr, nullptr, nullptr);
  }

  CPDF_Form* GetForm() const { return m_pAnnotForm.get(); }
  CPDF_Dictionary* GetAnnotDict() const { return m_pAnnotDict.Get(); }

 private:
  std::unique_ptr<CPDF_Form> m_pAnnotForm;
  CFX_UnownedPtr<CPDF_Dictionary> m_pAnnotDict;
  CFX_UnownedPtr<CPDF_Page> m_pPage;
};

CPDF_AnnotContext* CPDFAnnotContextFromFPDFAnnotation(FPDF_ANNOTATION annot) {
  return static_cast<CPDF_AnnotContext*>(annot);
}

}  // namespace

DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_IsSupportedSubtype(FPDF_ANNOTATION_SUBTYPE subtype) {
  return subtype == FPDF_ANNOT_CIRCLE || subtype == FPDF_ANNOT_HIGHLIGHT ||
         subtype == FPDF_ANNOT_INK || subtype == FPDF_ANNOT_POPUP ||
         subtype == FPDF_ANNOT_SQUARE || subtype == FPDF_ANNOT_SQUIGGLY ||
         subtype == FPDF_ANNOT_STRIKEOUT || subtype == FPDF_ANNOT_TEXT ||
         subtype == FPDF_ANNOT_UNDERLINE;
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
                              FS_QUADPOINTSF quadPoints) {
  if (!annot || !FPDFAnnot_HasAttachmentPoints(annot))
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  CPDF_Array* pQuadPoints = pAnnotDict->GetArrayFor("QuadPoints");
  if (pQuadPoints)
    pQuadPoints->Clear();
  else
    pQuadPoints = pAnnotDict->SetNewFor<CPDF_Array>("QuadPoints");

  pQuadPoints->AddNew<CPDF_Number>(quadPoints.x1);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.y1);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.x2);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.y2);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.x3);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.y3);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.x4);
  pQuadPoints->AddNew<CPDF_Number>(quadPoints.y4);
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

  CPDF_Array* pArray = pAnnotDict->GetArrayFor("QuadPoints");
  if (!pArray)
    return FS_QUADPOINTSF();

  FS_QUADPOINTSF quadPoints;
  quadPoints.x1 = pArray->GetNumberAt(0);
  quadPoints.y1 = pArray->GetNumberAt(1);
  quadPoints.x2 = pArray->GetNumberAt(2);
  quadPoints.y2 = pArray->GetNumberAt(3);
  quadPoints.x3 = pArray->GetNumberAt(4);
  quadPoints.y3 = pArray->GetNumberAt(5);
  quadPoints.x4 = pArray->GetNumberAt(6);
  quadPoints.y4 = pArray->GetNumberAt(7);
  return quadPoints;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetRect(FPDF_ANNOTATION annot,
                                              FS_RECTF rect) {
  if (!annot)
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  CPDF_Array* pRect = pAnnotDict->GetArrayFor("Rect");
  if (pRect)
    pRect->Clear();
  else
    pRect = pAnnotDict->SetNewFor<CPDF_Array>("Rect");

  pRect->AddNew<CPDF_Number>(rect.left);
  pRect->AddNew<CPDF_Number>(rect.bottom);
  pRect->AddNew<CPDF_Number>(rect.right);
  pRect->AddNew<CPDF_Number>(rect.top);
  return true;
}

DLLEXPORT FS_RECTF STDCALL FPDFAnnot_GetRect(FPDF_ANNOTATION annot) {
  if (!annot)
    return FS_RECTF();

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return FS_RECTF();

  CFX_FloatRect rt = pAnnotDict->GetRectFor("Rect");
  if (rt.IsEmpty())
    return FS_RECTF();

  FS_RECTF rect;
  rect.left = rt.left;
  rect.bottom = rt.bottom;
  rect.right = rt.right;
  rect.top = rt.top;
  return rect;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetText(FPDF_ANNOTATION annot,
                                              FPDFANNOT_TEXTTYPE type,
                                              FPDF_WIDESTRING text) {
  if (!annot)
    return false;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return false;

  CFX_ByteString key = type == FPDFANNOT_TEXTTYPE_Author ? "T" : "Contents";
  FX_STRSIZE len = CFX_WideString::WStringLength(text);
  CFX_WideString encodedText = CFX_WideString::FromUTF16LE(text, len);
  pAnnotDict->SetNewFor<CPDF_String>(key, encodedText.UTF8Encode(), false);
  return true;
}

DLLEXPORT unsigned long STDCALL FPDFAnnot_GetText(FPDF_ANNOTATION annot,
                                                  FPDFANNOT_TEXTTYPE type,
                                                  void* buffer,
                                                  unsigned long buflen) {
  if (!annot)
    return 0;

  CPDF_Dictionary* pAnnotDict =
      CPDFAnnotContextFromFPDFAnnotation(annot)->GetAnnotDict();
  if (!pAnnotDict)
    return 0;

  CFX_ByteString key = type == FPDFANNOT_TEXTTYPE_Author ? "T" : "Contents";
  CFX_ByteString contents = pAnnotDict->GetUnicodeTextFor(key).UTF16LE_Encode();
  unsigned long len = contents.GetLength();
  if (buffer && buflen >= len)
    memcpy(buffer, contents.c_str(), len);

  return len;
}
