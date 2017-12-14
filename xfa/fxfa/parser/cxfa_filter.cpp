// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_filter.h"

#include "fxjs/xfa/cjx_filter.h"
#include "third_party/base/ptr_util.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Mdp, 1, 0},           {XFA_Element::Certificates, 1, 0},
    {XFA_Element::TimeStamp, 1, 0},     {XFA_Element::Handler, 1, 0},
    {XFA_Element::DigestMethods, 1, 0}, {XFA_Element::Encodings, 1, 0},
    {XFA_Element::Reasons, 1, 0},       {XFA_Element::AppearanceFilter, 1, 0},
    {XFA_Element::LockDocument, 1, 0},  {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Version, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::AddRevocationInfo, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"filter";

}  // namespace

CXFA_Filter::CXFA_Filter(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Filter,
                kPropertyData,
                kAttributeData,
                kName,
                pdfium::MakeUnique<CJX_Filter>(this)) {}

CXFA_Filter::~CXFA_Filter() {}
