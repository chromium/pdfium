// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_XMLLOCALE_H_
#define XFA_FXFA_PARSER_CXFA_XMLLOCALE_H_

#include <memory>

#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/gc/heap.h"
#include "third_party/base/containers/span.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "xfa/fxfa/parser/gced_locale_iface.h"

class CFX_XMLDocument;
class CFX_XMLElement;

class CXFA_XMLLocale final : public GCedLocaleIface {
 public:
  // Object is created on cppgc heap.
  static CXFA_XMLLocale* Create(cppgc::Heap* heap, pdfium::span<uint8_t> data);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_XMLLocale() override;

  // GCedLocaleIface:
  void Trace(cppgc::Visitor* visitor) const override;
  WideString GetName() const override;
  WideString GetDecimalSymbol() const override;
  WideString GetGroupingSymbol() const override;
  WideString GetPercentSymbol() const override;
  WideString GetMinusSymbol() const override;
  WideString GetCurrencySymbol() const override;
  WideString GetDateTimeSymbols() const override;
  WideString GetMonthName(int32_t nMonth, bool bAbbr) const override;
  WideString GetDayName(int32_t nWeek, bool bAbbr) const override;
  WideString GetMeridiemName(bool bAM) const override;
  int GetTimeZoneInMinutes() const override;
  WideString GetEraName(bool bAD) const override;

  WideString GetDatePattern(DateTimeSubcategory eType) const override;
  WideString GetTimePattern(DateTimeSubcategory eType) const override;
  WideString GetNumPattern(NumSubcategory eType) const override;

 private:
  CXFA_XMLLocale(std::unique_ptr<CFX_XMLDocument> root,
                 const CFX_XMLElement* locale);

  WideString GetPattern(CFX_XMLElement* pElement,
                        WideStringView bsTag,
                        WideStringView wsName) const;
  WideString GetCalendarSymbol(WideStringView symbol,
                               size_t index,
                               bool bAbbr) const;

  std::unique_ptr<CFX_XMLDocument> xml_doc_;
  UnownedPtr<const CFX_XMLElement> locale_;
};

#endif  // XFA_FXFA_PARSER_CXFA_XMLLOCALE_H_
