// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_connectionset.h"

#include "fxjs/xfa/cjx_model.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CXFA_ConnectionSet::CXFA_ConnectionSet(CXFA_Document* doc,
                                       XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kConnectionSet,
                XFA_ObjectType::ModelNode,
                XFA_Element::ConnectionSet,
                {},
                {},
                cppgc::MakeGarbageCollected<CJX_Model>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_ConnectionSet::~CXFA_ConnectionSet() = default;
