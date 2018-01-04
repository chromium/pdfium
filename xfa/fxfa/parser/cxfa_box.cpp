// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_box.h"

#include <utility>

#include "xfa/fxfa/parser/cxfa_corner.h"
#include "xfa/fxfa/parser/cxfa_edge.h"
#include "xfa/fxfa/parser/cxfa_fill.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

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

CXFA_Edge* CXFA_Box::GetEdge(int32_t nIndex) {
  return JSObject()->GetProperty<CXFA_Edge>(nIndex, XFA_Element::Edge,
                                            nIndex == 0);
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

CXFA_Fill* CXFA_Box::GetFill(bool bModified) {
  return JSObject()->GetProperty<CXFA_Fill>(0, XFA_Element::Fill, bModified);
}

CXFA_Margin* CXFA_Box::GetMargin() {
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
    CXFA_Corner* corner =
        JSObject()->GetProperty<CXFA_Corner>(i, XFA_Element::Corner, i == 0);
    if (corner || i == 0) {
      strokes[j] = corner;
    } else if (!bNull) {
      if (i == 1 || i == 2)
        strokes[j] = strokes[0];
      else
        strokes[j] = strokes[2];
    }
    j++;

    CXFA_Edge* edge =
        JSObject()->GetProperty<CXFA_Edge>(i, XFA_Element::Edge, i == 0);
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
