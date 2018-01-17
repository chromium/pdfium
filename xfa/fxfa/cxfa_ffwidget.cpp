// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffwidget.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/page/cpdf_pageobjectholder.h"
#include "core/fxcodec/codec/ccodec_progressivedecoder.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/maybe_owned.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/dib/cfx_imagerenderer.h"
#include "xfa/fwl/fwl_widgethit.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_imagerenderer.h"
#include "xfa/fxfa/cxfa_textlayout.h"
#include "xfa/fxfa/cxfa_widgetacc.h"
#include "xfa/fxfa/parser/cxfa_border.h"
#include "xfa/fxfa/parser/cxfa_box.h"
#include "xfa/fxfa/parser/cxfa_corner.h"
#include "xfa/fxfa/parser/cxfa_edge.h"
#include "xfa/fxfa/parser/cxfa_fill.h"
#include "xfa/fxfa/parser/cxfa_image.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxgraphics/cxfa_gecolor.h"
#include "xfa/fxgraphics/cxfa_gepath.h"
#include "xfa/fxgraphics/cxfa_gepattern.h"
#include "xfa/fxgraphics/cxfa_geshading.h"
#include "xfa/fxgraphics/cxfa_graphics.h"

namespace {

void XFA_BOX_GetPath_Arc(CXFA_Box* box,
                         CFX_RectF rtDraw,
                         CXFA_GEPath& fillPath,
                         bool forceRound) {
  float a, b;
  a = rtDraw.width / 2.0f;
  b = rtDraw.height / 2.0f;
  if (box->IsCircular() || forceRound)
    a = b = std::min(a, b);

  CFX_PointF center = rtDraw.Center();
  rtDraw.left = center.x - a;
  rtDraw.top = center.y - b;
  rtDraw.width = a + a;
  rtDraw.height = b + b;
  Optional<int32_t> startAngle = box->GetStartAngle();
  Optional<int32_t> sweepAngle = box->GetSweepAngle();
  if (!startAngle && !sweepAngle) {
    fillPath.AddEllipse(rtDraw);
    return;
  }

  fillPath.AddArc(rtDraw.TopLeft(), rtDraw.Size(),
                  -startAngle.value_or(0) * FX_PI / 180.0f,
                  -sweepAngle.value_or(360) * FX_PI / 180.0f);
}

void XFA_BOX_GetPath(const std::vector<CXFA_Stroke*>& strokes,
                     CFX_RectF rtWidget,
                     CXFA_GEPath& path,
                     int32_t nIndex,
                     bool bStart,
                     bool bCorner) {
  ASSERT(nIndex >= 0 && nIndex < 8);

  int32_t n = (nIndex & 1) ? nIndex - 1 : nIndex;
  auto* corner1 = static_cast<CXFA_Corner*>(strokes[n]);
  auto* corner2 = static_cast<CXFA_Corner*>(strokes[(n + 2) % 8]);
  float fRadius1 = bCorner ? corner1->GetRadius() : 0.0f;
  float fRadius2 = bCorner ? corner2->GetRadius() : 0.0f;
  bool bInverted = corner1->IsInverted();
  float offsetY = 0.0f;
  float offsetX = 0.0f;
  bool bRound = corner1->GetJoinType() == XFA_AttributeEnum::Round;
  float halfAfter = 0.0f;
  float halfBefore = 0.0f;

  CXFA_Stroke* stroke = strokes[nIndex];
  if (stroke->IsCorner()) {
    CXFA_Stroke* strokeBefore = strokes[(nIndex + 1 * 8 - 1) % 8];
    CXFA_Stroke* strokeAfter = strokes[nIndex + 1];
    if (stroke->IsInverted()) {
      if (!stroke->SameStyles(strokeBefore, 0))
        halfBefore = strokeBefore->GetThickness() / 2;
      if (!stroke->SameStyles(strokeAfter, 0))
        halfAfter = strokeAfter->GetThickness() / 2;
    }
  } else {
    CXFA_Stroke* strokeBefore = strokes[(nIndex + 8 - 2) % 8];
    CXFA_Stroke* strokeAfter = strokes[(nIndex + 2) % 8];
    if (!bRound && !bInverted) {
      halfBefore = strokeBefore->GetThickness() / 2;
      halfAfter = strokeAfter->GetThickness() / 2;
    }
  }

  float offsetEX = 0.0f;
  float offsetEY = 0.0f;
  float sx = 0.0f;
  float sy = 0.0f;
  float vx = 1.0f;
  float vy = 1.0f;
  float nx = 1.0f;
  float ny = 1.0f;
  CFX_PointF cpStart;
  CFX_PointF cp1;
  CFX_PointF cp2;
  if (bRound) {
    sy = FX_PI / 2;
  }
  switch (nIndex) {
    case 0:
    case 1:
      cp1 = rtWidget.TopLeft();
      cp2 = rtWidget.TopRight();
      if (nIndex == 0) {
        cpStart.x = cp1.x - halfBefore;
        cpStart.y = cp1.y + fRadius1, offsetY = -halfAfter;
      } else {
        cpStart.x = cp1.x + fRadius1 - halfBefore, cpStart.y = cp1.y,
        offsetEX = halfAfter;
      }
      vx = 1, vy = 1;
      nx = -1, ny = 0;
      if (bRound) {
        sx = bInverted ? FX_PI / 2 : FX_PI;
      } else {
        sx = 1, sy = 0;
      }
      break;
    case 2:
    case 3:
      cp1 = rtWidget.TopRight();
      cp2 = rtWidget.BottomRight();
      if (nIndex == 2) {
        cpStart.x = cp1.x - fRadius1, cpStart.y = cp1.y - halfBefore,
        offsetX = halfAfter;
      } else {
        cpStart.x = cp1.x, cpStart.y = cp1.y + fRadius1 - halfBefore,
        offsetEY = halfAfter;
      }
      vx = -1, vy = 1;
      nx = 0, ny = -1;
      if (bRound) {
        sx = bInverted ? FX_PI : FX_PI * 3 / 2;
      } else {
        sx = 0, sy = 1;
      }
      break;
    case 4:
    case 5:
      cp1 = rtWidget.BottomRight();
      cp2 = rtWidget.BottomLeft();
      if (nIndex == 4) {
        cpStart.x = cp1.x + halfBefore, cpStart.y = cp1.y - fRadius1,
        offsetY = halfAfter;
      } else {
        cpStart.x = cp1.x - fRadius1 + halfBefore, cpStart.y = cp1.y,
        offsetEX = -halfAfter;
      }
      vx = -1, vy = -1;
      nx = 1, ny = 0;
      if (bRound) {
        sx = bInverted ? FX_PI * 3 / 2 : 0;
      } else {
        sx = -1, sy = 0;
      }
      break;
    case 6:
    case 7:
      cp1 = rtWidget.BottomLeft();
      cp2 = rtWidget.TopLeft();
      if (nIndex == 6) {
        cpStart.x = cp1.x + fRadius1, cpStart.y = cp1.y + halfBefore,
        offsetX = -halfAfter;
      } else {
        cpStart.x = cp1.x, cpStart.y = cp1.y - fRadius1 + halfBefore,
        offsetEY = -halfAfter;
      }
      vx = 1;
      vy = -1;
      nx = 0;
      ny = 1;
      if (bRound) {
        sx = bInverted ? 0 : FX_PI / 2;
      } else {
        sx = 0;
        sy = -1;
      }
      break;
  }
  if (bStart) {
    path.MoveTo(cpStart);
  }
  if (nIndex & 1) {
    path.LineTo(CFX_PointF(cp2.x + fRadius2 * nx + offsetEX,
                           cp2.y + fRadius2 * ny + offsetEY));
    return;
  }
  if (bRound) {
    if (fRadius1 < 0)
      sx -= FX_PI;
    if (bInverted)
      sy *= -1;

    CFX_RectF rtRadius(cp1.x + offsetX * 2, cp1.y + offsetY * 2,
                       fRadius1 * 2 * vx - offsetX * 2,
                       fRadius1 * 2 * vy - offsetY * 2);
    rtRadius.Normalize();
    if (bInverted)
      rtRadius.Offset(-fRadius1 * vx, -fRadius1 * vy);

    path.ArcTo(rtRadius.TopLeft(), rtRadius.Size(), sx, sy);
  } else {
    CFX_PointF cp;
    if (bInverted) {
      cp.x = cp1.x + fRadius1 * vx;
      cp.y = cp1.y + fRadius1 * vy;
    } else {
      cp = cp1;
    }
    path.LineTo(cp);
    path.LineTo(CFX_PointF(cp1.x + fRadius1 * sx + offsetX,
                           cp1.y + fRadius1 * sy + offsetY));
  }
}

void XFA_BOX_GetFillPath(CXFA_Box* box,
                         const std::vector<CXFA_Stroke*>& strokes,
                         CFX_RectF rtWidget,
                         CXFA_GEPath& fillPath,
                         bool forceRound) {
  if (box->IsArc() || forceRound) {
    CXFA_Edge* edge = box->GetEdgeIfExists(0);
    float fThickness = std::fmax(0.0, edge ? edge->GetThickness() : 0);
    float fHalf = fThickness / 2;
    XFA_AttributeEnum iHand = box->GetHand();
    if (iHand == XFA_AttributeEnum::Left)
      rtWidget.Inflate(fHalf, fHalf);
    else if (iHand == XFA_AttributeEnum::Right)
      rtWidget.Deflate(fHalf, fHalf);

    XFA_BOX_GetPath_Arc(box, rtWidget, fillPath, forceRound);
    return;
  }

  bool bSameStyles = true;
  CXFA_Stroke* stroke1 = strokes[0];
  for (int32_t i = 1; i < 8; i++) {
    CXFA_Stroke* stroke2 = strokes[i];
    if (!stroke1->SameStyles(stroke2, 0)) {
      bSameStyles = false;
      break;
    }
    stroke1 = stroke2;
  }
  if (bSameStyles) {
    stroke1 = strokes[0];
    for (int32_t i = 2; i < 8; i += 2) {
      CXFA_Stroke* stroke2 = strokes[i];
      if (!stroke1->SameStyles(stroke2, XFA_STROKE_SAMESTYLE_NoPresence |
                                            XFA_STROKE_SAMESTYLE_Corner)) {
        bSameStyles = false;
        break;
      }
      stroke1 = stroke2;
    }
    if (bSameStyles) {
      stroke1 = strokes[0];
      if (stroke1->IsInverted())
        bSameStyles = false;
      if (stroke1->GetJoinType() != XFA_AttributeEnum::Square)
        bSameStyles = false;
    }
  }
  if (bSameStyles) {
    fillPath.AddRectangle(rtWidget.left, rtWidget.top, rtWidget.width,
                          rtWidget.height);
    return;
  }

  for (int32_t i = 0; i < 8; i += 2) {
    float sx = 0.0f;
    float sy = 0.0f;
    float vx = 1.0f;
    float vy = 1.0f;
    float nx = 1.0f;
    float ny = 1.0f;
    CFX_PointF cp1, cp2;
    auto* corner1 = static_cast<CXFA_Corner*>(strokes[i]);
    auto* corner2 = static_cast<CXFA_Corner*>(strokes[(i + 2) % 8]);
    float fRadius1 = corner1->GetRadius();
    float fRadius2 = corner2->GetRadius();
    bool bInverted = corner1->IsInverted();
    bool bRound = corner1->GetJoinType() == XFA_AttributeEnum::Round;
    if (bRound) {
      sy = FX_PI / 2;
    }
    switch (i) {
      case 0:
        cp1 = rtWidget.TopLeft();
        cp2 = rtWidget.TopRight();
        vx = 1, vy = 1;
        nx = -1, ny = 0;
        if (bRound) {
          sx = bInverted ? FX_PI / 2 : FX_PI;
        } else {
          sx = 1, sy = 0;
        }
        break;
      case 2:
        cp1 = rtWidget.TopRight();
        cp2 = rtWidget.BottomRight();
        vx = -1, vy = 1;
        nx = 0, ny = -1;
        if (bRound) {
          sx = bInverted ? FX_PI : FX_PI * 3 / 2;
        } else {
          sx = 0, sy = 1;
        }
        break;
      case 4:
        cp1 = rtWidget.BottomRight();
        cp2 = rtWidget.BottomLeft();
        vx = -1, vy = -1;
        nx = 1, ny = 0;
        if (bRound) {
          sx = bInverted ? FX_PI * 3 / 2 : 0;
        } else {
          sx = -1, sy = 0;
        }
        break;
      case 6:
        cp1 = rtWidget.BottomLeft();
        cp2 = rtWidget.TopLeft();
        vx = 1, vy = -1;
        nx = 0, ny = 1;
        if (bRound) {
          sx = bInverted ? 0 : FX_PI / 2;
        } else {
          sx = 0;
          sy = -1;
        }
        break;
    }
    if (i == 0)
      fillPath.MoveTo(CFX_PointF(cp1.x, cp1.y + fRadius1));

    if (bRound) {
      if (fRadius1 < 0)
        sx -= FX_PI;
      if (bInverted)
        sy *= -1;

      CFX_RectF rtRadius(cp1.x, cp1.y, fRadius1 * 2 * vx, fRadius1 * 2 * vy);
      rtRadius.Normalize();
      if (bInverted)
        rtRadius.Offset(-fRadius1 * vx, -fRadius1 * vy);

      fillPath.ArcTo(rtRadius.TopLeft(), rtRadius.Size(), sx, sy);
    } else {
      CFX_PointF cp;
      if (bInverted) {
        cp.x = cp1.x + fRadius1 * vx;
        cp.y = cp1.y + fRadius1 * vy;
      } else {
        cp = cp1;
      }
      fillPath.LineTo(cp);
      fillPath.LineTo(CFX_PointF(cp1.x + fRadius1 * sx, cp1.y + fRadius1 * sy));
    }
    fillPath.LineTo(CFX_PointF(cp2.x + fRadius2 * nx, cp2.y + fRadius2 * ny));
  }
}

void XFA_BOX_Fill_Radial(CXFA_Fill* fill,
                         CXFA_Graphics* pGS,
                         CXFA_GEPath& fillPath,
                         CFX_RectF rtFill,
                         const CFX_Matrix& matrix) {
  ASSERT(fill);

  FX_ARGB crStart = fill->GetColor(false);
  FX_ARGB crEnd = fill->GetRadialColor();
  if (!fill->IsRadialToEdge())
    std::swap(crStart, crEnd);

  CXFA_GEShading shading(rtFill.Center(), rtFill.Center(), 0,
                         sqrt(rtFill.Width() * rtFill.Width() +
                              rtFill.Height() * rtFill.Height()) /
                             2,
                         true, true, crStart, crEnd);
  pGS->SetFillColor(CXFA_GEColor(&shading));
  pGS->FillPath(&fillPath, FXFILL_WINDING, &matrix);
}

void XFA_BOX_Fill_Pattern(CXFA_Fill* fill,
                          CXFA_Graphics* pGS,
                          CXFA_GEPath& fillPath,
                          CFX_RectF rtFill,
                          const CFX_Matrix& matrix) {
  ASSERT(fill);

  FX_ARGB crStart = fill->GetColor(false);
  FX_ARGB crEnd = fill->GetPatternColor();
  FX_HatchStyle iHatch = FX_HatchStyle::Cross;
  switch (fill->GetPatternType()) {
    case XFA_AttributeEnum::CrossDiagonal:
      iHatch = FX_HatchStyle::DiagonalCross;
      break;
    case XFA_AttributeEnum::DiagonalLeft:
      iHatch = FX_HatchStyle::ForwardDiagonal;
      break;
    case XFA_AttributeEnum::DiagonalRight:
      iHatch = FX_HatchStyle::BackwardDiagonal;
      break;
    case XFA_AttributeEnum::Horizontal:
      iHatch = FX_HatchStyle::Horizontal;
      break;
    case XFA_AttributeEnum::Vertical:
      iHatch = FX_HatchStyle::Vertical;
      break;
    default:
      break;
  }

  CXFA_GEPattern pattern(iHatch, crEnd, crStart);
  pGS->SetFillColor(CXFA_GEColor(&pattern, 0x0));
  pGS->FillPath(&fillPath, FXFILL_WINDING, &matrix);
}

void XFA_BOX_Fill_Linear(CXFA_Fill* fill,
                         CXFA_Graphics* pGS,
                         CXFA_GEPath& fillPath,
                         CFX_RectF rtFill,
                         const CFX_Matrix& matrix) {
  ASSERT(fill);

  FX_ARGB crStart = fill->GetColor(false);
  FX_ARGB crEnd = fill->GetLinearColor();

  CFX_PointF ptStart;
  CFX_PointF ptEnd;
  switch (fill->GetLinearType()) {
    case XFA_AttributeEnum::ToRight:
      ptStart = CFX_PointF(rtFill.left, rtFill.top);
      ptEnd = CFX_PointF(rtFill.right(), rtFill.top);
      break;
    case XFA_AttributeEnum::ToBottom:
      ptStart = CFX_PointF(rtFill.left, rtFill.top);
      ptEnd = CFX_PointF(rtFill.left, rtFill.bottom());
      break;
    case XFA_AttributeEnum::ToLeft:
      ptStart = CFX_PointF(rtFill.right(), rtFill.top);
      ptEnd = CFX_PointF(rtFill.left, rtFill.top);
      break;
    case XFA_AttributeEnum::ToTop:
      ptStart = CFX_PointF(rtFill.left, rtFill.bottom());
      ptEnd = CFX_PointF(rtFill.left, rtFill.top);
      break;
    default:
      break;
  }

  CXFA_GEShading shading(ptStart, ptEnd, false, false, crStart, crEnd);
  pGS->SetFillColor(CXFA_GEColor(&shading));
  pGS->FillPath(&fillPath, FXFILL_WINDING, &matrix);
}

void XFA_BOX_Fill(CXFA_Box* box,
                  const std::vector<CXFA_Stroke*>& strokes,
                  CXFA_Graphics* pGS,
                  const CFX_RectF& rtWidget,
                  const CFX_Matrix& matrix,
                  bool forceRound) {
  CXFA_Fill* fill = box->GetFillIfExists();
  if (!fill || !fill->IsVisible())
    return;

  pGS->SaveGraphState();
  CXFA_GEPath fillPath;
  XFA_BOX_GetFillPath(box, strokes, rtWidget, fillPath, forceRound);
  fillPath.Close();
  XFA_Element eType = fill->GetFillType();
  switch (eType) {
    case XFA_Element::Radial:
      XFA_BOX_Fill_Radial(fill, pGS, fillPath, rtWidget, matrix);
      break;
    case XFA_Element::Pattern:
      XFA_BOX_Fill_Pattern(fill, pGS, fillPath, rtWidget, matrix);
      break;
    case XFA_Element::Linear:
      XFA_BOX_Fill_Linear(fill, pGS, fillPath, rtWidget, matrix);
      break;
    default: {
      FX_ARGB cr;
      if (eType == XFA_Element::Stipple) {
        int32_t iRate = fill->GetStippleRate();
        if (iRate == 0)
          iRate = 100;

        int32_t a;
        FX_COLORREF rgb;
        std::tie(a, rgb) = ArgbToColorRef(fill->GetStippleColor());
        cr = ArgbEncode(iRate * a / 100, rgb);
      } else {
        cr = fill->GetColor(false);
      }
      pGS->SetFillColor(CXFA_GEColor(cr));
      pGS->FillPath(&fillPath, FXFILL_WINDING, &matrix);
    } break;
  }
  pGS->RestoreGraphState();
}

void XFA_BOX_StrokeArc(CXFA_Box* box,
                       CXFA_Graphics* pGS,
                       CFX_RectF rtWidget,
                       const CFX_Matrix& matrix,
                       bool forceRound) {
  CXFA_Edge* edge = box->GetEdgeIfExists(0);
  if (!edge || !edge->IsVisible())
    return;

  bool bVisible;
  float fThickness;
  XFA_AttributeEnum i3DType;
  std::tie(i3DType, bVisible, fThickness) = box->Get3DStyle();
  bool lowered3d = false;
  if (i3DType != XFA_AttributeEnum::Unknown) {
    if (bVisible && fThickness >= 0.001f)
      lowered3d = true;
  }

  float fHalf = edge->GetThickness() / 2;
  if (fHalf < 0) {
    fHalf = 0;
  }

  XFA_AttributeEnum iHand = box->GetHand();
  if (iHand == XFA_AttributeEnum::Left) {
    rtWidget.Inflate(fHalf, fHalf);
  } else if (iHand == XFA_AttributeEnum::Right) {
    rtWidget.Deflate(fHalf, fHalf);
  }
  if (!forceRound || !lowered3d) {
    if (fHalf < 0.001f)
      return;

    CXFA_GEPath arcPath;
    XFA_BOX_GetPath_Arc(box, rtWidget, arcPath, forceRound);
    if (edge)
      edge->Stroke(&arcPath, pGS, matrix);
    return;
  }
  pGS->SaveGraphState();
  pGS->SetLineWidth(fHalf);

  float a, b;
  a = rtWidget.width / 2.0f;
  b = rtWidget.height / 2.0f;
  if (forceRound) {
    a = std::min(a, b);
    b = a;
  }

  CFX_PointF center = rtWidget.Center();
  rtWidget.left = center.x - a;
  rtWidget.top = center.y - b;
  rtWidget.width = a + a;
  rtWidget.height = b + b;

  float startAngle = 0, sweepAngle = 360;
  startAngle = startAngle * FX_PI / 180.0f;
  sweepAngle = -sweepAngle * FX_PI / 180.0f;

  CXFA_GEPath arcPath;
  arcPath.AddArc(rtWidget.TopLeft(), rtWidget.Size(), 3.0f * FX_PI / 4.0f,
                 FX_PI);

  pGS->SetStrokeColor(CXFA_GEColor(0xFF808080));
  pGS->StrokePath(&arcPath, &matrix);
  arcPath.Clear();
  arcPath.AddArc(rtWidget.TopLeft(), rtWidget.Size(), -1.0f * FX_PI / 4.0f,
                 FX_PI);

  pGS->SetStrokeColor(CXFA_GEColor(0xFFFFFFFF));
  pGS->StrokePath(&arcPath, &matrix);
  rtWidget.Deflate(fHalf, fHalf);
  arcPath.Clear();
  arcPath.AddArc(rtWidget.TopLeft(), rtWidget.Size(), 3.0f * FX_PI / 4.0f,
                 FX_PI);

  pGS->SetStrokeColor(CXFA_GEColor(0xFF404040));
  pGS->StrokePath(&arcPath, &matrix);
  arcPath.Clear();
  arcPath.AddArc(rtWidget.TopLeft(), rtWidget.Size(), -1.0f * FX_PI / 4.0f,
                 FX_PI);

  pGS->SetStrokeColor(CXFA_GEColor(0xFFC0C0C0));
  pGS->StrokePath(&arcPath, &matrix);
  pGS->RestoreGraphState();
}

void XFA_Draw3DRect(CXFA_Graphics* pGraphic,
                    const CFX_RectF& rt,
                    float fLineWidth,
                    const CFX_Matrix& matrix,
                    FX_ARGB argbTopLeft,
                    FX_ARGB argbBottomRight) {
  float fBottom = rt.bottom();
  float fRight = rt.right();
  CXFA_GEPath pathLT;
  pathLT.MoveTo(CFX_PointF(rt.left, fBottom));
  pathLT.LineTo(CFX_PointF(rt.left, rt.top));
  pathLT.LineTo(CFX_PointF(fRight, rt.top));
  pathLT.LineTo(CFX_PointF(fRight - fLineWidth, rt.top + fLineWidth));
  pathLT.LineTo(CFX_PointF(rt.left + fLineWidth, rt.top + fLineWidth));
  pathLT.LineTo(CFX_PointF(rt.left + fLineWidth, fBottom - fLineWidth));
  pathLT.LineTo(CFX_PointF(rt.left, fBottom));
  pGraphic->SetFillColor(CXFA_GEColor(argbTopLeft));
  pGraphic->FillPath(&pathLT, FXFILL_WINDING, &matrix);

  CXFA_GEPath pathRB;
  pathRB.MoveTo(CFX_PointF(fRight, rt.top));
  pathRB.LineTo(CFX_PointF(fRight, fBottom));
  pathRB.LineTo(CFX_PointF(rt.left, fBottom));
  pathRB.LineTo(CFX_PointF(rt.left + fLineWidth, fBottom - fLineWidth));
  pathRB.LineTo(CFX_PointF(fRight - fLineWidth, fBottom - fLineWidth));
  pathRB.LineTo(CFX_PointF(fRight - fLineWidth, rt.top + fLineWidth));
  pathRB.LineTo(CFX_PointF(fRight, rt.top));
  pGraphic->SetFillColor(CXFA_GEColor(argbBottomRight));
  pGraphic->FillPath(&pathRB, FXFILL_WINDING, &matrix);
}

void XFA_BOX_Stroke_3DRect_Lowered(CXFA_Graphics* pGS,
                                   CFX_RectF rt,
                                   float fThickness,
                                   const CFX_Matrix& matrix) {
  float fHalfWidth = fThickness / 2.0f;
  CFX_RectF rtInner(rt);
  rtInner.Deflate(fHalfWidth, fHalfWidth);

  CXFA_GEPath path;
  path.AddRectangle(rt.left, rt.top, rt.width, rt.height);
  path.AddRectangle(rtInner.left, rtInner.top, rtInner.width, rtInner.height);
  pGS->SetFillColor(CXFA_GEColor(0xFF000000));
  pGS->FillPath(&path, FXFILL_ALTERNATE, &matrix);
  XFA_Draw3DRect(pGS, rtInner, fHalfWidth, matrix, 0xFF808080, 0xFFC0C0C0);
}

void XFA_BOX_Stroke_3DRect_Raised(CXFA_Graphics* pGS,
                                  CFX_RectF rt,
                                  float fThickness,
                                  const CFX_Matrix& matrix) {
  float fHalfWidth = fThickness / 2.0f;
  CFX_RectF rtInner(rt);
  rtInner.Deflate(fHalfWidth, fHalfWidth);

  CXFA_GEPath path;
  path.AddRectangle(rt.left, rt.top, rt.width, rt.height);
  path.AddRectangle(rtInner.left, rtInner.top, rtInner.width, rtInner.height);
  pGS->SetFillColor(CXFA_GEColor(0xFF000000));
  pGS->FillPath(&path, FXFILL_ALTERNATE, &matrix);
  XFA_Draw3DRect(pGS, rtInner, fHalfWidth, matrix, 0xFFFFFFFF, 0xFF808080);
}

void XFA_BOX_Stroke_3DRect_Etched(CXFA_Graphics* pGS,
                                  CFX_RectF rt,
                                  float fThickness,
                                  const CFX_Matrix& matrix) {
  float fHalfWidth = fThickness / 2.0f;
  XFA_Draw3DRect(pGS, rt, fThickness, matrix, 0xFF808080, 0xFFFFFFFF);
  CFX_RectF rtInner(rt);
  rtInner.Deflate(fHalfWidth, fHalfWidth);
  XFA_Draw3DRect(pGS, rtInner, fHalfWidth, matrix, 0xFFFFFFFF, 0xFF808080);
}

void XFA_BOX_Stroke_3DRect_Embossed(CXFA_Graphics* pGS,
                                    CFX_RectF rt,
                                    float fThickness,
                                    const CFX_Matrix& matrix) {
  float fHalfWidth = fThickness / 2.0f;
  XFA_Draw3DRect(pGS, rt, fThickness, matrix, 0xFF808080, 0xFF000000);
  CFX_RectF rtInner(rt);
  rtInner.Deflate(fHalfWidth, fHalfWidth);
  XFA_Draw3DRect(pGS, rtInner, fHalfWidth, matrix, 0xFF000000, 0xFF808080);
}

void XFA_BOX_Stroke_Rect(CXFA_Box* box,
                         const std::vector<CXFA_Stroke*>& strokes,
                         CXFA_Graphics* pGS,
                         CFX_RectF rtWidget,
                         const CFX_Matrix& matrix) {
  bool bVisible;
  float fThickness;
  XFA_AttributeEnum i3DType;
  std::tie(i3DType, bVisible, fThickness) = box->Get3DStyle();
  if (i3DType != XFA_AttributeEnum::Unknown) {
    if (!bVisible || fThickness < 0.001f) {
      return;
    }
    switch (i3DType) {
      case XFA_AttributeEnum::Lowered:
        XFA_BOX_Stroke_3DRect_Lowered(pGS, rtWidget, fThickness, matrix);
        break;
      case XFA_AttributeEnum::Raised:
        XFA_BOX_Stroke_3DRect_Raised(pGS, rtWidget, fThickness, matrix);
        break;
      case XFA_AttributeEnum::Etched:
        XFA_BOX_Stroke_3DRect_Etched(pGS, rtWidget, fThickness, matrix);
        break;
      case XFA_AttributeEnum::Embossed:
        XFA_BOX_Stroke_3DRect_Embossed(pGS, rtWidget, fThickness, matrix);
        break;
      default:
        NOTREACHED();
        break;
    }
    return;
  }
  bool bClose = false;
  bool bSameStyles = true;
  CXFA_Stroke* stroke1 = strokes[0];
  for (int32_t i = 1; i < 8; i++) {
    CXFA_Stroke* stroke2 = strokes[i];
    if (!stroke1->SameStyles(stroke2, 0)) {
      bSameStyles = false;
      break;
    }
    stroke1 = stroke2;
  }
  if (bSameStyles) {
    stroke1 = strokes[0];
    bClose = true;
    for (int32_t i = 2; i < 8; i += 2) {
      CXFA_Stroke* stroke2 = strokes[i];
      if (!stroke1->SameStyles(stroke2, XFA_STROKE_SAMESTYLE_NoPresence |
                                            XFA_STROKE_SAMESTYLE_Corner)) {
        bSameStyles = false;
        break;
      }
      stroke1 = stroke2;
    }
    if (bSameStyles) {
      stroke1 = strokes[0];
      if (stroke1->IsInverted())
        bSameStyles = false;
      if (stroke1->GetJoinType() != XFA_AttributeEnum::Square)
        bSameStyles = false;
    }
  }
  bool bStart = true;
  CXFA_GEPath path;
  for (int32_t i = 0; i < 8; i++) {
    CXFA_Stroke* stroke = strokes[i];
    if ((i % 1) == 0 && stroke->GetRadius() < 0) {
      bool bEmpty = path.IsEmpty();
      if (!bEmpty) {
        if (stroke)
          stroke->Stroke(&path, pGS, matrix);
        path.Clear();
      }
      bStart = true;
      continue;
    }
    XFA_BOX_GetPath(strokes, rtWidget, path, i, bStart, !bSameStyles);

    bStart = !stroke->SameStyles(strokes[(i + 1) % 8], 0);
    if (bStart) {
      if (stroke)
        stroke->Stroke(&path, pGS, matrix);
      path.Clear();
    }
  }
  bool bEmpty = path.IsEmpty();
  if (!bEmpty) {
    if (bClose) {
      path.Close();
    }
    if (strokes[7])
      strokes[7]->Stroke(&path, pGS, matrix);
  }
}

void XFA_BOX_Stroke(CXFA_Box* box,
                    const std::vector<CXFA_Stroke*>& strokes,
                    CXFA_Graphics* pGS,
                    CFX_RectF rtWidget,
                    const CFX_Matrix& matrix,
                    bool forceRound) {
  if (box->IsArc() || forceRound) {
    XFA_BOX_StrokeArc(box, pGS, rtWidget, matrix, forceRound);
    return;
  }

  bool bVisible = false;
  for (int32_t j = 0; j < 4; j++) {
    if (strokes[j * 2 + 1]->IsVisible()) {
      bVisible = true;
      break;
    }
  }
  if (!bVisible)
    return;

  for (int32_t i = 1; i < 8; i += 2) {
    float fThickness = std::fmax(0.0, strokes[i]->GetThickness());
    float fHalf = fThickness / 2;
    XFA_AttributeEnum iHand = box->GetHand();
    switch (i) {
      case 1:
        if (iHand == XFA_AttributeEnum::Left) {
          rtWidget.top -= fHalf;
          rtWidget.height += fHalf;
        } else if (iHand == XFA_AttributeEnum::Right) {
          rtWidget.top += fHalf;
          rtWidget.height -= fHalf;
        }
        break;
      case 3:
        if (iHand == XFA_AttributeEnum::Left) {
          rtWidget.width += fHalf;
        } else if (iHand == XFA_AttributeEnum::Right) {
          rtWidget.width -= fHalf;
        }
        break;
      case 5:
        if (iHand == XFA_AttributeEnum::Left) {
          rtWidget.height += fHalf;
        } else if (iHand == XFA_AttributeEnum::Right) {
          rtWidget.height -= fHalf;
        }
        break;
      case 7:
        if (iHand == XFA_AttributeEnum::Left) {
          rtWidget.left -= fHalf;
          rtWidget.width += fHalf;
        } else if (iHand == XFA_AttributeEnum::Right) {
          rtWidget.left += fHalf;
          rtWidget.width -= fHalf;
        }
        break;
    }
  }
  XFA_BOX_Stroke_Rect(box, strokes, pGS, rtWidget, matrix);
}

void XFA_DrawBox(CXFA_Box* box,
                 CXFA_Graphics* pGS,
                 const CFX_RectF& rtWidget,
                 const CFX_Matrix& matrix,
                 bool forceRound) {
  if (!box || box->GetPresence() != XFA_AttributeEnum::Visible)
    return;

  XFA_Element eType = box->GetElementType();
  if (eType != XFA_Element::Arc && eType != XFA_Element::Border &&
      eType != XFA_Element::Rectangle) {
    return;
  }
  std::vector<CXFA_Stroke*> strokes;
  if (!forceRound && eType != XFA_Element::Arc)
    strokes = box->GetStrokes();

  XFA_BOX_Fill(box, strokes, pGS, rtWidget, matrix, forceRound);
  XFA_BOX_Stroke(box, strokes, pGS, rtWidget, matrix, forceRound);
}

void XFA_GetMatrix(CFX_Matrix& m,
                   int32_t iRotate,
                   XFA_AttributeEnum at,
                   const CFX_RectF& rt) {
  if (!iRotate) {
    return;
  }
  float fAnchorX = 0;
  float fAnchorY = 0;
  switch (at) {
    case XFA_AttributeEnum::TopLeft:
      fAnchorX = rt.left, fAnchorY = rt.top;
      break;
    case XFA_AttributeEnum::TopCenter:
      fAnchorX = (rt.left + rt.right()) / 2, fAnchorY = rt.top;
      break;
    case XFA_AttributeEnum::TopRight:
      fAnchorX = rt.right(), fAnchorY = rt.top;
      break;
    case XFA_AttributeEnum::MiddleLeft:
      fAnchorX = rt.left, fAnchorY = (rt.top + rt.bottom()) / 2;
      break;
    case XFA_AttributeEnum::MiddleCenter:
      fAnchorX = (rt.left + rt.right()) / 2,
      fAnchorY = (rt.top + rt.bottom()) / 2;
      break;
    case XFA_AttributeEnum::MiddleRight:
      fAnchorX = rt.right(), fAnchorY = (rt.top + rt.bottom()) / 2;
      break;
    case XFA_AttributeEnum::BottomLeft:
      fAnchorX = rt.left, fAnchorY = rt.bottom();
      break;
    case XFA_AttributeEnum::BottomCenter:
      fAnchorX = (rt.left + rt.right()) / 2, fAnchorY = rt.bottom();
      break;
    case XFA_AttributeEnum::BottomRight:
      fAnchorX = rt.right(), fAnchorY = rt.bottom();
      break;
    default:
      break;
  }
  switch (iRotate) {
    case 90:
      m.a = 0, m.b = -1, m.c = 1, m.d = 0, m.e = fAnchorX - fAnchorY,
      m.f = fAnchorX + fAnchorY;
      break;
    case 180:
      m.a = -1, m.b = 0, m.c = 0, m.d = -1, m.e = fAnchorX * 2,
      m.f = fAnchorY * 2;
      break;
    case 270:
      m.a = 0, m.b = 1, m.c = -1, m.d = 0, m.e = fAnchorX + fAnchorY,
      m.f = fAnchorY - fAnchorX;
      break;
  }
}

FXDIB_Format XFA_GetDIBFormat(FXCODEC_IMAGE_TYPE type,
                              int32_t iComponents,
                              int32_t iBitsPerComponent) {
  FXDIB_Format dibFormat = FXDIB_Argb;
  switch (type) {
    case FXCODEC_IMAGE_BMP:
    case FXCODEC_IMAGE_JPG:
    case FXCODEC_IMAGE_TIF: {
      dibFormat = FXDIB_Rgb32;
      int32_t bpp = iComponents * iBitsPerComponent;
      if (bpp <= 24) {
        dibFormat = FXDIB_Rgb;
      }
    } break;
    case FXCODEC_IMAGE_PNG:
    default:
      break;
  }
  return dibFormat;
}

bool IsFXCodecErrorStatus(FXCODEC_STATUS status) {
  return (status == FXCODEC_STATUS_ERROR ||
#ifdef PDF_ENABLE_XFA
          status == FXCODEC_STATUS_ERR_MEMORY ||
#endif  // PDF_ENABLE_XFA
          status == FXCODEC_STATUS_ERR_READ ||
          status == FXCODEC_STATUS_ERR_FLUSH ||
          status == FXCODEC_STATUS_ERR_FORMAT ||
          status == FXCODEC_STATUS_ERR_PARAMS);
}

}  // namespace

