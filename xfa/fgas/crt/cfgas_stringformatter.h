// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_CRT_CFGAS_STRINGFORMATTER_H_
#define XFA_FGAS_CRT_CFGAS_STRINGFORMATTER_H_

#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "third_party/base/span.h"
#include "xfa/fgas/crt/locale_iface.h"
#include "xfa/fgas/crt/locale_mgr_iface.h"

bool FX_DateFromCanonical(pdfium::span<const wchar_t> wsTime,
                          CFX_DateTime* datetime);
bool FX_TimeFromCanonical(const LocaleIface* pLocale,
                          pdfium::span<const wchar_t> wsTime,
                          CFX_DateTime* datetime);

class CFGAS_StringFormatter {
 public:
  CFGAS_StringFormatter(LocaleMgrIface* pLocaleMgr,
                        const WideString& wsPattern);
  ~CFGAS_StringFormatter();

  static std::vector<WideString> SplitOnBars(const WideString& wsFormatString);

  FX_LOCALECATEGORY GetCategory() const;

  bool ParseText(const WideString& wsSrcText,
                 WideString* wsValue) const;
  bool ParseNum(const WideString& wsSrcNum,
                WideString* wsValue) const;
  bool ParseDateTime(const WideString& wsSrcDateTime,
                     FX_DATETIMETYPE eDateTimeType,
                     CFX_DateTime* dtValue) const;
  bool ParseZero(const WideString& wsSrcText) const;
  bool ParseNull(const WideString& wsSrcText) const;

  bool FormatText(const WideString& wsSrcText,
                  WideString* wsOutput) const;
  bool FormatNum(const WideString& wsSrcNum,
                 WideString* wsOutput) const;
  bool FormatDateTime(const WideString& wsSrcDateTime,
                      FX_DATETIMETYPE eDateTimeType,
                      WideString* wsOutput) const;
  bool FormatZero(WideString* wsOutput) const;
  bool FormatNull(WideString* wsOutput) const;

 private:
  WideString GetTextFormat(WideStringView wsCategory) const;
  LocaleIface* GetNumericFormat(size_t* iDotIndex,
                                uint32_t* dwStyle,
                                WideString* wsPurgePattern) const;
  FX_DATETIMETYPE GetDateTimeFormat(LocaleIface** pLocale,
                                    WideString* wsDatePattern,
                                    WideString* wsTimePattern) const;

  UnownedPtr<LocaleMgrIface> const m_pLocaleMgr;
  const WideString m_wsPattern;                   // keep pattern string alive.
  const pdfium::span<const wchar_t> m_spPattern;  // span into |m_wsPattern|.
};

#endif  // XFA_FGAS_CRT_CFGAS_STRINGFORMATTER_H_
