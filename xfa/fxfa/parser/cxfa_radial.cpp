// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_radial.h"

#include <math.h>

#include <utility>

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fgas/graphics/cfgas_geshading.h"
#include "xfa/fxfa/parser/cxfa_color.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kRadialPropertyData[] = {
    {XFA_Element::Color, 1, {}},
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kRadialAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Type, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::ToEdge},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Radial::CXFA_Radial(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Radial,
                kRadialPropertyData,
                kRadialAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Radial::~CXFA_Radial() = default;

bool CXFA_Radial::IsToEdge() {
  auto value = JSObject()->TryEnum(XFA_Attribute::Type, true);
  return !value.has_value() || value.value() == XFA_AttributeValue::ToEdge;
}

CXFA_Color* CXFA_Radial::GetColorIfExists() {
  return GetChild<CXFA_Color>(0, XFA_Element::Color, false);
}

void CXFA_Radial::Draw(CFGAS_GEGraphics* pGS,
                       const CFGAS_GEPath& fillPath,
                       FX_ARGB crStart,
                       const CFX_RectF& rtFill,
                       const CFX_Matrix& matrix) {
  CXFA_Color* pColor = GetColorIfExists();
  FX_ARGB crEnd = pColor ? pColor->GetValue() : CXFA_Color::kBlackColor;
  if (!IsToEdge())
    std::swap(crStart, crEnd);

  float end_radius = FXSYS_sqrt2(rtFill.Width(), rtFill.Height()) / 2;
  CFGAS_GEShading shading(rtFill.Center(), rtFill.Center(), 0, end_radius, true,
                          true, crStart, crEnd);

  CFGAS_GEGraphics::StateRestorer restorer(pGS);
  pGS->SetFillColor(CFGAS_GEColor(&shading));
  pGS->FillPath(fillPath, CFX_FillRenderOptions::FillType::kWinding, matrix);
}
