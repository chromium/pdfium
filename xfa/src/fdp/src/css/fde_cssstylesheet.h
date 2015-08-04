// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_CSSSTYLESHEET
#define _FDE_CSSSTYLESHEET
#include "fde_cssdeclaration.h"
class CFDE_CSSSelector : public IFDE_CSSSelector, public CFX_Target {
 public:
  CFDE_CSSSelector(FDE_CSSSELECTORTYPE eType,
                   const FX_WCHAR* psz,
                   int32_t iLen,
                   FX_BOOL bIgnoreCase)
      : m_eType(eType),
        m_dwHash(FX_HashCode_String_GetW(psz, iLen, bIgnoreCase)),
        m_pNext(NULL) {}
  virtual FDE_CSSSELECTORTYPE GetType() const { return m_eType; }

  virtual FX_DWORD GetNameHash() const { return m_dwHash; }

  virtual IFDE_CSSSelector* GetNextSelector() const { return m_pNext; }
  static IFDE_CSSSelector* FromString(IFX_MEMAllocator* pStaticStore,
                                      const FX_WCHAR* psz,
                                      int32_t iLen);
  void SetNext(IFDE_CSSSelector* pNext) { m_pNext = pNext; }

 protected:
  static CFDE_CSSSelector* ParseSelector(IFX_MEMAllocator* pStaticStore,
                                         const FX_WCHAR* psz,
                                         int32_t& iOff,
                                         int32_t iLen,
                                         FDE_CSSSELECTORTYPE eType);
  void SetType(FDE_CSSSELECTORTYPE eType) { m_eType = eType; }
  FDE_CSSSELECTORTYPE m_eType;
  FX_DWORD m_dwHash;
  IFDE_CSSSelector* m_pNext;
};
typedef CFX_ArrayTemplate<IFDE_CSSSelector*> CFDE_CSSSelectorArray;
class CFDE_CSSStyleRule : public IFDE_CSSStyleRule, public CFX_Target {
 public:
  CFDE_CSSStyleRule() : m_ppSelector(NULL), m_iSelectors(0) {}
  virtual int32_t CountSelectorLists() const { return m_iSelectors; }
  virtual IFDE_CSSSelector* GetSelectorList(int32_t index) const {
    return m_ppSelector[index];
  }

  virtual IFDE_CSSDeclaration* GetDeclaration() const {
    return (IFDE_CSSDeclaration*)&m_Declaration;
  }
  CFDE_CSSDeclaration& GetDeclImp() { return m_Declaration; }
  void SetSelector(IFX_MEMAllocator* pStaticStore,
                   const CFDE_CSSSelectorArray& list);

 protected:
  CFDE_CSSDeclaration m_Declaration;
  IFDE_CSSSelector** m_ppSelector;
  int32_t m_iSelectors;
};
class CFDE_CSSMediaRule : public IFDE_CSSMediaRule, public CFX_Target {
 public:
  CFDE_CSSMediaRule(FX_DWORD dwMediaList) : m_dwMediaList(dwMediaList) {}
  ~CFDE_CSSMediaRule();

  virtual FX_DWORD GetMediaList() const { return m_dwMediaList; }

  virtual int32_t CountRules() const { return m_RuleArray.GetSize(); }
  virtual IFDE_CSSRule* GetRule(int32_t index) {
    return m_RuleArray.GetAt(index);
  }
  CFDE_CSSRuleArray& GetArray() { return m_RuleArray; }

 protected:
  FX_DWORD m_dwMediaList;
  CFDE_CSSRuleArray m_RuleArray;
};
class CFDE_CSSFontFaceRule : public IFDE_CSSFontFaceRule, public CFX_Target {
 public:
  virtual IFDE_CSSDeclaration* GetDeclaration() const {
    return (IFDE_CSSDeclaration*)&m_Declaration;
  }
  CFDE_CSSDeclaration& GetDeclImp() { return m_Declaration; }

 protected:
  CFDE_CSSDeclaration m_Declaration;
};
#define FDE_CSSSWITCHDEFAULTS()     \
  case FDE_CSSSYNTAXSTATUS_EOS:     \
    return FDE_CSSSYNTAXSTATUS_EOS; \
  case FDE_CSSSYNTAXSTATUS_Error:   \
  default:                          \
    return FDE_CSSSYNTAXSTATUS_Error;
class CFDE_CSSStyleSheet : public IFDE_CSSStyleSheet, public CFX_Target {
 public:
  CFDE_CSSStyleSheet(FX_DWORD dwMediaList);
  ~CFDE_CSSStyleSheet();
  virtual FX_DWORD AddRef();
  virtual FX_DWORD Release();

  virtual FX_BOOL GetUrl(CFX_WideString& szUrl) {
    szUrl = m_szUrl;
    return szUrl.GetLength() > 0;
  }
  virtual FX_DWORD GetMediaList() const { return m_dwMediaList; }
  virtual FX_WORD GetCodePage() const { return m_wCodePage; }
  virtual int32_t CountRules() const;
  virtual IFDE_CSSRule* GetRule(int32_t index);
  FX_BOOL LoadFromStream(const CFX_WideString& szUrl,
                         IFX_Stream* pStream,
                         FX_WORD wCodePage);
  FX_BOOL LoadFromBuffer(const CFX_WideString& szUrl,
                         const FX_WCHAR* pBuffer,
                         int32_t iBufSize,
                         FX_WORD wCodePage);

 protected:
  void Reset();
  FX_BOOL LoadFromSyntax(IFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSYNTAXSTATUS LoadStyleRule(IFDE_CSSSyntaxParser* pSyntax,
                                    CFDE_CSSRuleArray& ruleArray);
  FDE_CSSSYNTAXSTATUS LoadImportRule(IFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSYNTAXSTATUS LoadPageRule(IFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSYNTAXSTATUS LoadMediaRule(IFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSYNTAXSTATUS LoadFontFaceRule(IFDE_CSSSyntaxParser* pSyntax,
                                       CFDE_CSSRuleArray& ruleArray);
  FDE_CSSSYNTAXSTATUS SkipRuleSet(IFDE_CSSSyntaxParser* pSyntax);
  FX_WORD m_wCodePage;
  FX_WORD m_wRefCount;
  FX_DWORD m_dwMediaList;
  IFX_MEMAllocator* m_pAllocator;
  CFDE_CSSRuleArray m_RuleArray;
  CFX_WideString m_szUrl;
  CFDE_CSSSelectorArray m_Selectors;
  CFX_MapPtrToPtr m_StringCache;
};
#endif
