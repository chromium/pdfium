// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFFIELD_H_
#define XFA_FXFA_CXFA_FFFIELD_H_

#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/ifwl_widgetdelegate.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"

class CXFA_FFDropDown;
class CXFA_Node;

class CXFA_FFField : public CXFA_FFWidget, public IFWL_WidgetDelegate {
 public:
  enum ShapeOption { kSquareShape = 0, kRoundShape };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFField() override;

  virtual CXFA_FFDropDown* AsDropDown();

  // CXFA_FFWidget:
  void Trace(cppgc::Visitor* visitor) const override;
  CXFA_FFField* AsField() override;
  CFX_RectF GetBBox(FocusOption focus) override;
  void RenderWidget(CFGAS_GEGraphics* pGS,
                    const CFX_Matrix& matrix,
                    HighlightOption highlight) override;
  bool IsLoaded() override;
  bool LoadWidget() override;
  bool PerformLayout() override;
  bool AcceptsFocusOnButtonDown(
      Mask<XFA_FWL_KeyFlag> dwFlags,
      const CFX_PointF& point,
      CFWL_MessageMouse::MouseCommand command) override;
  bool OnMouseEnter() override;
  bool OnMouseExit() override;
  bool OnLButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                     const CFX_PointF& point) override;
  bool OnLButtonUp(Mask<XFA_FWL_KeyFlag> dwFlags,
                   const CFX_PointF& point) override;
  bool OnLButtonDblClk(Mask<XFA_FWL_KeyFlag> dwFlags,
                       const CFX_PointF& point) override;
  bool OnMouseMove(Mask<XFA_FWL_KeyFlag> dwFlags,
                   const CFX_PointF& point) override;
  bool OnMouseWheel(Mask<XFA_FWL_KeyFlag> dwFlags,
                    const CFX_PointF& point,
                    const CFX_Vector& delta) override;
  bool OnRButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                     const CFX_PointF& point) override;
  bool OnRButtonUp(Mask<XFA_FWL_KeyFlag> dwFlags,
                   const CFX_PointF& point) override;
  bool OnRButtonDblClk(Mask<XFA_FWL_KeyFlag> dwFlags,
                       const CFX_PointF& point) override;
  bool OnSetFocus(CXFA_FFWidget* pOldWidget) override WARN_UNUSED_RESULT;
  bool OnKillFocus(CXFA_FFWidget* pNewWidget) override WARN_UNUSED_RESULT;
  bool OnKeyDown(XFA_FWL_VKEYCODE dwKeyCode,
                 Mask<XFA_FWL_KeyFlag> dwFlags) override;
  bool OnKeyUp(XFA_FWL_VKEYCODE dwKeyCode,
               Mask<XFA_FWL_KeyFlag> dwFlags) override;
  bool OnChar(uint32_t dwChar, Mask<XFA_FWL_KeyFlag> dwFlags) override;
  FWL_WidgetHit HitTest(const CFX_PointF& point) override;

  // IFWL_WidgetDelegate:
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;

  void UpdateFWL();
  uint32_t UpdateUIProperty();

 protected:
  explicit CXFA_FFField(CXFA_Node* pNode);

  bool PtInActiveRect(const CFX_PointF& point) override;

  virtual void SetFWLRect();
  virtual bool CommitData();
  virtual bool IsDataChanged();

  CFWL_Widget* GetNormalWidget();
  const CFWL_Widget* GetNormalWidget() const;
  void SetNormalWidget(CFWL_Widget* widget);
  CFX_PointF FWLToClient(const CFX_PointF& point);
  void LayoutCaption();
  void RenderCaption(CFGAS_GEGraphics* pGS, const CFX_Matrix& pMatrix);

  int32_t CalculateOverride();
  int32_t CalculateNode(CXFA_Node* pNode);
  bool ProcessCommittedData();
  void DrawHighlight(CFGAS_GEGraphics* pGS,
                     const CFX_Matrix& pMatrix,
                     HighlightOption highlight,
                     ShapeOption shape);
  void SendMessageToFWLWidget(CFWL_Message* pMessage);
  void CapPlacement();
  void CapTopBottomPlacement(const CXFA_Margin* margin,
                             const CFX_RectF& rtWidget,
                             XFA_AttributeValue iCapPlacement);
  void CapLeftRightPlacement(const CXFA_Margin* margin,
                             const CFX_RectF& rtWidget,
                             XFA_AttributeValue iCapPlacement);
  void SetEditScrollOffset();

  CFX_RectF m_UIRect;
  CFX_RectF m_CaptionRect;

 private:
  cppgc::Member<CFWL_Widget> m_pNormalWidget;
};

inline CXFA_FFDropDown* ToDropDown(CXFA_FFField* field) {
  return field ? field->AsDropDown() : nullptr;
}

#endif  // XFA_FXFA_CXFA_FFFIELD_H_
