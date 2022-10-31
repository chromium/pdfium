// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_bind.h"

#include "fxjs/xfa/cjx_node.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_picture.h"

namespace {

const CXFA_Node::PropertyData kBindPropertyData[] = {
    {XFA_Element::Picture, 1, {}},
};

const CXFA_Node::AttributeData kBindAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Ref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::ContentType, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::TransferEncoding, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::None},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Match, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Once},
};

}  // namespace

CXFA_Bind::CXFA_Bind(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kSourceSet, XFA_XDPPACKET::kTemplate,
                 XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Bind,
                kBindPropertyData,
                kBindAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Bind::~CXFA_Bind() = default;

WideString CXFA_Bind::GetPicture() const {
  const auto* pPicture = GetChild<CXFA_Picture>(0, XFA_Element::Picture, false);
  return pPicture ? pPicture->JSObject()->GetContent(false) : WideString();
}
