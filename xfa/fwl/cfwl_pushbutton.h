// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_PUSHBUTTON_H_
#define XFA_FWL_CFWL_PUSHBUTTON_H_

#include "xfa/fwl/cfwl_widget.h"

#define FWL_STATE_PSB_Hovered (1 << FWL_STATE_WGT_MAX)
#define FWL_STATE_PSB_Pressed (1 << (FWL_STATE_WGT_MAX + 1))
#define FWL_STATE_PSB_Default (1 << (FWL_STATE_WGT_MAX + 2))

class CFWL_MessageKey;
class CFWL_MessageMouse;

class CFWL_PushButton final : public CFWL_Widget {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_PushButton() override;

  // CFWL_Widget
  FWL_Type GetClassID() const override;
  void SetStates(uint32_t dwStates) override;
  void Update() override;
  void DrawWidget(CFGAS_GEGraphics* pGraphics,
                  const CFX_Matrix& matrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;

 private:
  explicit CFWL_PushButton(CFWL_App* pApp);

  void DrawBkground(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  Mask<CFWL_PartState> GetPartStates();
  void UpdateTextOutStyles();
  void OnFocusGained();
  void OnFocusLost();
  void OnLButtonDown(CFWL_MessageMouse* pMsg);
  void OnLButtonUp(CFWL_MessageMouse* pMsg);
  void OnMouseMove(CFWL_MessageMouse* pMsg);
  void OnMouseLeave(CFWL_MessageMouse* pMsg);
  void OnKeyDown(CFWL_MessageKey* pMsg);

  bool m_bBtnDown = false;
  CFX_RectF m_ClientRect;
  CFX_RectF m_CaptionRect;
};

#endif  // XFA_FWL_CFWL_PUSHBUTTON_H_
