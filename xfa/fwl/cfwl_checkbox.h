// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_CHECKBOX_H_
#define XFA_FWL_CFWL_CHECKBOX_H_

#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_widget.h"

#define FWL_STYLEEXT_CKB_3State (1L << 6)
#define FWL_STYLEEXT_CKB_RadioButton (1L << 7)
#define FWL_STYLEEXT_CKB_SignShapeCheck 0
#define FWL_STYLEEXT_CKB_SignShapeCircle (1L << 10)
#define FWL_STYLEEXT_CKB_SignShapeCross (2L << 10)
#define FWL_STYLEEXT_CKB_SignShapeDiamond (3L << 10)
#define FWL_STYLEEXT_CKB_SignShapeSquare (4L << 10)
#define FWL_STYLEEXT_CKB_SignShapeStar (5L << 10)
#define FWL_STYLEEXT_CKB_SignShapeMask (7L << 10)
#define FWL_STATE_CKB_Hovered (1 << FWL_STATE_WGT_MAX)
#define FWL_STATE_CKB_Pressed (1 << (FWL_STATE_WGT_MAX + 1))
#define FWL_STATE_CKB_Unchecked 0
#define FWL_STATE_CKB_Checked (1 << (FWL_STATE_WGT_MAX + 2))
#define FWL_STATE_CKB_Neutral (2 << (FWL_STATE_WGT_MAX + 2))
#define FWL_STATE_CKB_CheckMask (3L << (FWL_STATE_WGT_MAX + 2))

class CFWL_MessageKey;
class CFWL_MessageMouse;

class CFWL_CheckBox final : public CFWL_Widget {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_CheckBox() override;

  // CFWL_Widget
  FWL_Type GetClassID() const override;
  void Update() override;
  void DrawWidget(CFGAS_GEGraphics* pGraphics,
                  const CFX_Matrix& matrix) override;

  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;

  void SetBoxSize(float fHeight);

 private:
  explicit CFWL_CheckBox(CFWL_App* pApp);

  void SetCheckState(int32_t iCheck);
  void Layout();
  uint32_t GetPartStates() const;
  void UpdateTextOutStyles();
  void NextStates();
  void OnFocusGained();
  void OnFocusLost();
  void OnLButtonDown();
  void OnLButtonUp(CFWL_MessageMouse* pMsg);
  void OnMouseMove(CFWL_MessageMouse* pMsg);
  void OnMouseLeave();
  void OnKeyDown(CFWL_MessageKey* pMsg);

  CFX_RectF m_ClientRect;
  CFX_RectF m_BoxRect;
  CFX_RectF m_CaptionRect;
  CFX_RectF m_FocusRect;
  FDE_TextStyle m_TTOStyles;
  FDE_TextAlignment m_iTTOAlign = FDE_TextAlignment::kCenter;
  bool m_bBtnDown = false;
  float m_fBoxHeight = 16.0f;
};

#endif  // XFA_FWL_CFWL_CHECKBOX_H_
