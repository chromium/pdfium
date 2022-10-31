// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_choicelist.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kChoiceListPropertyData[] = {
    {XFA_Element::Margin, 1, {}},
    {XFA_Element::Border, 1, {}},
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kChoiceListAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Open, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::UserControl},
    {XFA_Attribute::CommitOn, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Select},
    {XFA_Attribute::TextEntry, XFA_AttributeType::Boolean, (void*)0},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_ChoiceList::CXFA_ChoiceList(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::ChoiceList,
                kChoiceListPropertyData,
                kChoiceListAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_ChoiceList::~CXFA_ChoiceList() = default;

XFA_Element CXFA_ChoiceList::GetValueNodeType() const {
  return JSObject()->GetEnum(XFA_Attribute::Open) ==
                 XFA_AttributeValue::MultiSelect
             ? XFA_Element::ExData
             : XFA_Element::Text;
}

XFA_FFWidgetType CXFA_ChoiceList::GetDefaultFFWidgetType() const {
  return XFA_FFWidgetType::kChoiceList;
}
