// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_barcode.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"

namespace {

const CXFA_Node::AttributeData kBarcodeAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::DataRowCount, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::DataPrep, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::None},
    {XFA_Attribute::Type, XFA_AttributeType::CData, (void*)nullptr},
    {XFA_Attribute::TextLocation, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Below},
    {XFA_Attribute::ModuleWidth, XFA_AttributeType::Measure, (void*)L"0.25mm"},
    {XFA_Attribute::PrintCheckDigit, XFA_AttributeType::Boolean, (void*)0},
    {XFA_Attribute::ModuleHeight, XFA_AttributeType::Measure, (void*)L"5mm"},
    {XFA_Attribute::StartChar, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Truncate, XFA_AttributeType::Boolean, (void*)0},
    {XFA_Attribute::WideNarrowRatio, XFA_AttributeType::CData, (void*)L"3:1"},
    {XFA_Attribute::ErrorCorrectionLevel, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::UpsMode, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::UsCarrier},
    {XFA_Attribute::Checksum, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::None},
    {XFA_Attribute::CharEncoding, XFA_AttributeType::CData, (void*)L"UTF-8"},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::DataColumnCount, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::RowColumnRatio, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::DataLength, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::EndChar, XFA_AttributeType::CData, nullptr},
};

}  // namespace

// static
CXFA_Barcode* CXFA_Barcode::FromNode(CXFA_Node* pNode) {
  return pNode && pNode->GetElementType() == XFA_Element::Barcode
             ? static_cast<CXFA_Barcode*>(pNode)
             : nullptr;
}

CXFA_Barcode::CXFA_Barcode(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Barcode,
                {},
                kBarcodeAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Barcode::~CXFA_Barcode() = default;

XFA_FFWidgetType CXFA_Barcode::GetDefaultFFWidgetType() const {
  return XFA_FFWidgetType::kBarcode;
}

WideString CXFA_Barcode::GetBarcodeType() {
  return WideString(JSObject()->GetCData(XFA_Attribute::Type));
}

absl::optional<WideString> CXFA_Barcode::GetCharEncoding() {
  return JSObject()->TryCData(XFA_Attribute::CharEncoding, true);
}

absl::optional<bool> CXFA_Barcode::GetChecksum() {
  absl::optional<XFA_AttributeValue> checksum =
      JSObject()->TryEnum(XFA_Attribute::Checksum, true);
  if (!checksum.has_value())
    return absl::nullopt;

  switch (checksum.value()) {
    case XFA_AttributeValue::None:
      return {false};
    case XFA_AttributeValue::Auto:
      return {true};
    case XFA_AttributeValue::Checksum_1mod10:
    case XFA_AttributeValue::Checksum_1mod10_1mod11:
    case XFA_AttributeValue::Checksum_2mod10:
    default:
      break;
  }
  return absl::nullopt;
}

absl::optional<int32_t> CXFA_Barcode::GetDataLength() {
  absl::optional<WideString> wsDataLength =
      JSObject()->TryCData(XFA_Attribute::DataLength, true);
  if (!wsDataLength.has_value())
    return absl::nullopt;

  return FXSYS_wtoi(wsDataLength->c_str());
}

absl::optional<char> CXFA_Barcode::GetStartChar() {
  absl::optional<WideString> wsStartEndChar =
      JSObject()->TryCData(XFA_Attribute::StartChar, true);
  if (!wsStartEndChar.has_value() || wsStartEndChar->IsEmpty())
    return absl::nullopt;

  return static_cast<char>(wsStartEndChar.value()[0]);
}

absl::optional<char> CXFA_Barcode::GetEndChar() {
  absl::optional<WideString> wsStartEndChar =
      JSObject()->TryCData(XFA_Attribute::EndChar, true);
  if (!wsStartEndChar.has_value() || wsStartEndChar->IsEmpty())
    return absl::nullopt;

  return static_cast<char>(wsStartEndChar.value()[0]);
}

absl::optional<int32_t> CXFA_Barcode::GetECLevel() {
  absl::optional<WideString> wsECLevel =
      JSObject()->TryCData(XFA_Attribute::ErrorCorrectionLevel, true);
  if (!wsECLevel.has_value())
    return absl::nullopt;
  return FXSYS_wtoi(wsECLevel->c_str());
}

absl::optional<int32_t> CXFA_Barcode::GetModuleWidth() {
  absl::optional<CXFA_Measurement> moduleWidthHeight =
      JSObject()->TryMeasure(XFA_Attribute::ModuleWidth, true);
  if (!moduleWidthHeight.has_value())
    return absl::nullopt;

  return static_cast<int32_t>(moduleWidthHeight->ToUnit(XFA_Unit::Pt));
}

absl::optional<int32_t> CXFA_Barcode::GetModuleHeight() {
  absl::optional<CXFA_Measurement> moduleWidthHeight =
      JSObject()->TryMeasure(XFA_Attribute::ModuleHeight, true);
  if (!moduleWidthHeight.has_value())
    return absl::nullopt;

  return static_cast<int32_t>(moduleWidthHeight->ToUnit(XFA_Unit::Pt));
}

absl::optional<bool> CXFA_Barcode::GetPrintChecksum() {
  return JSObject()->TryBoolean(XFA_Attribute::PrintCheckDigit, true);
}

absl::optional<XFA_AttributeValue> CXFA_Barcode::GetTextLocation() {
  return JSObject()->TryEnum(XFA_Attribute::TextLocation, true);
}

absl::optional<bool> CXFA_Barcode::GetTruncate() {
  return JSObject()->TryBoolean(XFA_Attribute::Truncate, true);
}

absl::optional<int8_t> CXFA_Barcode::GetWideNarrowRatio() {
  absl::optional<WideString> wsWideNarrowRatio =
      JSObject()->TryCData(XFA_Attribute::WideNarrowRatio, true);
  if (!wsWideNarrowRatio.has_value())
    return absl::nullopt;

  absl::optional<size_t> ptPos = wsWideNarrowRatio->Find(':');
  if (!ptPos.has_value())
    return static_cast<int8_t>(FXSYS_wtoi(wsWideNarrowRatio->c_str()));

  int32_t fB = FXSYS_wtoi(
      wsWideNarrowRatio
          ->Last(wsWideNarrowRatio->GetLength() - (ptPos.value() + 1))
          .c_str());
  if (!fB)
    return 0;

  int32_t fA = FXSYS_wtoi(wsWideNarrowRatio->First(ptPos.value()).c_str());
  float result = static_cast<float>(fA) / static_cast<float>(fB);
  return static_cast<int8_t>(result);
}
