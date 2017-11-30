// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_meridiemnames.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Meridiem, 2, 0},
                                                 {XFA_Element::Unknown, 0, 0}};

constexpr wchar_t kName[] = L"meridiemNames";

}  // namespace

CXFA_MeridiemNames::CXFA_MeridiemNames(CXFA_Document* doc,
                                       XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_LocaleSet,
                XFA_ObjectType::Node,
                XFA_Element::MeridiemNames,
                kPropertyData,
                nullptr,
                kName) {}

CXFA_MeridiemNames::~CXFA_MeridiemNames() {}
