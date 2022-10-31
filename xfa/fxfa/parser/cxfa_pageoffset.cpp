// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pageoffset.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kPageOffsetAttributeData[] = {
    {XFA_Attribute::X, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::Y, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_PageOffset::CXFA_PageOffset(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kConfig,
                XFA_ObjectType::Node,
                XFA_Element::PageOffset,
                {},
                kPageOffsetAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_PageOffset::~CXFA_PageOffset() = default;
