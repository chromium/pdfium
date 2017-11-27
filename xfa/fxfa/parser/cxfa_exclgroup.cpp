// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_exclgroup.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Margin, 1, 0},    {XFA_Element::Para, 1, 0},
    {XFA_Element::Border, 1, 0},    {XFA_Element::Assist, 1, 0},
    {XFA_Element::Traversal, 1, 0}, {XFA_Element::Validate, 1, 0},
    {XFA_Element::Caption, 1, 0},   {XFA_Element::Bind, 1, 0},
    {XFA_Element::Desc, 1, 0},      {XFA_Element::Calculate, 1, 0},
    {XFA_Element::Extras, 1, 0},    {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::H,         XFA_Attribute::W,
    XFA_Attribute::X,         XFA_Attribute::Y,
    XFA_Attribute::Id,        XFA_Attribute::HAlign,
    XFA_Attribute::Name,      XFA_Attribute::Use,
    XFA_Attribute::Access,    XFA_Attribute::Presence,
    XFA_Attribute::VAlign,    XFA_Attribute::MaxH,
    XFA_Attribute::MaxW,      XFA_Attribute::MinH,
    XFA_Attribute::MinW,      XFA_Attribute::Layout,
    XFA_Attribute::Relevant,  XFA_Attribute::ColSpan,
    XFA_Attribute::Usehref,   XFA_Attribute::AnchorType,
    XFA_Attribute::AccessKey, XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"exclGroup";

}  // namespace

CXFA_ExclGroup::CXFA_ExclGroup(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::ContainerNode,
                XFA_Element::ExclGroup,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_ExclGroup::~CXFA_ExclGroup() {}
