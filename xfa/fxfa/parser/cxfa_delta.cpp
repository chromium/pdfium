// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_delta.h"

#include "fxjs/xfa/cjx_delta.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CXFA_Delta::CXFA_Delta(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kForm,
                XFA_ObjectType::Object,
                XFA_Element::Delta,
                {},
                {},
                cppgc::MakeGarbageCollected<CJX_Delta>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Delta::~CXFA_Delta() = default;
