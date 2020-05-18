// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_instancemanager.h"

#include <memory>

#include "fxjs/xfa/cjx_instancemanager.h"

namespace {

const CXFA_Node::PropertyData kInstanceManagerPropertyData[] = {
    {XFA_Element::Occur, 1, 0},
};

const CXFA_Node::AttributeData kInstanceManagerAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_InstanceManager::CXFA_InstanceManager(CXFA_Document* doc,
                                           XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Form,
                XFA_ObjectType::Node,
                XFA_Element::InstanceManager,
                kInstanceManagerPropertyData,
                kInstanceManagerAttributeData,
                std::make_unique<CJX_InstanceManager>(this)) {}

CXFA_InstanceManager::~CXFA_InstanceManager() = default;
