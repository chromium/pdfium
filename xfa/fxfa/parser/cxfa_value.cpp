// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_value.h"

#include "fxjs/xfa/cjx_value.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_arc.h"
#include "xfa/fxfa/parser/cxfa_image.h"
#include "xfa/fxfa/parser/cxfa_line.h"
#include "xfa/fxfa/parser/cxfa_rectangle.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Arc, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Text, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Time, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::DateTime, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Image, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Decimal, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Boolean, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Integer, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::ExData, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Rectangle, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Date, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Float, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Line, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Relevant, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Override, XFA_AttributeType::Boolean, (void*)0},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"value";

}  // namespace

CXFA_Value::CXFA_Value(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Value,
                kPropertyData,
                kAttributeData,
                kName,
                pdfium::MakeUnique<CJX_Value>(this)) {}

CXFA_Value::~CXFA_Value() {}

XFA_Element CXFA_Value::GetChildValueClassID() const {
  CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_FirstChild);
  return pNode ? pNode->GetElementType() : XFA_Element::Unknown;
}

WideString CXFA_Value::GetChildValueContent() const {
  CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_FirstChild);
  if (!pNode)
    return L"";
  return pNode->JSObject()->TryContent(false, true).value_or(L"");
}

CXFA_Arc* CXFA_Value::GetArc() const {
  return static_cast<CXFA_Arc*>(GetNodeItem(XFA_NODEITEM_FirstChild));
}

CXFA_Line* CXFA_Value::GetLine() const {
  return static_cast<CXFA_Line*>(GetNodeItem(XFA_NODEITEM_FirstChild));
}

CXFA_Rectangle* CXFA_Value::GetRectangle() const {
  return static_cast<CXFA_Rectangle*>(GetNodeItem(XFA_NODEITEM_FirstChild));
}

CXFA_Text* CXFA_Value::GetText() const {
  return static_cast<CXFA_Text*>(GetNodeItem(XFA_NODEITEM_FirstChild));
}

CXFA_ExDataData CXFA_Value::GetExData() const {
  return CXFA_ExDataData(GetNodeItem(XFA_NODEITEM_FirstChild));
}

CXFA_Image* CXFA_Value::GetImage() const {
  return static_cast<CXFA_Image*>(GetNodeItem(XFA_NODEITEM_FirstChild));
}
