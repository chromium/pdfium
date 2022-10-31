// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_uri.h"

#include "fxjs/xfa/cjx_textnode.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kUriAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_Uri::CXFA_Uri(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kConfig, XFA_XDPPACKET::kConnectionSet},
                XFA_ObjectType::TextNode,
                XFA_Element::Uri,
                {},
                kUriAttributeData,
                cppgc::MakeGarbageCollected<CJX_TextNode>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Uri::~CXFA_Uri() = default;
