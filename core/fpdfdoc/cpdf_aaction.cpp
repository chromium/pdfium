// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_aaction.h"

namespace {

constexpr const char* g_sAATypes[] = {
    "E",   // CursorEnter
    "X",   // CursorExit
    "D",   // ButtonDown
    "U",   // ButtonUp
    "Fo",  // GetFocus
    "Bl",  // LoseFocus
    "PO",  // PageOpen
    "PC",  // PageClose
    "PV",  // PageVisible
    "PI",  // PageInvisible
    "O",   // OpenPage
    "C",   // ClosePage
    "K",   // KeyStroke
    "F",   // Format
    "V",   // Validate
    "C",   // Calculate
    "WC",  // CloseDocument
    "WS",  // SaveDocument
    "DS",  // DocumentSaved
    "WP",  // PrintDocument
    "DP",  // DocumentPrinted
};

// |g_sAATypes| should have one less element than enum AActionType due to
// DocumentOpen, which is an artificial type.
static_assert(FX_ArraySize(g_sAATypes) == CPDF_AAction::NumberOfActions - 1,
              "g_sAATypes count mismatch");

}  // namespace

CPDF_AAction::CPDF_AAction(const CPDF_Dictionary* pDict) : m_pDict(pDict) {}

CPDF_AAction::CPDF_AAction(const CPDF_AAction& that) = default;

CPDF_AAction::~CPDF_AAction() {}

bool CPDF_AAction::ActionExist(AActionType eType) const {
  return m_pDict && m_pDict->KeyExist(g_sAATypes[eType]);
}

CPDF_Action CPDF_AAction::GetAction(AActionType eType) const {
  return CPDF_Action(m_pDict ? m_pDict->GetDictFor(g_sAATypes[eType])
                             : nullptr);
}

// static
bool CPDF_AAction::IsUserClick(AActionType eType) {
  switch (eType) {
    case ButtonUp:
    case ButtonDown:
      return true;
    default:
      return false;
  }
}
