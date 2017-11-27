// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_caption.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Margin, 1, 0}, {XFA_Element::Para, 1, 0},
    {XFA_Element::Font, 1, 0},   {XFA_Element::Value, 1, 0},
    {XFA_Element::Extras, 1, 0}, {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,       XFA_Attribute::Use,     XFA_Attribute::Reserve,
    XFA_Attribute::Presence, XFA_Attribute::Usehref, XFA_Attribute::Placement,
    XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"caption";

}  // namespace

CXFA_Caption::CXFA_Caption(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Caption,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Caption::~CXFA_Caption() {}
