// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_box.h"

#include <utility>

#include "xfa/fxfa/parser/cxfa_corner.h"
#include "xfa/fxfa/parser/cxfa_cornerdata.h"
#include "xfa/fxfa/parser/cxfa_edge.h"
#include "xfa/fxfa/parser/cxfa_fill.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

namespace {

XFA_AttributeEnum Style3D(const std::vector<CXFA_StrokeData>& strokes,
                          CXFA_StrokeData& strokeData) {
  if (strokes.empty())
    return XFA_AttributeEnum::Unknown;

  strokeData = strokes[0];
  for (size_t i = 1; i < strokes.size(); i++) {
    CXFA_StrokeData find = strokes[i];
    if (!find.HasValidNode())
      continue;

    if (!strokeData.HasValidNode())
      strokeData = find;
    else if (strokeData.GetStrokeType() != find.GetStrokeType())
      strokeData = find;
    break;
  }

  XFA_AttributeEnum iType = strokeData.GetStrokeType();
  if (iType == XFA_AttributeEnum::Lowered ||
      iType == XFA_AttributeEnum::Raised ||
      iType == XFA_AttributeEnum::Etched ||
      iType == XFA_AttributeEnum::Embossed) {
    return iType;
  }
  return XFA_AttributeEnum::Unknown;
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

CXFA_EdgeData CXFA_Box::GetEdgeData(int32_t nIndex) {
  return CXFA_EdgeData(JSObject()->GetProperty<CXFA_Edge>(
      nIndex, XFA_Element::Edge, nIndex == 0));
}

std::vector<CXFA_StrokeData> CXFA_Box::GetStrokes() {
  return GetStrokesInternal(false);
}

bool CXFA_Box::IsCircular() {
  return JSObject()->GetBoolean(XFA_Attribute::Circular);
}

pdfium::Optional<int32_t> CXFA_Box::GetStartAngle() {
  return JSObject()->TryInteger(XFA_Attribute::StartAngle, false);
}

pdfium::Optional<int32_t> CXFA_Box::GetSweepAngle() {
  return JSObject()->TryInteger(XFA_Attribute::SweepAngle, false);
}

CXFA_FillData CXFA_Box::GetFillData(bool bModified) {
  CXFA_Node* pFillNode =
      JSObject()->GetProperty<CXFA_Fill>(0, XFA_Element::Fill, bModified);
  return CXFA_FillData(pFillNode);
}

CXFA_Margin* CXFA_Box::GetMargin() {
  return GetChild<CXFA_Margin>(0, XFA_Element::Margin, false);
}

std::tuple<XFA_AttributeEnum, bool, float> CXFA_Box::Get3DStyle() {
  if (IsArc())
    return {XFA_AttributeEnum::Unknown, false, 0.0f};

  std::vector<CXFA_StrokeData> strokes = GetStrokesInternal(true);
  CXFA_StrokeData strokeData(nullptr);
  XFA_AttributeEnum iType = Style3D(strokes, strokeData);
  if (iType == XFA_AttributeEnum::Unknown)
    return {XFA_AttributeEnum::Unknown, false, 0.0f};

  return {iType, strokeData.IsVisible(), strokeData.GetThickness()};
}

std::vector<CXFA_StrokeData> CXFA_Box::GetStrokesInternal(bool bNull) {
  std::vector<CXFA_StrokeData> strokes;
  strokes.resize(8);
  int32_t i, j;
  for (i = 0, j = 0; i < 4; i++) {
    CXFA_CornerData cornerData = CXFA_CornerData(
        JSObject()->GetProperty<CXFA_Corner>(i, XFA_Element::Corner, i == 0));
    if (cornerData.HasValidNode() || i == 0) {
      strokes[j] = cornerData;
    } else if (!bNull) {
      if (i == 1 || i == 2)
        strokes[j] = strokes[0];
      else
        strokes[j] = strokes[2];
    }
    j++;
    CXFA_EdgeData edgeData = CXFA_EdgeData(
        JSObject()->GetProperty<CXFA_Edge>(i, XFA_Element::Edge, i == 0));
    if (edgeData.HasValidNode() || i == 0) {
      strokes[j] = edgeData;
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
