// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_labelprinter.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kLabelPrinterPropertyData[] = {
    {XFA_Element::FontInfo, 1, {}},
    {XFA_Element::Xdc, 1, {}},
    {XFA_Element::BatchOutput, 1, {}},
    {XFA_Element::FlipLabel, 1, {}},
};

const CXFA_Node::AttributeData kLabelPrinterAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Zpl},
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_LabelPrinter::CXFA_LabelPrinter(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kConfig,
                XFA_ObjectType::Node,
                XFA_Element::LabelPrinter,
                kLabelPrinterPropertyData,
                kLabelPrinterAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_LabelPrinter::~CXFA_LabelPrinter() = default;
