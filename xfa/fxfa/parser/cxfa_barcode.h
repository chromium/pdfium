// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_BARCODE_H_
#define XFA_FXFA_PARSER_CXFA_BARCODE_H_

#include <optional>

#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_Barcode final : public CXFA_Node {
 public:
  static CXFA_Barcode* FromNode(CXFA_Node* pNode);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Barcode() override;

  XFA_FFWidgetType GetDefaultFFWidgetType() const override;

  WideString GetBarcodeType();
  std::optional<bool> GetChecksum();
  std::optional<int32_t> GetDataLength();
  std::optional<char> GetStartChar();
  std::optional<char> GetEndChar();
  std::optional<int32_t> GetECLevel();
  std::optional<int32_t> GetModuleWidth();
  std::optional<int32_t> GetModuleHeight();
  std::optional<bool> GetPrintChecksum();
  std::optional<XFA_AttributeValue> GetTextLocation();
  std::optional<bool> GetTruncate();
  std::optional<int8_t> GetWideNarrowRatio();

 private:
  CXFA_Barcode(CXFA_Document* doc, XFA_PacketType packet);
};

#endif  // XFA_FXFA_PARSER_CXFA_BARCODE_H_