int32_t XFA_StrokeTypeSetLineDash(CXFA_Graphics* pGraphics,
                                  XFA_AttributeEnum iStrokeType,
                                  XFA_AttributeEnum iCapType) {
  switch (iStrokeType) {
    case XFA_AttributeEnum::DashDot: {
      float dashArray[] = {4, 1, 2, 1};
      if (iCapType != XFA_AttributeEnum::Butt) {
        dashArray[1] = 2;
        dashArray[3] = 2;
      }
      pGraphics->SetLineDash(0, dashArray, 4);
      return FX_DASHSTYLE_DashDot;
    }
    case XFA_AttributeEnum::DashDotDot: {
      float dashArray[] = {4, 1, 2, 1, 2, 1};
      if (iCapType != XFA_AttributeEnum::Butt) {
        dashArray[1] = 2;
        dashArray[3] = 2;
        dashArray[5] = 2;
      }
      pGraphics->SetLineDash(0, dashArray, 6);
      return FX_DASHSTYLE_DashDotDot;
    }
    case XFA_AttributeEnum::Dashed: {
      float dashArray[] = {5, 1};
      if (iCapType != XFA_AttributeEnum::Butt)
        dashArray[1] = 2;

      pGraphics->SetLineDash(0, dashArray, 2);
      return FX_DASHSTYLE_Dash;
    }
    case XFA_AttributeEnum::Dotted: {
      float dashArray[] = {2, 1};
      if (iCapType != XFA_AttributeEnum::Butt)
        dashArray[1] = 2;

      pGraphics->SetLineDash(0, dashArray, 2);
      return FX_DASHSTYLE_Dot;
    }
    default:
      break;
  }
  pGraphics->SetLineDash(FX_DASHSTYLE_Solid);
  return FX_DASHSTYLE_Solid;
}

