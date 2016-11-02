// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_DATETIMECALENDAR_H_
#define XFA_FWL_CORE_IFWL_DATETIMECALENDAR_H_

#include "xfa/fwl/core/ifwl_monthcalendar.h"

class IFWL_DateTimeCalendar : public IFWL_MonthCalendar {
 public:
  IFWL_DateTimeCalendar(const IFWL_App* app,
                        const CFWL_WidgetImpProperties& properties,
                        IFWL_Widget* pOuter);

 protected:
  friend class CFWL_DateTimeCalendarImpDelegate;
};

class CFWL_DateTimeCalendarImpDelegate : public CFWL_MonthCalendarImpDelegate {
 public:
  CFWL_DateTimeCalendarImpDelegate(IFWL_DateTimeCalendar* pOwner);
  void OnProcessMessage(CFWL_Message* pMessage) override;

  void OnLButtonDownEx(CFWL_MsgMouse* pMsg);
  void OnLButtonUpEx(CFWL_MsgMouse* pMsg);
  void OnMouseMoveEx(CFWL_MsgMouse* pMsg);

 protected:
  IFWL_DateTimeCalendar* m_pOwner;
  FX_BOOL m_bFlag;

 private:
  void DisForm_OnProcessMessage(CFWL_Message* pMessage);
  void DisForm_OnLButtonUpEx(CFWL_MsgMouse* pMsg);
};

#endif  // XFA_FWL_CORE_IFWL_DATETIMECALENDAR_H_
