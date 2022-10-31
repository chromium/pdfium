// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_pdf.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kPdfPropertyData[] = {
    {XFA_Element::AdobeExtensionLevel, 1, {}},
    {XFA_Element::FontInfo, 1, {}},
    {XFA_Element::Xdc, 1, {}},
    {XFA_Element::Pdfa, 1, {}},
    {XFA_Element::BatchOutput, 1, {}},
    {XFA_Element::ViewerPreferences, 1, {}},
    {XFA_Element::ScriptModel, 1, {}},
    {XFA_Element::Version, 1, {}},
    {XFA_Element::SubmitFormat, 1, {}},
    {XFA_Element::SilentPrint, 1, {}},
    {XFA_Element::Producer, 1, {}},
    {XFA_Element::Compression, 1, {}},
    {XFA_Element::Interactive, 1, {}},
    {XFA_Element::Encryption, 1, {}},
    {XFA_Element::RenderPolicy, 1, {}},
    {XFA_Element::OpenAction, 1, {}},
    {XFA_Element::Creator, 1, {}},
    {XFA_Element::Linearized, 1, {}},
    {XFA_Element::Tagged, 1, {}},
};

const CXFA_Node::AttributeData kPdfAttributeData[] = {
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

CXFA_Pdf::CXFA_Pdf(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kConfig,
                XFA_ObjectType::Node,
                XFA_Element::Pdf,
                kPdfPropertyData,
                kPdfAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Pdf::~CXFA_Pdf() = default;
