// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_wsdlconnection.h"

#include "fxjs/xfa/cjx_wsdlconnection.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kWsdlConnectionPropertyData[] = {
    {XFA_Element::Operation, 1, {}},
    {XFA_Element::WsdlAddress, 1, {}},
    {XFA_Element::SoapAddress, 1, {}},
    {XFA_Element::SoapAction, 1, {}},
    {XFA_Element::EffectiveOutputPolicy, 1, {}},
    {XFA_Element::EffectiveInputPolicy, 1, {}},
};

const CXFA_Node::AttributeData kWsdlConnectionAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::DataDescription, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_WsdlConnection::CXFA_WsdlConnection(CXFA_Document* doc,
                                         XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kConnectionSet,
                XFA_ObjectType::Node,
                XFA_Element::WsdlConnection,
                kWsdlConnectionPropertyData,
                kWsdlConnectionAttributeData,
                cppgc::MakeGarbageCollected<CJX_WsdlConnection>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_WsdlConnection::~CXFA_WsdlConnection() = default;
