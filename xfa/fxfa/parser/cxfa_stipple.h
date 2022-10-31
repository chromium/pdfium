// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_STIPPLE_H_
#define XFA_FXFA_PARSER_CXFA_STIPPLE_H_

#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fxfa/parser/cxfa_node.h"

class CFGAS_GEGraphics;
class CXFA_Color;

class CXFA_Stipple final : public CXFA_Node {
 public:
  static int32_t GetDefaultRate() { return 50; }

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Stipple() override;

  void Draw(CFGAS_GEGraphics* pGS,
            const CFGAS_GEPath& fillPath,
            const CFX_RectF& rtFill,
            const CFX_Matrix& matrix);

 private:
  CXFA_Stipple(CXFA_Document* doc, XFA_PacketType packet);

  CXFA_Color* GetColorIfExists();
  int32_t GetRate();
};

#endif  // XFA_FXFA_PARSER_CXFA_STIPPLE_H_
