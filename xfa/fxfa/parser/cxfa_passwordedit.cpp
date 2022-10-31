// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_passwordedit.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kPasswordEditPropertyData[] = {
    {XFA_Element::Margin, 1, {}},
    {XFA_Element::Border, 1, {}},
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kPasswordEditAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::PasswordChar, XFA_AttributeType::CData, (void*)L"*"},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::HScrollPolicy, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Auto},
};

}  // namespace

// static
CXFA_PasswordEdit* CXFA_PasswordEdit::FromNode(CXFA_Node* pNode) {
  return pNode && pNode->GetElementType() == XFA_Element::PasswordEdit
             ? static_cast<CXFA_PasswordEdit*>(pNode)
             : nullptr;
}

CXFA_PasswordEdit::CXFA_PasswordEdit(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::PasswordEdit,
                kPasswordEditPropertyData,
                kPasswordEditAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_PasswordEdit::~CXFA_PasswordEdit() = default;

XFA_FFWidgetType CXFA_PasswordEdit::GetDefaultFFWidgetType() const {
  return XFA_FFWidgetType::kPasswordEdit;
}

WideString CXFA_PasswordEdit::GetPasswordChar() {
  return JSObject()->GetCData(XFA_Attribute::PasswordChar);
}
