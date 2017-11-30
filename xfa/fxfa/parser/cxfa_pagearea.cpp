// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pagearea.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Medium, 1, 0},
                                                 {XFA_Element::Desc, 1, 0},
                                                 {XFA_Element::Extras, 1, 0},
                                                 {XFA_Element::Occur, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Config | XFA_XDPPACKET_LocaleSet |
         XFA_XDPPACKET_Template | XFA_XDPPACKET_Datasets | XFA_XDPPACKET_Form |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::PagePosition, XFA_AttributeType::Enum,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)XFA_ATTRIBUTEENUM_Any},
    {XFA_Attribute::OddOrEven, XFA_AttributeType::Enum,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)XFA_ATTRIBUTEENUM_Any},
    {XFA_Attribute::Relevant, XFA_AttributeType::CData,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, nullptr},
    {XFA_Attribute::InitialNumber, XFA_AttributeType::Integer,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)1},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Numbered, XFA_AttributeType::Integer,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)1},
    {XFA_Attribute::BlankOrNotBlank, XFA_AttributeType::Enum,
     XFA_XDPPACKET_Template | XFA_XDPPACKET_Form, (void*)XFA_ATTRIBUTEENUM_Any},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, 0, nullptr}};

constexpr wchar_t kName[] = L"pageArea";

}  // namespace

CXFA_PageArea::CXFA_PageArea(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::ContainerNode,
                XFA_Element::PageArea,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_PageArea::~CXFA_PageArea() {}
