// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_color.h"

#include "core/fxcrt/fx_extension.h"
#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kColorPropertyData[] = {
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kColorAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::CSpace, XFA_AttributeType::CData, (void*)L"SRGB"},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Value, XFA_AttributeType::CData, nullptr},
};

}  // namespace

// static
FX_ARGB CXFA_Color::StringToFXARGB(WideStringView view) {
  static constexpr FX_ARGB kDefaultValue = 0xff000000;
  if (view.IsEmpty())
    return kDefaultValue;

  const wchar_t* str = view.unterminated_c_str();
  size_t len = view.GetLength();
  size_t cc = 0;
  while (cc < len && FXSYS_iswspace(str[cc]))
    cc++;

  if (cc >= len)
    return kDefaultValue;

  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  while (cc < len) {
    if (str[cc] == ',' || !FXSYS_IsDecimalDigit(str[cc]))
      break;

    r = r * 10 + str[cc] - '0';
    cc++;
  }
  if (cc < len && str[cc] == ',') {
    cc++;
    while (cc < len && FXSYS_iswspace(str[cc]))
      cc++;

    while (cc < len) {
      if (str[cc] == ',' || !FXSYS_IsDecimalDigit(str[cc]))
        break;

      g = g * 10 + str[cc] - '0';
      cc++;
    }
    if (cc < len && str[cc] == ',') {
      cc++;
      while (cc < len && FXSYS_iswspace(str[cc]))
        cc++;

      while (cc < len) {
        if (str[cc] == ',' || !FXSYS_IsDecimalDigit(str[cc]))
          break;

        b = b * 10 + str[cc] - '0';
        cc++;
      }
    }
  }
  return ArgbEncode(0xFF, r, g, b);
}

CXFA_Color::CXFA_Color(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Color,
                kColorPropertyData,
                kColorAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Color::~CXFA_Color() = default;

FX_ARGB CXFA_Color::GetValue() const {
  absl::optional<WideString> val =
      JSObject()->TryCData(XFA_Attribute::Value, false);
  return val.has_value() ? StringToFXARGB(val->AsStringView()) : 0xFF000000;
}

FX_ARGB CXFA_Color::GetValueOrDefault(FX_ARGB defaultValue) const {
  absl::optional<WideString> val =
      JSObject()->TryCData(XFA_Attribute::Value, false);
  return val.has_value() ? StringToFXARGB(val->AsStringView()) : defaultValue;
}

void CXFA_Color::SetValue(FX_ARGB color) {
  int a;
  int r;
  int g;
  int b;
  std::tie(a, r, g, b) = ArgbDecode(color);
  JSObject()->SetCData(XFA_Attribute::Value,
                       WideString::Format(L"%d,%d,%d", r, g, b));
}
