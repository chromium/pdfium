// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_ARB_H_
#define CORE_INCLUDE_FXCRT_FX_ARB_H_

#include "fx_system.h"
#include "fx_ucd.h"

class IFX_ArabicChar;

struct FX_ARBFORMTABLE {
  FX_WCHAR wIsolated;
  FX_WCHAR wFinal;
  FX_WCHAR wInitial;
  FX_WCHAR wMedial;
};

struct FX_ARAALEF {
  FX_WCHAR wAlef;
  FX_WCHAR wIsolated;
};

struct FX_ARASHADDA {
  FX_WCHAR wShadda;
  FX_WCHAR wIsolated;
};

const FX_ARBFORMTABLE* FX_GetArabicFormTable(FX_WCHAR unicode);
FX_WCHAR FX_GetArabicFromAlefTable(FX_WCHAR alef);
FX_WCHAR FX_GetArabicFromShaddaTable(FX_WCHAR shadda);

enum FX_ARBPOSITION {
  FX_ARBPOSITION_Isolated = 0,
  FX_ARBPOSITION_Final,
  FX_ARBPOSITION_Initial,
  FX_ARBPOSITION_Medial,
};

class IFX_ArabicChar {
 public:
  static IFX_ArabicChar* Create();
  virtual ~IFX_ArabicChar() {}
  virtual void Release() = 0;
  virtual FX_BOOL IsArabicChar(FX_WCHAR wch) const = 0;
  virtual FX_BOOL IsArabicFormChar(FX_WCHAR wch) const = 0;
  virtual FX_WCHAR GetFormChar(FX_WCHAR wch,
                               FX_WCHAR prev = 0,
                               FX_WCHAR next = 0) const = 0;
  virtual FX_WCHAR GetFormChar(const CFX_Char* cur,
                               const CFX_Char* prev,
                               const CFX_Char* next) const = 0;
};

void FX_BidiLine(CFX_WideString& wsText, int32_t iBaseLevel = 0);
void FX_BidiLine(CFX_TxtCharArray& chars,
                 int32_t iCount,
                 int32_t iBaseLevel = 0);
void FX_BidiLine(CFX_RTFCharArray& chars,
                 int32_t iCount,
                 int32_t iBaseLevel = 0);

#endif  // CORE_INCLUDE_FXCRT_FX_ARB_H_
