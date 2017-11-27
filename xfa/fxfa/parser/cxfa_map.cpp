// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_map.h"

namespace {

const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,   XFA_Attribute::Name,    XFA_Attribute::Use,
    XFA_Attribute::Bind, XFA_Attribute::Usehref, XFA_Attribute::Desc,
    XFA_Attribute::From, XFA_Attribute::Lock,    XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"map";

}  // namespace

CXFA_Map::CXFA_Map(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Config | XFA_XDPPACKET_SourceSet),
                XFA_ObjectType::Node,
                XFA_Element::Map,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_Map::~CXFA_Map() {}
