// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_bidi.h"

#include <algorithm>

#include "core/fxcrt/fx_unicode.h"
#include "third_party/base/stl_util.h"

#ifdef PDF_ENABLE_XFA
#include "core/fxcrt/fx_extension.h"
#endif  // PDF_ENABLE_XFA

namespace {

#ifdef PDF_ENABLE_XFA

#ifndef NDEBUG
constexpr int32_t kBidiMaxLevel = 61;
#endif  // NDEBUG

#undef PACK_NIBBLES
#define PACK_NIBBLES(hi, lo) \
  ((static_cast<uint32_t>(hi) << 4) + static_cast<uint32_t>(lo))

enum FX_BIDIWEAKSTATE : uint8_t {
  FX_BWSxa = 0,
  FX_BWSxr,
  FX_BWSxl,
  FX_BWSao,
  FX_BWSro,
  FX_BWSlo,
  FX_BWSrt,
  FX_BWSlt,
  FX_BWScn,
  FX_BWSra,
  FX_BWSre,
  FX_BWSla,
  FX_BWSle,
  FX_BWSac,
  FX_BWSrc,
  FX_BWSrs,
  FX_BWSlc,
  FX_BWSls,
  FX_BWSret,
  FX_BWSlet
};

// NOTE: Range of FX_BIDICLASS prevents encoding all possible values in this
// manner, but the ones used manage to fit. Except that I suspect that 0xF
// was intended to be used as a sentinel, even though it also means kRLE.
// TODO(tsepez): pick a better representation.
enum FX_BIDIWEAKACTION : uint16_t {
  FX_BWAIX = 0x100,
  FX_BWAXX = 0x0F,
  FX_BWAxxx = 0xFF,
  FX_BWAxIx = 0x100 + FX_BWAxxx,
  FX_BWAxxN = PACK_NIBBLES(0x0F, FX_BIDICLASS::kON),
  FX_BWAxxE = PACK_NIBBLES(0x0F, FX_BIDICLASS::kEN),
  FX_BWAxxA = PACK_NIBBLES(0x0F, FX_BIDICLASS::kAN),
  FX_BWAxxR = PACK_NIBBLES(0x0F, FX_BIDICLASS::kR),
  FX_BWAxxL = PACK_NIBBLES(0x0F, FX_BIDICLASS::kL),
  FX_BWANxx = PACK_NIBBLES(FX_BIDICLASS::kON, 0x0F),
  FX_BWAAxx = PACK_NIBBLES(FX_BIDICLASS::kAN, 0x0F),
  FX_BWAExE = PACK_NIBBLES(FX_BIDICLASS::kEN, FX_BIDICLASS::kEN),
  FX_BWANIx = 0x100 + PACK_NIBBLES(FX_BIDICLASS::kON, 0x0F),
  FX_BWANxN = PACK_NIBBLES(FX_BIDICLASS::kON, FX_BIDICLASS::kON),
  FX_BWANxR = PACK_NIBBLES(FX_BIDICLASS::kON, FX_BIDICLASS::kR),
  FX_BWANxE = PACK_NIBBLES(FX_BIDICLASS::kON, FX_BIDICLASS::kEN),
  FX_BWAAxA = PACK_NIBBLES(FX_BIDICLASS::kAN, FX_BIDICLASS::kAN),
  FX_BWANxL = PACK_NIBBLES(FX_BIDICLASS::kON, FX_BIDICLASS::kL),
  FX_BWALxL = PACK_NIBBLES(FX_BIDICLASS::kL, FX_BIDICLASS::kL),
  FX_BWAxIL = 0x100 + PACK_NIBBLES(0x0F, FX_BIDICLASS::kL),
  FX_BWAAxR = PACK_NIBBLES(FX_BIDICLASS::kAN, FX_BIDICLASS::kR),
  FX_BWALxx = PACK_NIBBLES(FX_BIDICLASS::kL, 0x0F),
};

enum FX_BIDINEUTRALSTATE : uint8_t {
  FX_BNSr = 0,
  FX_BNSl,
  FX_BNSrn,
  FX_BNSln,
  FX_BNSa,
  FX_BNSna
};

enum FX_BIDINEUTRALACTION : uint16_t {
  // For placeholders in table.
  FX_BNAZero = 0,

