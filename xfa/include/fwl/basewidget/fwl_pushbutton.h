// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_PUSHBUTTON_H
#define _FWL_PUSHBUTTON_H
class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_PushButtonDP;
class IFWL_PushButton;
#define FWL_CLASS_PushButton L"FWL_PUSHBUTTON"
#define FWL_CLASSHASH_PushButton 3521614244
#define FWL_STYLEEXT_PSB_Left (0L << 0)
#define FWL_STYLEEXT_PSB_Center (1L << 0)
#define FWL_STYLEEXT_PSB_Right (2L << 0)
#define FWL_STYLEEXT_PSB_Top (0L << 2)
#define FWL_STYLEEXT_PSB_VCenter (1L << 2)
#define FWL_STYLEEXT_PSB_Bottom (2L << 2)
#define FWL_STYLEEXT_PSB_TextOnly (0L << 4)
#define FWL_STYLEEXT_PSB_IconOnly (1L << 4)
#define FWL_STYLEEXT_PSB_TextIcon (2L << 4)
#define FWL_STYLEEXT_PSB_HLayoutMask (3L << 0)
#define FWL_STYLEEXT_PSB_VLayoutMask (3L << 2)
#define FWL_STYLEEXT_PSB_ModeMask (3L << 4)
#define FWL_STATE_PSB_Hovered (1 << FWL_WGTSTATE_MAX)
#define FWL_STATE_PSB_Pressed (1 << (FWL_WGTSTATE_MAX + 1))
#define FWL_STATE_PSB_Default (1 << (FWL_WGTSTATE_MAX + 2))
#define FWL_WGTCAPACITY_PSB_Margin (FWL_WGTCAPACITY_MAX + 1)
#define FWL_PART_PSB_Border 1
#define FWL_PART_PSB_Edge 2
#define FWL_PART_PSB_Background 3
#define FWL_PART_PSB_Caption 4
#define FWL_PARTSTATE_PSB_Normal (0L << 0)
#define FWL_PARTSTATE_PSB_Pressed (1L << 0)
#define FWL_PARTSTATE_PSB_Hovered (2L << 0)
#define FWL_PARTSTATE_PSB_Default (3L << 0)
#define FWL_PARTSTATE_PSB_Disabled (4L << 0)
#define FWL_PARTSTATE_PSB_Focused (1L << 3)
#define FWL_PARTSTATE_PSB_Mask (7L << 0)
class IFWL_PushButtonDP : public IFWL_DataProvider {
 public:
  virtual CFX_DIBitmap* GetPicture(IFWL_Widget* pWidget) = 0;
};
class IFWL_PushButton : public IFWL_Widget {
 public:
  static IFWL_PushButton* Create(const CFWL_WidgetImpProperties& properties,
                                 IFWL_Widget* pOuter);

 protected:
  IFWL_PushButton();
};
#endif
