// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_currencysymbols.h"

#include "fxjs/xfa/cjx_node.h"
#include "third_party/base/ptr_util.h"

namespace {

const CXFA_Node::PropertyData kCurrencySymbolsPropertyData[] = {
    {XFA_Element::CurrencySymbol, 3, 0},
};

}  // namespace

CXFA_CurrencySymbols::CXFA_CurrencySymbols(CXFA_Document* doc,
                                           XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_LocaleSet,
                XFA_ObjectType::Node,
                XFA_Element::CurrencySymbols,
                kCurrencySymbolsPropertyData,
                {},
                pdfium::MakeUnique<CJX_Node>(this)) {}

CXFA_CurrencySymbols::~CXFA_CurrencySymbols() = default;
