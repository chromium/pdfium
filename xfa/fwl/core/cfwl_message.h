// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_MESSAGE_H_
#define XFA_FWL_CORE_CFWL_MESSAGE_H_

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/cfwl_note.h"
#include "xfa/fwl/core/fwl_error.h"

#define FWL_MSGHASH_Activate 2410369469
#define FWL_MSGHASH_Deactivate 1184214790
#define FWL_MSGHASH_SetFocus 4174512504
#define FWL_MSGHASH_KillFocus 1557903832
#define FWL_MSGHASH_Mouse 706128309
#define FWL_MSGHASH_MouseWheel 893703466
#define FWL_MSGHASH_Key 3751372405
#define FWL_MSGHASH_Cursor 3182626218
#define FWL_MSGHASH_Size 160077735
#define FWL_MSGHASH_WindowMove 1032269377
#define FWL_MSGHASH_DropFiles 2004165236
#define FWL_MSGHASH_TaskClicked 3128231086
#define FWL_MSGHASH_Close 2977563906
#define FWL_MSGHASH_Post 1969633074
#define FWL_MSGHASH_WindowWillMove 2229175763

#define FWL_MSG_Activate L"FWL_MESSAGE_Activate"
#define FWL_MSG_Deactivate L"FWL_MESSAGE_Deactivate"
#define FWL_MSG_SetFocus L"FWL_MESSAGE_SetFocus"
#define FWL_MSG_KillFocus L"FWL_MESSAGE_KillFocus"
#define FWL_MSG_Mouse L"FWL_MESSAGE_Mouse"
#define FWL_MSG_MouseWheel L"FWL_MESSAGE_MouseWheel"
#define FWL_MSG_Key L"FWL_MESSAGE_Key"
#define FWL_MSG_Cursor L"FWL_MESSAGE_Cursor"
#define FWL_MSG_Size L"FWL_MESSAGE_Size"
#define FWL_MSG_WindowMove L"FWL_MESSAGE_WindowMove"
#define FWL_MSG_DropFiles L"FWL_MESSAGE_DropFiles"
#define FWL_MSG_TaskClicked L"FWL_MESSAGE_TaskClicked"
#define FWL_MSG_Close L"FWL_MESSAGE_Close"
#define FWL_MSG_Post L"FWL_MESSAGE_Post"
#define FWL_MSG_WindowWillMove L"FWL_MESSAGE_WindowWillMove"

#define FWL_MSGMOUSECMD_LButtonDown 1
#define FWL_MSGMOUSECMD_LButtonUp 2
#define FWL_MSGMOUSECMD_LButtonDblClk 3
#define FWL_MSGMOUSECMD_RButtonDown 4
#define FWL_MSGMOUSECMD_RButtonUp 5
#define FWL_MSGMOUSECMD_RButtonDblClk 6
#define FWL_MSGMOUSECMD_MButtonDown 7
#define FWL_MSGMOUSECMD_MButtonUp 8
#define FWL_MSGMOUSECMD_MButtonDblClk 9
#define FWL_MSGMOUSECMD_MouseMove 10
#define FWL_MSGMOUSECMD_MouseEnter 11
#define FWL_MSGMOUSECMD_MouseLeave 12
#define FWL_MSGMOUSECMD_MouseHover 13
#define FWL_MSGKEYCMD_KeyDown 1
#define FWL_MSGKEYCMD_KeyUp 2
#define FWL_MSGKEYCMD_Char 3

class IFWL_Widget;

class CFWL_Message : public CFWL_Note {
 public:
  CFWL_Message() : CFWL_Note(FALSE) {}
  virtual ~CFWL_Message() {}
};

#define BEGIN_FWL_MESSAGE_DEF(classname, msghashcode)             \
  class classname : public CFWL_Message {                         \
   public:                                                        \
    classname() : CFWL_Message() {}                               \
    virtual CFWL_Note* Clone() { return new classname(*this); }   \
    virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const { \
      wsClass = L## #classname;                                   \
      return FWL_ERR_Succeeded;                                   \
    }                                                             \
    virtual uint32_t GetClassID() const { return msghashcode; }

#define END_FWL_MESSAGE_DEF \
  }                         \
  ;  // NOLINT

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgActivate, FWL_MSGHASH_Activate)
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgDeactivate, FWL_MSGHASH_Deactivate)
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgMouse, FWL_MSGHASH_Mouse)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
uint32_t m_dwFlags;
uint32_t m_dwCmd;
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgMouseWheel, FWL_MSGHASH_MouseWheel)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
FX_FLOAT m_fDeltaX;
FX_FLOAT m_fDeltaY;
uint32_t m_dwFlags;
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgSetFocus, FWL_MSGHASH_SetFocus)
IFWL_Widget* m_pKillFocus;
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgKillFocus, FWL_MSGHASH_KillFocus)
IFWL_Widget* m_pSetFocus;
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgKey, FWL_MSGHASH_Key)
uint32_t m_dwKeyCode;
uint32_t m_dwFlags;
uint32_t m_dwCmd;
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgCursor, FWL_MSGHASH_Cursor)
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgSize, FWL_MSGHASH_Size)
int32_t m_iWidth;
int32_t m_iHeight;
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgWindowMove, FWL_MSGHASH_WindowMove)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgDropFiles, FWL_MSGHASH_DropFiles)
CFWL_MsgDropFiles(const CFWL_MsgDropFiles& copy) {
  m_pDstTarget = copy.m_pDstTarget;
  m_pSrcTarget = copy.m_pSrcTarget;
  m_fx = copy.m_fx;
  m_fy = copy.m_fy;
  m_files.Append(copy.m_files);
}
FX_FLOAT m_fx;
FX_FLOAT m_fy;
CFX_WideStringArray m_files;
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgTaskClicked, FWL_MSGHASH_TaskClicked)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgClose, FWL_MSGHASH_Close)
END_FWL_MESSAGE_DEF

BEGIN_FWL_MESSAGE_DEF(CFWL_MsgWindowWillMove, FWL_MSGHASH_WindowWillMove)
END_FWL_MESSAGE_DEF

#endif  // XFA_FWL_CORE_CFWL_MESSAGE_H_