void XFA_DrawImage(CXFA_Graphics* pGS,
                   const CFX_RectF& rtImage,
                   const CFX_Matrix& matrix,
                   const RetainPtr<CFX_DIBitmap>& pDIBitmap,
                   XFA_AttributeEnum iAspect,
                   int32_t iImageXDpi,
                   int32_t iImageYDpi,
                   XFA_AttributeEnum iHorzAlign,
                   XFA_AttributeEnum iVertAlign) {
  if (rtImage.IsEmpty())
    return;
  if (!pDIBitmap || !pDIBitmap->GetBuffer())
    return;

  CFX_RectF rtFit(
      rtImage.TopLeft(),
      XFA_UnitPx2Pt((float)pDIBitmap->GetWidth(), (float)iImageXDpi),
      XFA_UnitPx2Pt((float)pDIBitmap->GetHeight(), (float)iImageYDpi));
  switch (iAspect) {
    case XFA_AttributeEnum::Fit: {
      float f1 = rtImage.height / rtFit.height;
      float f2 = rtImage.width / rtFit.width;
      f1 = std::min(f1, f2);
      rtFit.height = rtFit.height * f1;
      rtFit.width = rtFit.width * f1;
      break;
    }
    case XFA_AttributeEnum::Height: {
      float f1 = rtImage.height / rtFit.height;
      rtFit.height = rtImage.height;
      rtFit.width = f1 * rtFit.width;
      break;
    }
    case XFA_AttributeEnum::None:
      rtFit.height = rtImage.height;
      rtFit.width = rtImage.width;
      break;
    case XFA_AttributeEnum::Width: {
      float f1 = rtImage.width / rtFit.width;
      rtFit.width = rtImage.width;
      rtFit.height = rtFit.height * f1;
      break;
    }
    case XFA_AttributeEnum::Actual:
    default:
      break;
  }

  if (iHorzAlign == XFA_AttributeEnum::Center)
    rtFit.left += (rtImage.width - rtFit.width) / 2;
  else if (iHorzAlign == XFA_AttributeEnum::Right)
    rtFit.left = rtImage.right() - rtFit.width;

  if (iVertAlign == XFA_AttributeEnum::Middle)
    rtFit.top += (rtImage.height - rtFit.height) / 2;
  else if (iVertAlign == XFA_AttributeEnum::Bottom)
    rtFit.top = rtImage.bottom() - rtImage.height;

  CFX_RenderDevice* pRenderDevice = pGS->GetRenderDevice();
  CFX_RenderDevice::StateRestorer restorer(pRenderDevice);
  CFX_PathData path;
  path.AppendRect(rtImage.left, rtImage.bottom(), rtImage.right(), rtImage.top);
  pRenderDevice->SetClip_PathFill(&path, &matrix, FXFILL_WINDING);

  CFX_Matrix mtImage(1, 0, 0, -1, 0, 1);
  mtImage.Concat(
      CFX_Matrix(rtFit.width, 0, 0, rtFit.height, rtFit.left, rtFit.top));
  mtImage.Concat(matrix);

  CXFA_ImageRenderer imageRender(pRenderDevice, pDIBitmap, 0, 255, &mtImage,
                                 FXDIB_INTERPOL);
  if (!imageRender.Start()) {
    return;
  }
  while (imageRender.Continue())
    continue;
}

