// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_desc.h"

#include "fxjs/xfa/cjx_desc.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kDescPropertyData[] = {
    {XFA_Element::Text, 1, {}},     {XFA_Element::Time, 1, {}},
    {XFA_Element::DateTime, 1, {}}, {XFA_Element::Image, 1, {}},
    {XFA_Element::Decimal, 1, {}},  {XFA_Element::Boolean, 1, {}},
    {XFA_Element::Integer, 1, {}},  {XFA_Element::ExData, 1, {}},
    {XFA_Element::Date, 1, {}},     {XFA_Element::Float, 1, {}},
};

const CXFA_Node::AttributeData kDescAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Desc::CXFA_Desc(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Desc,
                kDescPropertyData,
                kDescAttributeData,
                cppgc::MakeGarbageCollected<CJX_Desc>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Desc::~CXFA_Desc() = default;
