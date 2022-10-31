// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_data.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kDataPropertyData[] = {
    {XFA_Element::Uri, 1, {}},        {XFA_Element::Xsl, 1, {}},
    {XFA_Element::StartNode, 1, {}},  {XFA_Element::OutputXSL, 1, {}},
    {XFA_Element::AdjustData, 1, {}}, {XFA_Element::Attributes, 1, {}},
    {XFA_Element::Window, 1, {}},     {XFA_Element::Record, 1, {}},
    {XFA_Element::Range, 1, {}},      {XFA_Element::IncrementalLoad, 1, {}},
};

const CXFA_Node::AttributeData kDataAttributeData[] = {
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_Data::CXFA_Data(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kConfig,
                XFA_ObjectType::Node,
                XFA_Element::Data,
                kDataPropertyData,
                kDataAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Data::~CXFA_Data() = default;
