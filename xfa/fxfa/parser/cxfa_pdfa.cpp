// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pdfa.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kPdfaPropertyData[] = {
    {XFA_Element::Amd, 1, {}},
    {XFA_Element::Part, 1, {}},
    {XFA_Element::IncludeXDPContent, 1, {}},
    {XFA_Element::Conformance, 1, {}},
};

const CXFA_Node::AttributeData kPdfaAttributeData[] = {
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_Pdfa::CXFA_Pdfa(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kConfig,
                XFA_ObjectType::Node,
                XFA_Element::Pdfa,
                kPdfaPropertyData,
                kPdfaAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Pdfa::~CXFA_Pdfa() = default;
