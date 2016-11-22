// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_eventtarget.h"

#include "xfa/fwl/core/ifwl_widget.h"
#include "xfa/fwl/core/ifwl_widgetdelegate.h"

CFWL_EventTarget::CFWL_EventTarget(IFWL_Widget* pListener)
    : m_pListener(pListener), m_bInvalid(false) {}

CFWL_EventTarget::~CFWL_EventTarget() {
  m_eventSources.RemoveAll();
}

int32_t CFWL_EventTarget::SetEventSource(IFWL_Widget* pSource,
                                         uint32_t dwFilter) {
  if (pSource) {
    m_eventSources.SetAt(pSource, dwFilter);
    return m_eventSources.GetCount();
  }
  return 1;
}

bool CFWL_EventTarget::ProcessEvent(CFWL_Event* pEvent) {
  IFWL_WidgetDelegate* pDelegate = m_pListener->GetDelegate();
  if (!pDelegate)
    return false;
  if (m_eventSources.GetCount() == 0) {
    pDelegate->OnProcessEvent(pEvent);
    return true;
  }

  FX_POSITION pos = m_eventSources.GetStartPosition();
  while (pos) {
    IFWL_Widget* pSource = nullptr;
    uint32_t dwFilter = 0;
    m_eventSources.GetNextAssoc(pos, (void*&)pSource, dwFilter);
    if (pSource == pEvent->m_pSrcTarget) {
      if (IsFilterEvent(pEvent, dwFilter)) {
        pDelegate->OnProcessEvent(pEvent);
        return true;
      }
    }
  }
  return false;
}

bool CFWL_EventTarget::IsFilterEvent(CFWL_Event* pEvent,
                                     uint32_t dwFilter) const {
  if (dwFilter == FWL_EVENT_ALL_MASK)
    return true;

  switch (pEvent->GetClassID()) {
    case CFWL_EventType::Mouse:
      return !!(dwFilter & FWL_EVENT_MOUSE_MASK);
    case CFWL_EventType::MouseWheel:
      return !!(dwFilter & FWL_EVENT_MOUSEWHEEL_MASK);
    case CFWL_EventType::Key:
      return !!(dwFilter & FWL_EVENT_KEY_MASK);
    case CFWL_EventType::SetFocus:
    case CFWL_EventType::KillFocus:
      return !!(dwFilter & FWL_EVENT_FOCUSCHANGED_MASK);
    case CFWL_EventType::Close:
      return !!(dwFilter & FWL_EVENT_CLOSE_MASK);
    case CFWL_EventType::SizeChanged:
      return !!(dwFilter & FWL_EVENT_SIZECHANGED_MASK);
    default:
      return !!(dwFilter & FWL_EVENT_CONTROL_MASK);
  }
}
