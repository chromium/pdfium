// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_recordset.h"

namespace {

const XFA_Attribute kAttributeData[] = {
    XFA_Attribute::Id,        XFA_Attribute::Name,
    XFA_Attribute::Max,       XFA_Attribute::Use,
    XFA_Attribute::EofAction, XFA_Attribute::CursorType,
    XFA_Attribute::LockType,  XFA_Attribute::BofAction,
    XFA_Attribute::Usehref,   XFA_Attribute::CursorLocation,
    XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"recordSet";

}  // namespace

CXFA_RecordSet::CXFA_RecordSet(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_SourceSet,
                XFA_ObjectType::Node,
                XFA_Element::RecordSet,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_RecordSet::~CXFA_RecordSet() {}
