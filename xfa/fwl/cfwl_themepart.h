// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_THEMEPART_H_
#define XFA_FWL_CFWL_THEMEPART_H_

#include <type_traits>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"

class CFWL_Widget;

enum CFWL_PartState : uint32_t {
  CFWL_PartState_Normal = 0,

  CFWL_PartState_Checked = 1 << 1,
  CFWL_PartState_Default = 1 << 2,
  CFWL_PartState_Disabled = 1 << 3,
  CFWL_PartState_Flagged = 1 << 4,
  CFWL_PartState_Focused = 1 << 5,
  CFWL_PartState_HightLight = 1 << 6,
  CFWL_PartState_Hovered = 1 << 7,
  CFWL_PartState_Neutral = 1 << 9,
  CFWL_PartState_Pressed = 1 << 10,
  CFWL_PartState_ReadOnly = 1 << 11,
  CFWL_PartState_LSelected = 1 << 12,
  CFWL_PartState_RSelected = 1 << 13,
  CFWL_PartState_Selected = 1 << 14
};
using CFWL_PartStateMask = std::underlying_type<CFWL_PartState>::type;

class CFWL_ThemePart {
 public:
  enum class Part : uint8_t {
    kNone = 0,

    kBackArrow,
    kBackground,
    kBorder,
    kCaption,
    kCheck,
    kCheckBox,
    kCloseBox,
    kCombTextLine,
    kDateInBK,
    kDateInCircle,
    kDatesIn,
    kDatesOut,
    kDownButton,
    kDropDownButton,
    kForeArrow,
    kHSeparator,
    kHeadText,
    kHeader,
    kIcon,
    kImage,
    kLBtn,
    kListItem,
    kLowerTrack,
    kMinimizeBox,
    kMaximizeBox,
    kNarrowCaption,
    kRBtn,
    kThumb,
    kThumbBackArrow,
    kThumbForeArrow,
    kThumbLowerTrack,
    kThumbThumb,
    kThumbUpperTrack,
    kToday,
    kTodayCircle,
    kUpButton,
    kUpperTrack,
    kVSeparator,
    kWeek,
    kWeekNum,
    kWeekNumSep
  };

  explicit CFWL_ThemePart(CFWL_Widget* pWidget);
  ~CFWL_ThemePart();

  CFWL_Widget* GetWidget() const { return m_pWidget.Get(); }

  CFX_Matrix m_matrix;
  CFX_RectF m_PartRect;
  UnownedPtr<const CFX_RectF> m_pRtData;
  CFWL_PartStateMask m_dwStates = CFWL_PartState_Normal;
  Part m_iPart = Part::kNone;
  bool m_bMaximize = false;
  bool m_bStaticBackground = false;

 private:
  UnownedPtr<CFWL_Widget> const m_pWidget;
};

#endif  // XFA_FWL_CFWL_THEMEPART_H_
