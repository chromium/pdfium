// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_meridiemnames.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kMeridiemNamesPropertyData[] = {
    {XFA_Element::Meridiem, 2, {}},
};

}  // namespace

CXFA_MeridiemNames::CXFA_MeridiemNames(CXFA_Document* doc,
                                       XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kLocaleSet,
                XFA_ObjectType::Node,
                XFA_Element::MeridiemNames,
                kMeridiemNamesPropertyData,
                {},
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_MeridiemNames::~CXFA_MeridiemNames() = default;
