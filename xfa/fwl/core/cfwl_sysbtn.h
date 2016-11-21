// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_SYSBTN_H_
#define XFA_FWL_CORE_CFWL_SYSBTN_H_

#include "core/fxcrt/fx_coordinates.h"

#define FWL_SYSBUTTONSTATE_Hover 0x0001
#define FWL_SYSBUTTONSTATE_Pressed 0x0002
#define FWL_SYSBUTTONSTATE_Disabled 0x0010

class CFWL_SysBtn {
 public:
  CFWL_SysBtn();
  ~CFWL_SysBtn();

  bool IsDisabled() const;
  uint32_t GetPartState() const;

  void SetNormal();
  void SetPressed();
  void SetHover();
  void SetDisabled(bool bDisabled);

  CFX_RectF m_rtBtn;
  uint32_t m_dwState;
};

#endif  // XFA_FWL_CORE_CFWL_SYSBTN_H_
