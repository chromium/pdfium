// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xmlconnection.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kXmlConnectionPropertyData[] = {
    {XFA_Element::Uri, 1, {}},
};

const CXFA_Node::AttributeData kXmlConnectionAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::DataDescription, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_XmlConnection::CXFA_XmlConnection(CXFA_Document* doc,
                                       XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kConnectionSet,
                XFA_ObjectType::Node,
                XFA_Element::XmlConnection,
                kXmlConnectionPropertyData,
                kXmlConnectionAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_XmlConnection::~CXFA_XmlConnection() = default;
