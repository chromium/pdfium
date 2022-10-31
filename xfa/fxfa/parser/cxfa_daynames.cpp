// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_daynames.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kDayNamesPropertyData[] = {
    {XFA_Element::Day, 7, {}},
};

const CXFA_Node::AttributeData kDayNamesAttributeData[] = {
    {XFA_Attribute::Abbr, XFA_AttributeType::Boolean, (void*)0},
};

}  // namespace

CXFA_DayNames::CXFA_DayNames(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kLocaleSet,
                XFA_ObjectType::Node,
                XFA_Element::DayNames,
                kDayNamesPropertyData,
                kDayNamesAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_DayNames::~CXFA_DayNames() = default;
