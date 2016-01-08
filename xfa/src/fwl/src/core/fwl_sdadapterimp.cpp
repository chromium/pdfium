// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_threadimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetmgrimp.h"
CFWL_SDAdapterWidgetMgr::CFWL_SDAdapterWidgetMgr() {}
CFWL_SDAdapterWidgetMgr::~CFWL_SDAdapterWidgetMgr() {}
FWL_ERR CFWL_SDAdapterWidgetMgr::CreateWidget(IFWL_Widget* pWidget,
                                              IFWL_Widget* pParent) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::DestroyWidget(IFWL_Widget* pWidget) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetWidgetRect(IFWL_Widget* pWidget,
                                               const CFX_RectF& rect) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetWidgetPosition(IFWL_Widget* pWidget,
                                                   FX_FLOAT fx,
                                                   FX_FLOAT fy) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetParentWidget(IFWL_Widget* pWidget,
                                                 IFWL_Widget* pParent) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::ShowWidget(IFWL_Widget* pWidget) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::HideWidget(IFWL_Widget* pWidget) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetNormal(IFWL_Widget* pWidget) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetMaximize(IFWL_Widget* pWidget) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetMinimize(IFWL_Widget* pWidget) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::RunWidget(IFWL_Widget* pWidget) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::RepaintWidget(IFWL_Widget* pWidget,
                                               const CFX_RectF* pRect) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::Exit(int32_t iExitCode) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::CreateWidgetWithNativeId(IFWL_Widget* pWidget,
                                                          void* vp) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::GetWidgetRect(IFWL_Widget* pWidget,
                                               CFX_RectF& rect) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetWidgetIcon(IFWL_Widget* pWidget,
                                               const CFX_DIBitmap* pIcon,
                                               FX_BOOL bBig) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetWidgetCaption(
    IFWL_Widget* pWidget,
    const CFX_WideStringC& wsCaption) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetBorderRegion(IFWL_Widget* pWidget,
                                                 CFX_Path* pPath) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetTransparent(IFWL_Widget* pWidget,
                                                FX_DWORD dwAlpha) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetFullScreen(IFWL_Widget* pWidget,
                                               FX_BOOL bFullScreen) {
  return FWL_ERR_Succeeded;
}
FX_BOOL CFWL_SDAdapterWidgetMgr::CheckMessage() {
  return TRUE;
}
FX_BOOL CFWL_SDAdapterWidgetMgr::IsIdleMessage() {
  return TRUE;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::DispatchMessage() {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::GetWidgetDC(IFWL_Widget* pWidget, void*& pDC) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::ReleaseWidgetDC(IFWL_Widget* pWidget,
                                                 void* pDC,
                                                 CFX_RectF* pClip) {
  return FWL_ERR_Succeeded;
}
void* CFWL_SDAdapterWidgetMgr::GetWindow(IFWL_Widget* pWidget) {
  return NULL;
}
FX_DWORD CFWL_SDAdapterWidgetMgr::GetKeyState(FX_DWORD dwVirtKey) {
  return 0;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::RunLoop(IFWL_Widget* widget) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::EndLoop() {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::InitMenu(IFWL_Menu* pMenu,
                                          IFWL_MenuDP* pMenuData) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::UpdateMenu(IFWL_Menu* pMenu,
                                            const void* hItem,
                                            int32_t iType) {
  return FWL_ERR_Succeeded;
}
int32_t CFWL_SDAdapterWidgetMgr::TrackPopupMenu(IFWL_Menu* pMenu,
                                                IFWL_MenuDP* pMenuData) {
  return 0;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::SetMessageHook(IFWL_AdapterMessageHook* hook) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterWidgetMgr::GetSystemBorder(FX_FLOAT& l,
                                                 FX_FLOAT& t,
                                                 FX_FLOAT& r,
                                                 FX_FLOAT& b) {
  return FWL_ERR_Succeeded;
}
FX_BOOL CFWL_SDAdapterWidgetMgr::GetPopupPos(IFWL_Widget* pWidget,
                                             FX_FLOAT fMinHeight,
                                             FX_FLOAT fMaxHeight,
                                             const CFX_RectF& rtAnchor,
                                             CFX_RectF& rtPopup) {
  return FWL_ERR_Succeeded;
}
CFWL_SDAdapterThreadMgr::CFWL_SDAdapterThreadMgr() {}
CFWL_SDAdapterThreadMgr::~CFWL_SDAdapterThreadMgr() {}
FWL_ERR CFWL_SDAdapterThreadMgr::Start(IFWL_Thread* pThread,
                                       FWL_HTHREAD& hThread,
                                       FX_BOOL bSuspended) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterThreadMgr::Resume(FWL_HTHREAD hThread) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterThreadMgr::Suspend(FWL_HTHREAD hThread) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterThreadMgr::Kill(FWL_HTHREAD hThread, int32_t iExitCode) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SDAdapterThreadMgr::Stop(FWL_HTHREAD hThread, int32_t iExitCode) {
  return FWL_ERR_Succeeded;
}
IFWL_Thread* CFWL_SDAdapterThreadMgr::GetCurrentThread() {
  return FWL_GetApp();
}