  // Other values.
  FX_BNAnL = PACK_NIBBLES(0, FX_BIDICLASS::kL),
  FX_BNAEn = PACK_NIBBLES(FX_BIDICLASS::kAN, 0),
  FX_BNARn = PACK_NIBBLES(FX_BIDICLASS::kR, 0),
  FX_BNALn = PACK_NIBBLES(FX_BIDICLASS::kL, 0),
  FX_BNAIn = FX_BWAIX,
  FX_BNALnL = PACK_NIBBLES(FX_BIDICLASS::kL, FX_BIDICLASS::kL),
};
#undef PACK_NIBBLES

const FX_BIDICLASS gc_FX_BidiNTypes[] = {
    FX_BIDICLASS::kN,   FX_BIDICLASS::kL,   FX_BIDICLASS::kR,
    FX_BIDICLASS::kAN,  FX_BIDICLASS::kEN,  FX_BIDICLASS::kAL,
    FX_BIDICLASS::kNSM, FX_BIDICLASS::kCS,  FX_BIDICLASS::kES,
    FX_BIDICLASS::kET,  FX_BIDICLASS::kBN,  FX_BIDICLASS::kBN,
    FX_BIDICLASS::kN,   FX_BIDICLASS::kB,   FX_BIDICLASS::kRLO,
    FX_BIDICLASS::kRLE, FX_BIDICLASS::kLRO, FX_BIDICLASS::kLRE,
    FX_BIDICLASS::kPDF, FX_BIDICLASS::kON,
};

const FX_BIDIWEAKSTATE gc_FX_BidiWeakStates[20][10] = {
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWSxa,
     FX_BWSao, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSxr,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSxl,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWSao,
     FX_BWSao, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSro,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlo,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSrt,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlt,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWScn,
     FX_BWSac, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSra,
     FX_BWSrc, FX_BWSro, FX_BWSrt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSre,
     FX_BWSrs, FX_BWSrs, FX_BWSret},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSla,
     FX_BWSlc, FX_BWSlo, FX_BWSlt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSle,
     FX_BWSls, FX_BWSls, FX_BWSlet},
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWSao,
     FX_BWSao, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSro,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSro,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlo,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlo,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSret,
     FX_BWSro, FX_BWSro, FX_BWSret},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlet,
     FX_BWSlo, FX_BWSlo, FX_BWSlet},
};

const FX_BIDIWEAKACTION gc_FX_BidiWeakActions[20][10] = {
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxA, FX_BWAxxR,
     FX_BWAxxR, FX_BWAxxN, FX_BWAxxN, FX_BWAxxN},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxR, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxL, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxA, FX_BWAxxR,
     FX_BWAxxN, FX_BWAxxN, FX_BWAxxN, FX_BWAxxN},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxN, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxN, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAExE, FX_BWANxR,
     FX_BWAxIx, FX_BWANxN, FX_BWANxN, FX_BWAxIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWALxL, FX_BWANxR,
     FX_BWAxIx, FX_BWANxN, FX_BWANxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxA, FX_BWAxxR,
     FX_BWAxxA, FX_BWAxIx, FX_BWAxxN, FX_BWAxxN},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxA, FX_BWAxIx, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxE, FX_BWAxIx, FX_BWAxIx, FX_BWAxxE},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxA, FX_BWAxIx, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxL, FX_BWAxIx, FX_BWAxIx, FX_BWAxxL},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAAxx, FX_BWAAxA, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANxN},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAAxx, FX_BWANxE, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAExE, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAAxx, FX_BWANxL, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWALxL, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxE, FX_BWAxxN, FX_BWAxxN, FX_BWAxxE},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxL, FX_BWAxxN, FX_BWAxxN, FX_BWAxxL},
};

