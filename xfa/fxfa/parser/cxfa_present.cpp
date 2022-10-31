// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_present.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kPresentPropertyData[] = {
    {XFA_Element::Xdp, 1, {}},
    {XFA_Element::Cache, 1, {}},
    {XFA_Element::Pagination, 1, {}},
    {XFA_Element::Overprint, 1, {}},
    {XFA_Element::BehaviorOverride, 1, {}},
    {XFA_Element::Copies, 1, {}},
    {XFA_Element::Output, 1, {}},
    {XFA_Element::Validate, 1, {}},
    {XFA_Element::Layout, 1, {}},
    {XFA_Element::Script, 1, {}},
    {XFA_Element::Common, 1, {}},
    {XFA_Element::PaginationOverride, 1, {}},
    {XFA_Element::Destination, 1, {}},
    {XFA_Element::IncrementalMerge, 1, {}},
};

const CXFA_Node::AttributeData kPresentAttributeData[] = {
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_Present::CXFA_Present(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kConfig,
                XFA_ObjectType::Node,
                XFA_Element::Present,
                kPresentPropertyData,
                kPresentAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Present::~CXFA_Present() = default;
