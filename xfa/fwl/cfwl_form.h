// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_FORM_H_
#define XFA_FWL_CFWL_FORM_H_

#include <memory>

#include "core/fxcrt/fx_system.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/cfwl_widgetproperties.h"

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

class CFWL_MessageMouse;
class CFWL_NoteLoop;
class CFWL_Widget;
class IFWL_ThemeProvider;
class CFWL_SysBtn;

class CFWL_Form : public CFWL_Widget {
 public:
  CFWL_Form(const CFWL_App* app,
            std::unique_ptr<CFWL_WidgetProperties> properties,
            CFWL_Widget* pOuter);
  ~CFWL_Form() override;

  // CFWL_Widget
  FWL_Type GetClassID() const override;
  bool IsInstance(const CFX_WideStringC& wsClass) const override;
  void GetClientRect(CFX_RectF& rect) override;
  void Update() override;
  FWL_WidgetHit HitTest(FX_FLOAT fx, FX_FLOAT fy) override;
  void DrawWidget(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix) override;

  CFWL_Widget* DoModal();
  void EndDoModal();

  CFWL_Widget* GetSubFocus() const { return m_pSubFocus; }
  void SetSubFocus(CFWL_Widget* pWidget) { m_pSubFocus = pWidget; }

 private:
  void DrawBackground(CFX_Graphics* pGraphics, IFWL_ThemeProvider* pTheme);
  void RemoveSysButtons();
  CFWL_SysBtn* GetSysBtnAtPoint(FX_FLOAT fx, FX_FLOAT fy);
  CFWL_SysBtn* GetSysBtnByState(uint32_t dwState);
  CFWL_SysBtn* GetSysBtnByIndex(int32_t nIndex);
  int32_t GetSysBtnIndex(CFWL_SysBtn* pBtn);
  void GetEdgeRect(CFX_RectF& rtEdge);
  void SetWorkAreaRect();
  void Layout();
  void ResetSysBtn();
  void RegisterForm();
  void UnRegisterForm();
  void OnLButtonDown(CFWL_MessageMouse* pMsg);
  void OnLButtonUp(CFWL_MessageMouse* pMsg);
  void OnMouseMove(CFWL_MessageMouse* pMsg);
  void OnMouseLeave(CFWL_MessageMouse* pMsg);
  void OnLButtonDblClk(CFWL_MessageMouse* pMsg);

#if (_FX_OS_ == _FX_MACOSX_)
  bool m_bMouseIn;
#endif
  CFX_RectF m_rtRestore;
  CFX_RectF m_rtRelative;
  CFWL_SysBtn* m_pCloseBox;
  CFWL_SysBtn* m_pMinBox;
  CFWL_SysBtn* m_pMaxBox;
  std::unique_ptr<CFWL_NoteLoop> m_pNoteLoop;
  CFWL_Widget* m_pSubFocus;
  FX_FLOAT m_fCXBorder;
  FX_FLOAT m_fCYBorder;
  int32_t m_iCaptureBtn;
  int32_t m_iSysBox;
  bool m_bLButtonDown;
  bool m_bMaximized;
  bool m_bSetMaximize;
  bool m_bDoModalFlag;
};

#endif  // XFA_FWL_CFWL_FORM_H_
