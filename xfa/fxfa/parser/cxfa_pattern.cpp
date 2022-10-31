// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pattern.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fgas/graphics/cfgas_gepattern.h"
#include "xfa/fxfa/parser/cxfa_color.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kPatternPropertyData[] = {
    {XFA_Element::Color, 1, {}},
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kPatternAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Type, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::CrossHatch},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Pattern::CXFA_Pattern(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Pattern,
                kPatternPropertyData,
                kPatternAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Pattern::~CXFA_Pattern() = default;

CXFA_Color* CXFA_Pattern::GetColorIfExists() {
  return GetChild<CXFA_Color>(0, XFA_Element::Color, false);
}

XFA_AttributeValue CXFA_Pattern::GetType() {
  return JSObject()->GetEnum(XFA_Attribute::Type);
}

void CXFA_Pattern::Draw(CFGAS_GEGraphics* pGS,
                        const CFGAS_GEPath& fillPath,
                        FX_ARGB crStart,
                        const CFX_RectF& rtFill,
                        const CFX_Matrix& matrix) {
  CXFA_Color* pColor = GetColorIfExists();
  FX_ARGB crEnd = pColor ? pColor->GetValue() : CXFA_Color::kBlackColor;
  CFGAS_GEPattern::HatchStyle iHatch = CFGAS_GEPattern::HatchStyle::Cross;
  switch (GetType()) {
    case XFA_AttributeValue::CrossDiagonal:
      iHatch = CFGAS_GEPattern::HatchStyle::DiagonalCross;
      break;
    case XFA_AttributeValue::DiagonalLeft:
      iHatch = CFGAS_GEPattern::HatchStyle::ForwardDiagonal;
      break;
    case XFA_AttributeValue::DiagonalRight:
      iHatch = CFGAS_GEPattern::HatchStyle::BackwardDiagonal;
      break;
    case XFA_AttributeValue::Horizontal:
      iHatch = CFGAS_GEPattern::HatchStyle::Horizontal;
      break;
    case XFA_AttributeValue::Vertical:
      iHatch = CFGAS_GEPattern::HatchStyle::Vertical;
      break;
    default:
      break;
  }

  CFGAS_GEPattern pattern(iHatch, crEnd, crStart);
  CFGAS_GEGraphics::StateRestorer restorer(pGS);
  pGS->SetFillColor(CFGAS_GEColor(&pattern, 0x0));
  pGS->FillPath(fillPath, CFX_FillRenderOptions::FillType::kWinding, matrix);
}
