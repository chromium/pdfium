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

namespace pdfium {

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
    return adapter_->GetWidgetMgrAdapter();
  }
  CFX_Timer::HandlerIface* GetTimerHandler() const {
    return adapter_->GetTimerHandler();
  }
  IFWL_ThemeProvider* GetThemeProvider() const {
    return adapter_->GetThemeProvider();
  }
  cppgc::Heap* GetHeap() const { return adapter_->GetHeap(); }
  CFWL_WidgetMgr* GetWidgetMgr() const { return widget_mgr_; }
  CFWL_NoteDriver* GetNoteDriver() const { return note_driver_; }

 private:
  explicit CFWL_App(AdapterIface* pAdapter);

  cppgc::Member<AdapterIface> const adapter_;
  cppgc::Member<CFWL_WidgetMgr> widget_mgr_;
  cppgc::Member<CFWL_NoteDriver> note_driver_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_App;

#endif  // XFA_FWL_CFWL_APP_H_
