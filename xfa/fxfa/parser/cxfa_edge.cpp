// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_edge.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kEdgePropertyData[] = {
    {XFA_Element::Color, 1, {}},
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kEdgeAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Cap, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Square},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Stroke, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Solid},
    {XFA_Attribute::Presence, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Visible},
    {XFA_Attribute::Thickness, XFA_AttributeType::Measure, (void*)L"0.5pt"},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Edge::CXFA_Edge(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Stroke(doc,
                  packet,
                  {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                  XFA_ObjectType::Node,
                  XFA_Element::Edge,
                  kEdgePropertyData,
                  kEdgeAttributeData,
                  cppgc::MakeGarbageCollected<CJX_Node>(
                      doc->GetHeap()->GetAllocationHandle(),
                      this)) {}

CXFA_Edge::~CXFA_Edge() = default;
