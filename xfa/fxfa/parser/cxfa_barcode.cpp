// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_barcode.h"

namespace {

const XFA_Attribute kAttributeData[] = {XFA_Attribute::Id,
                                        XFA_Attribute::DataRowCount,
                                        XFA_Attribute::Use,
                                        XFA_Attribute::DataPrep,
                                        XFA_Attribute::Type,
                                        XFA_Attribute::TextLocation,
                                        XFA_Attribute::ModuleWidth,
                                        XFA_Attribute::PrintCheckDigit,
                                        XFA_Attribute::ModuleHeight,
                                        XFA_Attribute::StartChar,
                                        XFA_Attribute::Truncate,
                                        XFA_Attribute::WideNarrowRatio,
                                        XFA_Attribute::ErrorCorrectionLevel,
                                        XFA_Attribute::UpsMode,
                                        XFA_Attribute::Checksum,
                                        XFA_Attribute::CharEncoding,
                                        XFA_Attribute::Usehref,
                                        XFA_Attribute::DataColumnCount,
                                        XFA_Attribute::RowColumnRatio,
                                        XFA_Attribute::DataLength,
                                        XFA_Attribute::EndChar,
                                        XFA_Attribute::Unknown};

constexpr wchar_t kName[] = L"barcode";

}  // namespace

CXFA_Barcode::CXFA_Barcode(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Barcode,
                nullptr,
                kAttributeData,
                kName) {}

CXFA_Barcode::~CXFA_Barcode() {}
