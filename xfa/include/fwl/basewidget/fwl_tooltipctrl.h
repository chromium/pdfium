// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_TOOLTIP_H
#define _FWL_TOOLTIP_H
class CFWL_WidgetImpProperties;
class IFWL_Form;
class IFWL_ToolTipDP;
class IFWL_ToolTip;
#define FWL_CLASS_ToolTip L"FWL_TOOLTIP"
#define FWL_CLASSHASH_ToolTip 1111984755
#define FWL_STYLEEXT_TTP_Rectangle (0L << 3)
#define FWL_STYLEEXT_TTP_RoundCorner (1L << 3)
#define FWL_STYLEEXT_TTP_Balloon (1L << 4)
#define FWL_STYLEEXT_TTP_Multiline (1L << 5)
#define FWL_STYLEEXT_TTP_NoAnchor (1L << 6)
#define FWL_PART_TTP_Border 1
#define FWL_PART_TTP_Background 2
#define FWL_PART_TTP_Caption 3
class IFWL_ToolTipDP : public IFWL_DataProvider {
 public:
  virtual FWL_ERR GetCaption(IFWL_Widget* pWidget,
                             CFX_WideString& wsCaption) = 0;
  virtual int32_t GetInitialDelay(IFWL_Widget* pWidget) = 0;
  virtual int32_t GetAutoPopDelay(IFWL_Widget* pWidget) = 0;
  virtual CFX_DIBitmap* GetToolTipIcon(IFWL_Widget* pWidget) = 0;
  virtual CFX_SizeF GetToolTipIconSize(IFWL_Widget* pWidget) = 0;
};
class IFWL_ToolTip : public IFWL_Form {
 public:
  static IFWL_ToolTip* Create(const CFWL_WidgetImpProperties& properties,
                              IFWL_Widget* pOuter);

  FWL_ERR SetAnchor(const CFX_RectF& rtAnchor);
  FWL_ERR Show();
  FWL_ERR Hide();

 protected:
  IFWL_ToolTip();
};
#endif
