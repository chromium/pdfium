// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_UCD_H_
#define CORE_INCLUDE_FXCRT_FX_UCD_H_

#include "fx_system.h"

#define FX_BIDICLASSBITS 6
#define FX_BIDICLASSBITSMASK (31 << FX_BIDICLASSBITS)
enum FX_BIDICLASS {
  FX_BIDICLASS_ON = 0,    // Other Neutral
  FX_BIDICLASS_L = 1,     // Left Letter
  FX_BIDICLASS_R = 2,     // Right Letter
  FX_BIDICLASS_AN = 3,    // Arabic Number
  FX_BIDICLASS_EN = 4,    // European Number
  FX_BIDICLASS_AL = 5,    // Arabic Letter
  FX_BIDICLASS_NSM = 6,   // Non-spacing Mark
  FX_BIDICLASS_CS = 7,    // Common Number Separator
  FX_BIDICLASS_ES = 8,    // European Separator
  FX_BIDICLASS_ET = 9,    // European Number Terminator
  FX_BIDICLASS_BN = 10,   // Boundary Neutral
  FX_BIDICLASS_S = 11,    // Segment Separator
  FX_BIDICLASS_WS = 12,   // Whitespace
  FX_BIDICLASS_B = 13,    // Paragraph Separator
  FX_BIDICLASS_RLO = 14,  // Right-to-Left Override
  FX_BIDICLASS_RLE = 15,  // Right-to-Left Embedding
  FX_BIDICLASS_LRO = 16,  // Left-to-Right Override
  FX_BIDICLASS_LRE = 17,  // Left-to-Right Embedding
  FX_BIDICLASS_PDF = 18,  // Pop Directional Format
  FX_BIDICLASS_N = FX_BIDICLASS_ON,
};

extern const FX_DWORD kTextLayoutCodeProperties[];
extern const size_t kTextLayoutCodePropertiesSize;

extern const FX_WCHAR kFXTextLayoutVerticalMirror[];
extern const size_t kFXTextLayoutVerticalMirrorSize;

extern const FX_WCHAR kFXTextLayoutBidiMirror[];
extern const size_t kFXTextLayoutBidiMirrorSize;

FX_DWORD FX_GetUnicodeProperties(FX_WCHAR wch);
FX_WCHAR FX_GetMirrorChar(FX_WCHAR wch, FX_BOOL bRTL, FX_BOOL bVertical);

#endif  // CORE_INCLUDE_FXCRT_FX_UCD_H_
