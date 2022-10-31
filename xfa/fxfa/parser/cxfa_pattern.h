// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_PATTERN_H_
#define XFA_FXFA_PARSER_CXFA_PATTERN_H_

#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fxfa/parser/cxfa_node.h"

class CFGAS_GEGraphics;
class CXFA_Color;

class CXFA_Pattern final : public CXFA_Node {
 public:
  static constexpr XFA_AttributeValue kDefaultType =
      XFA_AttributeValue::Unknown;

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Pattern() override;

  void Draw(CFGAS_GEGraphics* pGS,
            const CFGAS_GEPath& fillPath,
            FX_ARGB crStart,
            const CFX_RectF& rtFill,
            const CFX_Matrix& matrix);

 private:
  CXFA_Pattern(CXFA_Document* doc, XFA_PacketType packet);

  XFA_AttributeValue GetType();
  CXFA_Color* GetColorIfExists();
};

#endif  // XFA_FXFA_PARSER_CXFA_PATTERN_H_
