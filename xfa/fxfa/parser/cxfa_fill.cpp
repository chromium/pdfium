// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_fill.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fxfa/parser/cxfa_color.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_linear.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_pattern.h"
#include "xfa/fxfa/parser/cxfa_radial.h"
#include "xfa/fxfa/parser/cxfa_stipple.h"

namespace {

const CXFA_Node::PropertyData kFillPropertyData[] = {
    {XFA_Element::Pattern, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::Solid,
     1,
     {XFA_PropertyFlag::kOneOf, XFA_PropertyFlag::kDefaultOneOf}},
    {XFA_Element::Stipple, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::Color, 1, {}},
    {XFA_Element::Linear, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::Extras, 1, {}},
    {XFA_Element::Radial, 1, {XFA_PropertyFlag::kOneOf}},
};

const CXFA_Node::AttributeData kFillAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Presence, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Visible},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Fill::CXFA_Fill(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Fill,
                kFillPropertyData,
                kFillAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Fill::~CXFA_Fill() = default;

bool CXFA_Fill::IsVisible() {
  return JSObject()
             ->TryEnum(XFA_Attribute::Presence, true)
             .value_or(XFA_AttributeValue::Visible) ==
         XFA_AttributeValue::Visible;
}

void CXFA_Fill::SetColor(FX_ARGB color) {
  CXFA_Color* pColor =
      JSObject()->GetOrCreateProperty<CXFA_Color>(0, XFA_Element::Color);
  if (!pColor)
    return;

  pColor->SetValue(color);
}

FX_ARGB CXFA_Fill::GetFillColor() const {
  const auto* pColor = GetChild<CXFA_Color>(0, XFA_Element::Color, false);
  return pColor ? pColor->GetValueOrDefault(0xFFFFFFFF) : 0xFFFFFFFF;
}

FX_ARGB CXFA_Fill::GetTextColor() const {
  const auto* pColor = GetChild<CXFA_Color>(0, XFA_Element::Color, false);
  return pColor ? pColor->GetValueOrDefault(0xFF000000) : 0xFF000000;
}

XFA_Element CXFA_Fill::GetType() const {
  CXFA_Node* pChild = GetFirstChild();
  while (pChild) {
    XFA_Element eType = pChild->GetElementType();
    if (eType != XFA_Element::Color && eType != XFA_Element::Extras)
      return eType;

    pChild = pChild->GetNextSibling();
  }
  return XFA_Element::Solid;
}

void CXFA_Fill::Draw(CFGAS_GEGraphics* pGS,
                     const CFGAS_GEPath& fillPath,
                     const CFX_RectF& rtWidget,
                     const CFX_Matrix& matrix) {
  CFGAS_GEGraphics::StateRestorer restorer(pGS);
  switch (GetType()) {
    case XFA_Element::Radial:
      DrawRadial(pGS, fillPath, rtWidget, matrix);
      break;
    case XFA_Element::Pattern:
      DrawPattern(pGS, fillPath, rtWidget, matrix);
      break;
    case XFA_Element::Linear:
      DrawLinear(pGS, fillPath, rtWidget, matrix);
      break;
    case XFA_Element::Stipple:
      DrawStipple(pGS, fillPath, rtWidget, matrix);
      break;
    default:
      pGS->SetFillColor(CFGAS_GEColor(GetFillColor()));
      pGS->FillPath(fillPath, CFX_FillRenderOptions::FillType::kWinding,
                    matrix);
      break;
  }
}

void CXFA_Fill::DrawStipple(CFGAS_GEGraphics* pGS,
                            const CFGAS_GEPath& fillPath,
                            const CFX_RectF& rtWidget,
                            const CFX_Matrix& matrix) {
  CXFA_Stipple* stipple =
      JSObject()->GetOrCreateProperty<CXFA_Stipple>(0, XFA_Element::Stipple);
  if (stipple)
    stipple->Draw(pGS, fillPath, rtWidget, matrix);
}

void CXFA_Fill::DrawRadial(CFGAS_GEGraphics* pGS,
                           const CFGAS_GEPath& fillPath,
                           const CFX_RectF& rtWidget,
                           const CFX_Matrix& matrix) {
  CXFA_Radial* radial =
      JSObject()->GetOrCreateProperty<CXFA_Radial>(0, XFA_Element::Radial);
  if (radial)
    radial->Draw(pGS, fillPath, GetFillColor(), rtWidget, matrix);
}

void CXFA_Fill::DrawLinear(CFGAS_GEGraphics* pGS,
                           const CFGAS_GEPath& fillPath,
                           const CFX_RectF& rtWidget,
                           const CFX_Matrix& matrix) {
  CXFA_Linear* linear =
      JSObject()->GetOrCreateProperty<CXFA_Linear>(0, XFA_Element::Linear);
  if (linear)
    linear->Draw(pGS, fillPath, GetFillColor(), rtWidget, matrix);
}

void CXFA_Fill::DrawPattern(CFGAS_GEGraphics* pGS,
                            const CFGAS_GEPath& fillPath,
                            const CFX_RectF& rtWidget,
                            const CFX_Matrix& matrix) {
  CXFA_Pattern* pattern =
      JSObject()->GetOrCreateProperty<CXFA_Pattern>(0, XFA_Element::Pattern);
  if (pattern)
    pattern->Draw(pGS, fillPath, GetFillColor(), rtWidget, matrix);
}
