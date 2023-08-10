// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/fgas_arabic.h"

#include <iterator>

#include "core/fxcrt/fx_unicode.h"
#include "xfa/fgas/layout/cfgas_char.h"

namespace {

struct FX_ARBFORMTABLE {
  uint16_t wIsolated;
  uint16_t wFinal;
  uint16_t wInitial;
  uint16_t wMedial;
};

struct FX_ARAALEF {
  uint16_t wAlef;
  uint16_t wIsolated;
};

constexpr FX_ARBFORMTABLE kFormTable[] = {
    {0xFE81, 0xFE82, 0xFE81, 0xFE82}, {0xFE83, 0xFE84, 0xFE83, 0xFE84},
    {0xFE85, 0xFE86, 0xFE85, 0xFE86}, {0xFE87, 0xFE88, 0xFE87, 0xFE88},
    {0xFE89, 0xFE8A, 0xFE8B, 0xFE8C}, {0xFE8D, 0xFE8E, 0xFE8D, 0xFE8E},
    {0xFE8F, 0xFE90, 0xFE91, 0xFE92}, {0xFE93, 0xFE94, 0xFE93, 0xFE94},
    {0xFE95, 0xFE96, 0xFE97, 0xFE98}, {0xFE99, 0xFE9A, 0xFE9B, 0xFE9C},
    {0xFE9D, 0xFE9E, 0xFE9F, 0xFEA0}, {0xFEA1, 0xFEA2, 0xFEA3, 0xFEA4},
    {0xFEA5, 0xFEA6, 0xFEA7, 0xFEA8}, {0xFEA9, 0xFEAA, 0xFEA9, 0xFEAA},
    {0xFEAB, 0xFEAC, 0xFEAB, 0xFEAC}, {0xFEAD, 0xFEAE, 0xFEAD, 0xFEAE},
    {0xFEAF, 0xFEB0, 0xFEAF, 0xFEB0}, {0xFEB1, 0xFEB2, 0xFEB3, 0xFEB4},
    {0xFEB5, 0xFEB6, 0xFEB7, 0xFEB8}, {0xFEB9, 0xFEBA, 0xFEBB, 0xFEBC},
    {0xFEBD, 0xFEBE, 0xFEBF, 0xFEC0}, {0xFEC1, 0xFEC2, 0xFEC3, 0xFEC4},
    {0xFEC5, 0xFEC6, 0xFEC7, 0xFEC8}, {0xFEC9, 0xFECA, 0xFECB, 0xFECC},
    {0xFECD, 0xFECE, 0xFECF, 0xFED0}, {0x063B, 0x063B, 0x063B, 0x063B},
    {0x063C, 0x063C, 0x063C, 0x063C}, {0x063D, 0x063D, 0x063D, 0x063D},
    {0x063E, 0x063E, 0x063E, 0x063E}, {0x063F, 0x063F, 0x063F, 0x063F},
    {0x0640, 0x0640, 0x0640, 0x0640}, {0xFED1, 0xFED2, 0xFED3, 0xFED4},
    {0xFED5, 0xFED6, 0xFED7, 0xFED8}, {0xFED9, 0xFEDA, 0xFEDB, 0xFEDC},
    {0xFEDD, 0xFEDE, 0xFEDF, 0xFEE0}, {0xFEE1, 0xFEE2, 0xFEE3, 0xFEE4},
    {0xFEE5, 0xFEE6, 0xFEE7, 0xFEE8}, {0xFEE9, 0xFEEA, 0xFEEB, 0xFEEC},
    {0xFEED, 0xFEEE, 0xFEED, 0xFEEE}, {0xFEEF, 0xFEF0, 0xFBFE, 0xFBFF},
    {0xFEF1, 0xFEF2, 0xFEF3, 0xFEF4}, {0x064B, 0x064B, 0x064B, 0x064B},
    {0x064C, 0x064C, 0x064C, 0x064C}, {0x064D, 0x064D, 0x064D, 0x064D},
    {0x064E, 0x064E, 0x064E, 0x064E}, {0x064F, 0x064F, 0x064F, 0x064F},
    {0x0650, 0x0650, 0x0650, 0x0650}, {0x0651, 0x0651, 0x0651, 0x0651},
    {0x0652, 0x0652, 0x0652, 0x0652}, {0x0653, 0x0653, 0x0653, 0x0653},
    {0x0654, 0x0654, 0x0654, 0x0654}, {0x0655, 0x0655, 0x0655, 0x0655},
    {0x0656, 0x0656, 0x0656, 0x0656}, {0x0657, 0x0657, 0x0657, 0x0657},
    {0x0658, 0x0658, 0x0658, 0x0658}, {0x0659, 0x0659, 0x0659, 0x0659},
    {0x065A, 0x065A, 0x065A, 0x065A}, {0x065B, 0x065B, 0x065B, 0x065B},
    {0x065C, 0x065C, 0x065C, 0x065C}, {0x065D, 0x065D, 0x065D, 0x065D},
    {0x065E, 0x065E, 0x065E, 0x065E}, {0x065F, 0x065F, 0x065F, 0x065F},
    {0x0660, 0x0660, 0x0660, 0x0660}, {0x0661, 0x0661, 0x0661, 0x0661},
    {0x0662, 0x0662, 0x0662, 0x0662}, {0x0663, 0x0663, 0x0663, 0x0663},
    {0x0664, 0x0664, 0x0664, 0x0664}, {0x0665, 0x0665, 0x0665, 0x0665},
    {0x0666, 0x0666, 0x0666, 0x0666}, {0x0667, 0x0667, 0x0667, 0x0667},
    {0x0668, 0x0668, 0x0668, 0x0668}, {0x0669, 0x0669, 0x0669, 0x0669},
    {0x066A, 0x066A, 0x066A, 0x066A}, {0x066B, 0x066B, 0x066B, 0x066B},
    {0x066C, 0x066C, 0x066C, 0x066C}, {0x066D, 0x066D, 0x066D, 0x066D},
    {0x066E, 0x066E, 0x066E, 0x066E}, {0x066F, 0x066F, 0x066F, 0x066F},
    {0x0670, 0x0670, 0x0670, 0x0670}, {0xFB50, 0xFB51, 0xFB50, 0xFB51},
    {0x0672, 0x0672, 0x0672, 0x0672}, {0x0673, 0x0673, 0x0673, 0x0673},
    {0x0674, 0x0674, 0x0674, 0x0674}, {0x0675, 0x0675, 0x0675, 0x0675},
    {0x0676, 0x0676, 0x0676, 0x0676}, {0x0677, 0x0677, 0x0677, 0x0677},
    {0x0678, 0x0678, 0x0678, 0x0678}, {0xFB66, 0xFB67, 0xFB68, 0xFB69},
    {0xFB5E, 0xFB5F, 0xFB60, 0xFB61}, {0xFB52, 0xFB53, 0xFB54, 0xFB55},
    {0x067C, 0x067C, 0x067C, 0x067C}, {0x067D, 0x067D, 0x067D, 0x067D},
    {0xFB56, 0xFB57, 0xFB58, 0xFB59}, {0xFB62, 0xFB63, 0xFB64, 0xFB65},
    {0xFB5A, 0xFB5B, 0xFB5C, 0xFB5D}, {0x0681, 0x0681, 0x0681, 0x0681},
    {0x0682, 0x0682, 0x0682, 0x0682}, {0xFB76, 0xFB77, 0xFB78, 0xFB79},
    {0xFB72, 0xFB73, 0xFB74, 0xFB75}, {0x0685, 0x0685, 0x0685, 0x0685},
    {0xFB7A, 0xFB7B, 0xFB7C, 0xFB7D}, {0xFB7E, 0xFB7F, 0xFB80, 0xFB81},
    {0xFB88, 0xFB89, 0xFB88, 0xFB89}, {0x0689, 0x0689, 0x0689, 0x0689},
    {0x068A, 0x068A, 0x068A, 0x068A}, {0x068B, 0x068B, 0x068B, 0x068B},
    {0xFB84, 0xFB85, 0xFB84, 0xFB85}, {0xFB82, 0xFB83, 0xFB82, 0xFB83},
    {0xFB86, 0xFB87, 0xFB86, 0xFB87}, {0x068F, 0x068F, 0x068F, 0x068F},
    {0x0690, 0x0690, 0x0690, 0x0690}, {0xFB8C, 0xFB8D, 0xFB8C, 0xFB8D},
    {0x0692, 0x0692, 0x0692, 0x0692}, {0x0693, 0x0693, 0x0693, 0x0693},
    {0x0694, 0x0694, 0x0694, 0x0694}, {0x0695, 0x0695, 0x0695, 0x0695},
    {0x0696, 0x0696, 0x0696, 0x0696}, {0x0697, 0x0697, 0x0697, 0x0697},
    {0xFB8A, 0xFB8B, 0xFB8A, 0xFB8B}, {0x0699, 0x0699, 0x0699, 0x0699},
    {0x069A, 0x069A, 0x069A, 0x069A}, {0x069B, 0x069B, 0x069B, 0x069B},
    {0x069C, 0x069C, 0x069C, 0x069C}, {0x069D, 0x069D, 0x069D, 0x069D},
    {0x069E, 0x069E, 0x069E, 0x069E}, {0x069F, 0x069F, 0x069F, 0x069F},
    {0x06A0, 0x06A0, 0x06A0, 0x06A0}, {0x06A1, 0x06A1, 0x06A1, 0x06A1},
    {0x06A2, 0x06A2, 0x06A2, 0x06A2}, {0x06A3, 0x06A3, 0x06A3, 0x06A3},
    {0xFB6A, 0xFB6B, 0xFB6C, 0xFB6D}, {0x06A5, 0x06A5, 0x06A5, 0x06A5},
    {0xFB6E, 0xFB6F, 0xFB70, 0xFB71}, {0x06A7, 0x06A7, 0x06A7, 0x06A7},
    {0x06A8, 0x06A8, 0x06A8, 0x06A8}, {0xFB8E, 0xFB8F, 0xFB90, 0xFB91},
    {0x06AA, 0x06AA, 0x06AA, 0x06AA}, {0x06AB, 0x06AB, 0x06AB, 0x06AB},
    {0x06AC, 0x06AC, 0x06AC, 0x06AC}, {0xFBD3, 0xFBD4, 0xFBD5, 0xFBD6},
    {0x06AE, 0x06AE, 0x06AE, 0x06AE}, {0xFB92, 0xFB93, 0xFB94, 0xFB95},
    {0x06B0, 0x06B0, 0x06B0, 0x06B0}, {0xFB9A, 0xFB9B, 0xFB9C, 0xFB9D},
    {0x06B2, 0x06B2, 0x06B2, 0x06B2}, {0xFB96, 0xFB97, 0xFB98, 0xFB99},
    {0x06B4, 0x06B4, 0x06B4, 0x06B4}, {0x06B5, 0x06B5, 0x06B5, 0x06B5},
    {0x06B6, 0x06B6, 0x06B6, 0x06B6}, {0x06B7, 0x06B7, 0x06B7, 0x06B7},
    {0x06B8, 0x06B8, 0x06B8, 0x06B8}, {0x06B9, 0x06B9, 0x06B9, 0x06B9},
    {0xFB9E, 0xFB9F, 0xFBE8, 0xFBE9}, {0xFBA0, 0xFBA1, 0xFBA2, 0xFBA3},
    {0x06BC, 0x06BC, 0x06BC, 0x06BC}, {0x06BD, 0x06BD, 0x06BD, 0x06BD},
    {0xFBAA, 0xFBAB, 0xFBAC, 0xFBAD}, {0x06BF, 0x06BF, 0x06BF, 0x06BF},
    {0xFBA4, 0xFBA5, 0xFBA4, 0xFBA5}, {0xFBA6, 0xFBA7, 0xFBA8, 0xFBA9},
    {0x06C2, 0x06C2, 0x06C2, 0x06C2}, {0x06C3, 0x06C3, 0x06C3, 0x06C3},
    {0x06C4, 0x06C4, 0x06C4, 0x06C4}, {0xFBE0, 0xFBE1, 0xFBE0, 0xFBE1},
    {0xFBD9, 0xFBDA, 0xFBD9, 0xFBDA}, {0xFBD7, 0xFBD8, 0xFBD7, 0xFBD8},
    {0xFBDB, 0xFBDC, 0xFBDB, 0xFBDC}, {0xFBE2, 0xFBE3, 0xFBE2, 0xFBE3},
    {0x06CA, 0x06CA, 0x06CA, 0x06CA}, {0xFBDE, 0xFBDF, 0xFBDE, 0xFBDF},
    {0xFBFC, 0xFBFD, 0xFBFE, 0xFBFF}, {0x06CD, 0x06CD, 0x06CD, 0x06CD},
    {0x06CE, 0x06CE, 0x06CE, 0x06CE}, {0x06CF, 0x06CF, 0x06CF, 0x06CF},
    {0xFBE4, 0xFBE5, 0xFBE6, 0xFBE7}, {0x06D1, 0x06D1, 0x06D1, 0x06D1},
    {0xFBAE, 0xFBAF, 0xFBAE, 0xFBAF}, {0xFBB0, 0xFBB1, 0xFBB0, 0xFBB1},
    {0x06D4, 0x06D4, 0x06D4, 0x06D4}, {0x06D5, 0x06D5, 0x06D5, 0x06D5},
};
constexpr uint16_t kFirstFormTableEntry = 0x0622;
constexpr uint16_t kLastFormTableEntry =
    kFirstFormTableEntry + std::size(kFormTable) - 1;

constexpr FX_ARAALEF kAlefTable[] = {
    {0x0622, 0xFEF5},
    {0x0623, 0xFEF7},
    {0x0625, 0xFEF9},
    {0x0627, 0xFEFB},
};

constexpr uint16_t kShaddaTable[] = {0xFC5E, 0xFC5F, 0xFC60, 0xFC61, 0xFC62};
constexpr uint16_t kFirstShaddaTableEntry = 0x064c;
constexpr uint16_t kLastShaddaTableEntry =
    kFirstShaddaTableEntry + std::size(kShaddaTable) - 1;

const FX_ARBFORMTABLE* GetArabicFormTable(wchar_t unicode) {
  if (unicode < kFirstFormTableEntry || unicode > kLastFormTableEntry)
    return nullptr;

  return &kFormTable[unicode - kFirstFormTableEntry];
}

const FX_ARBFORMTABLE* ParseChar(const CFGAS_Char* pTC,
                                 wchar_t* wChar,
                                 FX_CHARTYPE* eType) {
  if (!pTC) {
    *eType = FX_CHARTYPE::kUnknown;
    *wChar = pdfium::unicode::kZeroWidthNoBreakSpace;
    return nullptr;
  }

  *eType = pTC->GetCharType();
  *wChar = static_cast<wchar_t>(pTC->char_code());
  const FX_ARBFORMTABLE* pFT = GetArabicFormTable(*wChar);
  if (!pFT || *eType >= FX_CHARTYPE::kArabicNormal)
    *eType = FX_CHARTYPE::kUnknown;

  return pFT;
}

wchar_t GetArabicFromAlefTable(wchar_t alef) {
  for (const FX_ARAALEF& v : kAlefTable) {
    if (v.wAlef == alef)
      return v.wIsolated;
  }
  return alef;
}

}  // namespace

