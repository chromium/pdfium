// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_hyphenation.h"

namespace {

const XFA_Attribute kAttributeData[] = {XFA_Attribute::Id,
                                        XFA_Attribute::Use,
                                        XFA_Attribute::WordCharacterCount,
                                        XFA_Attribute::Hyphenate,
                                        XFA_Attribute::ExcludeInitialCap,
                                        XFA_Attribute::PushCharacterCount,
                                        XFA_Attribute::RemainCharacterCount,
                                        XFA_Attribute::Usehref,
                                        XFA_Attribute::ExcludeAllCaps,
                                        XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"hyphenation";

}  // namespace

CXFA_Hyphenation::CXFA_Hyphenation(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Hyphenation,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_Hyphenation::~CXFA_Hyphenation() {}
