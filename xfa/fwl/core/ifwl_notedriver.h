// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_NOTEDRIVER_H_
#define XFA_FWL_CORE_IFWL_NOTEDRIVER_H_

#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/fwl_error.h"

#define FWL_KEYFLAG_Ctrl (1 << 0)
#define FWL_KEYFLAG_Alt (1 << 1)
#define FWL_KEYFLAG_Shift (1 << 2)
#define FWL_KEYFLAG_Command (1 << 3)
#define FWL_KEYFLAG_LButton (1 << 4)
#define FWL_KEYFLAG_RButton (1 << 5)
#define FWL_KEYFLAG_MButton (1 << 6)

class CFWL_Message;
class CFWL_Note;
class IFWL_NoteLoop;
class IFWL_NoteThread;
class IFWL_Widget;

class IFWL_NoteDriver {
 public:
  virtual ~IFWL_NoteDriver() {}

  virtual FX_BOOL SendNote(CFWL_Note* pNote) = 0;
  virtual FWL_ERR RegisterEventTarget(
      IFWL_Widget* pListener,
      IFWL_Widget* pEventSource = NULL,
      uint32_t dwFilter = FWL_EVENT_ALL_MASK) = 0;
  virtual FWL_ERR UnregisterEventTarget(IFWL_Widget* pListener) = 0;
  virtual void ClearEventTargets(FX_BOOL bRemoveAll) = 0;
  virtual int32_t GetQueueMaxSize() const = 0;
  virtual FWL_ERR SetQueueMaxSize(const int32_t size) = 0;
  virtual IFWL_NoteThread* GetOwnerThread() const = 0;
  virtual FWL_ERR PushNoteLoop(IFWL_NoteLoop* pNoteLoop) = 0;
  virtual IFWL_NoteLoop* PopNoteLoop() = 0;
  virtual IFWL_Widget* GetFocus() = 0;
  virtual FX_BOOL SetFocus(IFWL_Widget* pFocus, FX_BOOL bNotify = FALSE) = 0;
  virtual void SetGrab(IFWL_Widget* pGrab, FX_BOOL bSet) = 0;
  virtual FWL_ERR Run() = 0;
};

IFWL_Widget* FWL_GetCurrentThreadModalWidget(IFWL_NoteThread* pNoteThread);

typedef FX_BOOL (*FWLMessageHookCallback)(CFWL_Message* msg, void* info);
FWL_ERR FWL_SetHook(IFWL_NoteDriver* driver,
                    FWLMessageHookCallback callback,
                    void* info);

#endif  // XFA_FWL_CORE_IFWL_NOTEDRIVER_H_
