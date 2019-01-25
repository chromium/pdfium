// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_dsigdata.h"

#include "fxjs/xfa/cjx_node.h"
#include "third_party/base/ptr_util.h"

namespace {

const CXFA_Node::AttributeData kDSigDataAttributeData[] = {
    {XFA_Attribute::Value, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_DSigData::CXFA_DSigData(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::DSigData,
                {},
                kDSigDataAttributeData,
                pdfium::MakeUnique<CJX_Node>(this)) {}

CXFA_DSigData::~CXFA_DSigData() = default;
