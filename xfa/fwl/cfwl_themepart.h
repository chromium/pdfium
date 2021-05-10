// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_THEMEPART_H_
#define XFA_FWL_CFWL_THEMEPART_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"

enum class CFWL_Part : uint8_t {
  None = 0,

  BackArrow,
  Background,
  Border,
  Caption,
  Check,
  CheckBox,
  CloseBox,
  CombTextLine,
  DateInBK,
  DateInCircle,
  DatesIn,
  DatesOut,
  DownButton,
  DropDownButton,
  ForeArrow,
  HSeparator,
  HeadText,
  Header,
  Icon,
  Image,
  LBtn,
  ListItem,
  LowerTrack,
  MinimizeBox,
  MaximizeBox,
  NarrowCaption,
  RBtn,
  Thumb,
  ThumbBackArrow,
  ThumbForeArrow,
  ThumbLowerTrack,
  ThumbThumb,
  ThumbUpperTrack,
  Today,
  TodayCircle,
  UpButton,
  UpperTrack,
  VSeparator,
  Week,
  WeekNum,
  WeekNumSep
};

enum CFWL_PartState {
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

class CFWL_Widget;

class CFWL_ThemePart {
 public:
  CFWL_ThemePart(CFWL_Widget* pWidget);
  ~CFWL_ThemePart();

  CFWL_Widget* GetWidget() const { return m_pWidget.Get(); }

  CFX_Matrix m_matrix;
  CFX_RectF m_PartRect;
  UnownedPtr<const CFX_RectF> m_pRtData;
  uint32_t m_dwStates = CFWL_PartState_Normal;
  CFWL_Part m_iPart = CFWL_Part::None;
  bool m_bMaximize = false;
  bool m_bStaticBackground = false;

 private:
  UnownedPtr<CFWL_Widget> const m_pWidget;
};

#endif  // XFA_FWL_CFWL_THEMEPART_H_
