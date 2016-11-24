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

enum class CFWL_EventType {
  None = 0,

  CheckStateChanged,
  CheckWord,
  Click,
  Close,
  EditChanged,
  Key,
  KillFocus,
  Mouse,
  MouseWheel,
  PostDropDown,
  PreDropDown,
  Scroll,
  SelectChanged,
  SetFocus,
  SizeChanged,
  TextChanged,
  TextFull,
  Validate
};

class CFX_Graphics;
class CFWL_Widget;

class CFWL_Event {
 public:
  CFWL_Event();
  virtual ~CFWL_Event();

  virtual CFWL_EventType GetClassID() const;

  CFWL_Widget* m_pSrcTarget;
  CFWL_Widget* m_pDstTarget;
};

#endif  // XFA_FWL_CORE_CFWL_EVENT_H_
