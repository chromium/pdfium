// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_bind.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Picture, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,          XFA_Attribute::Name,
    XFA_Attribute::Ref,         XFA_Attribute::Use,
    XFA_Attribute::ContentType, XFA_Attribute::TransferEncoding,
    XFA_Attribute::Usehref,     XFA_Attribute::Match,
    XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"bind";

}  // namespace

CXFA_Bind::CXFA_Bind(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
                 XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Bind,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Bind::~CXFA_Bind() {}
