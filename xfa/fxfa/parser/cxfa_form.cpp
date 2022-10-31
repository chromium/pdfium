// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_form.h"

#include "fxjs/xfa/cjx_form.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kFormAttributeData[] = {
    {XFA_Attribute::Checksum, XFA_AttributeType::CData, (void*)nullptr},
};

}  // namespace

CXFA_Form::CXFA_Form(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kForm,
                XFA_ObjectType::ModelNode,
                XFA_Element::Form,
                {},
                kFormAttributeData,
                cppgc::MakeGarbageCollected<CJX_Form>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Form::~CXFA_Form() = default;
