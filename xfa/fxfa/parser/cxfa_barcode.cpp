// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_barcode.h"

#include "fxjs/xfa/cjx_barcode.h"
#include "third_party/base/ptr_util.h"

namespace {

const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::DataRowCount, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::DataPrep, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::None},
    {XFA_Attribute::Type, XFA_AttributeType::CData, (void*)nullptr},
    {XFA_Attribute::TextLocation, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Below},
    {XFA_Attribute::ModuleWidth, XFA_AttributeType::Measure, (void*)L"0.25mm"},
    {XFA_Attribute::PrintCheckDigit, XFA_AttributeType::Boolean, (void*)0},
    {XFA_Attribute::ModuleHeight, XFA_AttributeType::Measure, (void*)L"5mm"},
    {XFA_Attribute::StartChar, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Truncate, XFA_AttributeType::Boolean, (void*)0},
    {XFA_Attribute::WideNarrowRatio, XFA_AttributeType::CData, (void*)L"3:1"},
    {XFA_Attribute::ErrorCorrectionLevel, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::UpsMode, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::UsCarrier},
    {XFA_Attribute::Checksum, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::None},
    {XFA_Attribute::CharEncoding, XFA_AttributeType::CData, (void*)L"UTF-8"},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::DataColumnCount, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::RowColumnRatio, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::DataLength, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::EndChar, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"barcode";

}  // namespace

CXFA_Barcode::CXFA_Barcode(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Barcode,
                nullptr,
                kAttributeData,
                kName,
                pdfium::MakeUnique<CJX_Barcode>(this)) {}

CXFA_Barcode::~CXFA_Barcode() {}
