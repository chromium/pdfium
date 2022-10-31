// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_encrypt.h"

#include "fxjs/xfa/cjx_encrypt.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kEncryptPropertyData[] = {
    {XFA_Element::Certificate, 1, {}},
};

const CXFA_Node::AttributeData kEncryptAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_Encrypt::CXFA_Encrypt(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kConfig,
                 XFA_XDPPACKET::kForm},
                XFA_ObjectType::ContentNode,
                XFA_Element::Encrypt,
                kEncryptPropertyData,
                kEncryptAttributeData,
                cppgc::MakeGarbageCollected<CJX_Encrypt>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Encrypt::~CXFA_Encrypt() = default;
