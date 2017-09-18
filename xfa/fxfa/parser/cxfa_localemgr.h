// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_LOCALEMGR_H_
#define XFA_FXFA_PARSER_CXFA_LOCALEMGR_H_

#include <memory>
#include <vector>

#include "core/fxcrt/cfx_datetime.h"
#include "core/fxcrt/ifx_locale.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"

class CXFA_Node;
class IFX_Locale;

class CXFA_LocaleMgr {
 public:
  CXFA_LocaleMgr(CXFA_Node* pLocaleSet, WideString wsDeflcid);
  ~CXFA_LocaleMgr();

  uint16_t GetDefLocaleID() const;
  IFX_Locale* GetDefLocale();
  IFX_Locale* GetLocaleByName(const WideString& wsLocaleName);

  void SetDefLocale(IFX_Locale* pLocale);
  WideStringView GetConfigLocaleName(CXFA_Node* pConfig);

 private:
  std::unique_ptr<IFX_Locale> GetLocale(uint16_t lcid);

  std::vector<std::unique_ptr<IFX_Locale>> m_LocaleArray;
  std::vector<std::unique_ptr<IFX_Locale>> m_XMLLocaleArray;
  IFX_Locale* m_pDefLocale;  // owned by m_LocaleArray or m_XMLLocaleArray.
  WideString m_wsConfigLocale;
  uint16_t m_dwDeflcid;
  uint16_t m_dwLocaleFlags;
};

#endif  // XFA_FXFA_PARSER_CXFA_LOCALEMGR_H_
