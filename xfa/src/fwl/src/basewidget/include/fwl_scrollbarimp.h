// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_SCROLLBAR_IMP_H
#define _FWL_SCROLLBAR_IMP_H
class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class IFWL_Widget;
class IFWL_Timer;
class IFWL_TimerDelegate;
class CFWL_ScrollBarImp;
class CFWL_ScrollBarImpDelegate;
class CFWL_ScrollBarImp : public CFWL_WidgetImp, public IFWL_Timer {
 public:
  CFWL_ScrollBarImp(const CFWL_WidgetImpProperties& properties,
                    IFWL_Widget* pOuter);
  ~CFWL_ScrollBarImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR Update();
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL IsVertical();
  virtual FWL_ERR GetRange(FX_FLOAT& fMin, FX_FLOAT& fMax);
  virtual FWL_ERR SetRange(FX_FLOAT fMin, FX_FLOAT fMax);
  virtual FX_FLOAT GetPageSize();
  virtual FWL_ERR SetPageSize(FX_FLOAT fPageSize);
  virtual FX_FLOAT GetStepSize();
  virtual FWL_ERR SetStepSize(FX_FLOAT fStepSize);
  virtual FX_FLOAT GetPos();
  virtual FWL_ERR SetPos(FX_FLOAT fPos);
  virtual FX_FLOAT GetTrackPos();
  virtual FWL_ERR SetTrackPos(FX_FLOAT fTrackPos);
  virtual FX_BOOL DoScroll(FX_DWORD dwCode, FX_FLOAT fPos = 0.0f);
  virtual FWL_ERR SetOuter(IFWL_Widget* pOuter);
  virtual int32_t Run(FWL_HTIMER hTimer);

 protected:
  void DrawTrack(CFX_Graphics* pGraphics,
                 IFWL_ThemeProvider* pTheme,
                 FX_BOOL bLower = TRUE,
                 const CFX_Matrix* pMatrix = NULL);
  void DrawArrowBtn(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    FX_BOOL bMinBtn = TRUE,
                    const CFX_Matrix* pMatrix = NULL);
  void DrawThumb(CFX_Graphics* pGraphics,
                 IFWL_ThemeProvider* pTheme,
                 const CFX_Matrix* pMatrix = NULL);
  void Layout();
  void CalcButtonLen();
  void CalcMinButtonRect(CFX_RectF& rect);
  void CalcMaxButtonRect(CFX_RectF& rect);
  void CalcThumbButtonRect(CFX_RectF& rect);
  void CalcMinTrackRect(CFX_RectF& rect);
  void CalcMaxTrackRect(CFX_RectF& rect);
  FX_FLOAT GetTrackPointPos(FX_FLOAT fx, FX_FLOAT fy);
  void GetTrackRect(CFX_RectF& rect, FX_BOOL bLower = TRUE);
  FX_BOOL SendEvent();
  FX_BOOL OnScroll(FX_DWORD dwCode, FX_FLOAT fPos);

  FWL_HTIMER m_hTimer;
  FX_FLOAT m_fRangeMin;
  FX_FLOAT m_fRangeMax;
  FX_FLOAT m_fPageSize;
  FX_FLOAT m_fStepSize;
  FX_FLOAT m_fPos;
  FX_FLOAT m_fTrackPos;
  int32_t m_iMinButtonState;
  int32_t m_iMaxButtonState;
  int32_t m_iThumbButtonState;
  int32_t m_iMinTrackState;
  int32_t m_iMaxTrackState;
  FX_FLOAT m_fLastTrackPos;
  FX_FLOAT m_cpTrackPointX;
  FX_FLOAT m_cpTrackPointY;
  int32_t m_iMouseWheel;
  FX_BOOL m_bTrackMouseLeave;
  FX_BOOL m_bMouseHover;
  FX_BOOL m_bMouseDown;
  FX_BOOL m_bRepaintThumb;
  FX_FLOAT m_fButtonLen;
  FX_BOOL m_bMinSize;
  CFX_RectF m_rtClient;
  CFX_RectF m_rtThumb;
  CFX_RectF m_rtMinBtn;
  CFX_RectF m_rtMaxBtn;
  CFX_RectF m_rtMinTrack;
  CFX_RectF m_rtMaxTrack;
  FX_BOOL m_bCustomLayout;
  FX_FLOAT m_fMinThumb;
  friend class CFWL_ScrollBarImpDelegate;
};
class CFWL_ScrollBarImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_ScrollBarImpDelegate(CFWL_ScrollBarImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;

 protected:
  void OnLButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  void OnLButtonUp(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  void OnMouseMove(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  void OnMouseLeave();
  void OnMouseWheel(FX_FLOAT fx,
                    FX_FLOAT fy,
                    FX_DWORD dwFlags,
                    FX_FLOAT fDeltaX,
                    FX_FLOAT fDeltaY);
  void DoMouseDown(int32_t iItem,
                   const CFX_RectF& rtItem,
                   int32_t& iState,
                   FX_FLOAT fx,
                   FX_FLOAT fy);
  void DoMouseUp(int32_t iItem,
                 const CFX_RectF& rtItem,
                 int32_t& iState,
                 FX_FLOAT fx,
                 FX_FLOAT fy);
  void DoMouseMove(int32_t iItem,
                   const CFX_RectF& rtItem,
                   int32_t& iState,
                   FX_FLOAT fx,
                   FX_FLOAT fy);
  void DoMouseLeave(int32_t iItem, const CFX_RectF& rtItem, int32_t& iState);
  void DoMouseHover(int32_t iItem, const CFX_RectF& rtItem, int32_t& iState);

  CFWL_ScrollBarImp* m_pOwner;
};
#endif
