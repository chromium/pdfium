// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_LOCALEMGR_IMP_H_
#define _FX_LOCALEMGR_IMP_H_
class CFX_LocaleMgr : public IFX_LocaleMgr {
 public:
  CFX_LocaleMgr(FX_WORD wDefLCID);
  virtual void Release() { delete this; }
  virtual FX_WORD GetDefLocaleID();
  virtual IFX_Locale* GetDefLocale();
  virtual IFX_Locale* GetLocale(FX_WORD lcid);
  virtual IFX_Locale* GetLocaleByName(const CFX_WideStringC& wsLocaleName);
  CFX_MapPtrToPtr m_lcid2xml;

 protected:
  ~CFX_LocaleMgr();
  CFX_MapPtrToPtr m_lcid2locale;
  FX_WORD m_wDefLCID;
};
#endif
