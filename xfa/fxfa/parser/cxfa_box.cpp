// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_box.h"

#include <algorithm>
#include <utility>

#include "xfa/fxfa/parser/cxfa_corner.h"
#include "xfa/fxfa/parser/cxfa_edge.h"
#include "xfa/fxfa/parser/cxfa_fill.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_rectangle.h"
#include "xfa/fxgraphics/cxfa_gepath.h"
#include "xfa/fxgraphics/cxfa_gepattern.h"
#include "xfa/fxgraphics/cxfa_geshading.h"
#include "xfa/fxgraphics/cxfa_graphics.h"

namespace {

std::pair<XFA_AttributeEnum, CXFA_Stroke*> Style3D(
    const std::vector<CXFA_Stroke*>& strokes) {
  if (strokes.empty())
    return {XFA_AttributeEnum::Unknown, nullptr};

  CXFA_Stroke* stroke = strokes[0];
  for (size_t i = 1; i < strokes.size(); i++) {
    CXFA_Stroke* find = strokes[i];
    if (!find)
      continue;
    if (!stroke)
      stroke = find;
    else if (stroke->GetStrokeType() != find->GetStrokeType())
      stroke = find;
    break;
  }

  XFA_AttributeEnum iType = stroke->GetStrokeType();
  if (iType == XFA_AttributeEnum::Lowered ||
      iType == XFA_AttributeEnum::Raised ||
      iType == XFA_AttributeEnum::Etched ||
      iType == XFA_AttributeEnum::Embossed) {
    return {iType, stroke};
  }
  return {XFA_AttributeEnum::Unknown, stroke};
}

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

}  // namespace

CXFA_Box::CXFA_Box(CXFA_Document* pDoc,
                   XFA_PacketType ePacket,
                   uint32_t validPackets,
                   XFA_ObjectType oType,
                   XFA_Element eType,
                   const PropertyData* properties,
                   const AttributeData* attributes,
                   const WideStringView& elementName,
                   std::unique_ptr<CJX_Object> js_node)
    : CXFA_Node(pDoc,
                ePacket,
                validPackets,
                oType,
                eType,
                properties,
                attributes,
                elementName,
                std::move(js_node)) {}

CXFA_Box::~CXFA_Box() = default;

XFA_AttributeEnum CXFA_Box::GetHand() {
  return JSObject()->GetEnum(XFA_Attribute::Hand);
}

XFA_AttributeEnum CXFA_Box::GetPresence() {
  return JSObject()
      ->TryEnum(XFA_Attribute::Presence, true)
      .value_or(XFA_AttributeEnum::Visible);
}

int32_t CXFA_Box::CountEdges() {
  return CountChildren(XFA_Element::Edge, false);
}

CXFA_Edge* CXFA_Box::GetEdgeIfExists(int32_t nIndex) {
  if (nIndex == 0)
    return JSObject()->GetOrCreateProperty<CXFA_Edge>(nIndex,
                                                      XFA_Element::Edge);
  return JSObject()->GetProperty<CXFA_Edge>(nIndex, XFA_Element::Edge);
}

std::vector<CXFA_Stroke*> CXFA_Box::GetStrokes() {
  return GetStrokesInternal(false);
}

bool CXFA_Box::IsCircular() {
  return JSObject()->GetBoolean(XFA_Attribute::Circular);
}

Optional<int32_t> CXFA_Box::GetStartAngle() {
  return JSObject()->TryInteger(XFA_Attribute::StartAngle, false);
}

Optional<int32_t> CXFA_Box::GetSweepAngle() {
  return JSObject()->TryInteger(XFA_Attribute::SweepAngle, false);
}

CXFA_Fill* CXFA_Box::GetFillIfExists() const {
  return JSObject()->GetProperty<CXFA_Fill>(0, XFA_Element::Fill);
}

CXFA_Fill* CXFA_Box::GetOrCreateFillIfPossible() {
  return JSObject()->GetOrCreateProperty<CXFA_Fill>(0, XFA_Element::Fill);
}

CXFA_Margin* CXFA_Box::GetMarginIfExists() {
  return GetChild<CXFA_Margin>(0, XFA_Element::Margin, false);
}

