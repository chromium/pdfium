// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_EVENT_H_
#define XFA_FWL_CFWL_EVENT_H_

#include "v8/include/cppgc/macros.h"

class CFWL_Widget;

class CFWL_Event {
  CPPGC_STACK_ALLOCATED();  // Allow Raw/Unowned pointers.

 public:
  enum class Type {
    CheckStateChanged,
    Click,
    Close,
    EditChanged,
    Mouse,
    PostDropDown,
    PreDropDown,
    Scroll,
    SelectChanged,
    TextWillChange,
    TextFull,
    Validate
  };

  explicit CFWL_Event(Type type);
  CFWL_Event(Type type, CFWL_Widget* pSrcTarget);
  CFWL_Event(Type type, CFWL_Widget* pSrcTarget, CFWL_Widget* pDstTarget);
  virtual ~CFWL_Event();

  Type GetType() const { return m_type; }
  CFWL_Widget* GetSrcTarget() const { return m_pSrcTarget; }
  CFWL_Widget* GetDstTarget() const { return m_pDstTarget; }

 private:
  const Type m_type;
  CFWL_Widget* const m_pSrcTarget = nullptr;
  CFWL_Widget* const m_pDstTarget = nullptr;
};

#endif  // XFA_FWL_CFWL_EVENT_H_
