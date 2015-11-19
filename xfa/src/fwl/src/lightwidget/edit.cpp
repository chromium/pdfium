// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_Edit* CFWL_Edit::Create() {
  return new CFWL_Edit;
}
FWL_ERR CFWL_Edit::Initialize(const CFWL_WidgetProperties* pProperties) {
  _FWL_RETURN_VALUE_IF_FAIL(!m_pIface, FWL_ERR_Indefinite);
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  CFWL_WidgetImpProperties prop;
  prop.m_dwStyles = m_pProperties->m_dwStyles;
  prop.m_dwStyleExes = m_pProperties->m_dwStyleExes;
  prop.m_dwStates = m_pProperties->m_dwStates;
  prop.m_ctmOnParent = m_pProperties->m_ctmOnParent;
  if (m_pProperties->m_pParent) {
    prop.m_pParent = m_pProperties->m_pParent->GetWidget();
  }
  if (m_pProperties->m_pOwner) {
    prop.m_pOwner = m_pProperties->m_pOwner->GetWidget();
  }
  prop.m_rtWidget = m_pProperties->m_rtWidget;
  m_pIface = IFWL_Edit::Create();
  FWL_ERR ret = ((IFWL_Edit*)m_pIface)->Initialize(prop, nullptr);
  if (ret == FWL_ERR_Succeeded) {
    CFWL_Widget::Initialize();
  }
  return ret;
}
FWL_ERR CFWL_Edit::SetText(const CFX_WideString& wsText) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->SetText(wsText);
}
int32_t CFWL_Edit::GetTextLength() const {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, 0);
  return ((IFWL_Edit*)m_pIface)->GetTextLength();
}
FWL_ERR CFWL_Edit::GetText(CFX_WideString& wsText,
                           int32_t nStart,
                           int32_t nCount) const {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->GetText(wsText, nStart, nCount);
}
FWL_ERR CFWL_Edit::ClearText() {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->ClearText();
}
int32_t CFWL_Edit::GetCaretPos() const {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, -1);
  return ((IFWL_Edit*)m_pIface)->GetCaretPos();
}
int32_t CFWL_Edit::SetCaretPos(int32_t nIndex, FX_BOOL bBefore) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, -1);
  return ((IFWL_Edit*)m_pIface)->SetCaretPos(nIndex, bBefore);
}
FWL_ERR CFWL_Edit::AddSelRange(int32_t nStart, int32_t nCount) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  ((IFWL_Edit*)m_pIface)->AddSelRange(nStart, nCount);
  int32_t pos = 0;
  int32_t sum = ((IFWL_Edit*)m_pIface)->GetTextLength();
  if (nCount == -1) {
    pos = sum;
  } else {
    pos = nStart + nCount;
  }
  return ((IFWL_Edit*)m_pIface)->SetCaretPos(pos);
}
int32_t CFWL_Edit::CountSelRanges() {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, 0);
  return ((IFWL_Edit*)m_pIface)->CountSelRanges();
}
int32_t CFWL_Edit::GetSelRange(int32_t nIndex, int32_t& nStart) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, 0);
  return ((IFWL_Edit*)m_pIface)->GetSelRange(nIndex, nStart);
}
FWL_ERR CFWL_Edit::ClearSelections() {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->ClearSelections();
}
int32_t CFWL_Edit::GetLimit() {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, -1);
  return ((IFWL_Edit*)m_pIface)->GetLimit();
}
FWL_ERR CFWL_Edit::SetLimit(int32_t nLimit) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->SetLimit(nLimit);
}
FWL_ERR CFWL_Edit::SetAliasChar(FX_WCHAR wAlias) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->SetAliasChar(wAlias);
}
FWL_ERR CFWL_Edit::SetFormatString(const CFX_WideString& wsFormat) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->SetFormatString(wsFormat);
}
FWL_ERR CFWL_Edit::Insert(int32_t nStart,
                          const FX_WCHAR* lpText,
                          int32_t nLen) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->Insert(nStart, lpText, nLen);
}
FWL_ERR CFWL_Edit::DeleteSelections() {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->DeleteSelections();
}
FWL_ERR CFWL_Edit::DeleteRange(int32_t nStart, int32_t nCount) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->DeleteRange(nStart, nCount);
}
FWL_ERR CFWL_Edit::ReplaceSelections(const CFX_WideStringC& wsReplace) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->ReplaceSelections(wsReplace);
}
FWL_ERR CFWL_Edit::Replace(int32_t nStart,
                           int32_t nLen,
                           const CFX_WideStringC& wsReplace) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->Replace(nStart, nLen, wsReplace);
}
FWL_ERR CFWL_Edit::DoClipboard(int32_t iCmd) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->DoClipboard(iCmd);
}
FX_BOOL CFWL_Edit::Redo(const CFX_ByteStringC& bsRecord) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FALSE);
  return ((IFWL_Edit*)m_pIface)->Redo(bsRecord);
}
FX_BOOL CFWL_Edit::Undo(const CFX_ByteStringC& bsRecord) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FALSE);
  return ((IFWL_Edit*)m_pIface)->Undo(bsRecord);
}
FWL_ERR CFWL_Edit::SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->SetTabWidth(fTabWidth, bEquidistant);
}
FWL_ERR CFWL_Edit::SetNumberRange(int32_t iMin, int32_t iMax) {
  if (iMin > iMax) {
    return FWL_ERR_Parameter_Invalid;
  }
  return ((IFWL_Edit*)m_pIface)->SetNumberRange(iMin, iMax);
}
FWL_ERR CFWL_Edit::SetBackColor(FX_DWORD dwColor) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->SetBackColor(dwColor);
}
FWL_ERR CFWL_Edit::SetFont(const CFX_WideString& wsFont, FX_FLOAT fSize) {
  _FWL_RETURN_VALUE_IF_FAIL(m_pIface, FWL_ERR_Indefinite);
  return ((IFWL_Edit*)m_pIface)->SetFont(wsFont, fSize);
}
FX_BOOL CFWL_Edit::CanUndo() {
  return ((IFWL_Edit*)m_pIface)->CanUndo();
}
FX_BOOL CFWL_Edit::CanRedo() {
  return ((IFWL_Edit*)m_pIface)->CanRedo();
}
FX_BOOL CFWL_Edit::Undo() {
  return ((IFWL_Edit*)m_pIface)->Undo();
}
FX_BOOL CFWL_Edit::Redo() {
  return ((IFWL_Edit*)m_pIface)->Undo();
}
FX_BOOL CFWL_Edit::Copy(CFX_WideString& wsCopy) {
  return ((IFWL_Edit*)m_pIface)->Copy(wsCopy);
}
FX_BOOL CFWL_Edit::Cut(CFX_WideString& wsCut) {
  return ((IFWL_Edit*)m_pIface)->Cut(wsCut);
}
FX_BOOL CFWL_Edit::Paste(const CFX_WideString& wsPaste) {
  return ((IFWL_Edit*)m_pIface)->Paste(wsPaste);
}
FX_BOOL CFWL_Edit::Delete() {
  return ((IFWL_Edit*)m_pIface)->Delete();
}
void CFWL_Edit::SetScrollOffset(FX_FLOAT fScrollOffset) {
  return ((IFWL_Edit*)m_pIface)->SetScrollOffset(fScrollOffset);
}
FX_BOOL CFWL_Edit::GetSuggestWords(CFX_PointF pointf,
                                   CFX_ByteStringArray& sSuggest) {
  return ((IFWL_Edit*)m_pIface)->GetSuggestWords(pointf, sSuggest);
}
FX_BOOL CFWL_Edit::ReplaceSpellCheckWord(CFX_PointF pointf,
                                         const CFX_ByteStringC& bsReplace) {
  return ((IFWL_Edit*)m_pIface)->ReplaceSpellCheckWord(pointf, bsReplace);
}
CFWL_Edit::CFWL_Edit() {}
CFWL_Edit::~CFWL_Edit() {}
