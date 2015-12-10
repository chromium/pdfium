// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FWL_WIDGETIMP_H_
#define FWL_WIDGETIMP_H_

#include "xfa/include/fwl/core/fwl_widget.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"

class CFWL_NoteTarget;
class CFWL_NoteThreadImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetMgr;
class IFWL_DataProvider;
class IFWL_ThemeProvider;
class IFWL_Widget;
class IFWL_WidgetDelegate;

class CFWL_WidgetImp : public CFWL_TargetImp {
 public:
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR GetGlobalRect(CFX_RectF& rect);
  virtual FWL_ERR SetWidgetRect(const CFX_RectF& rect);
  virtual FWL_ERR GetClientRect(CFX_RectF& rect);
  virtual IFWL_Widget* GetParent();
  virtual FWL_ERR SetParent(IFWL_Widget* pParent);
  virtual IFWL_Widget* GetOwner();
  virtual FWL_ERR SetOwner(IFWL_Widget* pOwner);
  virtual IFWL_Widget* GetOuter();
  virtual FX_DWORD GetStyles();
  virtual FWL_ERR ModifyStyles(FX_DWORD dwStylesAdded,
                               FX_DWORD dwStylesRemoved);
  virtual FX_DWORD GetStylesEx();
  virtual FWL_ERR ModifyStylesEx(FX_DWORD dwStylesExAdded,
                                 FX_DWORD dwStylesExRemoved);
  virtual FX_DWORD GetStates();
  virtual FWL_ERR SetStates(FX_DWORD dwStates, FX_BOOL bSet = TRUE);
  virtual FWL_ERR SetPrivateData(void* module_id,
                                 void* pData,
                                 PD_CALLBACK_FREEDATA callback);
  virtual void* GetPrivateData(void* module_id);
  virtual FWL_ERR Update();
  virtual FWL_ERR LockUpdate();
  virtual FWL_ERR UnlockUpdate();
  virtual FX_DWORD HitTest(FX_FLOAT fx, FX_FLOAT fy);
  virtual FWL_ERR TransformTo(IFWL_Widget* pWidget, FX_FLOAT& fx, FX_FLOAT& fy);
  virtual FWL_ERR TransformTo(IFWL_Widget* pWidget, CFX_RectF& rt);
  virtual FWL_ERR GetMatrix(CFX_Matrix& matrix, FX_BOOL bGlobal = FALSE);
  virtual FWL_ERR SetMatrix(const CFX_Matrix& matrix);
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);
  virtual IFWL_ThemeProvider* GetThemeProvider();
  virtual FWL_ERR SetThemeProvider(IFWL_ThemeProvider* pThemeProvider);
  virtual FWL_ERR SetDataProvider(IFWL_DataProvider* pDataProvider);
  virtual IFWL_WidgetDelegate* SetDelegate(IFWL_WidgetDelegate* pDelegate);
  virtual IFWL_NoteThread* GetOwnerThread() const;
  FWL_ERR SetOwnerThread(CFWL_NoteThreadImp* pOwnerThread);
  IFWL_Widget* GetInterface() const;
  void SetInterface(IFWL_Widget* pInterface);
  CFX_SizeF GetOffsetFromParent(IFWL_Widget* pParent);

 protected:
  CFWL_WidgetImp(const CFWL_WidgetImpProperties& properties,
                 IFWL_Widget* pOuter);
  virtual ~CFWL_WidgetImp();
  FX_BOOL IsEnabled() const;
  FX_BOOL IsVisible() const;
  FX_BOOL IsActive() const;
  FX_BOOL IsOverLapper() const;
  FX_BOOL IsPopup() const;
  FX_BOOL IsChild() const;
  FX_BOOL IsLocked() const;
  FX_BOOL IsOffscreen() const;
  FX_BOOL HasBorder() const;
  FX_BOOL HasEdge() const;
  void GetEdgeRect(CFX_RectF& rtEdge);
  FX_FLOAT GetBorderSize(FX_BOOL bCX = TRUE);
  FX_FLOAT GetEdgeWidth();
  void GetRelativeRect(CFX_RectF& rect);
  void* GetThemeCapacity(FX_DWORD dwCapacity);
  IFWL_ThemeProvider* GetAvailableTheme();
  CFWL_WidgetImp* GetRootOuter();
  CFX_SizeF CalcTextSize(const CFX_WideString& wsText,
                         IFWL_ThemeProvider* pTheme,
                         FX_BOOL bMultiLine = FALSE,
                         int32_t iLineWidth = -1);
  void CalcTextRect(const CFX_WideString& wsText,
                    IFWL_ThemeProvider* pTheme,
                    FX_DWORD dwTTOStyles,
                    int32_t iTTOAlign,
                    CFX_RectF& rect);
  void SetFocus(FX_BOOL bFocus);
  void SetGrab(FX_BOOL bSet);
  FX_BOOL GetPopupPos(FX_FLOAT fMinHeight,
                      FX_FLOAT fMaxHeight,
                      const CFX_RectF& rtAnchor,
                      CFX_RectF& rtPopup);
  FX_BOOL GetPopupPosMenu(FX_FLOAT fMinHeight,
                          FX_FLOAT fMaxHeight,
                          const CFX_RectF& rtAnchor,
                          CFX_RectF& rtPopup);
  FX_BOOL GetPopupPosComboBox(FX_FLOAT fMinHeight,
                              FX_FLOAT fMaxHeight,
                              const CFX_RectF& rtAnchor,
                              CFX_RectF& rtPopup);
  FX_BOOL GetPopupPosGeneral(FX_FLOAT fMinHeight,
                             FX_FLOAT fMaxHeight,
                             const CFX_RectF& rtAnchor,
                             CFX_RectF& rtPopup);
  FX_BOOL GetScreenSize(FX_FLOAT& fx, FX_FLOAT& fy);
  void RegisterEventTarget(IFWL_Widget* pEventSource = NULL,
                           FX_DWORD dwFilter = FWL_EVENT_ALL_MASK);
  void UnregisterEventTarget();
  void DispatchKeyEvent(CFWL_MsgKey* pNote);
  void DispatchEvent(CFWL_Event* pEvent);
  void Repaint(const CFX_RectF* pRect = NULL);
  void DrawBackground(CFX_Graphics* pGraphics,
                      int32_t iPartBk,
                      IFWL_ThemeProvider* pTheme,
                      const CFX_Matrix* pMatrix = NULL);
  void DrawBorder(CFX_Graphics* pGraphics,
                  int32_t iPartBorder,
                  IFWL_ThemeProvider* pTheme,
                  const CFX_Matrix* pMatrix = NULL);
  void DrawEdge(CFX_Graphics* pGraphics,
                int32_t iPartEdge,
                IFWL_ThemeProvider* pTheme,
                const CFX_Matrix* pMatrix = NULL);
  void NotifyDriver();

  FX_BOOL IsParent(IFWL_Widget* pParent);
  CFWL_WidgetMgr* m_pWidgetMgr;
  CFWL_NoteThreadImp* m_pOwnerThread;
  CFWL_WidgetImpProperties* m_pProperties;
  CFX_PrivateData* m_pPrivateData;
  IFWL_WidgetDelegate* m_pDelegate;
  IFWL_WidgetDelegate* m_pCurDelegate;
  IFWL_Widget* m_pOuter;
  IFWL_Widget* m_pInterface;
  int32_t m_iLock;
  friend class CFWL_WidgetImpDelegate;
  friend void FWL_SetWidgetRect(IFWL_Widget* widget, const CFX_RectF& rect);
  friend void FWL_SetWidgetStates(IFWL_Widget* widget, FX_DWORD dwStates);
  friend void FWL_SetWidgetStyles(IFWL_Widget* widget, FX_DWORD dwStyles);
};

class CFWL_WidgetImpDelegate : public IFWL_WidgetDelegate {
 public:
  CFWL_WidgetImpDelegate();
  ~CFWL_WidgetImpDelegate() override {}
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnProcessEvent(CFWL_Event* pEvent) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;
};

#endif  // FWL_WIDGETIMP_H_
