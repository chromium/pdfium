// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_sharpxhtml.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kSharpxHTMLAttributeData[] = {
    {XFA_Attribute::Value, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_SharpxHTML::CXFA_SharpxHTML(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kConfig,
                 XFA_XDPPACKET::kLocaleSet, XFA_XDPPACKET::kConnectionSet,
                 XFA_XDPPACKET::kSourceSet, XFA_XDPPACKET::kForm},
                XFA_ObjectType::NodeV,
                XFA_Element::SharpxHTML,
                {},
                kSharpxHTMLAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_SharpxHTML::~CXFA_SharpxHTML() = default;
