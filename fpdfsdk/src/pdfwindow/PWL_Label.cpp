// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/pdfwindow/PWL_Label.h"
#include "fpdfsdk/include/pdfwindow/PWL_Utils.h"
#include "fpdfsdk/include/pdfwindow/PWL_Wnd.h"

CPWL_Label::CPWL_Label() : m_pEdit(NULL) {
  m_pEdit = IFX_Edit::NewEdit();
  ASSERT(m_pEdit);
}

CPWL_Label::~CPWL_Label() {
  IFX_Edit::DelEdit(m_pEdit);
}

CFX_ByteString CPWL_Label::GetClassName() const {
  return "CPWL_Label";
}

void CPWL_Label::OnCreated() {
  SetParamByFlag();
  SetFontSize(GetCreationParam().fFontSize);

  m_pEdit->SetFontMap(GetFontMap());
  m_pEdit->Initialize();

  if (HasFlag(PES_TEXTOVERFLOW)) {
    SetClipRect(CPDF_Rect(0.0f, 0.0f, 0.0f, 0.0f));
    m_pEdit->SetTextOverflow(TRUE);
  }
}

void CPWL_Label::SetText(const FX_WCHAR* csText) {
  m_pEdit->SetText(csText);
}

void CPWL_Label::RePosChildWnd() {
  m_pEdit->SetPlateRect(GetClientRect());
}

void CPWL_Label::SetFontSize(FX_FLOAT fFontSize) {
  m_pEdit->SetFontSize(fFontSize);
}

FX_FLOAT CPWL_Label::GetFontSize() const {
  return m_pEdit->GetFontSize();
}

void CPWL_Label::SetParamByFlag() {
  if (HasFlag(PES_LEFT)) {
    m_pEdit->SetAlignmentH(0);
  } else if (HasFlag(PES_MIDDLE)) {
    m_pEdit->SetAlignmentH(1);
  } else if (HasFlag(PES_RIGHT)) {
    m_pEdit->SetAlignmentH(2);
  } else {
    m_pEdit->SetAlignmentH(0);
  }

  if (HasFlag(PES_TOP)) {
    m_pEdit->SetAlignmentV(0);
  } else if (HasFlag(PES_CENTER)) {
    m_pEdit->SetAlignmentV(1);
  } else if (HasFlag(PES_BOTTOM)) {
    m_pEdit->SetAlignmentV(2);
  } else {
    m_pEdit->SetAlignmentV(0);
  }

  if (HasFlag(PES_PASSWORD)) {
    m_pEdit->SetPasswordChar('*');
  }

  m_pEdit->SetMultiLine(HasFlag(PES_MULTILINE));
  m_pEdit->SetAutoReturn(HasFlag(PES_AUTORETURN));
  m_pEdit->SetAutoFontSize(HasFlag(PWS_AUTOFONTSIZE));
  m_pEdit->SetAutoScroll(HasFlag(PES_AUTOSCROLL));
}

void CPWL_Label::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                    CFX_Matrix* pUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, pUser2Device);

  GetClientRect();

  CPDF_Rect rcClip;
  CPVT_WordRange wrRange = m_pEdit->GetVisibleWordRange();
  CPVT_WordRange* pRange = NULL;

  if (!HasFlag(PES_TEXTOVERFLOW)) {
    rcClip = GetClientRect();
    pRange = &wrRange;
  }
  IFX_SystemHandler* pSysHandler = GetSystemHandler();
  IFX_Edit::DrawEdit(
      pDevice, pUser2Device, m_pEdit,
      CPWL_Utils::PWLColorToFXColor(GetTextColor(), GetTransparency()),
      CPWL_Utils::PWLColorToFXColor(GetTextStrokeColor(), GetTransparency()),
      rcClip, CPDF_Point(0.0f, 0.0f), pRange, pSysHandler, NULL);
}

void CPWL_Label::SetHorzScale(int32_t nHorzScale) {
  m_pEdit->SetHorzScale(nHorzScale);
}

void CPWL_Label::SetCharSpace(FX_FLOAT fCharSpace) {
  m_pEdit->SetCharSpace(fCharSpace);
}

CPDF_Rect CPWL_Label::GetContentRect() const {
  return m_pEdit->GetContentRect();
}

void CPWL_Label::GetThisAppearanceStream(CFX_ByteTextBuf& sAppStream) {
  CPWL_Wnd::GetThisAppearanceStream(sAppStream);

  sAppStream << GetTextAppearanceStream(CPDF_Point(0.0f, 0.0f));
}

CFX_ByteString CPWL_Label::GetTextAppearanceStream(
    const CPDF_Point& ptOffset) const {
  CFX_ByteTextBuf sRet;
  CFX_ByteString sEdit = CPWL_Utils::GetEditAppStream(m_pEdit, ptOffset);

  if (sEdit.GetLength() > 0) {
    sRet << "BT\n" << CPWL_Utils::GetColorAppStream(GetTextColor()) << sEdit
         << "ET\n";
  }

  return sRet.GetByteString();
}

CFX_WideString CPWL_Label::GetText() const {
  return m_pEdit->GetText();
}

void CPWL_Label::SetLimitChar(int32_t nLimitChar) {
  m_pEdit->SetLimitChar(nLimitChar);
}

int32_t CPWL_Label::GetTotalWords() {
  if (m_pEdit)
    return m_pEdit->GetTotalWords();

  return 0;
}
