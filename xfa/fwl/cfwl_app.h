// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_APP_H_
#define XFA_FWL_CFWL_APP_H_

#include "core/fxcrt/cfx_timer.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fwl/cfwl_widgetmgr.h"

class CFWL_NoteDriver;
class IFWL_ThemeProvider;

class CFWL_App final : public cppgc::GarbageCollected<CFWL_App> {
 public:
  class AdapterIface : public cppgc::GarbageCollectedMixin {
   public:
    virtual ~AdapterIface() = default;
    virtual CFWL_WidgetMgr::AdapterIface* GetWidgetMgrAdapter() = 0;
    virtual CFX_Timer::HandlerIface* GetTimerHandler() = 0;
    virtual IFWL_ThemeProvider* GetThemeProvider() = 0;
    virtual cppgc::Heap* GetHeap() = 0;
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_App();

  void Trace(cppgc::Visitor* visitor) const;

  CFWL_WidgetMgr::AdapterIface* GetWidgetMgrAdapter() const {
    return m_pAdapter->GetWidgetMgrAdapter();
  }
  CFX_Timer::HandlerIface* GetTimerHandler() const {
    return m_pAdapter->GetTimerHandler();
  }
  IFWL_ThemeProvider* GetThemeProvider() const {
    return m_pAdapter->GetThemeProvider();
  }
  cppgc::Heap* GetHeap() const { return m_pAdapter->GetHeap(); }
  CFWL_WidgetMgr* GetWidgetMgr() const { return m_pWidgetMgr; }
  CFWL_NoteDriver* GetNoteDriver() const { return m_pNoteDriver; }

 private:
  explicit CFWL_App(AdapterIface* pAdapter);

  cppgc::Member<AdapterIface> const m_pAdapter;
  cppgc::Member<CFWL_WidgetMgr> m_pWidgetMgr;
  cppgc::Member<CFWL_NoteDriver> m_pNoteDriver;
};

#endif  // XFA_FWL_CFWL_APP_H_
