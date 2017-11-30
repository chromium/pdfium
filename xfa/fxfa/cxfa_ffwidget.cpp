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
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_imagerenderer.h"
#include "core/fxge/dib/cfx_imagetransformer.h"
#include "xfa/fwl/fwl_widgethit.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_textlayout.h"
#include "xfa/fxfa/cxfa_widgetacc.h"
#include "xfa/fxfa/parser/cxfa_cornerdata.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxgraphics/cxfa_gecolor.h"
#include "xfa/fxgraphics/cxfa_gepath.h"
#include "xfa/fxgraphics/cxfa_gepattern.h"
#include "xfa/fxgraphics/cxfa_geshading.h"
#include "xfa/fxgraphics/cxfa_graphics.h"

namespace {

void XFA_BOX_GetPath_Arc(const CXFA_BoxData& boxData,
                         CFX_RectF rtDraw,
                         CXFA_GEPath& fillPath,
                         uint32_t dwFlags) {
  float a, b;
  a = rtDraw.width / 2.0f;
  b = rtDraw.height / 2.0f;
  if (boxData.IsCircular() || (dwFlags & XFA_DRAWBOX_ForceRound) != 0)
    a = b = std::min(a, b);

  CFX_PointF center = rtDraw.Center();
  rtDraw.left = center.x - a;
  rtDraw.top = center.y - b;
  rtDraw.width = a + a;
  rtDraw.height = b + b;
  pdfium::Optional<int32_t> startAngle = boxData.GetStartAngle();
  pdfium::Optional<int32_t> sweepAngle = boxData.GetSweepAngle();
  if (!startAngle && !sweepAngle) {
    fillPath.AddEllipse(rtDraw);
    return;
  }

  fillPath.AddArc(rtDraw.TopLeft(), rtDraw.Size(),
                  -startAngle.value_or(0) * FX_PI / 180.0f,
                  -sweepAngle.value_or(360) * FX_PI / 180.0f);
}

void XFA_BOX_GetPath(const std::vector<CXFA_StrokeData>& strokes,
                     CFX_RectF rtWidget,
                     CXFA_GEPath& path,
                     int32_t nIndex,
                     bool bStart,
                     bool bCorner) {
  ASSERT(nIndex >= 0 && nIndex < 8);
  int32_t n = (nIndex & 1) ? nIndex - 1 : nIndex;
  CXFA_CornerData cornerData1(strokes[n].GetNode());
  CXFA_CornerData cornerData2(strokes[(n + 2) % 8].GetNode());
  float fRadius1 = bCorner ? cornerData1.GetRadius() : 0.0f;
  float fRadius2 = bCorner ? cornerData2.GetRadius() : 0.0f;
  bool bInverted = cornerData1.IsInverted();
  float offsetY = 0.0f;
  float offsetX = 0.0f;
  bool bRound = cornerData1.GetJoinType() == XFA_AttributeEnum::Round;
  float halfAfter = 0.0f;
  float halfBefore = 0.0f;
  CXFA_StrokeData strokeData = strokes[nIndex];
  if (strokeData.IsCorner()) {
    CXFA_StrokeData strokeBeforeData = strokes[(nIndex + 1 * 8 - 1) % 8];
    CXFA_StrokeData strokeAfterData = strokes[nIndex + 1];
    if (strokeData.IsInverted()) {
      if (!strokeData.SameStyles(strokeBeforeData, 0))
        halfBefore = strokeBeforeData.GetThickness() / 2;
      if (!strokeData.SameStyles(strokeAfterData, 0))
        halfAfter = strokeAfterData.GetThickness() / 2;
    }
  } else {
    CXFA_StrokeData strokeBeforeData = strokes[(nIndex + 8 - 2) % 8];
    CXFA_StrokeData strokeAfterData = strokes[(nIndex + 2) % 8];
    if (!bRound && !bInverted) {
      halfBefore = strokeBeforeData.GetThickness() / 2;
      halfAfter = strokeAfterData.GetThickness() / 2;
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

void XFA_BOX_GetFillPath(const CXFA_BoxData& boxData,
                         const std::vector<CXFA_StrokeData>& strokes,
                         CFX_RectF rtWidget,
                         CXFA_GEPath& fillPath,
                         uint16_t dwFlags) {
  if (boxData.IsArc() || (dwFlags & XFA_DRAWBOX_ForceRound) != 0) {
    float fThickness = std::fmax(0.0, boxData.GetEdgeData(0).GetThickness());
    float fHalf = fThickness / 2;
    XFA_AttributeEnum iHand = boxData.GetHand();
    if (iHand == XFA_AttributeEnum::Left)
      rtWidget.Inflate(fHalf, fHalf);
    else if (iHand == XFA_AttributeEnum::Right)
      rtWidget.Deflate(fHalf, fHalf);

    XFA_BOX_GetPath_Arc(boxData, rtWidget, fillPath, dwFlags);
    return;
  }

  bool bSameStyles = true;
  CXFA_StrokeData strokeData1 = strokes[0];
  for (int32_t i = 1; i < 8; i++) {
    CXFA_StrokeData strokeData2 = strokes[i];
    if (!strokeData1.SameStyles(strokeData2, 0)) {
      bSameStyles = false;
      break;
    }
    strokeData1 = strokeData2;
  }
  if (bSameStyles) {
    strokeData1 = strokes[0];
    for (int32_t i = 2; i < 8; i += 2) {
      CXFA_StrokeData strokeData2 = strokes[i];
      if (!strokeData1.SameStyles(
              strokeData2,
              XFA_STROKE_SAMESTYLE_NoPresence | XFA_STROKE_SAMESTYLE_Corner)) {
        bSameStyles = false;
        break;
      }
      strokeData1 = strokeData2;
    }
    if (bSameStyles) {
      strokeData1 = strokes[0];
      if (strokeData1.IsInverted()) {
        bSameStyles = false;
      }
      if (strokeData1.GetJoinType() != XFA_AttributeEnum::Square) {
        bSameStyles = false;
      }
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
    CXFA_CornerData cornerData1(strokes[i].GetNode());
    CXFA_CornerData cornerData2(strokes[(i + 2) % 8].GetNode());
    float fRadius1 = cornerData1.GetRadius();
    float fRadius2 = cornerData2.GetRadius();
    bool bInverted = cornerData1.IsInverted();
    bool bRound = cornerData1.GetJoinType() == XFA_AttributeEnum::Round;
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

void XFA_BOX_Fill_Radial(const CXFA_BoxData& boxData,
                         CXFA_Graphics* pGS,
                         CXFA_GEPath& fillPath,
                         CFX_RectF rtFill,
                         const CFX_Matrix& matrix) {
  CXFA_FillData fillData = boxData.GetFillData(false);
  FX_ARGB crStart = fillData.GetColor(false);
  FX_ARGB crEnd = fillData.GetRadialColor();
  if (!fillData.IsRadialToEdge())
    std::swap(crStart, crEnd);

  CXFA_GEShading shading(rtFill.Center(), rtFill.Center(), 0,
                         sqrt(rtFill.Width() * rtFill.Width() +
                              rtFill.Height() * rtFill.Height()) /
                             2,
                         true, true, crStart, crEnd);
  pGS->SetFillColor(CXFA_GEColor(&shading));
  pGS->FillPath(&fillPath, FXFILL_WINDING, &matrix);
}

void XFA_BOX_Fill_Pattern(const CXFA_BoxData& boxData,
                          CXFA_Graphics* pGS,
                          CXFA_GEPath& fillPath,
                          CFX_RectF rtFill,
                          const CFX_Matrix& matrix) {
  CXFA_FillData fillData = boxData.GetFillData(false);
  FX_ARGB crStart = fillData.GetColor(false);
  FX_ARGB crEnd = fillData.GetPatternColor();
  FX_HatchStyle iHatch = FX_HatchStyle::Cross;
  switch (fillData.GetPatternType()) {
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

void XFA_BOX_Fill_Linear(const CXFA_BoxData& boxData,
                         CXFA_Graphics* pGS,
                         CXFA_GEPath& fillPath,
                         CFX_RectF rtFill,
                         const CFX_Matrix& matrix) {
  CXFA_FillData fillData = boxData.GetFillData(false);
  FX_ARGB crStart = fillData.GetColor(false);
  FX_ARGB crEnd = fillData.GetLinearColor();

  CFX_PointF ptStart;
  CFX_PointF ptEnd;
  switch (fillData.GetLinearType()) {
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

void XFA_BOX_Fill(const CXFA_BoxData& boxData,
                  const std::vector<CXFA_StrokeData>& strokes,
                  CXFA_Graphics* pGS,
                  const CFX_RectF& rtWidget,
                  const CFX_Matrix& matrix,
                  uint32_t dwFlags) {
  CXFA_FillData fillData = boxData.GetFillData(false);
  if (!fillData.HasValidNode() || !fillData.IsVisible())
    return;

  pGS->SaveGraphState();
  CXFA_GEPath fillPath;
  XFA_BOX_GetFillPath(boxData, strokes, rtWidget, fillPath,
                      (dwFlags & XFA_DRAWBOX_ForceRound) != 0);
  fillPath.Close();
  XFA_Element eType = fillData.GetFillType();
  switch (eType) {
    case XFA_Element::Radial:
      XFA_BOX_Fill_Radial(boxData, pGS, fillPath, rtWidget, matrix);
      break;
    case XFA_Element::Pattern:
      XFA_BOX_Fill_Pattern(boxData, pGS, fillPath, rtWidget, matrix);
      break;
    case XFA_Element::Linear:
      XFA_BOX_Fill_Linear(boxData, pGS, fillPath, rtWidget, matrix);
      break;
    default: {
      FX_ARGB cr;
      if (eType == XFA_Element::Stipple) {
        int32_t iRate = fillData.GetStippleRate();
        if (iRate == 0)
          iRate = 100;

        int32_t a;
        FX_COLORREF rgb;
        std::tie(a, rgb) = ArgbToColorRef(fillData.GetStippleColor());
        cr = ArgbEncode(iRate * a / 100, rgb);
      } else {
        cr = fillData.GetColor(false);
      }
      pGS->SetFillColor(CXFA_GEColor(cr));
      pGS->FillPath(&fillPath, FXFILL_WINDING, &matrix);
    } break;
  }
  pGS->RestoreGraphState();
}

void XFA_BOX_StrokePath(const CXFA_StrokeData& strokeData,
                        CXFA_GEPath* pPath,
                        CXFA_Graphics* pGS,
                        const CFX_Matrix& matrix) {
  if (!strokeData.HasValidNode() || !strokeData.IsVisible())
    return;

  float fThickness = strokeData.GetThickness();
  if (fThickness < 0.001f)
    return;

  pGS->SaveGraphState();
  if (strokeData.IsCorner() && fThickness > 2 * strokeData.GetRadius())
    fThickness = 2 * strokeData.GetRadius();

  pGS->SetLineWidth(fThickness);
  pGS->EnableActOnDash();
  pGS->SetLineCap(CFX_GraphStateData::LineCapButt);
  XFA_StrokeTypeSetLineDash(pGS, strokeData.GetStrokeType(),
                            XFA_AttributeEnum::Butt);
  pGS->SetStrokeColor(CXFA_GEColor(strokeData.GetColor()));
  pGS->StrokePath(pPath, &matrix);
  pGS->RestoreGraphState();
}

void XFA_BOX_StrokeArc(const CXFA_BoxData& boxData,
                       CXFA_Graphics* pGS,
                       CFX_RectF rtWidget,
                       const CFX_Matrix& matrix,
                       uint32_t dwFlags) {
  CXFA_EdgeData edgeData = boxData.GetEdgeData(0);
  if (!edgeData.HasValidNode() || !edgeData.IsVisible())
    return;

  bool bVisible;
  float fThickness;
  XFA_AttributeEnum i3DType;
  std::tie(i3DType, bVisible, fThickness) = boxData.Get3DStyle();
  if (i3DType != XFA_AttributeEnum::Unknown) {
    if (bVisible && fThickness >= 0.001f) {
      dwFlags |= XFA_DRAWBOX_Lowered3D;
    }
  }
  float fHalf = edgeData.GetThickness() / 2;
  if (fHalf < 0) {
    fHalf = 0;
  }

  XFA_AttributeEnum iHand = boxData.GetHand();
  if (iHand == XFA_AttributeEnum::Left) {
    rtWidget.Inflate(fHalf, fHalf);
  } else if (iHand == XFA_AttributeEnum::Right) {
    rtWidget.Deflate(fHalf, fHalf);
  }
  if ((dwFlags & XFA_DRAWBOX_ForceRound) == 0 ||
      (dwFlags & XFA_DRAWBOX_Lowered3D) == 0) {
    if (fHalf < 0.001f)
      return;

    CXFA_GEPath arcPath;
    XFA_BOX_GetPath_Arc(boxData, rtWidget, arcPath, dwFlags);
    XFA_BOX_StrokePath(edgeData, &arcPath, pGS, matrix);
    return;
  }
  pGS->SaveGraphState();
  pGS->SetLineWidth(fHalf);

  float a, b;
  a = rtWidget.width / 2.0f;
  b = rtWidget.height / 2.0f;
  if (dwFlags & XFA_DRAWBOX_ForceRound) {
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

void XFA_BOX_Stroke_Rect(CXFA_BoxData boxData,
                         const std::vector<CXFA_StrokeData>& strokes,
                         CXFA_Graphics* pGS,
                         CFX_RectF rtWidget,
                         const CFX_Matrix& matrix) {
  bool bVisible;
  float fThickness;
  XFA_AttributeEnum i3DType;
  std::tie(i3DType, bVisible, fThickness) = boxData.Get3DStyle();
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
  CXFA_StrokeData strokeData1 = strokes[0];
  for (int32_t i = 1; i < 8; i++) {
    CXFA_StrokeData strokeData2 = strokes[i];
    if (!strokeData1.SameStyles(strokeData2, 0)) {
      bSameStyles = false;
      break;
    }
    strokeData1 = strokeData2;
  }
  if (bSameStyles) {
    strokeData1 = strokes[0];
    bClose = true;
    for (int32_t i = 2; i < 8; i += 2) {
      CXFA_StrokeData strokeData2 = strokes[i];
      if (!strokeData1.SameStyles(
              strokeData2,
              XFA_STROKE_SAMESTYLE_NoPresence | XFA_STROKE_SAMESTYLE_Corner)) {
        bSameStyles = false;
        break;
      }
      strokeData1 = strokeData2;
    }
    if (bSameStyles) {
      strokeData1 = strokes[0];
      if (strokeData1.IsInverted())
        bSameStyles = false;
      if (strokeData1.GetJoinType() != XFA_AttributeEnum::Square)
        bSameStyles = false;
    }
  }
  bool bStart = true;
  CXFA_GEPath path;
  for (int32_t i = 0; i < 8; i++) {
    CXFA_StrokeData strokeData = strokes[i];
    if ((i % 1) == 0 && strokeData.GetRadius() < 0) {
      bool bEmpty = path.IsEmpty();
      if (!bEmpty) {
        XFA_BOX_StrokePath(strokeData, &path, pGS, matrix);
        path.Clear();
      }
      bStart = true;
      continue;
    }
    XFA_BOX_GetPath(strokes, rtWidget, path, i, bStart, !bSameStyles);
    bStart = !strokeData.SameStyles(strokes[(i + 1) % 8], 0);
    if (bStart) {
      XFA_BOX_StrokePath(strokeData, &path, pGS, matrix);
      path.Clear();
    }
  }
  bool bEmpty = path.IsEmpty();
  if (!bEmpty) {
    if (bClose) {
      path.Close();
    }
    XFA_BOX_StrokePath(strokes[7], &path, pGS, matrix);
  }
}

void XFA_BOX_Stroke(CXFA_BoxData boxData,
                    const std::vector<CXFA_StrokeData>& strokes,
                    CXFA_Graphics* pGS,
                    CFX_RectF rtWidget,
                    const CFX_Matrix& matrix,
                    uint32_t dwFlags) {
  if (boxData.IsArc() || (dwFlags & XFA_DRAWBOX_ForceRound) != 0) {
    XFA_BOX_StrokeArc(boxData, pGS, rtWidget, matrix, dwFlags);
    return;
  }
  bool bVisible = false;
  for (int32_t j = 0; j < 4; j++) {
    if (strokes[j * 2 + 1].IsVisible()) {
      bVisible = true;
      break;
    }
  }
  if (!bVisible) {
    return;
  }
  for (int32_t i = 1; i < 8; i += 2) {
    float fThickness =
        std::fmax(0.0, CXFA_EdgeData(strokes[i].GetNode()).GetThickness());
    float fHalf = fThickness / 2;
    XFA_AttributeEnum iHand = boxData.GetHand();
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
  XFA_BOX_Stroke_Rect(boxData, strokes, pGS, rtWidget, matrix);
}

void XFA_DrawBox(CXFA_BoxData boxData,
                 CXFA_Graphics* pGS,
                 const CFX_RectF& rtWidget,
                 const CFX_Matrix& matrix,
                 uint32_t dwFlags) {
  if (!boxData.HasValidNode() ||
      boxData.GetPresence() != XFA_AttributeEnum::Visible) {
    return;
  }

  XFA_Element eType = boxData.GetElementType();
  if (eType != XFA_Element::Arc && eType != XFA_Element::Border &&
      eType != XFA_Element::Rectangle) {
    return;
  }
  std::vector<CXFA_StrokeData> strokes;
  if (!(dwFlags & XFA_DRAWBOX_ForceRound) && eType != XFA_Element::Arc)
    strokes = boxData.GetStrokes();

  XFA_BOX_Fill(boxData, strokes, pGS, rtWidget, matrix, dwFlags);
  XFA_BOX_Stroke(boxData, strokes, pGS, rtWidget, matrix, dwFlags);
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

CXFA_FFWidget::CXFA_FFWidget(CXFA_WidgetAcc* pDataAcc)
    : CXFA_ContentLayoutItem(pDataAcc->GetNode()),
      m_pPageView(nullptr),
      m_pDataAcc(pDataAcc) {}

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
  switch (m_pDataAcc->GetRotate()) {
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

CXFA_WidgetAcc* CXFA_FFWidget::GetDataAcc() {
  return m_pDataAcc.Get();
}

void CXFA_FFWidget::RenderWidget(CXFA_Graphics* pGS,
                                 const CFX_Matrix& matrix,
                                 uint32_t dwStatus) {
  if (!IsMatchVisibleStatus(dwStatus))
    return;

  CXFA_BorderData borderData = m_pDataAcc->GetBorderData(false);
  if (!borderData.HasValidNode())
    return;

  CFX_RectF rtBorder = GetRectWithoutRotate();
  CXFA_MarginData marginData = borderData.GetMarginData();
  if (marginData.HasValidNode())
    XFA_RectWidthoutMargin(rtBorder, marginData);

  rtBorder.Normalize();
  DrawBorder(pGS, borderData, rtBorder, matrix);
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
                               const CXFA_BoxData& boxData,
                               const CFX_RectF& rtBorder,
                               const CFX_Matrix& matrix) {
  XFA_DrawBox(boxData, pGS, rtBorder, matrix, 0);
}

void CXFA_FFWidget::DrawBorderWithFlags(CXFA_Graphics* pGS,
                                        const CXFA_BoxData& boxData,
                                        const CFX_RectF& rtBorder,
                                        const CFX_Matrix& matrix,
                                        uint32_t dwFlags) {
  XFA_DrawBox(boxData, pGS, rtBorder, matrix, dwFlags);
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
  eParam.m_pTarget = m_pDataAcc.Get();
  m_pDataAcc->ProcessEvent(XFA_AttributeEnum::Enter, &eParam);
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

bool CXFA_FFWidget::Copy(WideString& wsCopy) {
  return false;
}

bool CXFA_FFWidget::Cut(WideString& wsCut) {
  return false;
}

bool CXFA_FFWidget::Paste(const WideString& wsPaste) {
  return false;
}

void CXFA_FFWidget::SelectAll() {}

void CXFA_FFWidget::Delete() {}

void CXFA_FFWidget::DeSelect() {}

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

static void XFA_GetMatrix(CFX_Matrix& m,
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

CFX_Matrix CXFA_FFWidget::GetRotateMatrix() {
  CFX_Matrix mt;
  int32_t iRotate = m_pDataAcc->GetRotate();
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
  CXFA_Node* pParentNode =
      m_pDataAcc->GetNode()->GetNodeItem(XFA_NODEITEM_Parent);
  if (pParentNode) {
    CXFA_WidgetAcc* pParentWidgetAcc =
        static_cast<CXFA_WidgetAcc*>(pParentNode->GetWidgetData());
    if (pParentWidgetAcc) {
      return pParentWidgetAcc->GetNextWidget(nullptr);
    }
  }
  return nullptr;
}

bool CXFA_FFWidget::IsAncestorOf(CXFA_FFWidget* pWidget) {
  if (!pWidget)
    return false;

  CXFA_Node* pNode = m_pDataAcc->GetNode();
  CXFA_Node* pChildNode = pWidget->GetDataAcc()->GetNode();
  while (pChildNode) {
    if (pChildNode == pNode)
      return true;

    pChildNode = pChildNode->GetNodeItem(XFA_NODEITEM_Parent);
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
  eParam.m_pTarget = m_pDataAcc.Get();
  m_pDataAcc->ProcessEvent(XFA_AttributeEnum::Exit, &eParam);
}

bool CXFA_FFWidget::IsButtonDown() {
  return (m_dwStatus & XFA_WidgetStatus_ButtonDown) != 0;
}

void CXFA_FFWidget::SetButtonDown(bool bSet) {
  bSet ? m_dwStatus |= XFA_WidgetStatus_ButtonDown
       : m_dwStatus &= ~XFA_WidgetStatus_ButtonDown;
}

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

class CXFA_ImageRenderer {
 public:
  CXFA_ImageRenderer();
  ~CXFA_ImageRenderer();

  bool Start(CFX_RenderDevice* pDevice,
             const RetainPtr<CFX_DIBSource>& pDIBSource,
             FX_ARGB bitmap_argb,
             int bitmap_alpha,
             const CFX_Matrix* pImage2Device,
             uint32_t flags,
             int blendType = FXDIB_BLEND_NORMAL);
  bool Continue();

 protected:
  bool StartDIBSource();
  void CompositeDIBitmap(const RetainPtr<CFX_DIBitmap>& pDIBitmap,
                         int left,
                         int top,
                         FX_ARGB mask_argb,
                         int bitmap_alpha,
                         int blend_mode,
                         int iTransparency);

  CFX_RenderDevice* m_pDevice;
  int m_Status;
  CFX_Matrix m_ImageMatrix;
  RetainPtr<CFX_DIBSource> m_pDIBSource;
  RetainPtr<CFX_DIBitmap> m_pCloneConvert;
  int m_BitmapAlpha;
  FX_ARGB m_FillArgb;
  uint32_t m_Flags;
  std::unique_ptr<CFX_ImageTransformer> m_pTransformer;
  std::unique_ptr<CFX_ImageRenderer> m_DeviceHandle;
  int32_t m_BlendType;
  bool m_Result;
  bool m_bPrint;
};

CXFA_ImageRenderer::CXFA_ImageRenderer()
    : m_pDevice(nullptr),
      m_Status(0),
      m_BitmapAlpha(255),
      m_FillArgb(0),
      m_Flags(0),
      m_DeviceHandle(nullptr),
      m_BlendType(FXDIB_BLEND_NORMAL),
      m_Result(true),
      m_bPrint(false) {}

CXFA_ImageRenderer::~CXFA_ImageRenderer() {}

bool CXFA_ImageRenderer::Start(CFX_RenderDevice* pDevice,
                               const RetainPtr<CFX_DIBSource>& pDIBSource,
                               FX_ARGB bitmap_argb,
                               int bitmap_alpha,
                               const CFX_Matrix* pImage2Device,
                               uint32_t flags,
                               int blendType) {
  m_pDevice = pDevice;
  m_pDIBSource = pDIBSource;
  m_FillArgb = bitmap_argb;
  m_BitmapAlpha = bitmap_alpha;
  m_ImageMatrix = *pImage2Device;
  m_Flags = flags;
  m_BlendType = blendType;
  return StartDIBSource();
}

bool CXFA_ImageRenderer::StartDIBSource() {
  if (m_pDevice->StartDIBitsWithBlend(m_pDIBSource, m_BitmapAlpha, m_FillArgb,
                                      &m_ImageMatrix, m_Flags, &m_DeviceHandle,
                                      m_BlendType)) {
    if (m_DeviceHandle) {
      m_Status = 3;
      return true;
    }
    return false;
  }
  CFX_FloatRect image_rect_f = m_ImageMatrix.GetUnitRect();
  FX_RECT image_rect = image_rect_f.GetOuterRect();
  int dest_width = image_rect.Width();
  int dest_height = image_rect.Height();
  if ((fabs(m_ImageMatrix.b) >= 0.5f || m_ImageMatrix.a == 0) ||
      (fabs(m_ImageMatrix.c) >= 0.5f || m_ImageMatrix.d == 0)) {
    if (m_bPrint && !(m_pDevice->GetRenderCaps() & FXRC_BLEND_MODE)) {
      m_Result = false;
      return false;
    }
    RetainPtr<CFX_DIBSource> pDib = m_pDIBSource;
    if (m_pDIBSource->HasAlpha() &&
        !(m_pDevice->GetRenderCaps() & FXRC_ALPHA_IMAGE) &&
        !(m_pDevice->GetRenderCaps() & FXRC_GET_BITS)) {
      m_pCloneConvert = m_pDIBSource->CloneConvert(FXDIB_Rgb);
      if (!m_pCloneConvert) {
        m_Result = false;
        return false;
      }
      pDib = m_pCloneConvert;
    }
    FX_RECT clip_box = m_pDevice->GetClipBox();
    clip_box.Intersect(image_rect);
    m_Status = 2;
    m_pTransformer = pdfium::MakeUnique<CFX_ImageTransformer>(
        pDib, &m_ImageMatrix, m_Flags, &clip_box);
    return true;
  }
  if (m_ImageMatrix.a < 0)
    dest_width = -dest_width;
  if (m_ImageMatrix.d > 0)
    dest_height = -dest_height;
  int dest_left, dest_top;
  dest_left = dest_width > 0 ? image_rect.left : image_rect.right;
  dest_top = dest_height > 0 ? image_rect.top : image_rect.bottom;
  if (m_pDIBSource->IsOpaqueImage() && m_BitmapAlpha == 255) {
    if (m_pDevice->StretchDIBitsWithFlagsAndBlend(
            m_pDIBSource, dest_left, dest_top, dest_width, dest_height, m_Flags,
            m_BlendType)) {
      return false;
    }
  }
  if (m_pDIBSource->IsAlphaMask()) {
    if (m_BitmapAlpha != 255) {
      m_FillArgb = FXARGB_MUL_ALPHA(m_FillArgb, m_BitmapAlpha);
    }
    if (m_pDevice->StretchBitMaskWithFlags(m_pDIBSource, dest_left, dest_top,
                                           dest_width, dest_height, m_FillArgb,
                                           m_Flags)) {
      return false;
    }
  }
  if (m_bPrint && !(m_pDevice->GetRenderCaps() & FXRC_BLEND_MODE)) {
    m_Result = false;
    return true;
  }
  FX_RECT clip_box = m_pDevice->GetClipBox();
  FX_RECT dest_rect = clip_box;
  dest_rect.Intersect(image_rect);
  FX_RECT dest_clip(
      dest_rect.left - image_rect.left, dest_rect.top - image_rect.top,
      dest_rect.right - image_rect.left, dest_rect.bottom - image_rect.top);
  RetainPtr<CFX_DIBitmap> pStretched =
      m_pDIBSource->StretchTo(dest_width, dest_height, m_Flags, &dest_clip);
  if (pStretched) {
    CompositeDIBitmap(pStretched, dest_rect.left, dest_rect.top, m_FillArgb,
                      m_BitmapAlpha, m_BlendType, false);
  }
  return false;
}

bool CXFA_ImageRenderer::Continue() {
  if (m_Status == 2) {
    if (m_pTransformer->Continue(nullptr))
      return true;

    RetainPtr<CFX_DIBitmap> pBitmap = m_pTransformer->DetachBitmap();
    if (!pBitmap)
      return false;

    if (pBitmap->IsAlphaMask()) {
      if (m_BitmapAlpha != 255)
        m_FillArgb = FXARGB_MUL_ALPHA(m_FillArgb, m_BitmapAlpha);
      m_Result =
          m_pDevice->SetBitMask(pBitmap, m_pTransformer->result().left,
                                m_pTransformer->result().top, m_FillArgb);
    } else {
      if (m_BitmapAlpha != 255)
        pBitmap->MultiplyAlpha(m_BitmapAlpha);
      m_Result = m_pDevice->SetDIBitsWithBlend(
          pBitmap, m_pTransformer->result().left, m_pTransformer->result().top,
          m_BlendType);
    }
    return false;
  }
  if (m_Status == 3)
    return m_pDevice->ContinueDIBits(m_DeviceHandle.get(), nullptr);

  return false;
}

void CXFA_ImageRenderer::CompositeDIBitmap(
    const RetainPtr<CFX_DIBitmap>& pDIBitmap,
    int left,
    int top,
    FX_ARGB mask_argb,
    int bitmap_alpha,
    int blend_mode,
    int iTransparency) {
  if (!pDIBitmap) {
    return;
  }
  bool bIsolated = !!(iTransparency & PDFTRANS_ISOLATED);
  bool bGroup = !!(iTransparency & PDFTRANS_GROUP);
  if (blend_mode == FXDIB_BLEND_NORMAL) {
    if (!pDIBitmap->IsAlphaMask()) {
      if (bitmap_alpha < 255) {
        pDIBitmap->MultiplyAlpha(bitmap_alpha);
      }
      if (m_pDevice->SetDIBits(pDIBitmap, left, top)) {
        return;
      }
    } else {
      uint32_t fill_argb = (mask_argb);
      if (bitmap_alpha < 255) {
        ((uint8_t*)&fill_argb)[3] =
            ((uint8_t*)&fill_argb)[3] * bitmap_alpha / 255;
      }
      if (m_pDevice->SetBitMask(pDIBitmap, left, top, fill_argb)) {
        return;
      }
    }
  }
  bool bBackAlphaRequired = blend_mode && bIsolated;
  bool bGetBackGround =
      ((m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT)) ||
      (!(m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT) &&
       (m_pDevice->GetRenderCaps() & FXRC_GET_BITS) && !bBackAlphaRequired);
  if (bGetBackGround) {
    if (bIsolated || !bGroup) {
      if (pDIBitmap->IsAlphaMask()) {
        return;
      }
      m_pDevice->SetDIBitsWithBlend(pDIBitmap, left, top, blend_mode);
    } else {
      FX_RECT rect(left, top, left + pDIBitmap->GetWidth(),
                   top + pDIBitmap->GetHeight());
      rect.Intersect(m_pDevice->GetClipBox());
      RetainPtr<CFX_DIBitmap> pClone;
      if (m_pDevice->GetBackDrop() && m_pDevice->GetBitmap()) {
        pClone = m_pDevice->GetBackDrop()->Clone(&rect);
        RetainPtr<CFX_DIBitmap> pForeBitmap = m_pDevice->GetBitmap();
        pClone->CompositeBitmap(0, 0, pClone->GetWidth(), pClone->GetHeight(),
                                pForeBitmap, rect.left, rect.top);
        left = left >= 0 ? 0 : left;
        top = top >= 0 ? 0 : top;
        if (!pDIBitmap->IsAlphaMask())
          pClone->CompositeBitmap(0, 0, pClone->GetWidth(), pClone->GetHeight(),
                                  pDIBitmap, left, top, blend_mode);
        else
          pClone->CompositeMask(0, 0, pClone->GetWidth(), pClone->GetHeight(),
                                pDIBitmap, mask_argb, left, top, blend_mode);
      } else {
        pClone = pDIBitmap;
      }
      if (m_pDevice->GetBackDrop()) {
        m_pDevice->SetDIBits(pClone, rect.left, rect.top);
      } else {
        if (pDIBitmap->IsAlphaMask())
          return;
        m_pDevice->SetDIBitsWithBlend(pDIBitmap, rect.left, rect.top,
                                      blend_mode);
      }
    }
    return;
  }
  if (!pDIBitmap->HasAlpha() ||
      (m_pDevice->GetRenderCaps() & FXRC_ALPHA_IMAGE)) {
    return;
  }
  RetainPtr<CFX_DIBitmap> pCloneConvert = pDIBitmap->CloneConvert(FXDIB_Rgb);
  if (!pCloneConvert)
    return;

  CXFA_ImageRenderer imageRender;
  if (!imageRender.Start(m_pDevice, pCloneConvert, m_FillArgb, m_BitmapAlpha,
                         &m_ImageMatrix, m_Flags)) {
    return;
  }
  while (imageRender.Continue())
    continue;
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

  CXFA_ImageRenderer imageRender;
  if (!imageRender.Start(pRenderDevice, pDIBitmap, 0, 255, &mtImage,
                         FXDIB_INTERPOL)) {
    return;
  }
  while (imageRender.Continue())
    continue;
}

static const uint8_t g_inv_base64[128] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62,  255,
    255, 255, 63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  255, 255,
    255, 255, 255, 255, 255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
    10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
    25,  255, 255, 255, 255, 255, 255, 26,  27,  28,  29,  30,  31,  32,  33,
    34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51,  255, 255, 255, 255, 255,
};

static uint8_t* XFA_RemoveBase64Whitespace(const uint8_t* pStr, int32_t iLen) {
  uint8_t* pCP;
  int32_t i = 0, j = 0;
  if (iLen == 0) {
    iLen = strlen((char*)pStr);
  }
  pCP = FX_Alloc(uint8_t, iLen + 1);
  for (; i < iLen; i++) {
    if ((pStr[i] & 128) == 0) {
      if (g_inv_base64[pStr[i]] != 0xFF || pStr[i] == '=') {
        pCP[j++] = pStr[i];
      }
    }
  }
  pCP[j] = '\0';
  return pCP;
}

static int32_t XFA_Base64Decode(const char* pStr, uint8_t* pOutBuffer) {
  if (!pStr) {
    return 0;
  }
  uint8_t* pBuffer =
      XFA_RemoveBase64Whitespace((uint8_t*)pStr, strlen((char*)pStr));
  if (!pBuffer) {
    return 0;
  }
  int32_t iLen = strlen((char*)pBuffer);
  int32_t i = 0, j = 0;
  uint32_t dwLimb = 0;
  for (; i + 3 < iLen; i += 4) {
    if (pBuffer[i] == '=' || pBuffer[i + 1] == '=' || pBuffer[i + 2] == '=' ||
        pBuffer[i + 3] == '=') {
      if (pBuffer[i] == '=' || pBuffer[i + 1] == '=') {
        break;
      }
      if (pBuffer[i + 2] == '=') {
        dwLimb = ((uint32_t)g_inv_base64[pBuffer[i]] << 6) |
                 ((uint32_t)g_inv_base64[pBuffer[i + 1]]);
        pOutBuffer[j] = (uint8_t)(dwLimb >> 4) & 0xFF;
        j++;
      } else {
        dwLimb = ((uint32_t)g_inv_base64[pBuffer[i]] << 12) |
                 ((uint32_t)g_inv_base64[pBuffer[i + 1]] << 6) |
                 ((uint32_t)g_inv_base64[pBuffer[i + 2]]);
        pOutBuffer[j] = (uint8_t)(dwLimb >> 10) & 0xFF;
        pOutBuffer[j + 1] = (uint8_t)(dwLimb >> 2) & 0xFF;
        j += 2;
      }
    } else {
      dwLimb = ((uint32_t)g_inv_base64[pBuffer[i]] << 18) |
               ((uint32_t)g_inv_base64[pBuffer[i + 1]] << 12) |
               ((uint32_t)g_inv_base64[pBuffer[i + 2]] << 6) |
               ((uint32_t)g_inv_base64[pBuffer[i + 3]]);
      pOutBuffer[j] = (uint8_t)(dwLimb >> 16) & 0xff;
      pOutBuffer[j + 1] = (uint8_t)(dwLimb >> 8) & 0xff;
      pOutBuffer[j + 2] = (uint8_t)(dwLimb)&0xff;
      j += 3;
    }
  }
  FX_Free(pBuffer);
  return j;
}

static const char g_base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* XFA_Base64Encode(const uint8_t* buf, int32_t buf_len) {
  char* out = nullptr;
  int i, j;
  uint32_t limb;
  out = FX_Alloc(char, ((buf_len * 8 + 5) / 6) + 5);
  for (i = 0, j = 0, limb = 0; i + 2 < buf_len; i += 3, j += 4) {
    limb = ((uint32_t)buf[i] << 16) | ((uint32_t)buf[i + 1] << 8) |
           ((uint32_t)buf[i + 2]);
    out[j] = g_base64_chars[(limb >> 18) & 63];
    out[j + 1] = g_base64_chars[(limb >> 12) & 63];
    out[j + 2] = g_base64_chars[(limb >> 6) & 63];
    out[j + 3] = g_base64_chars[(limb)&63];
  }
  switch (buf_len - i) {
    case 0:
      break;
    case 1:
      limb = ((uint32_t)buf[i]);
      out[j++] = g_base64_chars[(limb >> 2) & 63];
      out[j++] = g_base64_chars[(limb << 4) & 63];
      out[j++] = '=';
      out[j++] = '=';
      break;
    case 2:
      limb = ((uint32_t)buf[i] << 8) | ((uint32_t)buf[i + 1]);
      out[j++] = g_base64_chars[(limb >> 10) & 63];
      out[j++] = g_base64_chars[(limb >> 4) & 63];
      out[j++] = g_base64_chars[(limb << 2) & 63];
      out[j++] = '=';
      break;
    default:
      break;
  }
  out[j] = '\0';
  return out;
}
FXCODEC_IMAGE_TYPE XFA_GetImageType(const WideString& wsType) {
  WideString wsContentType(wsType);
  wsContentType.MakeLower();
  if (wsContentType == L"image/jpg")
    return FXCODEC_IMAGE_JPG;
  if (wsContentType == L"image/png")
    return FXCODEC_IMAGE_PNG;
  if (wsContentType == L"image/gif")
    return FXCODEC_IMAGE_GIF;
  if (wsContentType == L"image/bmp")
    return FXCODEC_IMAGE_BMP;
  if (wsContentType == L"image/tif")
    return FXCODEC_IMAGE_TIF;
  return FXCODEC_IMAGE_UNKNOWN;
}

RetainPtr<CFX_DIBitmap> XFA_LoadImageData(CXFA_FFDoc* pDoc,
                                          CXFA_ImageData* pImageData,
                                          bool& bNameImage,
                                          int32_t& iImageXDpi,
                                          int32_t& iImageYDpi) {
  WideString wsHref = pImageData->GetHref();
  WideString wsImage = pImageData->GetContent();
  if (wsHref.IsEmpty() && wsImage.IsEmpty())
    return nullptr;

  FXCODEC_IMAGE_TYPE type = XFA_GetImageType(pImageData->GetContentType());
  ByteString bsContent;
  uint8_t* pImageBuffer = nullptr;
  RetainPtr<IFX_SeekableReadStream> pImageFileRead;
  if (wsImage.GetLength() > 0) {
    XFA_AttributeEnum iEncoding = pImageData->GetTransferEncoding();
    if (iEncoding == XFA_AttributeEnum::Base64) {
      ByteString bsData = wsImage.UTF8Encode();
      int32_t iLength = bsData.GetLength();
      pImageBuffer = FX_Alloc(uint8_t, iLength);
      int32_t iRead = XFA_Base64Decode(bsData.c_str(), pImageBuffer);
      if (iRead > 0) {
        pImageFileRead =
            pdfium::MakeRetain<CFX_MemoryStream>(pImageBuffer, iRead, false);
      }
    } else {
      bsContent = ByteString::FromUnicode(wsImage);
      pImageFileRead = pdfium::MakeRetain<CFX_MemoryStream>(
          const_cast<uint8_t*>(bsContent.raw_str()), bsContent.GetLength(),
          false);
    }
  } else {
    WideString wsURL = wsHref;
    if (wsURL.Left(7) != L"http://" && wsURL.Left(6) != L"ftp://") {
      RetainPtr<CFX_DIBitmap> pBitmap =
          pDoc->GetPDFNamedImage(wsURL.AsStringView(), iImageXDpi, iImageYDpi);
      if (pBitmap) {
        bNameImage = true;
        return pBitmap;
      }
    }
    pImageFileRead = pDoc->GetDocEnvironment()->OpenLinkedFile(pDoc, wsURL);
  }
  if (!pImageFileRead) {
    FX_Free(pImageBuffer);
    return nullptr;
  }
  bNameImage = false;
  RetainPtr<CFX_DIBitmap> pBitmap =
      XFA_LoadImageFromBuffer(pImageFileRead, type, iImageXDpi, iImageYDpi);
  FX_Free(pImageBuffer);
  return pBitmap;
}

static FXDIB_Format XFA_GetDIBFormat(FXCODEC_IMAGE_TYPE type,
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

void XFA_RectWidthoutMargin(CFX_RectF& rt,
                            const CXFA_MarginData& marginData,
                            bool bUI) {
  if (!marginData.HasValidNode())
    return;

  rt.Deflate(marginData.GetLeftInset(), marginData.GetTopInset(),
             marginData.GetRightInset(), marginData.GetBottomInset());
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
