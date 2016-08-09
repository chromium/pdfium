// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/lightwidget/cfwl_edit.h"

#include <memory>
#include <vector>

IFWL_Edit* CFWL_Edit::GetWidget() {
  return static_cast<IFWL_Edit*>(m_pIface.get());
}

const IFWL_Edit* CFWL_Edit::GetWidget() const {
  return static_cast<IFWL_Edit*>(m_pIface.get());
}

CFWL_Edit* CFWL_Edit::Create() {
  return new CFWL_Edit;
}

FWL_Error CFWL_Edit::Initialize(const CFWL_WidgetProperties* pProperties) {
  if (m_pIface)
    return FWL_Error::Indefinite;
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  std::unique_ptr<IFWL_Edit> pEdit(IFWL_Edit::Create(
      m_pProperties->MakeWidgetImpProperties(nullptr), nullptr));
  FWL_Error ret = pEdit->Initialize();
  if (ret != FWL_Error::Succeeded) {
    return ret;
  }
  m_pIface = std::move(pEdit);
  CFWL_Widget::Initialize();
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_Edit::SetText(const CFX_WideString& wsText) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->SetText(wsText);
}

int32_t CFWL_Edit::GetTextLength() const {
  if (!GetWidget())
    return 0;
  return GetWidget()->GetTextLength();
}

FWL_Error CFWL_Edit::GetText(CFX_WideString& wsText,
                             int32_t nStart,
                             int32_t nCount) const {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->GetText(wsText, nStart, nCount);
}

FWL_Error CFWL_Edit::ClearText() {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->ClearText();
}

int32_t CFWL_Edit::GetCaretPos() const {
  if (!GetWidget())
    return -1;
  return GetWidget()->GetCaretPos();
}

int32_t CFWL_Edit::SetCaretPos(int32_t nIndex, FX_BOOL bBefore) {
  if (!GetWidget())
    return -1;
  return GetWidget()->SetCaretPos(nIndex, bBefore);
}

int32_t CFWL_Edit::AddSelRange(int32_t nStart, int32_t nCount) {
  if (!GetWidget())
    return -1;
  GetWidget()->AddSelRange(nStart, nCount);
  int32_t pos = 0;
  int32_t sum = GetWidget()->GetTextLength();
  if (nCount == -1) {
    pos = sum;
  } else {
    pos = nStart + nCount;
  }
  return GetWidget()->SetCaretPos(pos);
}

int32_t CFWL_Edit::CountSelRanges() {
  if (!GetWidget())
    return 0;
  return GetWidget()->CountSelRanges();
}

int32_t CFWL_Edit::GetSelRange(int32_t nIndex, int32_t& nStart) {
  if (!GetWidget())
    return 0;
  return GetWidget()->GetSelRange(nIndex, nStart);
}

FWL_Error CFWL_Edit::ClearSelections() {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->ClearSelections();
}

int32_t CFWL_Edit::GetLimit() {
  if (!GetWidget())
    return -1;
  return GetWidget()->GetLimit();
}

FWL_Error CFWL_Edit::SetLimit(int32_t nLimit) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->SetLimit(nLimit);
}

FWL_Error CFWL_Edit::SetAliasChar(FX_WCHAR wAlias) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->SetAliasChar(wAlias);
}

FWL_Error CFWL_Edit::Insert(int32_t nStart,
                            const FX_WCHAR* lpText,
                            int32_t nLen) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->Insert(nStart, lpText, nLen);
}

FWL_Error CFWL_Edit::DeleteSelections() {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->DeleteSelections();
}

FWL_Error CFWL_Edit::DeleteRange(int32_t nStart, int32_t nCount) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->DeleteRange(nStart, nCount);
}

FWL_Error CFWL_Edit::Replace(int32_t nStart,
                             int32_t nLen,
                             const CFX_WideStringC& wsReplace) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->Replace(nStart, nLen, wsReplace);
}

FWL_Error CFWL_Edit::DoClipboard(int32_t iCmd) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->DoClipboard(iCmd);
}

FX_BOOL CFWL_Edit::Redo(const IFDE_TxtEdtDoRecord* pRecord) {
  return GetWidget() && GetWidget()->Redo(pRecord);
}

FX_BOOL CFWL_Edit::Undo(const IFDE_TxtEdtDoRecord* pRecord) {
  return GetWidget() && GetWidget()->Undo(pRecord);
}

FWL_Error CFWL_Edit::SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->SetTabWidth(fTabWidth, bEquidistant);
}

FWL_Error CFWL_Edit::SetNumberRange(int32_t iMin, int32_t iMax) {
  if (iMin > iMax) {
    return FWL_Error::ParameterInvalid;
  }
  return GetWidget()->SetNumberRange(iMin, iMax);
}

FWL_Error CFWL_Edit::SetBackColor(uint32_t dwColor) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->SetBackColor(dwColor);
}

FWL_Error CFWL_Edit::SetFont(const CFX_WideString& wsFont, FX_FLOAT fSize) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return GetWidget()->SetFont(wsFont, fSize);
}

FX_BOOL CFWL_Edit::CanUndo() {
  return GetWidget()->CanUndo();
}

FX_BOOL CFWL_Edit::CanRedo() {
  return GetWidget()->CanRedo();
}

FX_BOOL CFWL_Edit::Undo() {
  return GetWidget()->Undo();
}

FX_BOOL CFWL_Edit::Redo() {
  return GetWidget()->Undo();
}

FX_BOOL CFWL_Edit::Copy(CFX_WideString& wsCopy) {
  return GetWidget()->Copy(wsCopy);
}

FX_BOOL CFWL_Edit::Cut(CFX_WideString& wsCut) {
  return GetWidget()->Cut(wsCut);
}

FX_BOOL CFWL_Edit::Paste(const CFX_WideString& wsPaste) {
  return GetWidget()->Paste(wsPaste);
}

FX_BOOL CFWL_Edit::Delete() {
  return GetWidget()->Delete();
}

void CFWL_Edit::SetScrollOffset(FX_FLOAT fScrollOffset) {
  return GetWidget()->SetScrollOffset(fScrollOffset);
}

FX_BOOL CFWL_Edit::GetSuggestWords(CFX_PointF pointf,
                                   std::vector<CFX_ByteString>& sSuggest) {
  return GetWidget()->GetSuggestWords(pointf, sSuggest);
}

FX_BOOL CFWL_Edit::ReplaceSpellCheckWord(CFX_PointF pointf,
                                         const CFX_ByteStringC& bsReplace) {
  return GetWidget()->ReplaceSpellCheckWord(pointf, bsReplace);
}

CFWL_Edit::CFWL_Edit() {}

CFWL_Edit::~CFWL_Edit() {}
