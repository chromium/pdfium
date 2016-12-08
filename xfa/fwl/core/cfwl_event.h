// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_EVENT_H_
#define XFA_FWL_CORE_CFWL_EVENT_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fwl/core/cfwl_msgkey.h"
#include "xfa/fwl/core/cfwl_msgmouse.h"
#include "xfa/fwl/core/fwl_error.h"

class CFX_Graphics;
class CFWL_Widget;

class CFWL_Event {
 public:
  enum class Type {
    CheckStateChanged,
    CheckWord,
    Click,
    Close,
    EditChanged,
    Mouse,
    PostDropDown,
    PreDropDown,
    Scroll,
    SelectChanged,
    TextChanged,
    TextFull,
    Validate
  };

  explicit CFWL_Event(Type type);
  CFWL_Event(Type type, CFWL_Widget* pSrcTarget);
  CFWL_Event(Type type, CFWL_Widget* pSrcTarget, CFWL_Widget* pDstTarget);
  virtual ~CFWL_Event();

  Type GetType() const { return m_type; }

  CFWL_Widget* m_pSrcTarget;
  CFWL_Widget* m_pDstTarget;

 private:
  Type m_type;
};

#endif  // XFA_FWL_CORE_CFWL_EVENT_H_