const FX_BIDINEUTRALSTATE gc_FX_BidiNeutralStates[6][5] = {
    {FX_BNSrn, FX_BNSl, FX_BNSr, FX_BNSr, FX_BNSr},
    {FX_BNSln, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
    {FX_BNSrn, FX_BNSl, FX_BNSr, FX_BNSr, FX_BNSr},
    {FX_BNSln, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
    {FX_BNSna, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
    {FX_BNSna, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
};

const FX_BIDINEUTRALACTION gc_FX_BidiNeutralActions[6][5] = {
    {FX_BNAIn, FX_BNAZero, FX_BNAZero, FX_BNAZero, FX_BNAZero},
    {FX_BNAIn, FX_BNAZero, FX_BNAZero, FX_BNAZero, FX_BNAnL},
    {FX_BNAIn, FX_BNAEn, FX_BNARn, FX_BNARn, FX_BNARn},
    {FX_BNAIn, FX_BNALn, FX_BNAEn, FX_BNAEn, FX_BNALnL},
    {FX_BNAIn, FX_BNAZero, FX_BNAZero, FX_BNAZero, FX_BNAnL},
    {FX_BNAIn, FX_BNAEn, FX_BNARn, FX_BNARn, FX_BNAEn},
};

const uint8_t gc_FX_BidiAddLevel[2][4] = {
    {0, 1, 2, 2},
    {1, 0, 1, 1},
};

FX_BIDICLASS Direction(int32_t val) {
  return FX_IsOdd(val) ? FX_BIDICLASS::kR : FX_BIDICLASS::kL;
}

FX_BIDICLASS GetDeferredType(int32_t val) {
  return static_cast<FX_BIDICLASS>((val >> 4) & 0x0F);
}

FX_BIDICLASS GetResolvedType(int32_t val) {
  return static_cast<FX_BIDICLASS>(val & 0x0F);
}

FX_BIDICLASS GetDeferredNeutrals(int32_t iAction, int32_t iLevel) {
  FX_BIDICLASS eClass = GetDeferredType(iAction);
  return eClass == FX_BIDICLASS::kAN ? Direction(iLevel) : eClass;
}

FX_BIDICLASS GetResolvedNeutrals(int32_t iAction) {
  return GetResolvedType(iAction);
}

void ReverseString(std::vector<CFX_Char>* chars, size_t iStart, size_t iCount) {
  ASSERT(pdfium::IndexInBounds(*chars, iStart));
  ASSERT(iStart + iCount <= chars->size());

  std::reverse(chars->begin() + iStart, chars->begin() + iStart + iCount);
}

void SetDeferredRunClass(std::vector<CFX_Char>* chars,
                         size_t iStart,
                         size_t iCount,
                         FX_BIDICLASS eValue) {
  ASSERT(iStart <= chars->size());
  ASSERT(iStart >= iCount);

  size_t iLast = iStart - iCount;
  for (size_t i = iStart; i > iLast; --i)
    (*chars)[i - 1].m_iBidiClass = eValue;
}

void SetDeferredRunLevel(std::vector<CFX_Char>* chars,
                         size_t iStart,
                         size_t iCount,
                         int32_t iValue) {
  ASSERT(iStart <= chars->size());
  ASSERT(iStart >= iCount);

  size_t iLast = iStart - iCount;
  for (size_t i = iStart; i > iLast; --i)
    (*chars)[i - 1].m_iBidiLevel = static_cast<int16_t>(iValue);
}

void Classify(std::vector<CFX_Char>* chars, size_t iCount, bool bWS) {
  if (bWS) {
    for (size_t i = 0; i < iCount; ++i) {
      CFX_Char& cur = (*chars)[i];
      cur.m_iBidiClass = FX_GetBidiClass(cur.char_code());
    }
    return;
  }

  for (size_t i = 0; i < iCount; ++i) {
    CFX_Char& cur = (*chars)[i];
    cur.m_iBidiClass =
        gc_FX_BidiNTypes[static_cast<size_t>(FX_GetBidiClass(cur.char_code()))];
  }
}

void ResolveExplicit(std::vector<CFX_Char>* chars, size_t iCount) {
  for (size_t i = 0; i < iCount; ++i)
    (*chars)[i].m_iBidiLevel = 0;
}

void ResolveWeak(std::vector<CFX_Char>* chars, size_t iCount) {
  if (iCount <= 1)
    return;
  --iCount;

  int32_t iLevelCur = 0;
  int32_t iState = FX_BWSxl;
  size_t iNum = 0;
  FX_BIDICLASS eClsCur;
  FX_BIDICLASS eClsRun;
  FX_BIDICLASS eClsNew;
  size_t i = 0;
  for (; i <= iCount; ++i) {
    CFX_Char* pTC = &(*chars)[i];
    eClsCur = pTC->m_iBidiClass;
    if (eClsCur == FX_BIDICLASS::kBN) {
      pTC->m_iBidiLevel = (int16_t)iLevelCur;
      if (i == iCount && iLevelCur != 0) {
        eClsCur = Direction(iLevelCur);
        pTC->m_iBidiClass = eClsCur;
      } else if (i < iCount) {
        CFX_Char* pTCNext = &(*chars)[i + 1];
        int32_t iLevelNext, iLevelNew;
        eClsNew = pTCNext->m_iBidiClass;
        iLevelNext = pTCNext->m_iBidiLevel;
        if (eClsNew != FX_BIDICLASS::kBN && iLevelCur != iLevelNext) {
          iLevelNew = std::max(iLevelNext, iLevelCur);
          pTC->m_iBidiLevel = static_cast<int16_t>(iLevelNew);
          eClsCur = Direction(iLevelNew);
          pTC->m_iBidiClass = eClsCur;
          iLevelCur = iLevelNext;
        } else {
          if (iNum > 0)
            ++iNum;
          continue;
        }
      } else {
        if (iNum > 0)
          ++iNum;
        continue;
      }
    }
    if (eClsCur > FX_BIDICLASS::kBN)
      continue;

    int32_t iAction =
        gc_FX_BidiWeakActions[iState][static_cast<size_t>(eClsCur)];
    eClsRun = GetDeferredType(iAction);
    if (eClsRun != static_cast<FX_BIDICLASS>(0xF) && iNum > 0) {
      SetDeferredRunClass(chars, i, iNum, eClsRun);
      iNum = 0;
    }
    eClsNew = GetResolvedType(iAction);
    if (eClsNew != static_cast<FX_BIDICLASS>(0xF))
      pTC->m_iBidiClass = eClsNew;
    if (FX_BWAIX & iAction)
      ++iNum;

    iState = gc_FX_BidiWeakStates[iState][static_cast<size_t>(eClsCur)];
  }
  if (iNum == 0)
    return;

  eClsCur = Direction(0);
  eClsRun = GetDeferredType(
      gc_FX_BidiWeakActions[iState][static_cast<size_t>(eClsCur)]);
  if (eClsRun != static_cast<FX_BIDICLASS>(0xF))
    SetDeferredRunClass(chars, i, iNum, eClsRun);
}

void ResolveNeutrals(std::vector<CFX_Char>* chars, size_t iCount) {
  if (iCount <= 1)
    return;
  --iCount;

  CFX_Char* pTC;
  int32_t iLevel = 0;
  int32_t iState = FX_BNSl;
  size_t i = 0;
  size_t iNum = 0;
  FX_BIDICLASS eClsCur;
  FX_BIDICLASS eClsRun;
  FX_BIDICLASS eClsNew;
  int32_t iAction;
  for (; i <= iCount; ++i) {
    pTC = &(*chars)[i];
    eClsCur = pTC->m_iBidiClass;
    if (eClsCur == FX_BIDICLASS::kBN) {
      if (iNum)
        ++iNum;
      continue;
    }
    if (eClsCur >= FX_BIDICLASS::kAL)
      continue;

    iAction = gc_FX_BidiNeutralActions[iState][static_cast<size_t>(eClsCur)];
    eClsRun = GetDeferredNeutrals(iAction, iLevel);
    if (eClsRun != FX_BIDICLASS::kN && iNum > 0) {
      SetDeferredRunClass(chars, i, iNum, eClsRun);
      iNum = 0;
    }

    eClsNew = GetResolvedNeutrals(iAction);
    if (eClsNew != FX_BIDICLASS::kN)
      pTC->m_iBidiClass = eClsNew;
    if (FX_BNAIn & iAction)
      ++iNum;

    iState = gc_FX_BidiNeutralStates[iState][static_cast<size_t>(eClsCur)];
    iLevel = pTC->m_iBidiLevel;
  }
  if (iNum == 0)
    return;

  eClsCur = Direction(iLevel);
  eClsRun = GetDeferredNeutrals(
      gc_FX_BidiNeutralActions[iState][static_cast<size_t>(eClsCur)], iLevel);
  if (eClsRun != FX_BIDICLASS::kN)
    SetDeferredRunClass(chars, i, iNum, eClsRun);
}

void ResolveImplicit(std::vector<CFX_Char>* chars, size_t iCount) {
  for (size_t i = 0; i < iCount; ++i) {
    FX_BIDICLASS eCls = (*chars)[i].m_iBidiClass;
    if (eCls == FX_BIDICLASS::kBN || eCls <= FX_BIDICLASS::kON ||
        eCls >= FX_BIDICLASS::kAL) {
      continue;
    }
    (*chars)[i].m_iBidiLevel +=
        gc_FX_BidiAddLevel[FX_IsOdd((*chars)[i].m_iBidiLevel)]
                          [static_cast<size_t>(eCls) - 1];
  }
}

void ResolveWhitespace(std::vector<CFX_Char>* chars, size_t iCount) {
  if (iCount <= 1)
    return;
  iCount--;

  int32_t iLevel = 0;
  size_t i = 0;
  size_t iNum = 0;
  for (; i <= iCount; ++i) {
    switch (static_cast<FX_BIDICLASS>((*chars)[i].m_iBidiClass)) {
      case FX_BIDICLASS::kWS:
        ++iNum;
        break;
      case FX_BIDICLASS::kRLE:
      case FX_BIDICLASS::kLRE:
      case FX_BIDICLASS::kLRO:
      case FX_BIDICLASS::kRLO:
      case FX_BIDICLASS::kPDF:
      case FX_BIDICLASS::kBN:
        (*chars)[i].m_iBidiLevel = static_cast<int16_t>(iLevel);
        ++iNum;
        break;
      case FX_BIDICLASS::kS:
      case FX_BIDICLASS::kB:
        if (iNum > 0)
          SetDeferredRunLevel(chars, i, iNum, 0);

        (*chars)[i].m_iBidiLevel = 0;
        iNum = 0;
        break;
      default:
        iNum = 0;
        break;
    }
    iLevel = (*chars)[i].m_iBidiLevel;
  }
  if (iNum > 0)
    SetDeferredRunLevel(chars, i, iNum, 0);
}

size_t ReorderLevel(std::vector<CFX_Char>* chars,
                    size_t iCount,
                    int32_t iBaseLevel,
                    size_t iStart,
                    bool bReverse) {
  ASSERT(iBaseLevel >= 0);
  ASSERT(iBaseLevel <= kBidiMaxLevel);
  ASSERT(iStart < iCount);

  if (iCount < 1)
    return 0;

  bReverse = bReverse || FX_IsOdd(iBaseLevel);
  size_t i = iStart;
  for (; i < iCount; ++i) {
    int32_t iLevel = (*chars)[i].m_iBidiLevel;
    if (iLevel == iBaseLevel)
      continue;
    if (iLevel < iBaseLevel)
      break;

    i += ReorderLevel(chars, iCount, iBaseLevel + 1, i, bReverse) - 1;
  }

  size_t iNum = i - iStart;
  if (bReverse && iNum > 1)
    ReverseString(chars, iStart, iNum);

  return iNum;
}

void Reorder(std::vector<CFX_Char>* chars, size_t iCount) {
  for (size_t i = 0; i < iCount;)
    i += ReorderLevel(chars, iCount, 0, i, false);
}

void Position(std::vector<CFX_Char>* chars, size_t iCount) {
  for (size_t i = 0; i < iCount; ++i) {
    if ((*chars)[i].m_iBidiPos > iCount)
      continue;

    (*chars)[(*chars)[i].m_iBidiPos].m_iBidiOrder = i;
  }
}

void BidiLine(std::vector<CFX_Char>* chars, size_t iCount) {
  ASSERT(iCount <= chars->size());
  if (iCount < 2)
    return;

  Classify(chars, iCount, false);
  ResolveExplicit(chars, iCount);
  ResolveWeak(chars, iCount);
  ResolveNeutrals(chars, iCount);
  ResolveImplicit(chars, iCount);
  Classify(chars, iCount, true);
  ResolveWhitespace(chars, iCount);
  Reorder(chars, iCount);
  Position(chars, iCount);
}
#endif  // PDF_ENABLE_XFA

}  // namespace

CFX_BidiChar::CFX_BidiChar()
    : m_CurrentSegment({0, 0, NEUTRAL}), m_LastSegment({0, 0, NEUTRAL}) {}

bool CFX_BidiChar::AppendChar(wchar_t wch) {
  Direction direction;
  switch (FX_GetBidiClass(wch)) {
    case FX_BIDICLASS::kL:
    case FX_BIDICLASS::kAN:
    case FX_BIDICLASS::kEN:
      direction = LEFT;
      break;
    case FX_BIDICLASS::kR:
    case FX_BIDICLASS::kAL:
      direction = RIGHT;
      break;
    default:
      direction = NEUTRAL;
      break;
  }

  bool bChangeDirection = (direction != m_CurrentSegment.direction);
  if (bChangeDirection)
    StartNewSegment(direction);

  m_CurrentSegment.count++;
  return bChangeDirection;
}

bool CFX_BidiChar::EndChar() {
  StartNewSegment(NEUTRAL);
  return m_LastSegment.count > 0;
}

void CFX_BidiChar::StartNewSegment(CFX_BidiChar::Direction direction) {
  m_LastSegment = m_CurrentSegment;
  m_CurrentSegment.start += m_CurrentSegment.count;
  m_CurrentSegment.count = 0;
  m_CurrentSegment.direction = direction;
}

CFX_BidiString::CFX_BidiString(const WideString& str) : m_Str(str) {
  CFX_BidiChar bidi;
  for (wchar_t c : m_Str) {
    if (bidi.AppendChar(c))
      m_Order.push_back(bidi.GetSegmentInfo());
  }
  if (bidi.EndChar())
    m_Order.push_back(bidi.GetSegmentInfo());

  size_t nR2L = std::count_if(m_Order.begin(), m_Order.end(),
                              [](const CFX_BidiChar::Segment& seg) {
                                return seg.direction == CFX_BidiChar::RIGHT;
                              });

  size_t nL2R = std::count_if(m_Order.begin(), m_Order.end(),
                              [](const CFX_BidiChar::Segment& seg) {
                                return seg.direction == CFX_BidiChar::LEFT;
                              });

  if (nR2L > 0 && nR2L >= nL2R)
    SetOverallDirectionRight();
}

CFX_BidiString::~CFX_BidiString() {}

CFX_BidiChar::Direction CFX_BidiString::OverallDirection() const {
  ASSERT(m_eOverallDirection != CFX_BidiChar::NEUTRAL);
  return m_eOverallDirection;
}

void CFX_BidiString::SetOverallDirectionRight() {
  if (m_eOverallDirection != CFX_BidiChar::RIGHT) {
    std::reverse(m_Order.begin(), m_Order.end());
    m_eOverallDirection = CFX_BidiChar::RIGHT;
  }
}

#ifdef PDF_ENABLE_XFA
void FX_BidiLine(std::vector<CFX_Char>* chars, size_t iCount) {
  BidiLine(chars, iCount);
}
#endif  // PDF_ENABLE_XFA
