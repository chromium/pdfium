// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>

#include "xfa/src/foxitlib.h"

CFWL_Edit* CFWL_Edit::Create() {
  return new CFWL_Edit;
}
FWL_ERR CFWL_Edit::Initialize(const CFWL_WidgetProperties* pProperties) {
  if (m_pIface)
    return FWL_ERR_Indefinite;
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  std::unique_ptr<IFWL_Edit> pEdit(IFWL_Edit::Create(
      m_pProperties->MakeWidgetImpProperties(nullptr), nullptr));
  FWL_ERR ret = pEdit->Initialize();
  if (ret != FWL_ERR_Succeeded) {
    return ret;
  }
  m_pIface = pEdit.release();
  CFWL_Widget::Initialize();
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_Edit::SetText(const CFX_WideString& wsText) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->SetText(wsText);
}
int32_t CFWL_Edit::GetTextLength() const {
  if (!m_pIface)
    return 0;
  return static_cast<IFWL_Edit*>(m_pIface)->GetTextLength();
}
FWL_ERR CFWL_Edit::GetText(CFX_WideString& wsText,
                           int32_t nStart,
                           int32_t nCount) const {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->GetText(wsText, nStart, nCount);
}
FWL_ERR CFWL_Edit::ClearText() {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->ClearText();
}
int32_t CFWL_Edit::GetCaretPos() const {
  if (!m_pIface)
    return -1;
  return static_cast<IFWL_Edit*>(m_pIface)->GetCaretPos();
}
int32_t CFWL_Edit::SetCaretPos(int32_t nIndex, FX_BOOL bBefore) {
  if (!m_pIface)
    return -1;
  return static_cast<IFWL_Edit*>(m_pIface)->SetCaretPos(nIndex, bBefore);
}
FWL_ERR CFWL_Edit::AddSelRange(int32_t nStart, int32_t nCount) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  static_cast<IFWL_Edit*>(m_pIface)->AddSelRange(nStart, nCount);
  int32_t pos = 0;
  int32_t sum = static_cast<IFWL_Edit*>(m_pIface)->GetTextLength();
  if (nCount == -1) {
    pos = sum;
  } else {
    pos = nStart + nCount;
  }
  return static_cast<IFWL_Edit*>(m_pIface)->SetCaretPos(pos);
}
int32_t CFWL_Edit::CountSelRanges() {
  if (!m_pIface)
    return 0;
  return static_cast<IFWL_Edit*>(m_pIface)->CountSelRanges();
}
int32_t CFWL_Edit::GetSelRange(int32_t nIndex, int32_t& nStart) {
  if (!m_pIface)
    return 0;
  return static_cast<IFWL_Edit*>(m_pIface)->GetSelRange(nIndex, nStart);
}
FWL_ERR CFWL_Edit::ClearSelections() {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->ClearSelections();
}
int32_t CFWL_Edit::GetLimit() {
  if (!m_pIface)
    return -1;
  return static_cast<IFWL_Edit*>(m_pIface)->GetLimit();
}
FWL_ERR CFWL_Edit::SetLimit(int32_t nLimit) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->SetLimit(nLimit);
}
FWL_ERR CFWL_Edit::SetAliasChar(FX_WCHAR wAlias) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->SetAliasChar(wAlias);
}
FWL_ERR CFWL_Edit::SetFormatString(const CFX_WideString& wsFormat) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->SetFormatString(wsFormat);
}
FWL_ERR CFWL_Edit::Insert(int32_t nStart,
                          const FX_WCHAR* lpText,
                          int32_t nLen) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->Insert(nStart, lpText, nLen);
}
FWL_ERR CFWL_Edit::DeleteSelections() {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->DeleteSelections();
}
FWL_ERR CFWL_Edit::DeleteRange(int32_t nStart, int32_t nCount) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->DeleteRange(nStart, nCount);
}
FWL_ERR CFWL_Edit::ReplaceSelections(const CFX_WideStringC& wsReplace) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->ReplaceSelections(wsReplace);
}
FWL_ERR CFWL_Edit::Replace(int32_t nStart,
                           int32_t nLen,
                           const CFX_WideStringC& wsReplace) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->Replace(nStart, nLen, wsReplace);
}
FWL_ERR CFWL_Edit::DoClipboard(int32_t iCmd) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->DoClipboard(iCmd);
}
FX_BOOL CFWL_Edit::Redo(const CFX_ByteStringC& bsRecord) {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_Edit*>(m_pIface)->Redo(bsRecord);
}
FX_BOOL CFWL_Edit::Undo(const CFX_ByteStringC& bsRecord) {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_Edit*>(m_pIface)->Undo(bsRecord);
}
FWL_ERR CFWL_Edit::SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)
      ->SetTabWidth(fTabWidth, bEquidistant);
}
FWL_ERR CFWL_Edit::SetNumberRange(int32_t iMin, int32_t iMax) {
  if (iMin > iMax) {
    return FWL_ERR_Parameter_Invalid;
  }
  return static_cast<IFWL_Edit*>(m_pIface)->SetNumberRange(iMin, iMax);
}
FWL_ERR CFWL_Edit::SetBackColor(FX_DWORD dwColor) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->SetBackColor(dwColor);
}
FWL_ERR CFWL_Edit::SetFont(const CFX_WideString& wsFont, FX_FLOAT fSize) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_Edit*>(m_pIface)->SetFont(wsFont, fSize);
}
FX_BOOL CFWL_Edit::CanUndo() {
  return static_cast<IFWL_Edit*>(m_pIface)->CanUndo();
}
FX_BOOL CFWL_Edit::CanRedo() {
  return static_cast<IFWL_Edit*>(m_pIface)->CanRedo();
}
FX_BOOL CFWL_Edit::Undo() {
  return static_cast<IFWL_Edit*>(m_pIface)->Undo();
}
FX_BOOL CFWL_Edit::Redo() {
  return static_cast<IFWL_Edit*>(m_pIface)->Undo();
}
FX_BOOL CFWL_Edit::Copy(CFX_WideString& wsCopy) {
  return static_cast<IFWL_Edit*>(m_pIface)->Copy(wsCopy);
}
FX_BOOL CFWL_Edit::Cut(CFX_WideString& wsCut) {
  return static_cast<IFWL_Edit*>(m_pIface)->Cut(wsCut);
}
FX_BOOL CFWL_Edit::Paste(const CFX_WideString& wsPaste) {
  return static_cast<IFWL_Edit*>(m_pIface)->Paste(wsPaste);
}
FX_BOOL CFWL_Edit::Delete() {
  return static_cast<IFWL_Edit*>(m_pIface)->Delete();
}
void CFWL_Edit::SetScrollOffset(FX_FLOAT fScrollOffset) {
  return static_cast<IFWL_Edit*>(m_pIface)->SetScrollOffset(fScrollOffset);
}
FX_BOOL CFWL_Edit::GetSuggestWords(CFX_PointF pointf,
                                   CFX_ByteStringArray& sSuggest) {
  return static_cast<IFWL_Edit*>(m_pIface)->GetSuggestWords(pointf, sSuggest);
}
FX_BOOL CFWL_Edit::ReplaceSpellCheckWord(CFX_PointF pointf,
                                         const CFX_ByteStringC& bsReplace) {
  return static_cast<IFWL_Edit*>(m_pIface)
      ->ReplaceSpellCheckWord(pointf, bsReplace);
}
CFWL_Edit::CFWL_Edit() {}
CFWL_Edit::~CFWL_Edit() {}