std::tuple<XFA_AttributeEnum, bool, float> CXFA_Box::Get3DStyle() {
  if (IsArc())
    return {XFA_AttributeEnum::Unknown, false, 0.0f};

  std::vector<CXFA_Stroke*> strokes = GetStrokesInternal(true);
  CXFA_Stroke* stroke;
  XFA_AttributeEnum iType;

  std::tie(iType, stroke) = Style3D(strokes);
  if (iType == XFA_AttributeEnum::Unknown)
    return {XFA_AttributeEnum::Unknown, false, 0.0f};

  return {iType, stroke->IsVisible(), stroke->GetThickness()};
}

std::vector<CXFA_Stroke*> CXFA_Box::GetStrokesInternal(bool bNull) {
  std::vector<CXFA_Stroke*> strokes;
  strokes.resize(8);

  for (int32_t i = 0, j = 0; i < 4; i++) {
    CXFA_Corner* corner;
    if (i == 0) {
      corner =
          JSObject()->GetOrCreateProperty<CXFA_Corner>(i, XFA_Element::Corner);
    } else {
      corner = JSObject()->GetProperty<CXFA_Corner>(i, XFA_Element::Corner);
    }

    // TODO(dsinclair): If i == 0 and GetOrCreateProperty failed, we can end up
    // with a null corner in the first position.
    if (corner || i == 0) {
      strokes[j] = corner;
    } else if (!bNull) {
      if (i == 1 || i == 2)
        strokes[j] = strokes[0];
      else
        strokes[j] = strokes[2];
    }
    j++;

    CXFA_Edge* edge;
    if (i == 0)
      edge = JSObject()->GetOrCreateProperty<CXFA_Edge>(i, XFA_Element::Edge);
    else
      edge = JSObject()->GetProperty<CXFA_Edge>(i, XFA_Element::Edge);

    // TODO(dsinclair): If i == 0 and GetOrCreateProperty failed, we can end up
    // with a null edge in the first position.
    if (edge || i == 0) {
      strokes[j] = edge;
    } else if (!bNull) {
      if (i == 1 || i == 2)
        strokes[j] = strokes[1];
      else
        strokes[j] = strokes[3];
    }
    j++;
  }
  return strokes;
}

void CXFA_Box::Draw(CXFA_Graphics* pGS,
                    const CFX_RectF& rtWidget,
                    const CFX_Matrix& matrix,
                    bool forceRound) {
  if (GetPresence() != XFA_AttributeEnum::Visible)
    return;

  XFA_Element eType = GetElementType();
  if (eType != XFA_Element::Arc && eType != XFA_Element::Border &&
      eType != XFA_Element::Rectangle) {
    return;
  }
  std::vector<CXFA_Stroke*> strokes;
  if (!forceRound && eType != XFA_Element::Arc)
    strokes = GetStrokes();

  DrawFill(strokes, pGS, rtWidget, matrix, forceRound);
  XFA_Element type = GetElementType();
  if (type == XFA_Element::Arc || forceRound) {
    XFA_BOX_StrokeArc(this, pGS, rtWidget, matrix, forceRound);
  } else if (type == XFA_Element::Rectangle || type == XFA_Element::Border) {
    static_cast<CXFA_Rectangle*>(this)->Draw(strokes, pGS, rtWidget, matrix);
  } else {
    NOTREACHED();
  }
}

void CXFA_Box::DrawFill(const std::vector<CXFA_Stroke*>& strokes,
                        CXFA_Graphics* pGS,
                        const CFX_RectF& rtWidget,
                        const CFX_Matrix& matrix,
                        bool forceRound) {
  CXFA_Fill* fill = GetFillIfExists();
  if (!fill || !fill->IsVisible())
    return;

  pGS->SaveGraphState();

  CXFA_GEPath fillPath;
  XFA_BOX_GetFillPath(this, strokes, rtWidget, fillPath, forceRound);
  fillPath.Close();

  fill->Draw(pGS, &fillPath, rtWidget, matrix);
  pGS->RestoreGraphState();
}
