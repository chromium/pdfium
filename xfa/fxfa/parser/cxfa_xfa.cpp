// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_xfa.h"

namespace {

const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::TimeStamp, XFA_AttributeType::CData, XFA_XDPPACKET_XDP,
     nullptr},
    {XFA_Attribute::Uuid, XFA_AttributeType::CData, XFA_XDPPACKET_XDP, nullptr},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, 0, nullptr}};

constexpr wchar_t kName[] = L"xfa";

}  // namespace

CXFA_Xfa::CXFA_Xfa(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_XDP,
                XFA_ObjectType::ModelNode,
                XFA_Element::Xfa,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_Xfa::~CXFA_Xfa() {}
