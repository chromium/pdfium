// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_LOCALEMGR_H
#define _FXFA_LOCALEMGR_H
class CXFA_Node;
class IFX_Locale;
class IFX_LocaleMgr;
class CXFA_LocaleMgr;
#define XFA_LANGID_zh_CN 0x0804
#define XFA_LANGID_zh_TW 0x0404
#define XFA_LANGID_zh_HK 0x0c04
#define XFA_LANGID_ja_JP 0x0411
#define XFA_LANGID_ko_KR 0x0412
#define XFA_LANGID_en_US 0x0409
#define XFA_LANGID_en_GB 0x0809
#define XFA_LANGID_es_ES 0x0c0a
#define XFA_LANGID_es_LA 0x080a
#define XFA_LANGID_de_DE 0x0407
#define XFA_LANGID_fr_FR 0x040c
#define XFA_LANGID_it_IT 0x0410
#define XFA_LANGID_pt_BR 0x0416
#define XFA_LANGID_nl_NL 0x0413
#define XFA_LANGID_ru_RU 0x0419
class CXFA_LocaleMgr : public IFX_LocaleMgr {
 public:
  CXFA_LocaleMgr(CXFA_Node* pLocaleSet, CFX_WideString wsDeflcid);
  virtual void Release();
  virtual FX_WORD GetDefLocaleID();
  virtual IFX_Locale* GetDefLocale();
  virtual IFX_Locale* GetLocale(FX_WORD lcid);
  virtual IFX_Locale* GetLocaleByName(const CFX_WideStringC& wsLocaleName);
  ~CXFA_LocaleMgr();
  void SetDefLocale(IFX_Locale* pLocale);
  CFX_WideStringC GetConfigLocaleName(CXFA_Node* pConfig);

 protected:
  CFX_PtrArray m_LocaleArray;
  CFX_PtrArray m_XMLLocaleArray;
  IFX_Locale* m_pDefLocale;
  CFX_WideString m_wsConfigLocale;
  FX_WORD m_dwDeflcid;
  FX_WORD m_dwLocaleFlags;
};
class IXFA_TimeZoneProvider {
 public:
  static IXFA_TimeZoneProvider* Create();
  static IXFA_TimeZoneProvider* Get();
  static void Destroy();

  virtual ~IXFA_TimeZoneProvider() {}

  virtual void SetTimeZone(FX_TIMEZONE& tz) = 0;

  virtual void GetTimeZone(FX_TIMEZONE& tz) = 0;
};
class CXFA_TimeZoneProvider : public IXFA_TimeZoneProvider {
 public:
  CXFA_TimeZoneProvider();
  virtual ~CXFA_TimeZoneProvider();
  virtual void SetTimeZone(FX_TIMEZONE& tz);
  virtual void GetTimeZone(FX_TIMEZONE& tz);

 private:
  FX_TIMEZONE m_tz;
};
#endif
