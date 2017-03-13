// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_UCD_H_
#define CORE_FXCRT_FX_UCD_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_basic.h"

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

extern const uint32_t kTextLayoutCodeProperties[];
extern const size_t kTextLayoutCodePropertiesSize;

extern const uint16_t kFXTextLayoutVerticalMirror[];
extern const size_t kFXTextLayoutVerticalMirrorSize;

extern const uint16_t kFXTextLayoutBidiMirror[];
extern const size_t kFXTextLayoutBidiMirrorSize;

uint32_t FX_GetUnicodeProperties(wchar_t wch);
wchar_t FX_GetMirrorChar(wchar_t wch, bool bRTL, bool bVertical);

#ifdef PDF_ENABLE_XFA

// As defined in http://www.unicode.org/reports/tr14/
enum FXCHAR_BREAKPROP {
  FX_CBP_OP = 0,   // Opening Punctuation
  FX_CBP_CL = 1,   // Closing Punctuation
  FX_CBP_QU = 2,   // Ambiguous Quotation
  FX_CBP_GL = 3,   // Non-breaking ("Glue")
  FX_CBP_NS = 4,   // Non Starter
  FX_CBP_EX = 5,   // Exclamation/Interrogation
  FX_CBP_SY = 6,   // Symbols Allowing Breaks
  FX_CBP_IS = 7,   // Infix Separator (Numeric)
  FX_CBP_PR = 8,   // Prefix (Numeric)
  FX_CBP_PO = 9,   // Postfix (Numeric)
  FX_CBP_NU = 10,  // Numeric
  FX_CBP_AL = 11,  // Ordinary Alphabetic and Symbol Characters
  FX_CBP_ID = 12,  // Ideographic
  FX_CBP_IN = 13,  // Inseparable
  FX_CBP_HY = 14,  // Hyphen
  FX_CBP_BA = 15,  // Break Opportunity After
  FX_CBP_BB = 16,  // Break Opportunity Before
  FX_CBP_B2 = 17,  // Break Opportunity Before and After
  FX_CBP_ZW = 18,  // Zero Width Space
  FX_CBP_CM = 19,  // Attached Characters and Combining Marks
  FX_CBP_WJ = 20,  // Word Joiner
  FX_CBP_H2 = 21,  // Hangul LV Syllable
  FX_CBP_H3 = 22,  // Hangul LVT Syllable
  FX_CBP_JL = 23,  // Hangul Leading Jamo
  FX_CBP_JV = 24,  // Hangul Vowel Jamo
  FX_CBP_JT = 25,  // Hangul Trailing Jamo

  FX_CBP_BK = 26,  // Mandatory Break
  FX_CBP_CR = 27,  // Carriage Return
  FX_CBP_LF = 28,  // Line Feed
  FX_CBP_NL = 29,  // Next Line
  FX_CBP_SA = 30,  // Complex Context (South East Asian)
  FX_CBP_SG = 31,  // Surrogate
  FX_CBP_CB = 32,  // Contingent Break Opportunity
  FX_CBP_XX = 33,  // Unknown
  FX_CBP_AI = 34,  // Ambiguous (Alphabetic or Ideographic)
  FX_CBP_SP = 35,  // Space
  FX_CBP_NONE = 36,
  FX_CBP_TB = 37,  // ?
};

#define FX_CHARTYPEBITS 11
#define FX_CHARTYPEBITSMASK (15 << FX_CHARTYPEBITS)
enum FX_CHARTYPE {
  FX_CHARTYPE_Unknown = 0,
  FX_CHARTYPE_Tab = (1 << FX_CHARTYPEBITS),
  FX_CHARTYPE_Space = (2 << FX_CHARTYPEBITS),
  FX_CHARTYPE_Control = (3 << FX_CHARTYPEBITS),
  FX_CHARTYPE_Combination = (4 << FX_CHARTYPEBITS),
  FX_CHARTYPE_Numeric = (5 << FX_CHARTYPEBITS),
  FX_CHARTYPE_Normal = (6 << FX_CHARTYPEBITS),
  FX_CHARTYPE_ArabicAlef = (7 << FX_CHARTYPEBITS),
  FX_CHARTYPE_ArabicSpecial = (8 << FX_CHARTYPEBITS),
  FX_CHARTYPE_ArabicDistortion = (9 << FX_CHARTYPEBITS),
  FX_CHARTYPE_ArabicNormal = (10 << FX_CHARTYPEBITS),
  FX_CHARTYPE_ArabicForm = (11 << FX_CHARTYPEBITS),
  FX_CHARTYPE_Arabic = (12 << FX_CHARTYPEBITS),
};
inline FX_CHARTYPE GetCharTypeFromProp(uint32_t prop) {
  return static_cast<FX_CHARTYPE>(prop & FX_CHARTYPEBITSMASK);
}

wchar_t FX_GetMirrorChar(wchar_t wch,
                         uint32_t dwProps,
                         bool bRTL,
                         bool bVertical);

#endif  // PDF_ENABLE_XFA

#endif  // CORE_FXCRT_FX_UCD_H_
