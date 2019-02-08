// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_TEXTPIECE_H_
#define XFA_FXFA_CXFA_TEXTPIECE_H_

#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fgas/layout/cfx_textpiece.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFX_LinkUserData;

class CXFA_TextPiece : public CFX_TextPiece {
 public:
  CXFA_TextPiece();
  ~CXFA_TextPiece();

  int32_t iUnderline;
  int32_t iLineThrough;
  XFA_AttributeValue iPeriod;
  FX_ARGB dwColor;
  RetainPtr<CFX_LinkUserData> pLinkData;
};

#endif  // XFA_FXFA_CXFA_TEXTPIECE_H_
