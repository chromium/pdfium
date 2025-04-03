// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_NOTEDRIVER_H_
#define XFA_FWL_CFWL_NOTEDRIVER_H_

#include <map>
#include <memory>
#include <set>

#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fwl/cfwl_widget.h"

namespace pdfium {

class CFWL_Event;

class CFWL_NoteDriver final : public cppgc::GarbageCollected<CFWL_NoteDriver> {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_NoteDriver();

  void Trace(cppgc::Visitor* visitor) const;

  void SendEvent(CFWL_Event* pNote);
  void ProcessMessage(CFWL_Message* pMessage);
  void RegisterEventTarget(CFWL_Widget* pListener, CFWL_Widget* pEventSource);
  void UnregisterEventTarget(CFWL_Widget* pListener);
  void NotifyTargetHide(CFWL_Widget* pNoteTarget);
  void NotifyTargetDestroy(CFWL_Widget* pNoteTarget);
  void SetGrab(CFWL_Widget* pGrab) { grab_ = pGrab; }

 private:
  class Target : public cppgc::GarbageCollected<Target> {
   public:
    explicit Target(CFWL_Widget* pListener);
    ~Target();

    void Trace(cppgc::Visitor* visitor) const;
    void SetEventSource(CFWL_Widget* pSource);
    bool ProcessEvent(CFWL_Event* pEvent);
    bool IsValid() const { return valid_; }
    void Invalidate() { valid_ = false; }

   private:
    bool valid_ = true;
    cppgc::Member<CFWL_Widget> const listener_;
    std::set<cppgc::Member<CFWL_Widget>> widgets_;
  };

  explicit CFWL_NoteDriver(CFWL_App* pApp);

  bool DispatchMessage(CFWL_Message* pMessage, CFWL_Widget* pMessageForm);
  bool DoSetFocus(CFWL_Message* pMsg, CFWL_Widget* pMessageForm);
  bool DoKillFocus(CFWL_Message* pMsg, CFWL_Widget* pMessageForm);
  bool DoKey(CFWL_Message* pMsg, CFWL_Widget* pMessageForm);
  bool DoMouse(CFWL_Message* pMsg, CFWL_Widget* pMessageForm);
  bool DoWheel(CFWL_Message* pMsg, CFWL_Widget* pMessageForm);
  bool DoMouseEx(CFWL_Message* pMsg, CFWL_Widget* pMessageForm);
  void MouseSecondary(CFWL_Message* pMsg);

  cppgc::Member<CFWL_App> app_;
  cppgc::Member<CFWL_Widget> hover_;
  cppgc::Member<CFWL_Widget> focus_;
  cppgc::Member<CFWL_Widget> grab_;
  std::map<uint64_t, cppgc::Member<Target>> event_targets_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_NoteDriver;

#endif  // XFA_FWL_CFWL_NOTEDRIVER_H_
