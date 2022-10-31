// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_locale.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kLocalePropertyData[] = {
    {XFA_Element::DatePatterns, 1, {}},
    {XFA_Element::CalendarSymbols, 1, {}},
    {XFA_Element::CurrencySymbols, 1, {}},
    {XFA_Element::Typefaces, 1, {}},
    {XFA_Element::DateTimeSymbols, 1, {}},
    {XFA_Element::NumberPatterns, 1, {}},
    {XFA_Element::NumberSymbols, 1, {}},
    {XFA_Element::TimePatterns, 1, {}},
};

const CXFA_Node::AttributeData kLocaleAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_Locale::CXFA_Locale(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kConfig, XFA_XDPPACKET::kLocaleSet},
                XFA_ObjectType::Node,
                XFA_Element::Locale,
                kLocalePropertyData,
                kLocaleAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Locale::~CXFA_Locale() = default;
