// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_CRT_CFGAS_FORMATSTRING_H_
#define XFA_FGAS_CRT_CFGAS_FORMATSTRING_H_

#include <vector>

#include "core/fxcrt/ifx_locale.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"

bool FX_DateFromCanonical(const CFX_WideString& wsDate, CFX_DateTime* datetime);
bool FX_TimeFromCanonical(const CFX_WideStringC& wsTime,
                          CFX_DateTime* datetime,
                          IFX_Locale* pLocale);

class CFGAS_FormatString {
 public:
  explicit CFGAS_FormatString(CXFA_LocaleMgr* pLocaleMgr);
  ~CFGAS_FormatString();

  void SplitFormatString(const CFX_WideString& wsFormatString,
                         std::vector<CFX_WideString>* wsPatterns);
  FX_LOCALECATEGORY GetCategory(const CFX_WideString& wsPattern);

  bool ParseText(const CFX_WideString& wsSrcText,
                 const CFX_WideString& wsPattern,
                 CFX_WideString* wsValue);
  bool ParseNum(const CFX_WideString& wsSrcNum,
                const CFX_WideString& wsPattern,
                CFX_WideString* wsValue);
  bool ParseDateTime(const CFX_WideString& wsSrcDateTime,
                     const CFX_WideString& wsPattern,
                     FX_DATETIMETYPE eDateTimeType,
                     CFX_DateTime* dtValue);
  bool ParseZero(const CFX_WideString& wsSrcText,
                 const CFX_WideString& wsPattern);
  bool ParseNull(const CFX_WideString& wsSrcText,
                 const CFX_WideString& wsPattern);

  bool FormatText(const CFX_WideString& wsSrcText,
                  const CFX_WideString& wsPattern,
                  CFX_WideString* wsOutput);
  bool FormatNum(const CFX_WideString& wsSrcNum,
                 const CFX_WideString& wsPattern,
                 CFX_WideString* wsOutput);
  bool FormatDateTime(const CFX_WideString& wsSrcDateTime,
                      const CFX_WideString& wsPattern,
                      FX_DATETIMETYPE eDateTimeType,
                      CFX_WideString* wsOutput);
  bool FormatZero(const CFX_WideString& wsPattern, CFX_WideString* wsOutput);
  bool FormatNull(const CFX_WideString& wsPattern, CFX_WideString* wsOutput);

 private:
  CFX_WideString GetTextFormat(const CFX_WideString& wsPattern,
                               const CFX_WideStringC& wsCategory);
  IFX_Locale* GetNumericFormat(const CFX_WideString& wsPattern,
                               int32_t* iDotIndex,
                               uint32_t* dwStyle,
                               CFX_WideString* wsPurgePattern);
  bool FormatStrNum(const CFX_WideStringC& wsInputNum,
                    const CFX_WideString& wsPattern,
                    CFX_WideString* wsOutput);
  FX_DATETIMETYPE GetDateTimeFormat(const CFX_WideString& wsPattern,
                                    IFX_Locale** pLocale,
                                    CFX_WideString* wsDatePattern,
                                    CFX_WideString* wsTimePattern);

  CXFA_LocaleMgr* m_pLocaleMgr;
};

#endif  // XFA_FGAS_CRT_CFGAS_FORMATSTRING_H_
