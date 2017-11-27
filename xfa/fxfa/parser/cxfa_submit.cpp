// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_submit.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Encrypt, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,           XFA_Attribute::Use,
    XFA_Attribute::Format,       XFA_Attribute::EmbedPDF,
    XFA_Attribute::Usehref,      XFA_Attribute::Target,
    XFA_Attribute::TextEncoding, XFA_Attribute::XdpContent,
    XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"submit";

}  // namespace

CXFA_Submit::CXFA_Submit(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Submit,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Submit::~CXFA_Submit() {}
