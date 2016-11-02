// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_edit.h"

#include <memory>
#include <vector>

#include "third_party/base/ptr_util.h"

namespace {

IFWL_Edit* ToEdit(IFWL_Widget* widget) {
  return static_cast<IFWL_Edit*>(widget);
}

const IFWL_Edit* ToEdit(const IFWL_Widget* widget) {
  return static_cast<const IFWL_Edit*>(widget);
}

}  // namespace

CFWL_Edit::CFWL_Edit(const IFWL_App* app) : CFWL_Widget(app) {}

CFWL_Edit::~CFWL_Edit() {}

void CFWL_Edit::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_Edit>(
      m_pApp, m_pProperties->MakeWidgetImpProperties(nullptr), nullptr);

  CFWL_Widget::Initialize();
}

FWL_Error CFWL_Edit::SetText(const CFX_WideString& wsText) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->SetText(wsText);
}

int32_t CFWL_Edit::GetTextLength() const {
  if (!GetWidget())
    return 0;
  return ToEdit(GetWidget())->GetTextLength();
}

FWL_Error CFWL_Edit::GetText(CFX_WideString& wsText,
                             int32_t nStart,
                             int32_t nCount) const {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->GetText(wsText, nStart, nCount);
}

FWL_Error CFWL_Edit::ClearText() {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->ClearText();
}

int32_t CFWL_Edit::GetCaretPos() const {
  if (!GetWidget())
    return -1;
  return ToEdit(GetWidget())->GetCaretPos();
}

int32_t CFWL_Edit::SetCaretPos(int32_t nIndex, bool bBefore) {
  if (!GetWidget())
    return -1;
  return ToEdit(GetWidget())->SetCaretPos(nIndex, bBefore);
}

int32_t CFWL_Edit::AddSelRange(int32_t nStart, int32_t nCount) {
  if (!GetWidget())
    return -1;
  ToEdit(GetWidget())->AddSelRange(nStart, nCount);
  int32_t pos = 0;
  int32_t sum = ToEdit(GetWidget())->GetTextLength();
  if (nCount == -1) {
    pos = sum;
  } else {
    pos = nStart + nCount;
  }
  return ToEdit(GetWidget())->SetCaretPos(pos);
}

int32_t CFWL_Edit::CountSelRanges() {
  if (!GetWidget())
    return 0;
  return ToEdit(GetWidget())->CountSelRanges();
}

int32_t CFWL_Edit::GetSelRange(int32_t nIndex, int32_t& nStart) {
  if (!GetWidget())
    return 0;
  return ToEdit(GetWidget())->GetSelRange(nIndex, nStart);
}

FWL_Error CFWL_Edit::ClearSelections() {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->ClearSelections();
}

int32_t CFWL_Edit::GetLimit() {
  if (!GetWidget())
    return -1;
  return ToEdit(GetWidget())->GetLimit();
}

FWL_Error CFWL_Edit::SetLimit(int32_t nLimit) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->SetLimit(nLimit);
}

FWL_Error CFWL_Edit::SetAliasChar(FX_WCHAR wAlias) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->SetAliasChar(wAlias);
}

FWL_Error CFWL_Edit::Insert(int32_t nStart,
                            const FX_WCHAR* lpText,
                            int32_t nLen) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->Insert(nStart, lpText, nLen);
}

FWL_Error CFWL_Edit::DeleteSelections() {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->DeleteSelections();
}

FWL_Error CFWL_Edit::DeleteRange(int32_t nStart, int32_t nCount) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->DeleteRange(nStart, nCount);
}

FWL_Error CFWL_Edit::Replace(int32_t nStart,
                             int32_t nLen,
                             const CFX_WideStringC& wsReplace) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->Replace(nStart, nLen, wsReplace);
}

FWL_Error CFWL_Edit::DoClipboard(int32_t iCmd) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->DoClipboard(iCmd);
}

bool CFWL_Edit::Redo(const IFDE_TxtEdtDoRecord* pRecord) {
  return GetWidget() && ToEdit(GetWidget())->Redo(pRecord);
}

bool CFWL_Edit::Undo(const IFDE_TxtEdtDoRecord* pRecord) {
  return GetWidget() && ToEdit(GetWidget())->Undo(pRecord);
}

FWL_Error CFWL_Edit::SetTabWidth(FX_FLOAT fTabWidth, bool bEquidistant) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->SetTabWidth(fTabWidth, bEquidistant);
}

FWL_Error CFWL_Edit::SetNumberRange(int32_t iMin, int32_t iMax) {
  if (iMin > iMax)
    return FWL_Error::ParameterInvalid;
  return ToEdit(GetWidget())->SetNumberRange(iMin, iMax);
}

FWL_Error CFWL_Edit::SetBackColor(uint32_t dwColor) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->SetBackgroundColor(dwColor);
}

FWL_Error CFWL_Edit::SetFont(const CFX_WideString& wsFont, FX_FLOAT fSize) {
  if (!GetWidget())
    return FWL_Error::Indefinite;
  return ToEdit(GetWidget())->SetFont(wsFont, fSize);
}

bool CFWL_Edit::CanUndo() {
  return ToEdit(GetWidget())->CanUndo();
}

bool CFWL_Edit::CanRedo() {
  return ToEdit(GetWidget())->CanRedo();
}

bool CFWL_Edit::Undo() {
  return ToEdit(GetWidget())->Undo();
}

bool CFWL_Edit::Redo() {
  return ToEdit(GetWidget())->Undo();
}

bool CFWL_Edit::Copy(CFX_WideString& wsCopy) {
  return ToEdit(GetWidget())->Copy(wsCopy);
}

bool CFWL_Edit::Cut(CFX_WideString& wsCut) {
  return ToEdit(GetWidget())->Cut(wsCut);
}

bool CFWL_Edit::Paste(const CFX_WideString& wsPaste) {
  return ToEdit(GetWidget())->Paste(wsPaste);
}

bool CFWL_Edit::Delete() {
  return ToEdit(GetWidget())->Delete();
}

void CFWL_Edit::SetScrollOffset(FX_FLOAT fScrollOffset) {
  return ToEdit(GetWidget())->SetScrollOffset(fScrollOffset);
}

bool CFWL_Edit::GetSuggestWords(CFX_PointF pointf,
                                std::vector<CFX_ByteString>& sSuggest) {
  return ToEdit(GetWidget())->GetSuggestWords(pointf, sSuggest);
}

bool CFWL_Edit::ReplaceSpellCheckWord(CFX_PointF pointf,
                                      const CFX_ByteStringC& bsReplace) {
  return ToEdit(GetWidget())->ReplaceSpellCheckWord(pointf, bsReplace);
}
