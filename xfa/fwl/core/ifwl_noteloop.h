// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_NOTELOOP_H_
#define XFA_FWL_CORE_IFWL_NOTELOOP_H_

#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/fwl_error.h"

class CFWL_Message;

class IFWL_NoteLoop {
 public:
  virtual ~IFWL_NoteLoop() {}
  virtual FX_BOOL PreProcessMessage(CFWL_Message* pMessage) = 0;
  virtual FWL_ERR Idle(int32_t count) = 0;
};

#endif  // XFA_FWL_CORE_IFWL_NOTELOOP_H_
