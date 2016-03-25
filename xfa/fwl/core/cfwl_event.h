// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_EVENT_H_
#define XFA_FWL_CORE_CFWL_EVENT_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/cfwl_note.h"
#include "xfa/fwl/core/fwl_error.h"

#define FWL_EVT_Mouse L"FWL_EVENT_Mouse"
#define FWL_EVT_MouseWheel L"FWL_EVENT_MouseWheel"
#define FWL_EVT_Key L"FWL_EVENT_Key"
#define FWL_EVT_SetFocus L"FWL_EVENT_SetFocus"
#define FWL_EVT_KillFocus L"FWL_EVENT_KillFocus"
#define FWL_EVT_Click L"FWL_EVENT_Click"
#define FWL_EVT_Draw L"FWL_EVENT_Draw"
#define FWL_EVT_Scroll L"FWL_EVENT_Scroll"
#define FWL_EVT_Close L"FWL_EVENT_Close"
#define FWL_EVT_ContextMenu L"FWL_EVENT_ContextMenu"
#define FWL_EVT_MenuCommand L"FWL_EVENT_MenuCommand"
#define FWL_EVT_SizeChanged L"FWL_EVENT_SizeChanged"

#define FWL_EVTHASH_Mouse 1765258002
#define FWL_EVTHASH_MouseWheel 3907114407
#define FWL_EVTHASH_Key 2408354450
#define FWL_EVTHASH_SetFocus 3909721269
#define FWL_EVTHASH_KillFocus 1779363253
#define FWL_EVTHASH_Draw 2430713303
#define FWL_EVTHASH_Click 4026328783
#define FWL_EVTHASH_Scroll 2965158968
#define FWL_EVTHASH_Close 4036693599
#define FWL_EVTHASH_ContextMenu 2717307715
#define FWL_EVTHASH_MenuCommand 497763741
#define FWL_EVTHASH_SizeChanged 3083958510
#define FWL_EVTHASH_Idle 839546759

typedef enum {
  FWL_EVENT_MOUSE_MASK = 1 << 0,
  FWL_EVENT_MOUSEWHEEL_MASK = 1 << 1,
  FWL_EVENT_KEY_MASK = 1 << 2,
  FWL_EVENT_FOCUSCHANGED_MASK = 1 << 3,
  FWL_EVENT_DRAW_MASK = 1 << 4,
  FWL_EVENT_CLOSE_MASK = 1 << 5,
  FWL_EVENT_SIZECHANGED_MASK = 1 << 6,
  FWL_EVENT_IDLE_MASK = 1 << 7,
  FWL_EVENT_CONTROL_MASK = 1 << 8,
  FWL_EVENT_ALL_MASK = 0xFF
} FWLEventMask;

class CFX_Graphics;
class IFWL_Widget;

class CFWL_Event : public CFWL_Note {
 public:
  CFWL_Event() : CFWL_Note(TRUE) {}
  virtual ~CFWL_Event() {}
};

#define BEGIN_FWL_EVENT_DEF(classname, eventhashcode)             \
  class classname : public CFWL_Event {                           \
   public:                                                        \
    classname() : CFWL_Event() {}                                 \
    virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const { \
      wsClass = L## #classname;                                   \
      return FWL_ERR_Succeeded;                                   \
    }                                                             \
    virtual uint32_t GetClassID() const { return eventhashcode; }

#define END_FWL_EVENT_DEF \
  }                       \
  ;  // NOLINT

BEGIN_FWL_EVENT_DEF(CFWL_EvtMouse, FWL_EVTHASH_Mouse)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
uint32_t m_dwFlags;
uint32_t m_dwCmd;
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtMouseWheel, FWL_EVTHASH_MouseWheel)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
FX_FLOAT m_fDeltaX;
FX_FLOAT m_fDeltaY;
uint32_t m_dwFlags;
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtKey, FWL_EVTHASH_Key)
uint32_t m_dwKeyCode;
uint32_t m_dwFlags;
uint32_t m_dwCmd;
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtSetFocus, FWL_EVTHASH_SetFocus)
IFWL_Widget* m_pSetFocus;
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtKillFocus, FWL_EVTHASH_KillFocus)
IFWL_Widget* m_pKillFocus;
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtDraw, FWL_EVTHASH_Draw)
CFX_Graphics* m_pGraphics;
IFWL_Widget* m_pWidget;
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtClick, FWL_EVTHASH_Click)
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtScroll, FWL_EVTHASH_Scroll)
uint32_t m_iScrollCode;
FX_FLOAT m_fPos;
FX_BOOL* m_pRet;
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtClose, FWL_EVTHASH_Close)
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtContextMenu, FWL_EVTHASH_ContextMenu)
FX_FLOAT m_fPosX;
FX_FLOAT m_fPosY;
IFWL_Widget* m_pOwner;
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtMenuCommand, FWL_EVTHASH_MenuCommand)
int32_t m_iCommand;
void* m_pData;
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtSizeChanged, FWL_EVTHASH_SizeChanged)
IFWL_Widget* m_pWidget;
CFX_RectF m_rtOld;
CFX_RectF m_rtNew;
END_FWL_EVENT_DEF

BEGIN_FWL_EVENT_DEF(CFWL_EvtIdle, FWL_EVTHASH_Idle)
END_FWL_EVENT_DEF

#endif  // XFA_FWL_CORE_CFWL_EVENT_H_
