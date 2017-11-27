// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_comb.h"

namespace {

const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id, XFA_Attribute::Use, XFA_Attribute::NumberOfCells,
    XFA_Attribute::Usehref, XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"comb";

}  // namespace

CXFA_Comb::CXFA_Comb(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Comb,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_Comb::~CXFA_Comb() {}
