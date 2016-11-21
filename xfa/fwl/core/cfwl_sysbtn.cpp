// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_sysbtn.h"

#include "xfa/fwl/core/cfwl_themepart.h"

CFWL_SysBtn::CFWL_SysBtn() {
  m_rtBtn.Set(0, 0, 0, 0);
  m_dwState = 0;
}

CFWL_SysBtn::~CFWL_SysBtn() {}

bool CFWL_SysBtn::IsDisabled() const {
  return !!(m_dwState & FWL_SYSBUTTONSTATE_Disabled);
}

void CFWL_SysBtn::SetNormal() {
  m_dwState &= 0xFFF0;
}

void CFWL_SysBtn::SetPressed() {
  SetNormal();
  m_dwState |= FWL_SYSBUTTONSTATE_Pressed;
}

void CFWL_SysBtn::SetHover() {
  SetNormal();
  m_dwState |= FWL_SYSBUTTONSTATE_Hover;
}

void CFWL_SysBtn::SetDisabled(bool bDisabled) {
  bDisabled ? m_dwState |= FWL_SYSBUTTONSTATE_Disabled
            : m_dwState &= ~FWL_SYSBUTTONSTATE_Disabled;
}

uint32_t CFWL_SysBtn::GetPartState() const {
  if (IsDisabled())
    return CFWL_PartState_Disabled;
  if (m_dwState & FWL_SYSBUTTONSTATE_Pressed)
    return CFWL_PartState_Pressed;
  if (m_dwState & FWL_SYSBUTTONSTATE_Hover)
    return CFWL_PartState_Hovered;
  return CFWL_PartState_Normal;
}
