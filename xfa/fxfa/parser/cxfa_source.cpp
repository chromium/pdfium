// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_source.h"

#include "fxjs/xfa/cjx_source.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kSourcePropertyData[] = {
    {XFA_Element::Connect, 1, {}},
};

const CXFA_Node::AttributeData kSourceAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Source::CXFA_Source(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kSourceSet,
                XFA_ObjectType::Node,
                XFA_Element::Source,
                kSourcePropertyData,
                kSourceAttributeData,
                cppgc::MakeGarbageCollected<CJX_Source>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Source::~CXFA_Source() = default;
