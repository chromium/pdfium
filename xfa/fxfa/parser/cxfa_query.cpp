// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_query.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::RecordSet, 1, 0},
                                                 {XFA_Element::Select, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,          XFA_Attribute::Name,    XFA_Attribute::Use,
    XFA_Attribute::CommandType, XFA_Attribute::Usehref, XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"query";

}  // namespace

CXFA_Query::CXFA_Query(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_SourceSet,
                XFA_ObjectType::Node,
                XFA_Element::Query,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Query::~CXFA_Query() {}
