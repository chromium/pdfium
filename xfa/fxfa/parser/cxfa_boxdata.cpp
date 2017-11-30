// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_boxdata.h"

#include "xfa/fxfa/parser/cxfa_cornerdata.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

namespace {

std::vector<CXFA_StrokeData> GetStrokesInternal(CXFA_Node* pNode, bool bNull) {
  if (!pNode)
    return {};

  std::vector<CXFA_StrokeData> strokes;
  strokes.resize(8);
  int32_t i, j;
  for (i = 0, j = 0; i < 4; i++) {
    CXFA_CornerData cornerData = CXFA_CornerData(
        pNode->JSNode()->GetProperty(i, XFA_Element::Corner, i == 0));
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
        pNode->JSNode()->GetProperty(i, XFA_Element::Edge, i == 0));
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

static XFA_AttributeEnum Style3D(const std::vector<CXFA_StrokeData>& strokes,
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

XFA_AttributeEnum CXFA_BoxData::GetHand() const {
  if (!m_pNode)
    return XFA_AttributeEnum::Even;
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::Hand);
}

XFA_AttributeEnum CXFA_BoxData::GetPresence() const {
  if (!m_pNode)
    return XFA_AttributeEnum::Hidden;
  return m_pNode->JSNode()
      ->TryEnum(XFA_Attribute::Presence, true)
      .value_or(XFA_AttributeEnum::Visible);
}

int32_t CXFA_BoxData::CountEdges() const {
  if (!m_pNode)
    return 0;
  return m_pNode->CountChildren(XFA_Element::Edge, false);
}

CXFA_EdgeData CXFA_BoxData::GetEdgeData(int32_t nIndex) const {
  return CXFA_EdgeData(m_pNode ? m_pNode->JSNode()->GetProperty(
                                     nIndex, XFA_Element::Edge, nIndex == 0)
                               : nullptr);
}

std::vector<CXFA_StrokeData> CXFA_BoxData::GetStrokes() const {
  return GetStrokesInternal(m_pNode, false);
}

bool CXFA_BoxData::IsCircular() const {
  if (!m_pNode)
    return false;
  return m_pNode->JSNode()->GetBoolean(XFA_Attribute::Circular);
}

pdfium::Optional<int32_t> CXFA_BoxData::GetStartAngle() const {
  if (!m_pNode)
    return {};
  return m_pNode->JSNode()->TryInteger(XFA_Attribute::StartAngle, false);
}

pdfium::Optional<int32_t> CXFA_BoxData::GetSweepAngle() const {
  if (!m_pNode)
    return {};
  return m_pNode->JSNode()->TryInteger(XFA_Attribute::SweepAngle, false);
}

CXFA_FillData CXFA_BoxData::GetFillData(bool bModified) const {
  if (!m_pNode)
    return CXFA_FillData(nullptr);

  CXFA_Node* pFillNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Fill, bModified);
  return CXFA_FillData(pFillNode);
}

CXFA_MarginData CXFA_BoxData::GetMarginData() const {
  return CXFA_MarginData(
      m_pNode ? m_pNode->GetChild(0, XFA_Element::Margin, false) : nullptr);
}

std::tuple<XFA_AttributeEnum, bool, float> CXFA_BoxData::Get3DStyle() const {
  if (IsArc())
    return {XFA_AttributeEnum::Unknown, false, 0.0f};

  std::vector<CXFA_StrokeData> strokes = GetStrokesInternal(m_pNode, true);
  CXFA_StrokeData strokeData(nullptr);
  XFA_AttributeEnum iType = Style3D(strokes, strokeData);
  if (iType == XFA_AttributeEnum::Unknown)
    return {XFA_AttributeEnum::Unknown, false, 0.0f};

  return {iType, strokeData.IsVisible(), strokeData.GetThickness()};
}
