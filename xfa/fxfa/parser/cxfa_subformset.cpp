// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_subformset.h"

#include "fxjs/xfa/cjx_container.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kSubformSetPropertyData[] = {
    {XFA_Element::Break, 1, {}},  {XFA_Element::Overflow, 1, {}},
    {XFA_Element::Desc, 1, {}},   {XFA_Element::Bookend, 1, {}},
    {XFA_Element::Extras, 1, {}}, {XFA_Element::Occur, 1, {}},
};

const CXFA_Node::AttributeData kSubformSetAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Relation, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Ordered},
    {XFA_Attribute::Relevant, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_SubformSet::CXFA_SubformSet(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::ContainerNode,
                XFA_Element::SubformSet,
                kSubformSetPropertyData,
                kSubformSetAttributeData,
                cppgc::MakeGarbageCollected<CJX_Container>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_SubformSet::~CXFA_SubformSet() = default;
