// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_line.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_edge.h"
#include "xfa/fxfa/parser/cxfa_node.h"

namespace {

const CXFA_Node::PropertyData kLinePropertyData[] = {
    {XFA_Element::Edge, 1, {}},
};

const CXFA_Node::AttributeData kLineAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Slope, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Backslash},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Hand, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Even},
};

}  // namespace

// static
CXFA_Line* CXFA_Line::FromNode(CXFA_Node* pNode) {
  return pNode && pNode->GetElementType() == XFA_Element::Line
             ? static_cast<CXFA_Line*>(pNode)
             : nullptr;
}

CXFA_Line::CXFA_Line(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Line,
                kLinePropertyData,
                kLineAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Line::~CXFA_Line() = default;

XFA_AttributeValue CXFA_Line::GetHand() {
  return JSObject()->GetEnum(XFA_Attribute::Hand);
}

bool CXFA_Line::GetSlope() {
  return JSObject()->GetEnum(XFA_Attribute::Slope) == XFA_AttributeValue::Slash;
}

CXFA_Edge* CXFA_Line::GetEdgeIfExists() {
  return GetChild<CXFA_Edge>(0, XFA_Element::Edge, false);
}
