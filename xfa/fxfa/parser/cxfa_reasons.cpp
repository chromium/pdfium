// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_reasons.h"

namespace {

const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id, XFA_Attribute::Use, XFA_Attribute::Type,
    XFA_Attribute::Usehref, XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"reasons";

}  // namespace

CXFA_Reasons::CXFA_Reasons(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Reasons,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_Reasons::~CXFA_Reasons() {}