RetainPtr<CFX_DIBitmap> XFA_LoadImageFromBuffer(
    const RetainPtr<IFX_SeekableReadStream>& pImageFileRead,
    FXCODEC_IMAGE_TYPE type,
    int32_t& iImageXDpi,
    int32_t& iImageYDpi) {
  CCodec_ModuleMgr* pCodecMgr = CPDF_ModuleMgr::Get()->GetCodecModule();
  std::unique_ptr<CCodec_ProgressiveDecoder> pProgressiveDecoder =
      pCodecMgr->CreateProgressiveDecoder();

  CFX_DIBAttribute dibAttr;
  pProgressiveDecoder->LoadImageInfo(pImageFileRead, type, &dibAttr, false);
  switch (dibAttr.m_wDPIUnit) {
    case FXCODEC_RESUNIT_CENTIMETER:
      dibAttr.m_nXDPI = (int32_t)(dibAttr.m_nXDPI * 2.54f);
      dibAttr.m_nYDPI = (int32_t)(dibAttr.m_nYDPI * 2.54f);
      break;
    case FXCODEC_RESUNIT_METER:
      dibAttr.m_nXDPI = (int32_t)(dibAttr.m_nXDPI / (float)100 * 2.54f);
      dibAttr.m_nYDPI = (int32_t)(dibAttr.m_nYDPI / (float)100 * 2.54f);
      break;
    default:
      break;
  }
  iImageXDpi = dibAttr.m_nXDPI > 1 ? dibAttr.m_nXDPI : (96);
  iImageYDpi = dibAttr.m_nYDPI > 1 ? dibAttr.m_nYDPI : (96);
  if (pProgressiveDecoder->GetWidth() <= 0 ||
      pProgressiveDecoder->GetHeight() <= 0) {
    return nullptr;
  }

  type = pProgressiveDecoder->GetType();
  int32_t iComponents = pProgressiveDecoder->GetNumComponents();
  int32_t iBpc = pProgressiveDecoder->GetBPC();
  FXDIB_Format dibFormat = XFA_GetDIBFormat(type, iComponents, iBpc);
  RetainPtr<CFX_DIBitmap> pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  pBitmap->Create(pProgressiveDecoder->GetWidth(),
                  pProgressiveDecoder->GetHeight(), dibFormat);
  pBitmap->Clear(0xffffffff);

  size_t nFrames;
  FXCODEC_STATUS status;
  std::tie(status, nFrames) = pProgressiveDecoder->GetFrames();
  if (status != FXCODEC_STATUS_DECODE_READY || nFrames == 0) {
    pBitmap = nullptr;
    return pBitmap;
  }

  status = pProgressiveDecoder->StartDecode(pBitmap, 0, 0, pBitmap->GetWidth(),
                                            pBitmap->GetHeight());
  if (IsFXCodecErrorStatus(status)) {
    pBitmap = nullptr;
    return pBitmap;
  }

  while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
    status = pProgressiveDecoder->ContinueDecode();
    if (IsFXCodecErrorStatus(status)) {
      pBitmap = nullptr;
      return pBitmap;
    }
  }

  return pBitmap;
}

