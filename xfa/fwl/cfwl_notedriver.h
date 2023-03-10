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
  void SetGrab(CFWL_Widget* pGrab) { m_pGrab = pGrab; }

 private:
  class Target : public cppgc::GarbageCollected<Target> {
   public:
    explicit Target(CFWL_Widget* pListener);
    ~Target();

    void Trace(cppgc::Visitor* visitor) const;
    void SetEventSource(CFWL_Widget* pSource);
    bool ProcessEvent(CFWL_Event* pEvent);
    bool IsValid() const { return m_bValid; }
    void Invalidate() { m_bValid = false; }

   private:
    bool m_bValid = true;
    cppgc::Member<CFWL_Widget> const m_pListener;
    std::set<cppgc::Member<CFWL_Widget>> m_widgets;
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

  cppgc::Member<CFWL_App> m_pApp;
  std::map<uint64_t, cppgc::Member<Target>> m_eventTargets;
  cppgc::Member<CFWL_Widget> m_pHover;
  cppgc::Member<CFWL_Widget> m_pFocus;
  cppgc::Member<CFWL_Widget> m_pGrab;
};

#endif  // XFA_FWL_CFWL_NOTEDRIVER_H_
