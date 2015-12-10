// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_SCROLLBAR_H
#define _FWL_SCROLLBAR_H
class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_ScrollBarDP;
class IFWL_ScrollBar;
#define FWL_CLASS_ScrollBar L"FWL_SCROLLBAR"
#define FWL_CLASSHASH_ScrollBar 2826584844
#define FWL_STYLEEXT_SCB_Horz (0L << 0)
#define FWL_STYLEEXT_SCB_Vert (1L << 0)
#define FWL_PART_SCB_Border 1
#define FWL_PART_SCB_Edge 2
#define FWL_PART_SCB_Background 3
#define FWL_PART_SCB_ForeArrow 4
#define FWL_PART_SCB_BackArrow 5
#define FWL_PART_SCB_Thumb 6
#define FWL_PART_SCB_LowerTrack 7
#define FWL_PART_SCB_UpperTrack 8
#define FWL_PARTSTATE_SCB_Normal (0L << 0)
#define FWL_PARTSTATE_SCB_Hovered (1L << 0)
#define FWL_PARTSTATE_SCB_Pressed (2L << 0)
#define FWL_PARTSTATE_SCB_Disabled (3L << 0)
#define FWL_WGTHITTEST_SCB_ForeArrow FWL_WGTHITTEST_MAX + 1
#define FWL_WGTHITTEST_SCB_BackArrow FWL_WGTHITTEST_MAX + 2
#define FWL_WGTHITTEST_SCB_LowerTrack FWL_WGTHITTEST_MAX + 3
#define FWL_WGTHITTEST_SCB_UpperTrack FWL_WGTHITTEST_MAX + 4
#define FWL_WGTHITTEST_SCB_Thumb FWL_WGTHITTEST_MAX + 5
#define FWL_CAPACITY_SCB_Size FWL_WGTCAPACITY_MAX
enum FWL_SCBCODE {
  FWL_SCBCODE_None = 1,
  FWL_SCBCODE_Min,
  FWL_SCBCODE_Max,
  FWL_SCBCODE_PageBackward,
  FWL_SCBCODE_PageForward,
  FWL_SCBCODE_StepBackward,
  FWL_SCBCODE_StepForward,
  FWL_SCBCODE_Pos,
  FWL_SCBCODE_TrackPos,
  FWL_SCBCODE_EndScroll,
};
class IFWL_ScrollBarDP : public IFWL_DataProvider {};
class IFWL_ScrollBar : public IFWL_Widget {
 public:
  static IFWL_ScrollBar* Create(const CFWL_WidgetImpProperties& properties,
                                IFWL_Widget* pOuter);

  FX_BOOL IsVertical();
  FWL_ERR GetRange(FX_FLOAT& fMin, FX_FLOAT& fMax);
  FWL_ERR SetRange(FX_FLOAT fMin, FX_FLOAT fMax);
  FX_FLOAT GetPageSize();
  FWL_ERR SetPageSize(FX_FLOAT fPageSize);
  FX_FLOAT GetStepSize();
  FWL_ERR SetStepSize(FX_FLOAT fStepSize);
  FX_FLOAT GetPos();
  FWL_ERR SetPos(FX_FLOAT fPos);
  FX_FLOAT GetTrackPos();
  FWL_ERR SetTrackPos(FX_FLOAT fTrackPos);
  FX_BOOL DoScroll(FX_DWORD dwCode, FX_FLOAT fPos = 0.0f);

 protected:
  IFWL_ScrollBar();
};
#endif