void XFA_RectWithoutMargin(CFX_RectF& rt, const CXFA_Margin* margin, bool bUI) {
  if (!margin)
    return;

  rt.Deflate(margin->GetLeftInset(), margin->GetTopInset(),
             margin->GetRightInset(), margin->GetBottomInset());
}

CXFA_FFWidget* XFA_GetWidgetFromLayoutItem(CXFA_LayoutItem* pLayoutItem) {
  if (XFA_IsCreateWidget(pLayoutItem->GetFormNode()->GetElementType()))
    return static_cast<CXFA_FFWidget*>(pLayoutItem);
  return nullptr;
}

bool XFA_IsCreateWidget(XFA_Element eType) {
  return eType == XFA_Element::Field || eType == XFA_Element::Draw ||
         eType == XFA_Element::Subform || eType == XFA_Element::ExclGroup;
}

CXFA_CalcData::CXFA_CalcData() : m_iRefCount(0) {}

CXFA_CalcData::~CXFA_CalcData() {}

CXFA_FFWidget::CXFA_FFWidget(CXFA_Node* node)
    : CXFA_ContentLayoutItem(node), m_pNode(node) {}

CXFA_FFWidget::~CXFA_FFWidget() {}

const CFWL_App* CXFA_FFWidget::GetFWLApp() {
  return GetPageView()->GetDocView()->GetDoc()->GetApp()->GetFWLApp();
}

