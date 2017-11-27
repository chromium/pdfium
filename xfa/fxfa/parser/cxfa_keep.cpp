// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_keep.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Extras, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,       XFA_Attribute::Next,    XFA_Attribute::Use,
    XFA_Attribute::Previous, XFA_Attribute::Usehref, XFA_Attribute::Intact,
    XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"keep";

}  // namespace

CXFA_Keep::CXFA_Keep(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Keep,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Keep::~CXFA_Keep() {}
