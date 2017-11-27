// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_subform.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Break, 1, 0},   {XFA_Element::Margin, 1, 0},
    {XFA_Element::Para, 1, 0},    {XFA_Element::Border, 1, 0},
    {XFA_Element::Assist, 1, 0},  {XFA_Element::Traversal, 1, 0},
    {XFA_Element::Keep, 1, 0},    {XFA_Element::Validate, 1, 0},
    {XFA_Element::PageSet, 1, 0}, {XFA_Element::Overflow, 1, 0},
    {XFA_Element::Bind, 1, 0},    {XFA_Element::Desc, 1, 0},
    {XFA_Element::Bookend, 1, 0}, {XFA_Element::Calculate, 1, 0},
    {XFA_Element::Extras, 1, 0},  {XFA_Element::Variables, 1, 0},
    {XFA_Element::Occur, 1, 0},   {XFA_Element::Unknown, 0, 0}};
const XFA_Attribute kAttributeData[] = {XFA_Attribute::H,
                                        XFA_Attribute::W,
                                        XFA_Attribute::X,
                                        XFA_Attribute::Y,
                                        XFA_Attribute::Id,
                                        XFA_Attribute::HAlign,
                                        XFA_Attribute::Name,
                                        XFA_Attribute::Use,
                                        XFA_Attribute::AllowMacro,
                                        XFA_Attribute::ColumnWidths,
                                        XFA_Attribute::Access,
                                        XFA_Attribute::Presence,
                                        XFA_Attribute::VAlign,
                                        XFA_Attribute::MaxH,
                                        XFA_Attribute::MaxW,
                                        XFA_Attribute::MinH,
                                        XFA_Attribute::MinW,
                                        XFA_Attribute::Layout,
                                        XFA_Attribute::Relevant,
                                        XFA_Attribute::MergeMode,
                                        XFA_Attribute::ColSpan,
                                        XFA_Attribute::Usehref,
                                        XFA_Attribute::Locale,
                                        XFA_Attribute::AnchorType,
                                        XFA_Attribute::RestoreState,
                                        XFA_Attribute::Scope,
                                        XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"subform";

}  // namespace

CXFA_Subform::CXFA_Subform(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::ContainerNode,
                XFA_Element::Subform,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Subform::~CXFA_Subform() {}