namespace pdfium::arabic {

wchar_t GetFormChar(wchar_t wch, wchar_t prev, wchar_t next) {
  CFGAS_Char c(wch);
  CFGAS_Char p(prev);
  CFGAS_Char n(next);
  return GetFormChar(&c, &p, &n);
}

wchar_t GetFormChar(const CFGAS_Char* cur,
                    const CFGAS_Char* prev,
                    const CFGAS_Char* next) {
  FX_CHARTYPE eCur;
  wchar_t wCur;
  const FX_ARBFORMTABLE* ft = ParseChar(cur, &wCur, &eCur);
  if (eCur < FX_CHARTYPE::kArabicAlef || eCur >= FX_CHARTYPE::kArabicNormal)
    return wCur;

  FX_CHARTYPE ePrev;
  wchar_t wPrev;
  ParseChar(prev, &wPrev, &ePrev);
  if (wPrev == kArabicLetterLam && eCur == FX_CHARTYPE::kArabicAlef)
    return pdfium::unicode::kZeroWidthNoBreakSpace;

  FX_CHARTYPE eNext;
  wchar_t wNext;
  ParseChar(next, &wNext, &eNext);
  bool bAlef = (eNext == FX_CHARTYPE::kArabicAlef && wCur == kArabicLetterLam);
  if (ePrev < FX_CHARTYPE::kArabicAlef) {
    if (bAlef)
      return GetArabicFromAlefTable(wNext);
    return (eNext < FX_CHARTYPE::kArabicAlef) ? ft->wIsolated : ft->wInitial;
  }

  if (bAlef) {
    wCur = GetArabicFromAlefTable(wNext);
    return (ePrev != FX_CHARTYPE::kArabicDistortion) ? wCur : ++wCur;
  }

  if (ePrev == FX_CHARTYPE::kArabicAlef || ePrev == FX_CHARTYPE::kArabicSpecial)
    return (eNext < FX_CHARTYPE::kArabicAlef) ? ft->wIsolated : ft->wInitial;
  return (eNext < FX_CHARTYPE::kArabicAlef) ? ft->wFinal : ft->wMedial;
}

absl::optional<wchar_t> GetArabicFromShaddaTable(wchar_t shadda) {
  if (shadda < kFirstShaddaTableEntry || shadda > kLastShaddaTableEntry)
    return absl::nullopt;

  return kShaddaTable[shadda - kFirstShaddaTableEntry];
}

}  // namespace pdfium::arabic
