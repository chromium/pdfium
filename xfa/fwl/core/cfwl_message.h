// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_MESSAGE_H_
#define XFA_FWL_CORE_CFWL_MESSAGE_H_

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fwl/core/fwl_error.h"

enum class CFWL_MessageType {
  None = 0,
  Key,
  KillFocus,
  Mouse,
  MouseWheel,
  SetFocus
};

enum class FWL_MouseCommand {
  LeftButtonDown,
  LeftButtonUp,
  LeftButtonDblClk,
  RightButtonDown,
  RightButtonUp,
  RightButtonDblClk,
  Move,
  Enter,
  Leave,
  Hover
};

enum class FWL_KeyCommand { KeyDown, KeyUp, Char };

class IFWL_Widget;

class CFWL_Message {
 public:
  CFWL_Message();
  virtual ~CFWL_Message();

  virtual CFWL_Message* Clone();
  virtual CFWL_MessageType GetClassID() const;

  uint32_t Release();
  CFWL_Message* Retain();

  IFWL_Widget* m_pSrcTarget;
  IFWL_Widget* m_pDstTarget;
  uint32_t m_dwExtend;

 private:
  uint32_t m_dwRefCount;
};

inline CFWL_Message::CFWL_Message()
    : m_pSrcTarget(nullptr),
      m_pDstTarget(nullptr),
      m_dwExtend(0),
      m_dwRefCount(1) {}

inline CFWL_Message::~CFWL_Message() {}

inline CFWL_Message* CFWL_Message::Clone() {
  return nullptr;
}

inline CFWL_MessageType CFWL_Message::GetClassID() const {
  return CFWL_MessageType::None;
}

inline uint32_t CFWL_Message::Release() {
  m_dwRefCount--;
  uint32_t dwRefCount = m_dwRefCount;
  if (!m_dwRefCount)
    delete this;
  return dwRefCount;
}

inline CFWL_Message* CFWL_Message::Retain() {
  m_dwRefCount++;
  return this;
}

#define FWL_MESSAGE_DEF(classname, msgType, ...)                           \
  class classname : public CFWL_Message {                                  \
   public:                                                                 \
    classname();                                                           \
    ~classname() override;                                                 \
    CFWL_Message* Clone() override;                                        \
    CFWL_MessageType GetClassID() const override;                          \
    __VA_ARGS__                                                            \
  };                                                                       \
  inline classname::classname() {}                                         \
  inline classname::~classname() {}                                        \
  inline CFWL_Message* classname::Clone() { return new classname(*this); } \
  inline CFWL_MessageType classname::GetClassID() const { return msgType; }

FWL_MESSAGE_DEF(CFWL_MsgMouse, CFWL_MessageType::Mouse, FX_FLOAT m_fx;
                FX_FLOAT m_fy;
                uint32_t m_dwFlags;
                FWL_MouseCommand m_dwCmd;)

FWL_MESSAGE_DEF(CFWL_MsgMouseWheel, CFWL_MessageType::MouseWheel, FX_FLOAT m_fx;
                FX_FLOAT m_fy;
                FX_FLOAT m_fDeltaX;
                FX_FLOAT m_fDeltaY;
                uint32_t m_dwFlags;)

FWL_MESSAGE_DEF(CFWL_MsgSetFocus,
                CFWL_MessageType::SetFocus,
                IFWL_Widget* m_pKillFocus;)

FWL_MESSAGE_DEF(CFWL_MsgKillFocus,
                CFWL_MessageType::KillFocus,
                IFWL_Widget* m_pSetFocus;)

FWL_MESSAGE_DEF(CFWL_MsgKey, CFWL_MessageType::Key, uint32_t m_dwKeyCode;
                uint32_t m_dwFlags;
                FWL_KeyCommand m_dwCmd;)

#endif  // XFA_FWL_CORE_CFWL_MESSAGE_H_
