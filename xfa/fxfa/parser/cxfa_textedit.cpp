// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_textedit.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Margin, 1, 0},
                                                 {XFA_Element::Border, 1, 0},
                                                 {XFA_Element::Comb, 1, 0},
                                                 {XFA_Element::Extras, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {XFA_Attribute::Id,
                                        XFA_Attribute::VScrollPolicy,
                                        XFA_Attribute::Use,
                                        XFA_Attribute::AllowRichText,
                                        XFA_Attribute::MultiLine,
                                        XFA_Attribute::Usehref,
                                        XFA_Attribute::HScrollPolicy,
                                        XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"textEdit";

}  // namespace

CXFA_TextEdit::CXFA_TextEdit(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::TextEdit,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_TextEdit::~CXFA_TextEdit() {}
