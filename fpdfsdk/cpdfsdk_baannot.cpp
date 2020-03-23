// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_baannot.h"

#include <algorithm>
#include <utility>

#include "constants/annotation_common.h"
#include "constants/annotation_flags.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "fpdfsdk/cpdfsdk_pageview.h"

CPDFSDK_BAAnnot::CPDFSDK_BAAnnot(CPDF_Annot* pAnnot,
                                 CPDFSDK_PageView* pPageView)
    : CPDFSDK_Annot(pPageView), m_pAnnot(pAnnot) {}

CPDFSDK_BAAnnot::~CPDFSDK_BAAnnot() {}

CPDFSDK_BAAnnot* CPDFSDK_BAAnnot::AsBAAnnot() {
  return this;
}

CPDF_Annot* CPDFSDK_BAAnnot::GetPDFAnnot() const {
  return m_pAnnot.Get();
}

CPDF_Annot* CPDFSDK_BAAnnot::GetPDFPopupAnnot() const {
  return m_pAnnot->GetPopupAnnot();
}

CPDF_Dictionary* CPDFSDK_BAAnnot::GetAnnotDict() const {
  return m_pAnnot->GetAnnotDict();
}

CPDF_Dictionary* CPDFSDK_BAAnnot::GetAPDict() const {
  CPDF_Dictionary* pAPDict =
      GetAnnotDict()->GetDictFor(pdfium::annotation::kAP);
  if (pAPDict)
    return pAPDict;
  return GetAnnotDict()->SetNewFor<CPDF_Dictionary>(pdfium::annotation::kAP);
}

void CPDFSDK_BAAnnot::SetRect(const CFX_FloatRect& rect) {
  ASSERT(rect.right - rect.left >= 1.0f);
  ASSERT(rect.top - rect.bottom >= 1.0f);
  GetAnnotDict()->SetRectFor(pdfium::annotation::kRect, rect);
}

CFX_FloatRect CPDFSDK_BAAnnot::GetRect() const {
  return m_pAnnot->GetRect();
}

CPDF_Annot::Subtype CPDFSDK_BAAnnot::GetAnnotSubtype() const {
  return m_pAnnot->GetSubtype();
}

void CPDFSDK_BAAnnot::DrawAppearance(CFX_RenderDevice* pDevice,
                                     const CFX_Matrix& mtUser2Device,
                                     CPDF_Annot::AppearanceMode mode,
                                     const CPDF_RenderOptions* pOptions) {
  m_pAnnot->DrawAppearance(m_pPageView->GetPDFPage(), pDevice, mtUser2Device,
                           mode, pOptions);
}

bool CPDFSDK_BAAnnot::IsAppearanceValid() {
  return !!GetAnnotDict()->GetDictFor(pdfium::annotation::kAP);
}

void CPDFSDK_BAAnnot::SetAnnotName(const WideString& sName) {
  CPDF_Dictionary* pDict = GetAnnotDict();
  if (sName.IsEmpty())
    pDict->RemoveFor(pdfium::annotation::kNM);
  else
    pDict->SetNewFor<CPDF_String>(pdfium::annotation::kNM, sName);
}

WideString CPDFSDK_BAAnnot::GetAnnotName() const {
  return GetAnnotDict()->GetUnicodeTextFor(pdfium::annotation::kNM);
}

void CPDFSDK_BAAnnot::SetFlags(uint32_t nFlags) {
  GetAnnotDict()->SetNewFor<CPDF_Number>(pdfium::annotation::kF,
                                         static_cast<int>(nFlags));
}

uint32_t CPDFSDK_BAAnnot::GetFlags() const {
  return GetAnnotDict()->GetIntegerFor(pdfium::annotation::kF);
}

void CPDFSDK_BAAnnot::SetAppState(const ByteString& str) {
  CPDF_Dictionary* pDict = GetAnnotDict();
  if (str.IsEmpty())
    pDict->RemoveFor(pdfium::annotation::kAS);
  else
    pDict->SetNewFor<CPDF_String>(pdfium::annotation::kAS, str, false);
}

ByteString CPDFSDK_BAAnnot::GetAppState() const {
  return GetAnnotDict()->GetStringFor(pdfium::annotation::kAS);
}

