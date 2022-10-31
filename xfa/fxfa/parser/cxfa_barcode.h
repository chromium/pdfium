// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_BARCODE_H_
#define XFA_FXFA_PARSER_CXFA_BARCODE_H_

#include "third_party/abseil-cpp/absl/types/optional.h"
#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_Barcode final : public CXFA_Node {
 public:
  static CXFA_Barcode* FromNode(CXFA_Node* pNode);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Barcode() override;

  XFA_FFWidgetType GetDefaultFFWidgetType() const override;

  WideString GetBarcodeType();
  absl::optional<WideString> GetCharEncoding();
  absl::optional<bool> GetChecksum();
  absl::optional<int32_t> GetDataLength();
  absl::optional<char> GetStartChar();
  absl::optional<char> GetEndChar();
  absl::optional<int32_t> GetECLevel();
  absl::optional<int32_t> GetModuleWidth();
  absl::optional<int32_t> GetModuleHeight();
  absl::optional<bool> GetPrintChecksum();
  absl::optional<XFA_AttributeValue> GetTextLocation();
  absl::optional<bool> GetTruncate();
  absl::optional<int8_t> GetWideNarrowRatio();

 private:
  CXFA_Barcode(CXFA_Document* doc, XFA_PacketType packet);
};

#endif  // XFA_FXFA_PARSER_CXFA_BARCODE_H_