const CFX_RectF& CXFA_FFWidget::GetWidgetRect() const {
  if ((m_dwStatus & XFA_WidgetStatus_RectCached) == 0)
    RecacheWidgetRect();
  return m_rtWidget;
}

const CFX_RectF& CXFA_FFWidget::RecacheWidgetRect() const {
  m_dwStatus |= XFA_WidgetStatus_RectCached;
  m_rtWidget = GetRect(false);
  return m_rtWidget;
}

CFX_RectF CXFA_FFWidget::GetRectWithoutRotate() {
  CFX_RectF rtWidget = GetWidgetRect();
  float fValue = 0;
  switch (m_pNode->GetRotate()) {
    case 90:
      rtWidget.top = rtWidget.bottom();
      fValue = rtWidget.width;
      rtWidget.width = rtWidget.height;
      rtWidget.height = fValue;
      break;
    case 180:
      rtWidget.left = rtWidget.right();
      rtWidget.top = rtWidget.bottom();
      break;
    case 270:
      rtWidget.left = rtWidget.right();
      fValue = rtWidget.width;
      rtWidget.width = rtWidget.height;
      rtWidget.height = fValue;
      break;
  }
  return rtWidget;
}

uint32_t CXFA_FFWidget::GetStatus() {
  return m_dwStatus;
}

