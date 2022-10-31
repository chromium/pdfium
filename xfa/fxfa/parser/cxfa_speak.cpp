// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_speak.h"

#include "fxjs/xfa/cjx_textnode.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kSpeakAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Rid, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Priority, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Custom},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Disable, XFA_AttributeType::Boolean, (void*)0},
};

}  // namespace

CXFA_Speak::CXFA_Speak(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::TextNode,
                XFA_Element::Speak,
                {},
                kSpeakAttributeData,
                cppgc::MakeGarbageCollected<CJX_TextNode>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Speak::~CXFA_Speak() = default;
