// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_template.h"

#include "fxjs/xfa/cjx_template.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kTemplatePropertyData[] = {
    {XFA_Element::Uri, 1, {}},       {XFA_Element::Xsl, 1, {}},
    {XFA_Element::StartPage, 1, {}}, {XFA_Element::Relevant, 1, {}},
    {XFA_Element::Base, 1, {}},      {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kTemplateAttributeData[] = {
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::BaseProfile, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Full},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_Template::CXFA_Template(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kConfig, XFA_XDPPACKET::kTemplate,
                 XFA_XDPPACKET::kForm},
                XFA_ObjectType::ModelNode,
                XFA_Element::Template,
                kTemplatePropertyData,
                kTemplateAttributeData,
                cppgc::MakeGarbageCollected<CJX_Template>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Template::~CXFA_Template() = default;
