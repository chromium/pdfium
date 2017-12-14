// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_checkbutton.h"

#include "fxjs/xfa/cjx_checkbutton.h"
#include "third_party/base/ptr_util.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Margin, 1, 0},
                                                 {XFA_Element::Border, 1, 0},
                                                 {XFA_Element::Extras, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::AllowNeutral, XFA_AttributeType::Boolean, (void*)0},
    {XFA_Attribute::Mark, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Default},
    {XFA_Attribute::Shape, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Square},
    {XFA_Attribute::Size, XFA_AttributeType::Measure, (void*)L"10pt"},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"checkButton";

}  // namespace

CXFA_CheckButton::CXFA_CheckButton(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::CheckButton,
                kPropertyData,
                kAttributeData,
                kName,
                pdfium::MakeUnique<CJX_CheckButton>(this)) {}

CXFA_CheckButton::~CXFA_CheckButton() {}
