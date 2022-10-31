// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_transform.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kTransformPropertyData[] = {
    {XFA_Element::Whitespace, 1, {}},  {XFA_Element::Rename, 1, {}},
    {XFA_Element::IfEmpty, 1, {}},     {XFA_Element::Presence, 1, {}},
    {XFA_Element::Picture, 1, {}},     {XFA_Element::NameAttr, 1, {}},
    {XFA_Element::GroupParent, 1, {}},
};

const CXFA_Node::AttributeData kTransformAttributeData[] = {
    {XFA_Attribute::Ref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_Transform::CXFA_Transform(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kConfig,
                XFA_ObjectType::Node,
                XFA_Element::Transform,
                kTransformPropertyData,
                kTransformAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Transform::~CXFA_Transform() = default;
