// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xfa.h"

#include "fxjs/xfa/cjx_xfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kXfaAttributeData[] = {
    {XFA_Attribute::TimeStamp, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Uuid, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Xfa::CXFA_Xfa(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kXdp,
                XFA_ObjectType::ModelNode,
                XFA_Element::Xfa,
                {},
                kXfaAttributeData,
                cppgc::MakeGarbageCollected<CJX_Xfa>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Xfa::~CXFA_Xfa() = default;
