// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/formfiller/cffl_textfield.h"

#include <utility>

#include "constants/ascii.h"
#include "constants/form_flags.h"
#include "core/fpdfdoc/cpdf_bafontmap.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/pwl/cpwl_edit.h"
#include "public/fpdf_fwlevent.h"
#include "third_party/base/check.h"

namespace {

// PDF 1.7 spec, Table 8.25
enum Alignment {
  kLeft = 0,
  kCenter = 1,
  kRight = 2,
};

}  // namespace

CFFL_TextField::CFFL_TextField(CPDFSDK_FormFillEnvironment* pApp,
                               CPDFSDK_Widget* pWidget)
    : CFFL_TextObject(pApp, pWidget) {}

CFFL_TextField::~CFFL_TextField() {
  for (const auto& it : m_Maps)
    it.second->InvalidateFocusHandler(this);

  // See comment in cffl_formfiller.h.
  // The font map should be stored somewhere more appropriate so it will live
  // until the PWL_Edit is done with it. pdfium:566
  DestroyWindows();
}

CPWL_Wnd::CreateParams CFFL_TextField::GetCreateParam() {
  CPWL_Wnd::CreateParams cp = CFFL_TextObject::GetCreateParam();
  int nFlags = m_pWidget->GetFieldFlags();
  if (nFlags & pdfium::form_flags::kTextPassword)
    cp.dwFlags |= PES_PASSWORD;

  if (nFlags & pdfium::form_flags::kTextMultiline) {
    cp.dwFlags |= PES_MULTILINE | PES_AUTORETURN | PES_TOP;
    if (!(nFlags & pdfium::form_flags::kTextDoNotScroll))
      cp.dwFlags |= PWS_VSCROLL | PES_AUTOSCROLL;
  } else {
    cp.dwFlags |= PES_CENTER;
    if (!(nFlags & pdfium::form_flags::kTextDoNotScroll))
      cp.dwFlags |= PES_AUTOSCROLL;
  }

  if (nFlags & pdfium::form_flags::kTextComb)
    cp.dwFlags |= PES_CHARARRAY;

  if (nFlags & pdfium::form_flags::kTextRichText)
    cp.dwFlags |= PES_RICH;

  cp.dwFlags |= PES_UNDO;

  switch (m_pWidget->GetAlignment()) {
    default:
    case kLeft:
      cp.dwFlags |= PES_LEFT;
      break;
    case kCenter:
      cp.dwFlags |= PES_MIDDLE;
      break;
    case kRight:
      cp.dwFlags |= PES_RIGHT;
      break;
  }
  cp.pFontMap = GetOrCreateFontMap();
  cp.pFocusHandler = this;
  return cp;
}

std::unique_ptr<CPWL_Wnd> CFFL_TextField::NewPWLWindow(
    const CPWL_Wnd::CreateParams& cp,
    std::unique_ptr<IPWL_SystemHandler::PerWindowData> pAttachedData) {
  auto pWnd = std::make_unique<CPWL_Edit>(cp, std::move(pAttachedData));
  pWnd->AttachFFLData(this);
  pWnd->Realize();
  pWnd->SetFillerNotify(m_pFormFillEnv->GetInteractiveFormFiller());

  int32_t nMaxLen = m_pWidget->GetMaxLen();
  WideString swValue = m_pWidget->GetValue();
  if (nMaxLen > 0) {
    if (pWnd->HasFlag(PES_CHARARRAY)) {
      pWnd->SetCharArray(nMaxLen);
      pWnd->SetAlignFormatVerticalCenter();
    } else {
      pWnd->SetLimitChar(nMaxLen);
    }
  }
  pWnd->SetText(swValue);
  return std::move(pWnd);
}

bool CFFL_TextField::OnChar(CPDFSDK_Annot* pAnnot,
                            uint32_t nChar,
                            uint32_t nFlags) {
  switch (nChar) {
    case pdfium::ascii::kReturn: {
      if (m_pWidget->GetFieldFlags() & pdfium::form_flags::kTextMultiline)
        break;

      CPDFSDK_PageView* pPageView = GetCurPageView();
      DCHECK(pPageView);
      m_bValid = !m_bValid;
      m_pFormFillEnv->Invalidate(pAnnot->GetPage(),
                                 pAnnot->GetRect().GetOuterRect());

      if (m_bValid) {
        if (CPWL_Wnd* pWnd = CreateOrUpdatePWLWindow(pPageView))
          pWnd->SetFocus();
        break;
      }

      if (!CommitData(pPageView, nFlags))
        return false;

      DestroyPWLWindow(pPageView);
      return true;
    }
    case pdfium::ascii::kEscape: {
      CPDFSDK_PageView* pPageView = GetCurPageView();
      DCHECK(pPageView);
      EscapeFiller(pPageView, true);
      return true;
    }
  }

  return CFFL_TextObject::OnChar(pAnnot, nChar, nFlags);
}