void CXFA_FFWidget::ModifyStatus(uint32_t dwAdded, uint32_t dwRemoved) {
  m_dwStatus = (m_dwStatus & ~dwRemoved) | dwAdded;
}

CFX_RectF CXFA_FFWidget::GetBBox(uint32_t dwStatus, bool bDrawFocus) {
  if (bDrawFocus || !m_pPageView)
    return CFX_RectF();
  return m_pPageView->GetPageViewRect();
}

void CXFA_FFWidget::RenderWidget(CXFA_Graphics* pGS,
                                 const CFX_Matrix& matrix,
                                 uint32_t dwStatus) {
  if (!IsMatchVisibleStatus(dwStatus))
    return;

  CXFA_Border* border = m_pNode->GetBorderIfExists();
  if (!border)
    return;

  CFX_RectF rtBorder = GetRectWithoutRotate();
  CXFA_Margin* margin = border->GetMarginIfExists();
  if (margin)
    XFA_RectWithoutMargin(rtBorder, margin);

  rtBorder.Normalize();
  DrawBorder(pGS, border, rtBorder, matrix);
}

bool CXFA_FFWidget::IsLoaded() {
  return !!m_pPageView;
}

bool CXFA_FFWidget::LoadWidget() {
  PerformLayout();
  return true;
}

void CXFA_FFWidget::UnloadWidget() {}

bool CXFA_FFWidget::PerformLayout() {
  RecacheWidgetRect();
  return true;
}

bool CXFA_FFWidget::UpdateFWLData() {
  return false;
}

void CXFA_FFWidget::UpdateWidgetProperty() {}

void CXFA_FFWidget::DrawBorder(CXFA_Graphics* pGS,
                               CXFA_Box* box,
                               const CFX_RectF& rtBorder,
                               const CFX_Matrix& matrix) {
  XFA_DrawBox(box, pGS, rtBorder, matrix, false);
}

void CXFA_FFWidget::DrawBorderWithFlag(CXFA_Graphics* pGS,
                                       CXFA_Box* box,
                                       const CFX_RectF& rtBorder,
                                       const CFX_Matrix& matrix,
                                       bool forceRound) {
  XFA_DrawBox(box, pGS, rtBorder, matrix, forceRound);
}

void CXFA_FFWidget::AddInvalidateRect() {
  CFX_RectF rtWidget = GetBBox(XFA_WidgetStatus_Focused);
  rtWidget.Inflate(2, 2);
  m_pDocView->AddInvalidateRect(m_pPageView, rtWidget);
}

bool CXFA_FFWidget::OnMouseEnter() {
  return false;
}

bool CXFA_FFWidget::OnMouseExit() {
  return false;
}

bool CXFA_FFWidget::OnLButtonDown(uint32_t dwFlags, const CFX_PointF& point) {
  return false;
}

bool CXFA_FFWidget::OnLButtonUp(uint32_t dwFlags, const CFX_PointF& point) {
  return false;
}

bool CXFA_FFWidget::OnLButtonDblClk(uint32_t dwFlags, const CFX_PointF& point) {
  return false;
}

