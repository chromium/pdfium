// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_WIDGETMGR_H
#define _FWL_WIDGETMGR_H
class IFWL_Widget;
class IFWL_WidgetMgr;
class IFWL_WidgetMgrDelegate;
enum FWL_WGTRELATION {
  FWL_WGTRELATION_Parent = 0,
  FWL_WGTRELATION_Owner,
  FWL_WGTRELATION_FirstSibling,
  FWL_WGTRELATION_PriorSibling,
  FWL_WGTRELATION_NextSibling,
  FWL_WGTRELATION_LastSibling,
  FWL_WGTRELATION_FirstChild,
  FWL_WGTRELATION_LastChild,
  FWL_WGTRELATION_SystemForm
};
class IFWL_WidgetMgr {
 public:
  virtual ~IFWL_WidgetMgr() {}
  virtual int32_t CountWidgets(IFWL_Widget* pParent = NULL) = 0;
  virtual IFWL_Widget* GetWidget(int32_t nIndex,
                                 IFWL_Widget* pParent = NULL) = 0;
  virtual IFWL_Widget* GetWidget(IFWL_Widget* pWidget,
                                 FWL_WGTRELATION eRelation) = 0;
  virtual int32_t GetWidgetIndex(IFWL_Widget* pWidget) = 0;
  virtual FX_BOOL SetWidgetIndex(IFWL_Widget* pWidget, int32_t nIndex) = 0;
  virtual FWL_ERR RepaintWidget(IFWL_Widget* pWidget,
                                const CFX_RectF* pRect = NULL) = 0;
  virtual FX_DWORD GetCapability() = 0;
};
IFWL_WidgetMgr* FWL_GetWidgetMgr();
FX_BOOL FWL_WidgetIsChild(IFWL_Widget* parent, IFWL_Widget* find);
#define FWL_WGTMGR_DisableThread 0x00000001
#define FWL_WGTMGR_DisableForm 0x00000002
class IFWL_WidgetMgrDelegate {
 public:
  virtual ~IFWL_WidgetMgrDelegate() {}
  virtual FWL_ERR OnSetCapability(
      FX_DWORD dwCapability = FWL_WGTMGR_DisableThread) = 0;
  virtual int32_t OnProcessMessageToForm(CFWL_Message* pMessage) = 0;
  virtual FWL_ERR OnDrawWidget(IFWL_Widget* pWidget,
                               CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = NULL) = 0;
};
FWL_ERR FWL_WidgetMgrSnapshot(IFWL_Widget* pWidget,
                              const CFX_WideString* saveFile,
                              const CFX_Matrix* pMatrix = NULL);
#endif
