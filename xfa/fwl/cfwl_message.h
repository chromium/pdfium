// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_MESSAGE_H_
#define XFA_FWL_CFWL_MESSAGE_H_

#include <type_traits>

#include "core/fxcrt/mask.h"
#include "core/fxcrt/unowned_ptr.h"
#include "v8/include/cppgc/macros.h"

class CFWL_Widget;

enum FWL_KeyFlag : uint8_t {
  FWL_KEYFLAG_Ctrl = 1 << 0,
  FWL_KEYFLAG_Alt = 1 << 1,
  FWL_KEYFLAG_Shift = 1 << 2,
  FWL_KEYFLAG_Command = 1 << 3,
  FWL_KEYFLAG_LButton = 1 << 4,
  FWL_KEYFLAG_RButton = 1 << 5,
  FWL_KEYFLAG_MButton = 1 << 6
};
using FWL_KeyFlagMask = Mask<FWL_KeyFlag>;

class CFWL_Message {
  CPPGC_STACK_ALLOCATED();  // Allow Raw/Unowned pointers.

 public:
  enum class Type { kKey, kKillFocus, kMouse, kMouseWheel, kSetFocus };

  virtual ~CFWL_Message();

  Type GetType() const { return m_type; }
  CFWL_Widget* GetSrcTarget() const { return m_pSrcTarget; }
  CFWL_Widget* GetDstTarget() const { return m_pDstTarget; }
  void SetSrcTarget(CFWL_Widget* pWidget) { m_pSrcTarget = pWidget; }
  void SetDstTarget(CFWL_Widget* pWidget) { m_pDstTarget = pWidget; }

 protected:
  CFWL_Message(Type type, CFWL_Widget* pSrcTarget, CFWL_Widget* pDstTarget);
  CFWL_Message(const CFWL_Message& that) = delete;
  CFWL_Message& operator=(const CFWL_Message& that) = delete;

 private:
  const Type m_type;
  UnownedPtr<CFWL_Widget> m_pSrcTarget;
  UnownedPtr<CFWL_Widget> m_pDstTarget;
};

#endif  // XFA_FWL_CFWL_MESSAGE_H_
