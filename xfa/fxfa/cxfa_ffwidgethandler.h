// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFWIDGETHANDLER_H_
#define XFA_FXFA_CXFA_FFWIDGETHANDLER_H_

#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"

class CXFA_FFDocView;
class CXFA_Graphics;
enum class FWL_WidgetHit;

class CXFA_FFWidgetHandler {
 public:
  explicit CXFA_FFWidgetHandler(CXFA_FFDocView* pDocView);
  ~CXFA_FFWidgetHandler();

  bool OnMouseEnter(CXFA_FFWidget* hWidget);
  bool OnMouseExit(CXFA_FFWidget* hWidget);
  bool OnLButtonDown(CXFA_FFWidget* hWidget,
                     uint32_t dwFlags,
                     const CFX_PointF& point);
  bool OnLButtonUp(CXFA_FFWidget* hWidget,
                   uint32_t dwFlags,
                   const CFX_PointF& point);
  bool OnLButtonDblClk(CXFA_FFWidget* hWidget,
                       uint32_t dwFlags,
                       const CFX_PointF& point);
  bool OnMouseMove(CXFA_FFWidget* hWidget,
                   uint32_t dwFlags,
                   const CFX_PointF& point);
  bool OnMouseWheel(CXFA_FFWidget* hWidget,
                    uint32_t dwFlags,
                    int16_t zDelta,
                    const CFX_PointF& point);
  bool OnRButtonDown(CXFA_FFWidget* hWidget,
                     uint32_t dwFlags,
                     const CFX_PointF& point);
  bool OnRButtonUp(CXFA_FFWidget* hWidget,
                   uint32_t dwFlags,
                   const CFX_PointF& point);
  bool OnRButtonDblClk(CXFA_FFWidget* hWidget,
                       uint32_t dwFlags,
                       const CFX_PointF& point);

  WideString GetText(CXFA_FFWidget* widget);
  WideString GetSelectedText(CXFA_FFWidget* widget);
  void PasteText(CXFA_FFWidget* widget, const WideString& text);

  bool CanUndo(CXFA_FFWidget* widget);
  bool CanRedo(CXFA_FFWidget* widget);
  bool Undo(CXFA_FFWidget* widget);
  bool Redo(CXFA_FFWidget* widget);

  bool OnKeyDown(CXFA_FFWidget* hWidget, uint32_t dwKeyCode, uint32_t dwFlags);
  bool OnKeyUp(CXFA_FFWidget* hWidget, uint32_t dwKeyCode, uint32_t dwFlags);
  bool OnChar(CXFA_FFWidget* hWidget, uint32_t dwChar, uint32_t dwFlags);
  FWL_WidgetHit HitTest(CXFA_FFWidget* pWidget, const CFX_PointF& point);
  void RenderWidget(CXFA_FFWidget* hWidget,
                    CXFA_Graphics* pGS,
                    const CFX_Matrix& matrix,
                    bool bHighlight);
  bool HasEvent(CXFA_Node* pNode, XFA_EVENTTYPE eEventType);
  XFA_EventError ProcessEvent(CXFA_Node* pNode, CXFA_EventParam* pParam);

 private:
  UnownedPtr<CXFA_FFDocView> m_pDocView;
};

#endif  //  XFA_FXFA_CXFA_FFWIDGETHANDLER_H_
