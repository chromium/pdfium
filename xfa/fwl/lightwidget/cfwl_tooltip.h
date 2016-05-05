// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_LIGHTWIDGET_CFWL_TOOLTIP_H_
#define XFA_FWL_LIGHTWIDGET_CFWL_TOOLTIP_H_

#include "xfa/fwl/basewidget/ifwl_tooltip.h"
#include "xfa/fwl/lightwidget/cfwl_widget.h"

class CFWL_ToolTip : public CFWL_Widget {
 public:
  static CFWL_ToolTip* Create();

  CFWL_ToolTip();
  virtual ~CFWL_ToolTip();

  FWL_Error Initialize(const CFWL_WidgetProperties* pProperties = nullptr);

  void GetCaption(CFX_WideString& wsCaption);
  void SetCaption(const CFX_WideStringC& wsCaption);

  int32_t GetInitialDelay();
  void SetInitialDelay(int32_t nDelayTime);

  int32_t GetAutoPopDelay();
  void SetAutoPopDelay(int32_t nDelayTime);

  CFX_DIBitmap* GetToolTipIcon();
  void SetToolTipIcon(CFX_DIBitmap* pBitmap);
  CFX_SizeF GetToolTipIconSize();
  void SetToolTipIconSize(CFX_SizeF fSize);

  void SetAnchor(const CFX_RectF& rtAnchor);
  void Show();
  void Hide();

 protected:
  class CFWL_ToolTipDP : public IFWL_ToolTipDP {
   public:
    CFWL_ToolTipDP();
    FWL_Error GetCaption(IFWL_Widget* pWidget, CFX_WideString& wsCaption);
    int32_t GetInitialDelay(IFWL_Widget* pWidget);
    int32_t GetAutoPopDelay(IFWL_Widget* pWidget);
    CFX_DIBitmap* GetToolTipIcon(IFWL_Widget* pWidget);
    CFX_SizeF GetToolTipIconSize(IFWL_Widget* pWidget);
    CFX_RectF GetAnchor();
    CFX_WideString m_wsCaption;
    int32_t m_nInitDelayTime;
    int32_t m_nAutoPopDelayTime;
    CFX_DIBitmap* m_pBitmap;
    CFX_SizeF m_fIconSize;
    CFX_RectF m_fAnchor;
  };
  CFWL_ToolTipDP m_tooltipData;
};

#endif  // XFA_FWL_LIGHTWIDGET_CFWL_TOOLTIP_H_
