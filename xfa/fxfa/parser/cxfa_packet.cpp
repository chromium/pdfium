// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_packet.h"

#include "fxjs/xfa/cjx_packet.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CXFA_Packet::CXFA_Packet(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kXdp,
                XFA_ObjectType::NodeC,
                XFA_Element::Packet,
                {},
                {},
                cppgc::MakeGarbageCollected<CJX_Packet>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Packet::~CXFA_Packet() = default;
