// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_filter.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kFilterPropertyData[] = {
    {XFA_Element::Mdp, 1, {}},           {XFA_Element::Certificates, 1, {}},
    {XFA_Element::TimeStamp, 1, {}},     {XFA_Element::Handler, 1, {}},
    {XFA_Element::DigestMethods, 1, {}}, {XFA_Element::Encodings, 1, {}},
    {XFA_Element::Reasons, 1, {}},       {XFA_Element::AppearanceFilter, 1, {}},
    {XFA_Element::LockDocument, 1, {}},
};

const CXFA_Node::AttributeData kFilterAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Version, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::AddRevocationInfo, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Filter::CXFA_Filter(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Filter,
                kFilterPropertyData,
                kFilterAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Filter::~CXFA_Filter() = default;
