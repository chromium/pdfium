// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_para.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Hyphenation, 1, 0},
    {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,         XFA_Attribute::HAlign,
    XFA_Attribute::TextIndent, XFA_Attribute::Use,
    XFA_Attribute::Widows,     XFA_Attribute::MarginRight,
    XFA_Attribute::MarginLeft, XFA_Attribute::RadixOffset,
    XFA_Attribute::Preserve,   XFA_Attribute::SpaceBelow,
    XFA_Attribute::VAlign,     XFA_Attribute::TabDefault,
    XFA_Attribute::TabStops,   XFA_Attribute::Orphans,
    XFA_Attribute::Usehref,    XFA_Attribute::LineHeight,
    XFA_Attribute::SpaceAbove, XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"para";

}  // namespace

CXFA_Para::CXFA_Para(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Para,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Para::~CXFA_Para() {}