bool CXFA_FFWidget::OnMouseMove(uint32_t dwFlags, const CFX_PointF& point) {
  return false;
}

bool CXFA_FFWidget::OnMouseWheel(uint32_t dwFlags,
                                 int16_t zDelta,
                                 const CFX_PointF& point) {
  return false;
}

bool CXFA_FFWidget::OnRButtonDown(uint32_t dwFlags, const CFX_PointF& point) {
  return false;
}

bool CXFA_FFWidget::OnRButtonUp(uint32_t dwFlags, const CFX_PointF& point) {
  return false;
}

bool CXFA_FFWidget::OnRButtonDblClk(uint32_t dwFlags, const CFX_PointF& point) {
  return false;
}

bool CXFA_FFWidget::OnSetFocus(CXFA_FFWidget* pOldWidget) {
  CXFA_FFWidget* pParent = GetParent();
  if (pParent && !pParent->IsAncestorOf(pOldWidget)) {
    pParent->OnSetFocus(pOldWidget);
  }
  m_dwStatus |= XFA_WidgetStatus_Focused;
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Enter;
  eParam.m_pTarget = m_pNode->GetWidgetAcc();
  m_pNode->ProcessEvent(GetDocView(), XFA_AttributeEnum::Enter, &eParam);
  return true;
}

bool CXFA_FFWidget::OnKillFocus(CXFA_FFWidget* pNewWidget) {
  m_dwStatus &= ~XFA_WidgetStatus_Focused;
  EventKillFocus();
  if (pNewWidget) {
    CXFA_FFWidget* pParent = GetParent();
    if (pParent && !pParent->IsAncestorOf(pNewWidget)) {
      pParent->OnKillFocus(pNewWidget);
    }
  }
  return true;
}

bool CXFA_FFWidget::OnKeyDown(uint32_t dwKeyCode, uint32_t dwFlags) {
  return false;
}

bool CXFA_FFWidget::OnKeyUp(uint32_t dwKeyCode, uint32_t dwFlags) {
  return false;
}

bool CXFA_FFWidget::OnChar(uint32_t dwChar, uint32_t dwFlags) {
  return false;
}

FWL_WidgetHit CXFA_FFWidget::OnHitTest(const CFX_PointF& point) {
  return FWL_WidgetHit::Unknown;
}

bool CXFA_FFWidget::OnSetCursor(const CFX_PointF& point) {
  return false;
}

bool CXFA_FFWidget::CanUndo() {
  return false;
}

bool CXFA_FFWidget::CanRedo() {
  return false;
}

bool CXFA_FFWidget::Undo() {
  return false;
}

bool CXFA_FFWidget::Redo() {
  return false;
}

bool CXFA_FFWidget::CanCopy() {
  return false;
}

bool CXFA_FFWidget::CanCut() {
  return false;
}

bool CXFA_FFWidget::CanPaste() {
  return false;
}

bool CXFA_FFWidget::CanSelectAll() {
  return false;
}

bool CXFA_FFWidget::CanDelete() {
  return CanCut();
}

bool CXFA_FFWidget::CanDeSelect() {
  return CanCopy();
}

Optional<WideString> CXFA_FFWidget::Copy() {
  return {};
}

Optional<WideString> CXFA_FFWidget::Cut() {
  return {};
}

bool CXFA_FFWidget::Paste(const WideString& wsPaste) {
  return false;
}

void CXFA_FFWidget::SelectAll() {}

void CXFA_FFWidget::Delete() {}

void CXFA_FFWidget::DeSelect() {}

FormFieldType CXFA_FFWidget::GetFormFieldType() {
  return FormFieldType::kXFA;
}

void CXFA_FFWidget::GetSuggestWords(CFX_PointF pointf,
                                    std::vector<ByteString>* pWords) {
  pWords->clear();
}

bool CXFA_FFWidget::ReplaceSpellCheckWord(CFX_PointF pointf,
                                          const ByteStringView& bsReplace) {
  return false;
}

CFX_PointF CXFA_FFWidget::Rotate2Normal(const CFX_PointF& point) {
  CFX_Matrix mt = GetRotateMatrix();
  if (mt.IsIdentity())
    return point;

  return mt.GetInverse().Transform(point);
}

CFX_Matrix CXFA_FFWidget::GetRotateMatrix() {
  CFX_Matrix mt;
  int32_t iRotate = m_pNode->GetRotate();
  if (!iRotate)
    return mt;

  CFX_RectF rcWidget = GetRectWithoutRotate();
  XFA_AttributeEnum at = XFA_AttributeEnum::TopLeft;
  XFA_GetMatrix(mt, iRotate, at, rcWidget);

  return mt;
}

bool CXFA_FFWidget::IsLayoutRectEmpty() {
  CFX_RectF rtLayout = GetRectWithoutRotate();
  return rtLayout.width < 0.1f && rtLayout.height < 0.1f;
}
CXFA_FFWidget* CXFA_FFWidget::GetParent() {
  CXFA_Node* pParentNode = m_pNode->GetParent();
  if (pParentNode) {
    CXFA_WidgetAcc* pParentWidgetAcc =
        static_cast<CXFA_WidgetAcc*>(pParentNode->GetWidgetAcc());
    if (pParentWidgetAcc) {
      CXFA_LayoutProcessor* layout = GetDocView()->GetXFALayout();
      return static_cast<CXFA_FFWidget*>(
          layout->GetLayoutItem(pParentWidgetAcc->GetNode()));
    }
  }
  return nullptr;
}

bool CXFA_FFWidget::IsAncestorOf(CXFA_FFWidget* pWidget) {
  if (!pWidget)
    return false;

  CXFA_Node* pChildNode = pWidget->GetNode();
  while (pChildNode) {
    if (pChildNode == m_pNode)
      return true;

    pChildNode = pChildNode->GetParent();
  }
  return false;
}

bool CXFA_FFWidget::PtInActiveRect(const CFX_PointF& point) {
  return GetWidgetRect().Contains(point);
}

CXFA_FFDocView* CXFA_FFWidget::GetDocView() {
  return m_pDocView;
}

void CXFA_FFWidget::SetDocView(CXFA_FFDocView* pDocView) {
  m_pDocView = pDocView;
}

CXFA_FFDoc* CXFA_FFWidget::GetDoc() {
  return m_pDocView->GetDoc();
}

CXFA_FFApp* CXFA_FFWidget::GetApp() {
  return GetDoc()->GetApp();
}

IXFA_AppProvider* CXFA_FFWidget::GetAppProvider() {
  return GetApp()->GetAppProvider();
}

bool CXFA_FFWidget::IsMatchVisibleStatus(uint32_t dwStatus) {
  return !!(m_dwStatus & XFA_WidgetStatus_Visible);
}

void CXFA_FFWidget::EventKillFocus() {
  if (m_dwStatus & XFA_WidgetStatus_Access) {
    m_dwStatus &= ~XFA_WidgetStatus_Access;
    return;
  }
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Exit;
  eParam.m_pTarget = m_pNode->GetWidgetAcc();
  m_pNode->ProcessEvent(GetDocView(), XFA_AttributeEnum::Exit, &eParam);
}

bool CXFA_FFWidget::IsButtonDown() {
  return (m_dwStatus & XFA_WidgetStatus_ButtonDown) != 0;
}

void CXFA_FFWidget::SetButtonDown(bool bSet) {
  bSet ? m_dwStatus |= XFA_WidgetStatus_ButtonDown
       : m_dwStatus &= ~XFA_WidgetStatus_ButtonDown;
}
