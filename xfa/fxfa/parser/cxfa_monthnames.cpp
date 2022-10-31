// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_monthnames.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kMonthNamesPropertyData[] = {
    {XFA_Element::Month, 12, {}},
};

const CXFA_Node::AttributeData kMonthNamesAttributeData[] = {
    {XFA_Attribute::Abbr, XFA_AttributeType::Boolean, (void*)0},
};

}  // namespace

CXFA_MonthNames::CXFA_MonthNames(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kLocaleSet,
                XFA_ObjectType::Node,
                XFA_Element::MonthNames,
                kMonthNamesPropertyData,
                kMonthNamesAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_MonthNames::~CXFA_MonthNames() = default;
