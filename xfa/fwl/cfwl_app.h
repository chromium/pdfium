// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_APP_H_
#define XFA_FWL_CFWL_APP_H_

#include "core/fxcrt/timerhandler_iface.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fwl/cfwl_widgetmgr.h"

class CFWL_NoteDriver;
class IFWL_ThemeProvider;

enum FWL_KeyFlag {
  FWL_KEYFLAG_Ctrl = 1 << 0,
  FWL_KEYFLAG_Alt = 1 << 1,
  FWL_KEYFLAG_Shift = 1 << 2,
  FWL_KEYFLAG_Command = 1 << 3,
  FWL_KEYFLAG_LButton = 1 << 4,
  FWL_KEYFLAG_RButton = 1 << 5,
  FWL_KEYFLAG_MButton = 1 << 6
};

class CFWL_App final : public cppgc::GarbageCollected<CFWL_App> {
 public:
  class AdapterIface : public cppgc::GarbageCollectedMixin {
   public:
    virtual ~AdapterIface() = default;
    virtual CFWL_WidgetMgr::AdapterIface* GetWidgetMgrAdapter() = 0;
    virtual TimerHandlerIface* GetTimerHandler() = 0;
    virtual IFWL_ThemeProvider* GetThemeProvider() = 0;
    virtual cppgc::Heap* GetHeap() = 0;
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_App();

  void Trace(cppgc::Visitor* visitor) const;

  CFWL_WidgetMgr::AdapterIface* GetWidgetMgrAdapter() const {
    return m_pAdapter->GetWidgetMgrAdapter();
  }
  TimerHandlerIface* GetTimerHandler() const {
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
