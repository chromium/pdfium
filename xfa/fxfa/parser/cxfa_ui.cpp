// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_ui.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

constexpr CXFA_Node::PropertyData kUiPropertyData[] = {
    {XFA_Element::CheckButton, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::ChoiceList, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::DefaultUi, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::Barcode, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::Button, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::DateTimeEdit, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::Picture, 1, {}},
    {XFA_Element::ImageEdit, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::PasswordEdit, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::NumericEdit, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::Signature, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::TextEdit, 1, {XFA_PropertyFlag::kOneOf}},
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kUiAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Ui::CXFA_Ui(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Ui,
                kUiPropertyData,
                kUiAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Ui::~CXFA_Ui() = default;

bool CXFA_Ui::IsAOneOfChild(CXFA_Node* child) const {
  for (auto& prop : kUiPropertyData) {
    if (prop.property != child->GetElementType())
      continue;
    if (!!(prop.flags & XFA_PropertyFlag::kOneOf))
      return true;
  }
  return false;
}
