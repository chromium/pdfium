// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_EVENTTARGET_H_
#define XFA_FWL_CFWL_EVENTTARGET_H_

#include <set>

#include "core/fxcrt/fx_basic.h"
#include "xfa/fwl/cfwl_event.h"

class CFWL_Event;
class CFWL_Widget;

class CFWL_EventTarget {
 public:
  explicit CFWL_EventTarget(CFWL_Widget* pListener);
  ~CFWL_EventTarget();

  void SetEventSource(CFWL_Widget* pSource);
  bool ProcessEvent(CFWL_Event* pEvent);

  bool IsInvalid() const { return m_bInvalid; }
  void FlagInvalid() { m_bInvalid = true; }

 private:
  std::set<CFWL_Widget*> m_widgets;
  CFWL_Widget* m_pListener;
  bool m_bInvalid;
};

#endif  // XFA_FWL_CFWL_EVENTTARGET_H_
