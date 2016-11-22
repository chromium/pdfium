// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_EVENTTARGET_H_
#define XFA_FWL_CORE_CFWL_EVENTTARGET_H_

#include <set>

#include "core/fxcrt/fx_basic.h"
#include "xfa/fwl/core/cfwl_event.h"

class CFWL_Event;
class IFWL_Widget;

class CFWL_EventTarget {
 public:
  explicit CFWL_EventTarget(IFWL_Widget* pListener);
  ~CFWL_EventTarget();

  void SetEventSource(IFWL_Widget* pSource);
  bool ProcessEvent(CFWL_Event* pEvent);

  bool IsInvalid() const { return m_bInvalid; }
  void FlagInvalid() { m_bInvalid = true; }

 private:
  std::set<IFWL_Widget*> m_widgets;
  IFWL_Widget* m_pListener;
  bool m_bInvalid;
};

#endif  // XFA_FWL_CORE_CFWL_EVENTTARGET_H_
