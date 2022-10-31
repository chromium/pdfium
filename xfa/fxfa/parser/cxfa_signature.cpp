// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_signature.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kSignaturePropertyData[] = {
    {XFA_Element::Margin, 1, {}}, {XFA_Element::Filter, 1, {}},
    {XFA_Element::Border, 1, {}}, {XFA_Element::Manifest, 1, {}},
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kSignatureAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Type, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::PDF1_3},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Signature::CXFA_Signature(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Signature,
                kSignaturePropertyData,
                kSignatureAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Signature::~CXFA_Signature() = default;

XFA_FFWidgetType CXFA_Signature::GetDefaultFFWidgetType() const {
  return XFA_FFWidgetType::kSignature;
}
