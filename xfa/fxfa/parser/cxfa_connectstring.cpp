// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_connectstring.h"

#include "fxjs/xfa/cjx_textnode.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kConnectStringAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_ConnectString::CXFA_ConnectString(CXFA_Document* doc,
                                       XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kSourceSet,
                XFA_ObjectType::TextNode,
                XFA_Element::ConnectString,
                {},
                kConnectStringAttributeData,
                cppgc::MakeGarbageCollected<CJX_TextNode>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_ConnectString::~CXFA_ConnectString() = default;
