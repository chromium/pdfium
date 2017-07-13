// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_ffline.h"

#include "xfa/fxgraphics/cxfa_color.h"
#include "xfa/fxgraphics/cxfa_graphics.h"
#include "xfa/fxgraphics/cxfa_path.h"

CXFA_FFLine::CXFA_FFLine(CXFA_WidgetAcc* pDataAcc) : CXFA_FFDraw(pDataAcc) {}

CXFA_FFLine::~CXFA_FFLine() {}

void CXFA_FFLine::GetRectFromHand(CFX_RectF& rect,
                                  int32_t iHand,
                                  float fLineWidth) {
  float fHalfWidth = fLineWidth / 2.0f;
  if (rect.height < 1.0f) {
    switch (iHand) {
      case XFA_ATTRIBUTEENUM_Left:
        rect.top -= fHalfWidth;
        break;
      case XFA_ATTRIBUTEENUM_Right:
        rect.top += fHalfWidth;
    }
  } else if (rect.width < 1.0f) {
    switch (iHand) {
      case XFA_ATTRIBUTEENUM_Left:
        rect.left += fHalfWidth;
        break;
      case XFA_ATTRIBUTEENUM_Right:
        rect.left += fHalfWidth;
        break;
    }
  } else {
    switch (iHand) {
      case XFA_ATTRIBUTEENUM_Left:
        rect.Inflate(fHalfWidth, fHalfWidth);
        break;
      case XFA_ATTRIBUTEENUM_Right:
        rect.Deflate(fHalfWidth, fHalfWidth);
        break;
    }
  }
}

void CXFA_FFLine::RenderWidget(CXFA_Graphics* pGS,
                               CFX_Matrix* pMatrix,
                               uint32_t dwStatus) {
  if (!IsMatchVisibleStatus(dwStatus))
    return;

  CXFA_Value value = m_pDataAcc->GetFormValue();
  if (!value)
    return;

  CXFA_Line lineObj = value.GetLine();
  FX_ARGB lineColor = 0xFF000000;
  int32_t iStrokeType = 0;
  float fLineWidth = 1.0f;
  int32_t iCap = 0;
  CXFA_Edge edge = lineObj.GetEdge();
  if (edge) {
    if (edge.GetPresence() != XFA_ATTRIBUTEENUM_Visible)
      return;

    lineColor = edge.GetColor();
    iStrokeType = edge.GetStrokeType();
    fLineWidth = edge.GetThickness();
    iCap = edge.GetCapType();
  }

  CFX_Matrix mtRotate = GetRotateMatrix();
  if (pMatrix)
    mtRotate.Concat(*pMatrix);

  CFX_RectF rtLine = GetRectWithoutRotate();
  if (CXFA_Margin mgWidget = m_pDataAcc->GetMargin())
    XFA_RectWidthoutMargin(rtLine, mgWidget);

  GetRectFromHand(rtLine, lineObj.GetHand(), fLineWidth);
  CXFA_Path linePath;
  if (lineObj.GetSlope() && rtLine.right() > 0.0f && rtLine.bottom() > 0.0f)
    linePath.AddLine(rtLine.TopRight(), rtLine.BottomLeft());
  else
    linePath.AddLine(rtLine.TopLeft(), rtLine.BottomRight());

  CXFA_Color color(lineColor);
  pGS->SaveGraphState();
  pGS->SetLineWidth(fLineWidth, true);
  XFA_StrokeTypeSetLineDash(pGS, iStrokeType, iCap);
  pGS->SetStrokeColor(&color);
  pGS->SetLineCap(XFA_LineCapToFXGE(iCap));
  pGS->StrokePath(&linePath, &mtRotate);
  pGS->RestoreGraphState();
}
