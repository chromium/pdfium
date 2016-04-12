// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FEE_FX_WORDBREAK_FX_WORDBREAK_IMPL_H_
#define XFA_FEE_FX_WORDBREAK_FX_WORDBREAK_IMPL_H_

#include <cstdint>

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fee/fx_wordbreak/fx_wordbreak.h"
#include "xfa/fee/fx_wordbreak/fx_wordbreak_impl.h"
#include "xfa/fee/ifde_txtedtengine.h"

extern const uint16_t gs_FX_WordBreak_Table[16];
extern const uint8_t gs_FX_WordBreak_CodePointProperties[(0xFFFF - 1) / 2 + 1];
enum FX_WordBreakProp {
  FX_WordBreakProp_None = 0,
  FX_WordBreakProp_CR,
  FX_WordBreakProp_LF,
  FX_WordBreakProp_NewLine,
  FX_WordBreakProp_Extend,
  FX_WordBreakProp_Format,
  FX_WordBreakProp_KataKana,
  FX_WordBreakProp_ALetter,
  FX_WordBreakProp_MidLetter,
  FX_WordBreakProp_MidNum,
  FX_WordBreakProp_MidNumLet,
  FX_WordBreakProp_Numberic,
  FX_WordBreakProp_ExtendNumLet,
};
FX_WordBreakProp FX_GetWordBreakProperty(FX_WCHAR wcCodePoint);

#endif  // XFA_FEE_FX_WORDBREAK_FX_WORDBREAK_IMPL_H_
