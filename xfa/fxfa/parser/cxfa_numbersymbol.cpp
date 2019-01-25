// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_numbersymbol.h"

#include "fxjs/xfa/cjx_node.h"
#include "third_party/base/ptr_util.h"

namespace {

const CXFA_Node::AttributeData kNumberSymbolAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Decimal},
};

}  // namespace

CXFA_NumberSymbol::CXFA_NumberSymbol(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_LocaleSet,
                XFA_ObjectType::ContentNode,
                XFA_Element::NumberSymbol,
                {},
                kNumberSymbolAttributeData,
                pdfium::MakeUnique<CJX_Node>(this)) {}

CXFA_NumberSymbol::~CXFA_NumberSymbol() = default;
