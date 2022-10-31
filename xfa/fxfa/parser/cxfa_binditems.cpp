// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_binditems.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kBindItemsAttributeData[] = {
    {XFA_Attribute::Ref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Connection, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::LabelRef, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::ValueRef, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_BindItems::CXFA_BindItems(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::BindItems,
                {},
                kBindItemsAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_BindItems::~CXFA_BindItems() = default;

WideString CXFA_BindItems::GetLabelRef() {
  return JSObject()->GetCData(XFA_Attribute::LabelRef);
}

WideString CXFA_BindItems::GetValueRef() {
  return JSObject()->GetCData(XFA_Attribute::ValueRef);
}

WideString CXFA_BindItems::GetRef() {
  return JSObject()->GetCData(XFA_Attribute::Ref);
}
