// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LOCALIZATION_FGAS_LOCALEMGR_H_
#define XFA_FGAS_LOCALIZATION_FGAS_LOCALEMGR_H_

#include "xfa/fgas/localization/fgas_locale.h"

class CFX_LocaleMgr : public IFX_LocaleMgr {
 public:
  CFX_LocaleMgr(uint16_t wDefLCID);

  void Release() override { delete this; }
  uint16_t GetDefLocaleID() override;
  IFX_Locale* GetDefLocale() override;
  IFX_Locale* GetLocale(uint16_t lcid) override;
  IFX_Locale* GetLocaleByName(const CFX_WideString& wsLocaleName) override;

  CFX_MapPtrToPtr m_lcid2xml;

 protected:
  ~CFX_LocaleMgr() override;
  CFX_MapPtrToPtr m_lcid2locale;
  uint16_t m_wDefLCID;
};

#endif  // XFA_FGAS_LOCALIZATION_FGAS_LOCALEMGR_H_
