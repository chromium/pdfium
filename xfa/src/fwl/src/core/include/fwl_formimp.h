// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_FORM_IMP_H
#define _FWL_FORM_IMP_H
class CFWL_NoteLoop;
class CFWL_PanelImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class CFWL_MsgMouse;
class IFWL_Widget;
class IFWL_ThemeProvider;
class CFWL_SysBtn;
class CFWL_FormImp;
class CFWL_FormImpDelegate;
#define FWL_SYSBUTTONSTATE_Hover 0x0001
#define FWL_SYSBUTTONSTATE_Pressed 0x0002
#define FWL_SYSBUTTONSTATE_Disabled 0x0010
class CFWL_SysBtn {
 public:
  CFWL_SysBtn() {
    m_rtBtn.Set(0, 0, 0, 0);
    m_dwState = 0;
  }

  FX_BOOL IsHover() { return m_dwState & FWL_SYSBUTTONSTATE_Hover; }
  FX_BOOL IsPressed() { return m_dwState & FWL_SYSBUTTONSTATE_Pressed; }
  FX_BOOL IsDisabled() { return m_dwState & FWL_SYSBUTTONSTATE_Disabled; }
  void SetNormal() { m_dwState &= 0xFFF0; }
  void SetPressed() {
    SetNormal();
    m_dwState |= FWL_SYSBUTTONSTATE_Pressed;
  }
  void SetHover() {
    SetNormal();
    m_dwState |= FWL_SYSBUTTONSTATE_Hover;
  }
  void SetDisabled(FX_BOOL bDisabled) {
    bDisabled ? m_dwState |= FWL_SYSBUTTONSTATE_Disabled
              : m_dwState &= ~FWL_SYSBUTTONSTATE_Disabled;
  }
  int32_t GetPartState() {
    return (IsDisabled() ? FWL_PARTSTATE_FRM_Disabled : (m_dwState + 1));
  }

