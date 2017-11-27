// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_command.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Query, 1, 0},
                                                 {XFA_Element::Insert, 1, 0},
                                                 {XFA_Element::Update, 1, 0},
                                                 {XFA_Element::Delete, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,      XFA_Attribute::Name,    XFA_Attribute::Use,
    XFA_Attribute::Timeout, XFA_Attribute::Usehref, XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"command";

}  // namespace

CXFA_Command::CXFA_Command(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_SourceSet,
                XFA_ObjectType::Node,
                XFA_Element::Command,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Command::~CXFA_Command() {}
