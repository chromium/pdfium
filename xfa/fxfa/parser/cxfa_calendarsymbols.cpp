// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_calendarsymbols.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kCalendarSymbolsPropertyData[] = {
    {XFA_Element::EraNames, 1, {}},
    {XFA_Element::DayNames, 2, {}},
    {XFA_Element::MeridiemNames, 1, {}},
    {XFA_Element::MonthNames, 2, {}},
};

const CXFA_Node::AttributeData kCalendarSymbolsAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Gregorian},
};

}  // namespace

CXFA_CalendarSymbols::CXFA_CalendarSymbols(CXFA_Document* doc,
                                           XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kLocaleSet,
                XFA_ObjectType::Node,
                XFA_Element::CalendarSymbols,
                kCalendarSymbolsPropertyData,
                kCalendarSymbolsAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_CalendarSymbols::~CXFA_CalendarSymbols() = default;
