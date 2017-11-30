// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datavalue.h"

namespace {

const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Config | XFA_XDPPACKET_LocaleSet |
         XFA_XDPPACKET_Template | XFA_XDPPACKET_Datasets | XFA_XDPPACKET_Form |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::ContentType, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Template | XFA_XDPPACKET_Datasets |
         XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::Contains, XFA_AttributeType::Enum, XFA_XDPPACKET_Datasets,
     (void*)XFA_ATTRIBUTEENUM_Data},
    {XFA_Attribute::Value, XFA_AttributeType::CData,
     XFA_XDPPACKET_SourceSet | XFA_XDPPACKET_Config | XFA_XDPPACKET_LocaleSet |
         XFA_XDPPACKET_Template | XFA_XDPPACKET_Datasets |
         XFA_XDPPACKET_ConnectionSet | XFA_XDPPACKET_Form,
     nullptr},
    {XFA_Attribute::IsNull, XFA_AttributeType::Boolean, XFA_XDPPACKET_Datasets,
     (void*)0},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, 0, nullptr}};

constexpr wchar_t kName[] = L"dataValue";

}  // namespace

CXFA_DataValue::CXFA_DataValue(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Datasets,
                XFA_ObjectType::Node,
                XFA_Element::DataValue,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_DataValue::~CXFA_DataValue() {}