void CPDFSDK_BAAnnot::SetBorderWidth(int nWidth) {
  CPDF_Array* pBorder =
      GetAnnotDict()->GetArrayFor(pdfium::annotation::kBorder);
  if (pBorder) {
    pBorder->SetNewAt<CPDF_Number>(2, nWidth);
  } else {
    CPDF_Dictionary* pBSDict = GetAnnotDict()->GetDictFor("BS");
    if (!pBSDict)
      pBSDict = GetAnnotDict()->SetNewFor<CPDF_Dictionary>("BS");
    pBSDict->SetNewFor<CPDF_Number>("W", nWidth);
  }
}

int CPDFSDK_BAAnnot::GetBorderWidth() const {
  if (const CPDF_Array* pBorder =
          GetAnnotDict()->GetArrayFor(pdfium::annotation::kBorder)) {
    return pBorder->GetIntegerAt(2);
  }

  if (CPDF_Dictionary* pBSDict = GetAnnotDict()->GetDictFor("BS"))
    return pBSDict->GetIntegerFor("W", 1);

  return 1;
}

void CPDFSDK_BAAnnot::SetBorderStyle(BorderStyle nStyle) {
  CPDF_Dictionary* pBSDict = GetAnnotDict()->GetDictFor("BS");
  if (!pBSDict)
    pBSDict = GetAnnotDict()->SetNewFor<CPDF_Dictionary>("BS");

  const char* name = nullptr;
  switch (nStyle) {
    case BorderStyle::SOLID:
      name = "S";
      break;
    case BorderStyle::DASH:
      name = "D";
      break;
    case BorderStyle::BEVELED:
      name = "B";
      break;
    case BorderStyle::INSET:
      name = "I";
      break;
    case BorderStyle::UNDERLINE:
      name = "U";
      break;
    default:
      return;
  }
  pBSDict->SetNewFor<CPDF_Name>("S", name);
}

BorderStyle CPDFSDK_BAAnnot::GetBorderStyle() const {
  CPDF_Dictionary* pBSDict = GetAnnotDict()->GetDictFor("BS");
  if (pBSDict) {
    ByteString sBorderStyle = pBSDict->GetStringFor("S", "S");
    if (sBorderStyle == "S")
      return BorderStyle::SOLID;
    if (sBorderStyle == "D")
      return BorderStyle::DASH;
    if (sBorderStyle == "B")
      return BorderStyle::BEVELED;
    if (sBorderStyle == "I")
      return BorderStyle::INSET;
    if (sBorderStyle == "U")
      return BorderStyle::UNDERLINE;
  }

  const CPDF_Array* pBorder =
      GetAnnotDict()->GetArrayFor(pdfium::annotation::kBorder);
  if (pBorder) {
    if (pBorder->size() >= 4) {
      const CPDF_Array* pDP = pBorder->GetArrayAt(3);
      if (pDP && pDP->size() > 0)
        return BorderStyle::DASH;
    }
  }

  return BorderStyle::SOLID;
}

bool CPDFSDK_BAAnnot::IsVisible() const {
  uint32_t nFlags = GetFlags();
  return !((nFlags & pdfium::annotation_flags::kInvisible) ||
           (nFlags & pdfium::annotation_flags::kHidden) ||
           (nFlags & pdfium::annotation_flags::kNoView));
}

CPDF_Action CPDFSDK_BAAnnot::GetAction() const {
  return CPDF_Action(GetAnnotDict()->GetDictFor("A"));
}

CPDF_AAction CPDFSDK_BAAnnot::GetAAction() const {
  return CPDF_AAction(GetAnnotDict()->GetDictFor("AA"));
}

CPDF_Action CPDFSDK_BAAnnot::GetAAction(CPDF_AAction::AActionType eAAT) {
  CPDF_AAction AAction = GetAAction();
  if (AAction.ActionExist(eAAT))
    return AAction.GetAction(eAAT);

  if (eAAT == CPDF_AAction::kButtonUp)
    return GetAction();

  return CPDF_Action(nullptr);
}

void CPDFSDK_BAAnnot::SetOpenState(bool bOpenState) {
  if (CPDF_Annot* pAnnot = m_pAnnot->GetPopupAnnot())
    pAnnot->SetOpenState(bOpenState);
}

int CPDFSDK_BAAnnot::GetLayoutOrder() const {
  if (m_pAnnot->GetSubtype() == CPDF_Annot::Subtype::POPUP)
    return 1;

  return CPDFSDK_Annot::GetLayoutOrder();
}
