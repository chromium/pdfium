// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_datetimeedit.h"

#include "xfa/fwl/cfwl_datetimepicker.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_widgetmgr.h"

namespace pdfium {

CFWL_DateTimeEdit::CFWL_DateTimeEdit(CFWL_App* app,
                                     const Properties& properties,
                                     CFWL_Widget* pOuter)
    : CFWL_Edit(app, properties, pOuter) {}

CFWL_DateTimeEdit::~CFWL_DateTimeEdit() = default;

void CFWL_DateTimeEdit::OnProcessMessage(CFWL_Message* pMessage) {
  if (pMessage->GetType() != CFWL_Message::Type::kMouse) {
    CFWL_Edit::OnProcessMessage(pMessage);
    return;
  }

  CFWL_MessageMouse* pMouse = static_cast<CFWL_MessageMouse*>(pMessage);
  if (pMouse->cmd_ == CFWL_MessageMouse::MouseCommand::kLeftButtonDown ||
      pMouse->cmd_ == CFWL_MessageMouse::MouseCommand::kRightButtonDown) {
    if ((properties_.states_ & FWL_STATE_WGT_Focused) == 0) {
      properties_.states_ |= FWL_STATE_WGT_Focused;
    }

    CFWL_DateTimePicker* pDateTime =
        static_cast<CFWL_DateTimePicker*>(GetOuter());
    if (pDateTime->IsMonthCalendarVisible()) {
      CFX_RectF rtInvalidate = pDateTime->GetWidgetRect();
      pDateTime->HideMonthCalendar();
      rtInvalidate.Offset(-rtInvalidate.left, -rtInvalidate.top);
      pDateTime->RepaintRect(rtInvalidate);
    }
  }
  CFWL_Edit::OnProcessMessage(pMessage);
}

}  // namespace pdfium