  CFX_RectF m_rtBtn;
  FX_DWORD m_dwState;
};
enum FORM_RESIZETYPE {
  FORM_RESIZETYPE_None = 0,
  FORM_RESIZETYPE_Cap,
  FORM_RESIZETYPE_Left,
  FORM_RESIZETYPE_Top,
  FORM_RESIZETYPE_Right,
  FORM_RESIZETYPE_Bottom,
  FORM_RESIZETYPE_LeftTop,
  FORM_RESIZETYPE_LeftBottom,
  FORM_RESIZETYPE_RightTop,
  FORM_RESIZETYPE_RightBottom
};
typedef struct RestoreResizeInfo {
  CFX_PointF m_ptStart;
  CFX_SizeF m_szStart;
} RestoreInfo;
class CFWL_FormImp : public CFWL_PanelImp {
 public:
  CFWL_FormImp(const CFWL_WidgetImpProperties& properties, IFWL_Widget* pOuter);
  virtual ~CFWL_FormImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FX_BOOL IsInstance(const CFX_WideStringC& wsClass) const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR GetClientRect(CFX_RectF& rect);
  virtual FWL_ERR Update();
  virtual FX_DWORD HitTest(FX_FLOAT fx, FX_FLOAT fy);
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);
  virtual FWL_FORMSIZE GetFormSize();
  virtual FWL_ERR SetFormSize(FWL_FORMSIZE eFormSize);
  virtual IFWL_Widget* DoModal();
  virtual IFWL_Widget* DoModal(FX_DWORD& dwCommandID);
  virtual FWL_ERR EndDoModal();
  virtual FWL_ERR SetBorderRegion(CFX_Path* pPath);
  virtual void DrawBackground(CFX_Graphics* pGraphics,
                              IFWL_ThemeProvider* pTheme);
  CFWL_WidgetImp* GetSubFocus();
  void SetSubFocus(CFWL_WidgetImp* pWidget);
  CFX_MapAccelerators& GetAccelerator();
  void SetAccelerator(CFX_MapAccelerators* pAccelerators);

 protected:
  void ShowChildWidget(IFWL_Widget* pParent);
  void RemoveSysButtons();
  void CalcContentRect(CFX_RectF& rtContent);
  CFWL_SysBtn* GetSysBtnAtPoint(FX_FLOAT fx, FX_FLOAT fy);
  CFWL_SysBtn* GetSysBtnByState(FX_DWORD dwState);
  CFWL_SysBtn* GetSysBtnByIndex(int32_t nIndex);
  int32_t GetSysBtnIndex(CFWL_SysBtn* pBtn);
  FX_FLOAT GetCaptionHeight();
  void DrawCaptionText(CFX_Graphics* pGs,
                       IFWL_ThemeProvider* pTheme,
                       const CFX_Matrix* pMatrix = NULL);
  void DrawIconImage(CFX_Graphics* pGs,
                     IFWL_ThemeProvider* pTheme,
                     const CFX_Matrix* pMatrix = NULL);
  void GetEdgeRect(CFX_RectF& rtEdge);
  void SetWorkAreaRect();
  void SetCursor(FX_FLOAT fx, FX_FLOAT fy);
  void Layout();
  void ReSetSysBtn();
  void RegisterForm();
  void UnRegisterForm();
  FX_BOOL IsDoModal();
  void SetThemeData();
  FX_BOOL HasIcon();
  void UpdateIcon();
  void UpdateCaption();
  void DoWidthLimit(FX_FLOAT& fLeft,
                    FX_FLOAT& fWidth,
                    FX_FLOAT fCurX,
                    FX_FLOAT fSpace,
                    FX_FLOAT fLimitMin,
                    FX_FLOAT fLimitMax,
                    FX_BOOL bLeft);
  void DoHeightLimit(FX_FLOAT& fTop,
                     FX_FLOAT& fHeight,
                     FX_FLOAT fCurY,
                     FX_FLOAT fSpace,
                     FX_FLOAT fLimitMin,
                     FX_FLOAT fLimitMax,
                     FX_BOOL bTop);
  CFX_MapAccelerators m_mapAccelerators;
  CFX_RectF m_rtRestore;
  CFX_RectF m_rtCaptionText;
  CFX_RectF m_rtRelative;
  CFX_RectF m_rtCaption;
  CFX_RectF m_rtIcon;
  CFWL_SysBtn* m_pCloseBox;
  CFWL_SysBtn* m_pMinBox;
  CFWL_SysBtn* m_pMaxBox;
  CFWL_SysBtn* m_pCaptionBox;
  CFWL_NoteLoop* m_pNoteLoop;
  CFWL_WidgetImp* m_pSubFocus;
  RestoreInfo m_InfoStart;
  FX_FLOAT m_fCXBorder;
  FX_FLOAT m_fCYBorder;
  int32_t m_iCaptureBtn;
  int32_t m_iSysBox;
  int32_t m_eResizeType;
  FX_BOOL m_bLButtonDown;
  FX_BOOL m_bMaximized;
  FX_BOOL m_bSetMaximize;
  FX_BOOL m_bCustomizeLayout;
  FWL_FORMSIZE m_eFormSize;
  FX_BOOL m_bDoModalFlag;
  FX_FLOAT m_fSmallIconSz;
  FX_FLOAT m_fBigIconSz;
  CFX_DIBitmap* m_pBigIcon;
  CFX_DIBitmap* m_pSmallIcon;
  FX_BOOL m_bMouseIn;
  friend class CFWL_FormImpDelegate;
};
class CFWL_FormImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_FormImpDelegate(CFWL_FormImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnProcessEvent(CFWL_Event* pEvent) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;

 protected:
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnMouseHover(CFWL_MsgMouse* pMsg);
  void OnMouseLeave(CFWL_MsgMouse* pMsg);
  void OnLButtonDblClk(CFWL_MsgMouse* pMsg);
  void OnWindowMove(CFWL_MsgWindowMove* pMsg);
  void OnClose(CFWL_MsgClose* pMsg);
  CFWL_FormImp* m_pOwner;
};
#endif
