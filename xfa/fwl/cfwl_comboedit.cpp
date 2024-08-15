// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_comboedit.h"

#include "xfa/fde/cfde_texteditengine.h"
#include "xfa/fwl/cfwl_combobox.h"
#include "xfa/fwl/cfwl_messagemouse.h"

namespace pdfium {

CFWL_ComboEdit::CFWL_ComboEdit(CFWL_App* app,
                               const Properties& properties,
                               CFWL_Widget* pOuter)
    : CFWL_Edit(app, properties, pOuter) {}

CFWL_ComboEdit::~CFWL_ComboEdit() = default;

void CFWL_ComboEdit::ClearSelected() {
  ClearSelection();
  RepaintRect(GetRTClient());
}

void CFWL_ComboEdit::SetSelected() {
  m_Properties.m_dwStates |= FWL_STATE_WGT_Focused;
  SelectAll();
}

void CFWL_ComboEdit::OnProcessMessage(CFWL_Message* pMessage) {
  bool backDefault = true;
  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus: {
      m_Properties.m_dwStates |= FWL_STATE_WGT_Focused;
      backDefault = false;
      break;
    }
    case CFWL_Message::Type::kKillFocus: {
      m_Properties.m_dwStates &= ~FWL_STATE_WGT_Focused;
      backDefault = false;
      break;
    }
    case CFWL_Message::Type::kMouse: {
      CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
      if ((pMsg->m_dwCmd == CFWL_MessageMouse::MouseCommand::kLeftButtonDown) &&
          ((m_Properties.m_dwStates & FWL_STATE_WGT_Focused) == 0)) {
        SetSelected();
      }
      break;
    }
    default:
      break;
  }
  if (backDefault)
    CFWL_Edit::OnProcessMessage(pMessage);
}

}  // namespace pdfium
