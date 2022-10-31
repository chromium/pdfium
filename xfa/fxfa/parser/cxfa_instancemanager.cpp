// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_instancemanager.h"

#include "fxjs/xfa/cjx_instancemanager.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kInstanceManagerPropertyData[] = {
    {XFA_Element::Occur, 1, {}},
};

const CXFA_Node::AttributeData kInstanceManagerAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_InstanceManager::CXFA_InstanceManager(CXFA_Document* doc,
                                           XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kForm,
                XFA_ObjectType::Node,
                XFA_Element::InstanceManager,
                kInstanceManagerPropertyData,
                kInstanceManagerAttributeData,
                cppgc::MakeGarbageCollected<CJX_InstanceManager>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_InstanceManager::~CXFA_InstanceManager() = default;
