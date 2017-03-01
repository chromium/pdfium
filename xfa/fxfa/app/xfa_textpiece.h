// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_TEXTPIECE_H_
#define XFA_FXFA_APP_XFA_TEXTPIECE_H_

#include <vector>

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fgas/font/cfgas_gefont.h"

class CXFA_LinkUserData;

class XFA_TextPiece {
 public:
  XFA_TextPiece();
  ~XFA_TextPiece();

  CFX_WideString szText;
  std::vector<int32_t> Widths;
  int32_t iChars;
  int32_t iHorScale;
  int32_t iVerScale;
  int32_t iBidiLevel;
  int32_t iUnderline;
  int32_t iPeriod;
  int32_t iLineThrough;
  FX_ARGB dwColor;
  FX_FLOAT fFontSize;
  CFX_RectF rtPiece;
  CFX_RetainPtr<CFGAS_GEFont> pFont;
  CFX_RetainPtr<CXFA_LinkUserData> pLinkData;
};

#endif  // XFA_FXFA_APP_XFA_TEXTPIECE_H_
