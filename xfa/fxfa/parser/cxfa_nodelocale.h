// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_NODELOCALE_H_
#define XFA_FXFA_PARSER_CXFA_NODELOCALE_H_

#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "xfa/fxfa/fxfa_basic.h"
#include "xfa/fxfa/parser/gced_locale_iface.h"

class CXFA_Node;

WideString XFA_PatternToString(LocaleIface::NumSubcategory category);

class CXFA_NodeLocale final : public GCedLocaleIface {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_NodeLocale() override;

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
  explicit CXFA_NodeLocale(CXFA_Node* pNode);

  CXFA_Node* GetNodeByName(CXFA_Node* pParent, WideStringView wsName) const;
  WideString GetSymbol(XFA_Element eElement, WideStringView symbol_type) const;
  WideString GetCalendarSymbol(XFA_Element eElement,
                               int index,
                               bool bAbbr) const;

  cppgc::Member<CXFA_Node> const m_pNode;
};

#endif  // XFA_FXFA_PARSER_CXFA_NODELOCALE_H_
