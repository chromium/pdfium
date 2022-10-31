// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_viewerpreferences.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kViewerPreferencesPropertyData[] = {
    {XFA_Element::PrintScaling, 1, {}},
    {XFA_Element::Enforce, 1, {}},
    {XFA_Element::NumberOfCopies, 1, {}},
    {XFA_Element::PageRange, 1, {}},
    {XFA_Element::AddViewerPreferences, 1, {}},
    {XFA_Element::ADBE_JSConsole, 1, {}},
    {XFA_Element::DuplexOption, 1, {}},
    {XFA_Element::ADBE_JSDebugger, 1, {}},
    {XFA_Element::PickTrayByPDFSize, 1, {}},
};

const CXFA_Node::AttributeData kViewerPreferencesAttributeData[] = {
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_ViewerPreferences::CXFA_ViewerPreferences(CXFA_Document* doc,
                                               XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kConfig,
                XFA_ObjectType::Node,
                XFA_Element::ViewerPreferences,
                kViewerPreferencesPropertyData,
                kViewerPreferencesAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_ViewerPreferences::~CXFA_ViewerPreferences() = default;
