// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_EVENTPARAM_H_
#define XFA_FXFA_CXFA_EVENTPARAM_H_

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "v8/include/cppgc/macros.h"
#include "xfa/fxfa/fxfa_basic.h"

enum XFA_EVENTTYPE : uint8_t {
  XFA_EVENT_Click,
  XFA_EVENT_Change,
  XFA_EVENT_DocClose,
  XFA_EVENT_DocReady,
  XFA_EVENT_Enter,
  XFA_EVENT_Exit,
  XFA_EVENT_Full,
  XFA_EVENT_IndexChange,
  XFA_EVENT_Initialize,
  XFA_EVENT_MouseDown,
  XFA_EVENT_MouseEnter,
  XFA_EVENT_MouseExit,
  XFA_EVENT_MouseUp,
  XFA_EVENT_PostExecute,
  XFA_EVENT_PostOpen,
  XFA_EVENT_PostPrint,
  XFA_EVENT_PostSave,
  XFA_EVENT_PostSign,
  XFA_EVENT_PostSubmit,
  XFA_EVENT_PreExecute,
  XFA_EVENT_PreOpen,
  XFA_EVENT_PrePrint,
  XFA_EVENT_PreSave,
  XFA_EVENT_PreSign,
  XFA_EVENT_PreSubmit,
  XFA_EVENT_Ready,
  XFA_EVENT_InitCalculate,
  XFA_EVENT_InitVariables,
  XFA_EVENT_Calculate,
  XFA_EVENT_Validate,
  XFA_EVENT_Unknown,
};

class CXFA_EventParam {
 public:
  explicit CXFA_EventParam(XFA_EVENTTYPE type);
  CXFA_EventParam(const CXFA_EventParam& other);
  ~CXFA_EventParam();

  CXFA_EventParam& operator=(const CXFA_EventParam& other);
  CXFA_EventParam& operator=(CXFA_EventParam&& other) noexcept;

  WideString GetNewText() const;

  XFA_EVENTTYPE type_;
  bool cancel_action_ = false;
  bool key_down_ = false;
  bool modifier_ = false;
  bool reenter_ = false;
  bool shift_ = false;
  bool is_form_ready_ = false;
  bool targeted_ = true;
  int32_t commit_key_ = 0;
  int32_t sel_end_ = 0;
  int32_t sel_start_ = 0;
  WideString result_;
  WideString change_;
  WideString full_text_;
  WideString new_content_type_;
  WideString prev_content_type_;
  WideString prev_text_;
  WideString soap_fault_code_;
  WideString soap_fault_string_;
};

#endif  // XFA_FXFA_CXFA_EVENTPARAM_H_
