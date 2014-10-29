// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_ToolTip_LIGHT_H
#define _FWL_ToolTip_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class IFWL_ToolTipDP;
class CFWL_ToolTip;
class CFWL_ToolTipDP;
class CFWL_ToolTip : public CFWL_Widget
{
public:
    static CFWL_ToolTip* Create();
    FWL_ERR Initialize(const CFWL_WidgetProperties *pProperties = NULL);
    FWL_ERR			GetCaption(CFX_WideString &wsCaption);
    FWL_ERR			SetCaption(FX_WSTR wsCaption);
    FX_INT32	 	GetInitialDelay();
    FX_INT32	 	SetInitialDelay(FX_INT32 nDelayTime);
    FX_INT32	 	GetAutoPopDelay();
    FX_INT32	 	SetAutoPopDelay(FX_INT32 nDelayTime);
    CFX_DIBitmap*	GetToolTipIcon();
    FWL_ERR			SetToolTipIcon(CFX_DIBitmap* pBitmap);
    CFX_SizeF		GetToolTipIconSize();
    FWL_ERR			SetToolTipIconSize( CFX_SizeF fSize);
    FWL_ERR		SetAnchor(const CFX_RectF &rtAnchor);
    FWL_ERR		Show();
    FWL_ERR		Hide();
    CFWL_ToolTip();
    virtual ~CFWL_ToolTip();
protected:
    class CFWL_ToolTipDP : public IFWL_ToolTipDP
    {
    public:
        CFWL_ToolTipDP();
        FWL_ERR GetCaption(IFWL_Widget *pWidget, CFX_WideString &wsCaption);
        FX_INT32	 	GetInitialDelay(IFWL_Widget *pWidget);
        FX_INT32	 	GetAutoPopDelay(IFWL_Widget *pWidget);
        CFX_DIBitmap*	GetToolTipIcon(IFWL_Widget *pWidget);
        CFX_SizeF		GetToolTipIconSize(IFWL_Widget *pWidget);
        CFX_RectF		GetAnchor();
        CFX_WideString	m_wsCaption;
        FX_INT32		m_nInitDelayTime;
        FX_INT32		m_nAutoPopDelayTime;
        CFX_DIBitmap*	m_pBitmap;
        CFX_SizeF		m_fIconSize;
        CFX_RectF		m_fAnchor;
    };
    CFWL_ToolTipDP m_tooltipData;
};
#endif
