// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_medium.h"

namespace {

const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,          XFA_Attribute::TrayOut,
    XFA_Attribute::Use,         XFA_Attribute::Orientation,
    XFA_Attribute::ImagingBBox, XFA_Attribute::Short,
    XFA_Attribute::TrayIn,      XFA_Attribute::Usehref,
    XFA_Attribute::Stock,       XFA_Attribute::Long,
    XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"medium";

}  // namespace

CXFA_Medium::CXFA_Medium(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Medium,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_Medium::~CXFA_Medium() {}