bool CFFL_TextField::IsDataChanged(const CPDFSDK_PageView* pPageView) {
  CPWL_Edit* pEdit = GetEdit(pPageView);
  return pEdit && pEdit->GetText() != m_pWidget->GetValue();
}

void CFFL_TextField::SaveData(const CPDFSDK_PageView* pPageView) {
  CPWL_Edit* pWnd = GetEdit(pPageView);
  if (!pWnd)
    return;

  WideString sOldValue = m_pWidget->GetValue();
  WideString sNewValue = pWnd->GetText();
  ObservedPtr<CPDFSDK_Widget> observed_widget(m_pWidget.Get());
  ObservedPtr<CFFL_TextField> observed_this(this);
  m_pWidget->SetValue(sNewValue);
  if (!observed_widget)
    return;

  m_pWidget->ResetFieldAppearance();
  if (!observed_widget)
    return;

  m_pWidget->UpdateField();
  if (!observed_widget || !observed_this)
    return;

  SetChangeMark();
}

void CFFL_TextField::GetActionData(const CPDFSDK_PageView* pPageView,
                                   CPDF_AAction::AActionType type,
                                   CPDFSDK_FieldAction& fa) {
  switch (type) {
    case CPDF_AAction::kKeyStroke:
      if (CPWL_Edit* pWnd = GetEdit(pPageView)) {
        fa.bFieldFull = pWnd->IsTextFull();

        fa.sValue = pWnd->GetText();

        if (fa.bFieldFull) {
          fa.sChange.clear();
          fa.sChangeEx.clear();
        }
      }
      break;
    case CPDF_AAction::kValidate:
      if (CPWL_Edit* pWnd = GetEdit(pPageView)) {
        fa.sValue = pWnd->GetText();
      }
      break;
    case CPDF_AAction::kLoseFocus:
    case CPDF_AAction::kGetFocus:
      fa.sValue = m_pWidget->GetValue();
      break;
    default:
      break;
  }
}

void CFFL_TextField::SetActionData(const CPDFSDK_PageView* pPageView,
                                   CPDF_AAction::AActionType type,
                                   const CPDFSDK_FieldAction& fa) {
  switch (type) {
    case CPDF_AAction::kKeyStroke:
      if (CPWL_Edit* pEdit = GetEdit(pPageView)) {
        pEdit->SetFocus();
        pEdit->SetSelection(fa.nSelStart, fa.nSelEnd);
        pEdit->ReplaceSelection(fa.sChange);
      }
      break;
    default:
      break;
  }
}

void CFFL_TextField::SavePWLWindowState(const CPDFSDK_PageView* pPageView) {
  CPWL_Edit* pWnd = GetEdit(pPageView);
  if (!pWnd)
    return;

  std::tie(m_State.nStart, m_State.nEnd) = pWnd->GetSelection();
  m_State.sValue = pWnd->GetText();
}

void CFFL_TextField::RecreatePWLWindowFromSavedState(
    const CPDFSDK_PageView* pPageView) {
  CPWL_Edit* pWnd = CreateOrUpdateEdit(pPageView);
  if (!pWnd)
    return;

  pWnd->SetText(m_State.sValue);
  pWnd->SetSelection(m_State.nStart, m_State.nEnd);
}

#ifdef PDF_ENABLE_XFA
bool CFFL_TextField::IsFieldFull(const CPDFSDK_PageView* pPageView) {
  CPWL_Edit* pWnd = GetEdit(pPageView);
  return pWnd && pWnd->IsTextFull();
}
#endif  // PDF_ENABLE_XFA

void CFFL_TextField::OnSetFocus(CPWL_Edit* pEdit) {
  pEdit->SetCharSet(FX_Charset::kChineseSimplified);
  pEdit->SetReadyToInput();

  WideString wsText = pEdit->GetText();
  int nCharacters = wsText.GetLength();
  ByteString bsUTFText = wsText.ToUTF16LE();
  auto* pBuffer = reinterpret_cast<const unsigned short*>(bsUTFText.c_str());
  m_pFormFillEnv->OnSetFieldInputFocus(pBuffer, nCharacters, true);
}

CPWL_Edit* CFFL_TextField::GetEdit(const CPDFSDK_PageView* pPageView) const {
  return static_cast<CPWL_Edit*>(GetPWLWindow(pPageView));
}

CPWL_Edit* CFFL_TextField::CreateOrUpdateEdit(
    const CPDFSDK_PageView* pPageView) {
  return static_cast<CPWL_Edit*>(CreateOrUpdatePWLWindow(pPageView));
}
