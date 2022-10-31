// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_decimal.h"

#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kDecimalAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::FracDigits, XFA_AttributeType::Integer, (void*)2},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::LeadDigits, XFA_AttributeType::Integer, (void*)-1},
};

}  // namespace

CXFA_Decimal::CXFA_Decimal(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::ContentNode,
                XFA_Element::Decimal,
                {},
                kDecimalAttributeData,
                cppgc::MakeGarbageCollected<CJX_Object>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Decimal::~CXFA_Decimal() = default;
