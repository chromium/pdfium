// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_boolean.h"

#include "fxjs/xfa/cjx_boolean.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kBooleanAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Boolean::CXFA_Boolean(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kSourceSet, XFA_XDPPACKET::kTemplate,
                 XFA_XDPPACKET::kForm},
                XFA_ObjectType::ContentNode,
                XFA_Element::Boolean,
                {},
                kBooleanAttributeData,
                cppgc::MakeGarbageCollected<CJX_Boolean>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Boolean::~CXFA_Boolean() = default;
