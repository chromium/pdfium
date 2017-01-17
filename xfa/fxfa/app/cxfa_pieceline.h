// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_PIECELINE_H_
#define XFA_FXFA_APP_CXFA_PIECELINE_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_basic.h"

class XFA_TextPiece;

class CXFA_PieceLine {
 public:
  CXFA_PieceLine();
  ~CXFA_PieceLine();

  std::vector<std::unique_ptr<XFA_TextPiece>> m_textPieces;
  CFX_ArrayTemplate<int32_t> m_charCounts;
};

#endif  // XFA_FXFA_APP_CXFA_PIECELINE_H_
