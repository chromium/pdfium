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
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Reserve, XFA_AttributeType::Measure,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)L"-1un"},
    {XFA_Attribute::Presence, XFA_AttributeType::Enum,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form,
     (void*)XFA_ATTRIBUTEENUM_Visible},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Placement, XFA_AttributeType::Enum,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form,
     (void*)XFA_ATTRIBUTEENUM_Left},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, 0, nullptr}};

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
