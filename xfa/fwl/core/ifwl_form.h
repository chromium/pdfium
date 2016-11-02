// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_FORM_H_
#define XFA_FWL_CORE_IFWL_FORM_H_

#include <memory>

#include "core/fxcrt/fx_system.h"
#include "xfa/fwl/core/cfwl_widgetimpproperties.h"
#include "xfa/fwl/core/ifwl_dataprovider.h"
#include "xfa/fwl/core/ifwl_widget.h"

#define FWL_CLASS_Form L"FWL_FORM"
#define FWL_CLASS_FormProxy L"FWL_FORMPROXY"
#define FWL_STYLEEXT_FRM_Resize (1L << 0)
#define FWL_STYLEEXT_FRM_NativeBorder (1L << 1)
#define FWL_STYLEEXT_FRM_RoundCorner (2L << 1)
#define FWL_STYLEEXT_FRM_RoundCorner4 (3L << 1)
#define FWL_STYLEEXT_FRM_NoDrawClient (1L << 3)
#define FWL_STYLEEXT_FRM_BorderCornerMask (3L << 1)
#define FWL_STYLEEXT_FRM_Max (3)

#if (_FX_OS_ == _FX_MACOSX_)
#define FWL_UseMacSystemBorder
#endif

#define FWL_SYSBUTTONSTATE_Hover 0x0001
#define FWL_SYSBUTTONSTATE_Pressed 0x0002
#define FWL_SYSBUTTONSTATE_Disabled 0x0010

enum FWL_FORMSIZE {
  FWL_FORMSIZE_Manual = 0,
  FWL_FORMSIZE_Width,
  FWL_FORMSIZE_Height,
  FWL_FORMSIZE_All,
};

class CFWL_SysBtn {
 public:
  CFWL_SysBtn();

  bool IsDisabled() const;
  uint32_t GetPartState() const;

  void SetNormal();
  void SetPressed();
  void SetHover();
  void SetDisabled(FX_BOOL bDisabled);

  CFX_RectF m_rtBtn;
  uint32_t m_dwState;
};

enum FORM_RESIZETYPE {
  FORM_RESIZETYPE_None = 0,
  FORM_RESIZETYPE_Cap,
};

struct RestoreInfo {
  RestoreInfo();
  ~RestoreInfo();

  CFX_PointF m_ptStart;
  CFX_SizeF m_szStart;
};

class CFWL_MsgMouse;
class CFWL_MsgClose;
class CFWL_MsgWindowMove;
class CFWL_NoteLoop;
class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_ThemeProvider;
class CFWL_SysBtn;

class IFWL_FormDP : public IFWL_DataProvider {
 public:
  virtual CFX_DIBitmap* GetIcon(IFWL_Widget* pWidget, FX_BOOL bBig) = 0;
};

class IFWL_Form : public IFWL_Widget {
 public:
  IFWL_Form(const IFWL_App* app,
            const CFWL_WidgetImpProperties& properties,
            IFWL_Widget* pOuter);
  ~IFWL_Form() override;

  // IFWL_Widget
  FWL_Type GetClassID() const override;
  FX_BOOL IsInstance(const CFX_WideStringC& wsClass) const override;
  FWL_Error GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE) override;
  FWL_Error GetClientRect(CFX_RectF& rect) override;
  FWL_Error Update() override;
  FWL_WidgetHit HitTest(FX_FLOAT fx, FX_FLOAT fy) override;
  FWL_Error DrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = nullptr) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix) override;

  FWL_FORMSIZE GetFormSize();
  FWL_Error SetFormSize(FWL_FORMSIZE eFormSize);
  IFWL_Widget* DoModal();
  IFWL_Widget* DoModal(uint32_t& dwCommandID);
  FWL_Error EndDoModal();
  FWL_Error SetBorderRegion(CFX_Path* pPath);
  void DrawBackground(CFX_Graphics* pGraphics, IFWL_ThemeProvider* pTheme);
  IFWL_Widget* GetSubFocus();
  void SetSubFocus(IFWL_Widget* pWidget);

 protected:
  friend class CFWL_FormImpDelegate;

  void ShowChildWidget(IFWL_Widget* pParent);
  void RemoveSysButtons();
  void CalcContentRect(CFX_RectF& rtContent);
  CFWL_SysBtn* GetSysBtnAtPoint(FX_FLOAT fx, FX_FLOAT fy);
  CFWL_SysBtn* GetSysBtnByState(uint32_t dwState);
  CFWL_SysBtn* GetSysBtnByIndex(int32_t nIndex);
  int32_t GetSysBtnIndex(CFWL_SysBtn* pBtn);
  FX_FLOAT GetCaptionHeight();
  void DrawCaptionText(CFX_Graphics* pGs,
                       IFWL_ThemeProvider* pTheme,
                       const CFX_Matrix* pMatrix = nullptr);
  void DrawIconImage(CFX_Graphics* pGs,
                     IFWL_ThemeProvider* pTheme,
                     const CFX_Matrix* pMatrix = nullptr);
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

  CFX_RectF m_rtRestore;
  CFX_RectF m_rtCaptionText;
  CFX_RectF m_rtRelative;
  CFX_RectF m_rtCaption;
  CFX_RectF m_rtIcon;
  CFWL_SysBtn* m_pCloseBox;
  CFWL_SysBtn* m_pMinBox;
  CFWL_SysBtn* m_pMaxBox;
  CFWL_SysBtn* m_pCaptionBox;
  std::unique_ptr<CFWL_NoteLoop> m_pNoteLoop;
  IFWL_Widget* m_pSubFocus;
  RestoreInfo m_InfoStart;
  FX_FLOAT m_fCXBorder;
  FX_FLOAT m_fCYBorder;
  int32_t m_iCaptureBtn;
  int32_t m_iSysBox;
  int32_t m_eResizeType;
  FX_BOOL m_bLButtonDown;
  bool m_bMaximized;
  FX_BOOL m_bSetMaximize;
  bool m_bCustomizeLayout;
  FWL_FORMSIZE m_eFormSize;
  FX_BOOL m_bDoModalFlag;
  FX_FLOAT m_fSmallIconSz;
  FX_FLOAT m_fBigIconSz;
  CFX_DIBitmap* m_pBigIcon;
  CFX_DIBitmap* m_pSmallIcon;
  FX_BOOL m_bMouseIn;

 private:
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnMouseHover(CFWL_MsgMouse* pMsg);
  void OnMouseLeave(CFWL_MsgMouse* pMsg);
  void OnLButtonDblClk(CFWL_MsgMouse* pMsg);
  void OnWindowMove(CFWL_MsgWindowMove* pMsg);
  void OnClose(CFWL_MsgClose* pMsg);
};

#endif  // XFA_FWL_CORE_IFWL_FORM_H_
