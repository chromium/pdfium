// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_EVENTTARGET_H_
#define XFA_FWL_CORE_CFWL_EVENTTARGET_H_

#include "core/fxcrt/fx_basic.h"
#include "xfa/fwl/core/cfwl_event.h"

class CFWL_Event;
class IFWL_Widget;

class CFWL_EventTarget {
 public:
  explicit CFWL_EventTarget(IFWL_Widget* pListener);
  ~CFWL_EventTarget();

  int32_t SetEventSource(IFWL_Widget* pSource,
                         uint32_t dwFilter = FWL_EVENT_ALL_MASK);
  bool ProcessEvent(CFWL_Event* pEvent);

  bool IsInvalid() const { return m_bInvalid; }
  void FlagInvalid() { m_bInvalid = true; }

 private:
  bool IsFilterEvent(CFWL_Event* pEvent, uint32_t dwFilter) const;

  CFX_MapPtrTemplate<void*, uint32_t> m_eventSources;
  IFWL_Widget* m_pListener;
  bool m_bInvalid;
};

#endif  // XFA_FWL_CORE_CFWL_EVENTTARGET_H_
