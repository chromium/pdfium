// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_validate.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Message, 1, 0},
                                                 {XFA_Element::Picture, 1, 0},
                                                 {XFA_Element::Script, 1, 0},
                                                 {XFA_Element::Extras, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::ScriptTest, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Error},
    {XFA_Attribute::NullTest, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Disabled},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::FormatTest, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Warning},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"validate";

}  // namespace

CXFA_Validate::CXFA_Validate(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(
          doc,
          packet,
          (XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
          XFA_ObjectType::ContentNode,
          XFA_Element::Validate,
          kPropertyData,
          kAttributeData,
          kName) {}

CXFA_Validate::~CXFA_Validate() {}
