// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_COLOR_H_
#define XFA_FXFA_PARSER_CXFA_COLOR_H_

#include "core/fxcrt/widestring.h"
#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_Color final : public CXFA_Node {
 public:
  static constexpr FX_ARGB kBlackColor = 0xFF000000;
  static FX_ARGB StringToFXARGB(WideStringView view);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Color() override;

  FX_ARGB GetValue() const;
  FX_ARGB GetValueOrDefault(FX_ARGB defaultValue) const;
  void SetValue(FX_ARGB color);

 private:
  CXFA_Color(CXFA_Document* doc, XFA_PacketType packet);
};

#endif  // XFA_FXFA_PARSER_CXFA_COLOR_H_
