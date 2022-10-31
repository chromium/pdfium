// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_exdata.h"

#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kExDataAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Rid, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::ContentType, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::TransferEncoding, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::None},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::MaxLength, XFA_AttributeType::Integer, (void*)-1},
    {XFA_Attribute::Href, XFA_AttributeType::CData, nullptr},
};

}  // namespace

// static
CXFA_ExData* CXFA_ExData::FromNode(CXFA_Node* pNode) {
  return pNode && pNode->GetElementType() == XFA_Element::ExData
             ? static_cast<CXFA_ExData*>(pNode)
             : nullptr;
}

CXFA_ExData::CXFA_ExData(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::ContentNode,
                XFA_Element::ExData,
                {},
                kExDataAttributeData,
                cppgc::MakeGarbageCollected<CJX_Object>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_ExData::~CXFA_ExData() = default;

void CXFA_ExData::SetContentType(const WideString& wsContentType) {
  JSObject()->SetCData(XFA_Attribute::ContentType, wsContentType);
}
