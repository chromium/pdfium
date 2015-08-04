// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_ADAPTER_WIDGETMGR_H
#define _FWL_ADAPTER_WIDGETMGR_H
class IFWL_Widget;
class IFWL_Menu;
class IFWL_MenuDP;
class CFX_Path;
class CFX_DIBitmap;
class IFWL_AdapterMessageHook;
class IFWL_AppDelegate;

class IFWL_AdapterWidgetMgr {
 public:
  virtual ~IFWL_AdapterWidgetMgr() {}
  virtual FWL_ERR CreateWidget(IFWL_Widget* pWidget,
                               IFWL_Widget* pParent = NULL) = 0;
  virtual FWL_ERR DestroyWidget(IFWL_Widget* pWidget) = 0;
  virtual FWL_ERR GetWidgetRect(IFWL_Widget* pWidget, CFX_RectF& rect) = 0;
  virtual FWL_ERR SetWidgetRect(IFWL_Widget* pWidget,
                                const CFX_RectF& rect) = 0;
  virtual FWL_ERR SetWidgetPosition(IFWL_Widget* pWidget,
                                    FX_FLOAT fx,
                                    FX_FLOAT fy) = 0;
  virtual FWL_ERR SetParentWidget(IFWL_Widget* pWidget,
                                  IFWL_Widget* pParent) = 0;
  virtual FWL_ERR SetWidgetIcon(IFWL_Widget* pWidget,
                                const CFX_DIBitmap* pIcon,
                                FX_BOOL bBig) = 0;
  virtual FWL_ERR SetWidgetCaption(IFWL_Widget* pWidget,
                                   const CFX_WideStringC& wsCaption) = 0;
  virtual FWL_ERR SetBorderRegion(IFWL_Widget* pWidget, CFX_Path* pPath) = 0;
  virtual FWL_ERR SetTransparent(IFWL_Widget* pWidget, FX_DWORD dwAlpha) = 0;
  virtual FWL_ERR ShowWidget(IFWL_Widget* pWidget) = 0;
  virtual FWL_ERR HideWidget(IFWL_Widget* pWidget) = 0;
  virtual FWL_ERR SetNormal(IFWL_Widget* pWidget) = 0;
  virtual FWL_ERR SetMaximize(IFWL_Widget* pWidget) = 0;
  virtual FWL_ERR SetMinimize(IFWL_Widget* pWidget) = 0;
  virtual FWL_ERR SetFullScreen(IFWL_Widget* pWidget, FX_BOOL bFullScreen) = 0;
  virtual FX_BOOL CheckMessage() = 0;
  virtual FX_BOOL IsIdleMessage() = 0;
  virtual FWL_ERR DispatchMessage() = 0;
  virtual FWL_ERR RepaintWidget(IFWL_Widget* pWidget,
                                const CFX_RectF* pRect) = 0;
  virtual FWL_ERR Exit(int32_t iExitCode) = 0;
  virtual FWL_ERR CreateWidgetWithNativeId(IFWL_Widget* pWidget,
                                           void* UserData) = 0;
  virtual FWL_ERR GetWidgetDC(IFWL_Widget* pWidget, void*& pDC) = 0;
  virtual FWL_ERR ReleaseWidgetDC(IFWL_Widget* pWidget,
                                  void* pDC,
                                  CFX_RectF* pClip = 0) = 0;
  virtual void* GetWindow(IFWL_Widget* pWidget) = 0;
  virtual FX_DWORD GetKeyState(FX_DWORD dwVirtKey) = 0;
  virtual FWL_ERR RunLoop(IFWL_Widget* widget) = 0;
  virtual FWL_ERR EndLoop() = 0;
  virtual FWL_ERR InitMenu(IFWL_Menu* pMenu, IFWL_MenuDP* pMenuData) = 0;
  virtual FWL_ERR UpdateMenu(IFWL_Menu* pMenu,
                             const void* hItem,
                             int32_t iType) = 0;
  virtual int32_t TrackPopupMenu(IFWL_Menu* pMenu, IFWL_MenuDP* pMenuData) = 0;
  virtual FWL_ERR SetMessageHook(IFWL_AdapterMessageHook* hook) = 0;
  virtual FWL_ERR GetSystemBorder(FX_FLOAT& l,
                                  FX_FLOAT& t,
                                  FX_FLOAT& r,
                                  FX_FLOAT& b) = 0;
  virtual FX_BOOL GetPopupPos(IFWL_Widget* pWidget,
                              FX_FLOAT fMinHeight,
                              FX_FLOAT fMaxHeight,
                              const CFX_RectF& rtAnchor,
                              CFX_RectF& rtPopup) = 0;
};
#endif
