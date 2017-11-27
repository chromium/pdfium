// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_event.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Execute, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Script, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::SignData, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Extras, 1, 0},
    {XFA_Element::Submit, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,       XFA_Attribute::Name,   XFA_Attribute::Ref,
    XFA_Attribute::Use,      XFA_Attribute::Listen, XFA_Attribute::Usehref,
    XFA_Attribute::Activity, XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"event";

}  // namespace

CXFA_Event::CXFA_Event(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Event,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Event::~CXFA_Event() {}
