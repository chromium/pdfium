// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_SDAPATER_IMP_H
#define _FWL_SDAPATER_IMP_H
class IFWL_AdapterNative;
class IFWL_AdapterWidgetMgr;
class IFWL_AdapterThreadMgr;
class IFWL_AdapterTimerMgr;
class IFWL_WidgetMgrDelegate;
class CFWL_SDAdatperNative;
class CFWL_SDAdapterWidgetMgr;
class CFWL_SDAdapterTimerMgr;
class CFWL_SDAdapterWidgetMgr : public IFWL_AdapterWidgetMgr {
 public:
  CFWL_SDAdapterWidgetMgr();
  ~CFWL_SDAdapterWidgetMgr();
  virtual FWL_ERR CreateWidget(IFWL_Widget* pWidget,
                               IFWL_Widget* pParent = NULL);
  virtual FWL_ERR DestroyWidget(IFWL_Widget* pWidget);
  virtual FWL_ERR SetWidgetRect(IFWL_Widget* pWidget, const CFX_RectF& rect);
  virtual FWL_ERR SetWidgetPosition(IFWL_Widget* pWidget,
                                    FX_FLOAT fx,
                                    FX_FLOAT fy);
  virtual FWL_ERR SetParentWidget(IFWL_Widget* pWidget, IFWL_Widget* pParent);
  virtual FWL_ERR ShowWidget(IFWL_Widget* pWidget);
  virtual FWL_ERR HideWidget(IFWL_Widget* pWidget);
  virtual FWL_ERR SetNormal(IFWL_Widget* pWidget);
  virtual FWL_ERR SetMaximize(IFWL_Widget* pWidget);
  virtual FWL_ERR SetMinimize(IFWL_Widget* pWidget);
  virtual FWL_ERR RunWidget(IFWL_Widget* pWidget);
  virtual FWL_ERR RepaintWidget(IFWL_Widget* pWidget, const CFX_RectF* pRect);
  virtual FWL_ERR Exit(int32_t iExitCode);
  virtual FWL_ERR CreateWidgetWithNativeId(IFWL_Widget* pWidget, void* vp);
  virtual FX_BOOL GetPopupPos(IFWL_Widget* pWidget,
                              FX_FLOAT fMinHeight,
                              FX_FLOAT fMaxHeight,
                              const CFX_RectF& rtAnchor,
                              CFX_RectF& rtPopup);

 public:
  virtual FWL_ERR GetWidgetRect(IFWL_Widget* pWidget, CFX_RectF& rect);
  virtual FWL_ERR SetWidgetIcon(IFWL_Widget* pWidget,
                                const CFX_DIBitmap* pIcon,
                                FX_BOOL bBig);
  virtual FWL_ERR SetWidgetCaption(IFWL_Widget* pWidget,
                                   const CFX_WideStringC& wsCaption);
  virtual FWL_ERR SetBorderRegion(IFWL_Widget* pWidget, CFX_Path* pPath);
  virtual FWL_ERR SetTransparent(IFWL_Widget* pWidget, FX_DWORD dwAlpha);
  virtual FWL_ERR SetFullScreen(IFWL_Widget* pWidget, FX_BOOL bFullScreen);
  virtual FX_BOOL CheckMessage();
  virtual FX_BOOL IsIdleMessage();
  virtual FWL_ERR DispatchMessage();
  virtual FWL_ERR GetWidgetDC(IFWL_Widget* pWidget, void*& pDC);
  virtual FWL_ERR ReleaseWidgetDC(IFWL_Widget* pWidget,
                                  void* pDC,
                                  CFX_RectF* pClip = 0);
  virtual void* GetWindow(IFWL_Widget* pWidget);
  virtual FX_DWORD GetKeyState(FX_DWORD dwVirtKey);
  virtual FWL_ERR RunLoop(IFWL_Widget* widget);
  virtual FWL_ERR EndLoop();
  virtual FWL_ERR InitMenu(IFWL_Menu* pMenu, IFWL_MenuDP* pMenuData);
  virtual FWL_ERR UpdateMenu(IFWL_Menu* pMenu,
                             const void* hItem,
                             int32_t iType);
  virtual int32_t TrackPopupMenu(IFWL_Menu* pMenu, IFWL_MenuDP* pMenuData);
  virtual FWL_ERR SetMessageHook(IFWL_AdapterMessageHook* hook);
  virtual FWL_ERR GetSystemBorder(FX_FLOAT& l,
                                  FX_FLOAT& t,
                                  FX_FLOAT& r,
                                  FX_FLOAT& b);
};
class CFWL_SDAdapterThreadMgr : public IFWL_AdapterThreadMgr {
 public:
  CFWL_SDAdapterThreadMgr();
  ~CFWL_SDAdapterThreadMgr();

 public:
  virtual FWL_ERR Start(IFWL_Thread* pThread,
                        FWL_HTHREAD& hThread,
                        FX_BOOL bSuspended = FALSE);
  virtual FWL_ERR Resume(FWL_HTHREAD hThread);
  virtual FWL_ERR Suspend(FWL_HTHREAD hThread);
  virtual FWL_ERR Kill(FWL_HTHREAD hThread, int32_t iExitCode);
  virtual FWL_ERR Stop(FWL_HTHREAD hThread, int32_t iExitCode);
  virtual IFWL_Thread* GetCurrentThread();
};
#endif
