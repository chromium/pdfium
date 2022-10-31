// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_connect.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kConnectPropertyData[] = {
    {XFA_Element::Picture, 1, {}},
    {XFA_Element::ConnectString, 1, {}},
    {XFA_Element::User, 1, {}},
    {XFA_Element::Password, 1, {}},
};

const CXFA_Node::AttributeData kConnectAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Ref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Timeout, XFA_AttributeType::Integer, (void*)15},
    {XFA_Attribute::Connection, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usage, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::ExportAndImport},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::DelayedOpen, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Connect::CXFA_Connect(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kSourceSet, XFA_XDPPACKET::kTemplate,
                 XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Connect,
                kConnectPropertyData,
                kConnectAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Connect::~CXFA_Connect() = default;
