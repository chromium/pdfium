// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "fde_csssyntax.h"
#include "fde_cssdatatable.h"
#include "fde_cssstylesheet.h"
IFDE_CSSStyleSheet* IFDE_CSSStyleSheet::LoadHTMLStandardStyleSheet() {
  static const FX_WCHAR* s_pStyle =
      L"html,address,blockquote,body,dd,div,dl,dt,fieldset,form,frame,frameset,"
      L"h1,h2,h3,h4,h5,h6,noframes,ol,p,ul,center,dir,hr,menu,pre{display:"
      L"block}"
      L"li{display:list-item}head{display:none}table{display:table}tr{display:"
      L"table-row}thead{display:table-header-group}tbody{display:table-row-"
      L"group}tfoot{display:table-footer-group}"
      L"col{display:table-column}colgroup{display:table-column-group}td,th{"
      L"display:table-cell}caption{display:table-caption}th{font-weight:bolder;"
      L"text-align:center}caption{text-align:center}"
      L"body{margin:0}h1{font-size:2em;margin:.67em "
      L"0}h2{font-size:1.5em;margin:.75em 0}h3{font-size:1.17em;margin:.83em "
      L"0}h4,p,blockquote,ul,fieldset,form,ol,dl,dir,menu{margin:1.12em 0}"
      L"h5{font-size:.83em;margin:1.5em 0}h6{font-size:.75em;margin:1.67em "
      L"0}h1,h2,h3,h4,h5,h6,b,strong{font-weight:bolder}blockquote{margin-left:"
      L"40px;margin-right:40px}i,cite,em,var,address{font-style:italic}"
      L"pre,tt,code,kbd,samp{font-family:monospace}pre{white-space:pre}button,"
      L"textarea,input,select{display:inline-block}big{font-size:1.17em}small,"
      L"sub,sup{font-size:.83em}sub{vertical-align:sub}"
      L"sup{vertical-align:super}table{border-spacing:2px}thead,tbody,tfoot{"
      L"vertical-align:middle}td,th,tr{vertical-align:inherit}s,strike,del{"
      L"text-decoration:line-through}hr{border:1px inset silver}"
      L"ol,ul,dir,menu,dd{margin-left:40px}ol{list-style-type:decimal}ol ul,ul "
      L"ol,ul ul,ol "
      L"ol{margin-top:0;margin-bottom:0}u,ins{text-decoration:underline}center{"
      L"text-align:center}"
      L"ruby{display:ruby}rt{display:ruby-text;font-size:.5em}rb{display:ruby-"
      L"base}rbc{display:ruby-base-group}rtc{display:ruby-text-group}"
      L"q:before{content:open-quote}q:after{content:close-quote}"
      L"rp{display:none}";
  return IFDE_CSSStyleSheet::LoadFromBuffer(
      CFX_WideString(), s_pStyle, FXSYS_wcslen(s_pStyle), FX_CODEPAGE_UTF8);
}
IFDE_CSSStyleSheet* IFDE_CSSStyleSheet::LoadFromStream(
    const CFX_WideString& szUrl,
    IFX_Stream* pStream,
    FX_WORD wCodePage,
    FX_DWORD dwMediaList) {
  CFDE_CSSStyleSheet* pStyleSheet = new CFDE_CSSStyleSheet(dwMediaList);
  if (!pStyleSheet->LoadFromStream(szUrl, pStream, wCodePage)) {
    pStyleSheet->Release();
    pStyleSheet = NULL;
  }
  return pStyleSheet;
}
IFDE_CSSStyleSheet* IFDE_CSSStyleSheet::LoadFromBuffer(
    const CFX_WideString& szUrl,
    const FX_WCHAR* pBuffer,
    int32_t iBufSize,
    FX_WORD wCodePage,
    FX_DWORD dwMediaList) {
  CFDE_CSSStyleSheet* pStyleSheet = new CFDE_CSSStyleSheet(dwMediaList);
  if (!pStyleSheet->LoadFromBuffer(szUrl, pBuffer, iBufSize, wCodePage)) {
    pStyleSheet->Release();
    pStyleSheet = NULL;
  }
  return pStyleSheet;
}
CFDE_CSSStyleSheet::CFDE_CSSStyleSheet(FX_DWORD dwMediaList)
    : m_wCodePage(FX_CODEPAGE_UTF8),
      m_wRefCount(1),
      m_dwMediaList(dwMediaList),
      m_pAllocator(NULL) {
  FXSYS_assert(m_dwMediaList > 0);
}
CFDE_CSSStyleSheet::~CFDE_CSSStyleSheet() {
  Reset();
}
void CFDE_CSSStyleSheet::Reset() {
  for (int32_t i = m_RuleArray.GetSize() - 1; i >= 0; --i) {
    IFDE_CSSRule* pRule = m_RuleArray.GetAt(i);
    switch (pRule->GetType()) {
      case FDE_CSSRULETYPE_Style:
        ((CFDE_CSSStyleRule*)pRule)->~CFDE_CSSStyleRule();
        break;
      case FDE_CSSRULETYPE_Media:
        ((CFDE_CSSMediaRule*)pRule)->~CFDE_CSSMediaRule();
        break;
      case FDE_CSSRULETYPE_FontFace:
        ((CFDE_CSSFontFaceRule*)pRule)->~CFDE_CSSFontFaceRule();
        break;
      default:
        FXSYS_assert(FALSE);
        break;
    }
  }
  m_RuleArray.RemoveAll();
  m_Selectors.RemoveAll();
  m_StringCache.RemoveAll();
  if (m_pAllocator) {
    m_pAllocator->Release();
    m_pAllocator = NULL;
  }
}
FX_DWORD CFDE_CSSStyleSheet::AddRef() {
  return ++m_wRefCount;
}
FX_DWORD CFDE_CSSStyleSheet::Release() {
  FX_DWORD dwRefCount = --m_wRefCount;
  if (dwRefCount == 0) {
    delete this;
  }
  return dwRefCount;
}
int32_t CFDE_CSSStyleSheet::CountRules() const {
  return m_RuleArray.GetSize();
}
IFDE_CSSRule* CFDE_CSSStyleSheet::GetRule(int32_t index) {
  return m_RuleArray.GetAt(index);
}
FX_BOOL CFDE_CSSStyleSheet::LoadFromStream(const CFX_WideString& szUrl,
                                           IFX_Stream* pStream,
                                           FX_WORD wCodePage) {
  FXSYS_assert(pStream != NULL);
  IFDE_CSSSyntaxParser* pSyntax = IFDE_CSSSyntaxParser::Create();
  if (pSyntax == NULL) {
    return FALSE;
  }
  if (pStream->GetCodePage() != wCodePage) {
    pStream->SetCodePage(wCodePage);
  }
  FX_BOOL bRet = pSyntax->Init(pStream, 4096) && LoadFromSyntax(pSyntax);
  pSyntax->Release();
  m_wCodePage = wCodePage;
  m_szUrl = szUrl;
  return bRet;
}
FX_BOOL CFDE_CSSStyleSheet::LoadFromBuffer(const CFX_WideString& szUrl,
                                           const FX_WCHAR* pBuffer,
                                           int32_t iBufSize,
                                           FX_WORD wCodePage) {
  FXSYS_assert(pBuffer != NULL && iBufSize > 0);
  IFDE_CSSSyntaxParser* pSyntax = IFDE_CSSSyntaxParser::Create();
  if (pSyntax == NULL) {
    return FALSE;
  }
  FX_BOOL bRet = pSyntax->Init(pBuffer, iBufSize) && LoadFromSyntax(pSyntax);
  pSyntax->Release();
  m_wCodePage = wCodePage;
  m_szUrl = szUrl;
  return bRet;
}
FX_BOOL CFDE_CSSStyleSheet::LoadFromSyntax(IFDE_CSSSyntaxParser* pSyntax) {
  Reset();
  m_pAllocator = FX_CreateAllocator(FX_ALLOCTYPE_Static, 1024, 0);
  if (m_pAllocator == NULL) {
    return FALSE;
  }
  FDE_CSSSYNTAXSTATUS eStatus;
  do {
    switch (eStatus = pSyntax->DoSyntaxParse()) {
      case FDE_CSSSYNTAXSTATUS_StyleRule:
        eStatus = LoadStyleRule(pSyntax, m_RuleArray);
        break;
      case FDE_CSSSYNTAXSTATUS_MediaRule:
        eStatus = LoadMediaRule(pSyntax);
        break;
      case FDE_CSSSYNTAXSTATUS_FontFaceRule:
        eStatus = LoadFontFaceRule(pSyntax, m_RuleArray);
        break;
      case FDE_CSSSYNTAXSTATUS_ImportRule:
        eStatus = LoadImportRule(pSyntax);
        break;
      case FDE_CSSSYNTAXSTATUS_PageRule:
        eStatus = LoadPageRule(pSyntax);
        break;
      default:
        break;
    }
  } while (eStatus >= FDE_CSSSYNTAXSTATUS_None);
  m_Selectors.RemoveAll();
  m_StringCache.RemoveAll();
  return eStatus != FDE_CSSSYNTAXSTATUS_Error;
}
FDE_CSSSYNTAXSTATUS CFDE_CSSStyleSheet::LoadMediaRule(
    IFDE_CSSSyntaxParser* pSyntax) {
  FX_DWORD dwMediaList = 0;
  CFDE_CSSMediaRule* pMediaRule = NULL;
  for (;;) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSYNTAXSTATUS_MediaType: {
        int32_t iLen;
        const FX_WCHAR* psz = pSyntax->GetCurrentString(iLen);
        FDE_LPCCSSMEDIATYPETABLE pMediaType =
            FDE_GetCSSMediaTypeByName(psz, iLen);
        if (pMediaType != NULL) {
          dwMediaList |= pMediaType->wValue;
        }
      } break;
      case FDE_CSSSYNTAXSTATUS_StyleRule:
        if (pMediaRule == NULL) {
          SkipRuleSet(pSyntax);
        } else {
          FDE_CSSSYNTAXSTATUS eStatus =
              LoadStyleRule(pSyntax, pMediaRule->GetArray());
          if (eStatus < FDE_CSSSYNTAXSTATUS_None) {
            return eStatus;
          }
        }
        break;
      case FDE_CSSSYNTAXSTATUS_DeclOpen:
        if ((dwMediaList & m_dwMediaList) > 0 && pMediaRule == NULL) {
          pMediaRule = FDE_NewWith(m_pAllocator) CFDE_CSSMediaRule(dwMediaList);
          m_RuleArray.Add(pMediaRule);
        }
        break;
      case FDE_CSSSYNTAXSTATUS_DeclClose:
        return FDE_CSSSYNTAXSTATUS_None;
        FDE_CSSSWITCHDEFAULTS();
    }
  }
}
FDE_CSSSYNTAXSTATUS CFDE_CSSStyleSheet::LoadStyleRule(
    IFDE_CSSSyntaxParser* pSyntax,
    CFDE_CSSRuleArray& ruleArray) {
  m_Selectors.RemoveAt(0, m_Selectors.GetSize());
  CFDE_CSSStyleRule* pStyleRule = NULL;
  const FX_WCHAR* pszValue = NULL;
  int32_t iValueLen = 0;
  FDE_CSSPROPERTYARGS propertyArgs;
  propertyArgs.pStaticStore = m_pAllocator;
  propertyArgs.pStringCache = &m_StringCache;
  propertyArgs.pProperty = NULL;
  CFX_WideString wsName;
  for (;;) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSYNTAXSTATUS_Selector: {
        pszValue = pSyntax->GetCurrentString(iValueLen);
        IFDE_CSSSelector* pSelector =
            CFDE_CSSSelector::FromString(m_pAllocator, pszValue, iValueLen);
        if (pSelector != NULL) {
          m_Selectors.Add(pSelector);
        }
      } break;
      case FDE_CSSSYNTAXSTATUS_PropertyName:
        pszValue = pSyntax->GetCurrentString(iValueLen);
        propertyArgs.pProperty = FDE_GetCSSPropertyByName(pszValue, iValueLen);
        if (propertyArgs.pProperty == NULL) {
          wsName = CFX_WideStringC(pszValue, iValueLen);
        }
        break;
      case FDE_CSSSYNTAXSTATUS_PropertyValue:
        if (propertyArgs.pProperty != NULL) {
          pszValue = pSyntax->GetCurrentString(iValueLen);
          if (iValueLen > 0) {
            pStyleRule->GetDeclImp().AddProperty(&propertyArgs, pszValue,
                                                 iValueLen);
          }
        } else if (iValueLen > 0) {
          pszValue = pSyntax->GetCurrentString(iValueLen);
          if (iValueLen > 0) {
            pStyleRule->GetDeclImp().AddProperty(
                &propertyArgs, wsName, wsName.GetLength(), pszValue, iValueLen);
          }
        }
        break;
      case FDE_CSSSYNTAXSTATUS_DeclOpen:
        if (pStyleRule == NULL && m_Selectors.GetSize() > 0) {
          pStyleRule = FDE_NewWith(m_pAllocator) CFDE_CSSStyleRule;
          pStyleRule->SetSelector(m_pAllocator, m_Selectors);
          ruleArray.Add(pStyleRule);
        } else {
          SkipRuleSet(pSyntax);
          return FDE_CSSSYNTAXSTATUS_None;
        }
        break;
      case FDE_CSSSYNTAXSTATUS_DeclClose:
        if (pStyleRule != NULL &&
            pStyleRule->GetDeclImp().GetStartPosition() == NULL) {
          pStyleRule->~CFDE_CSSStyleRule();
          ruleArray.RemoveLast(1);
        }
        return FDE_CSSSYNTAXSTATUS_None;
        FDE_CSSSWITCHDEFAULTS();
    }
  }
}
FDE_CSSSYNTAXSTATUS CFDE_CSSStyleSheet::LoadFontFaceRule(
    IFDE_CSSSyntaxParser* pSyntax,
    CFDE_CSSRuleArray& ruleArray) {
  CFDE_CSSFontFaceRule* pFontFaceRule = NULL;
  const FX_WCHAR* pszValue = NULL;
  int32_t iValueLen = 0;
  FDE_CSSPROPERTYARGS propertyArgs;
  propertyArgs.pStaticStore = m_pAllocator;
  propertyArgs.pStringCache = &m_StringCache;
  propertyArgs.pProperty = NULL;
  for (;;) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSYNTAXSTATUS_PropertyName:
        pszValue = pSyntax->GetCurrentString(iValueLen);
        propertyArgs.pProperty = FDE_GetCSSPropertyByName(pszValue, iValueLen);
        break;
      case FDE_CSSSYNTAXSTATUS_PropertyValue:
        if (propertyArgs.pProperty != NULL) {
          pszValue = pSyntax->GetCurrentString(iValueLen);
          if (iValueLen > 0) {
            pFontFaceRule->GetDeclImp().AddProperty(&propertyArgs, pszValue,
                                                    iValueLen);
          }
        }
        break;
      case FDE_CSSSYNTAXSTATUS_DeclOpen:
        if (pFontFaceRule == NULL) {
          pFontFaceRule = FDE_NewWith(m_pAllocator) CFDE_CSSFontFaceRule;
          ruleArray.Add(pFontFaceRule);
        }
        break;
      case FDE_CSSSYNTAXSTATUS_DeclClose:
        return FDE_CSSSYNTAXSTATUS_None;
        FDE_CSSSWITCHDEFAULTS();
    }
  }
  return FDE_CSSSYNTAXSTATUS_None;
}
FDE_CSSSYNTAXSTATUS CFDE_CSSStyleSheet::LoadImportRule(
    IFDE_CSSSyntaxParser* pSyntax) {
  for (;;) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSYNTAXSTATUS_ImportClose:
        return FDE_CSSSYNTAXSTATUS_None;
      case FDE_CSSSYNTAXSTATUS_URI:
        break;
        FDE_CSSSWITCHDEFAULTS();
    }
  }
}
FDE_CSSSYNTAXSTATUS CFDE_CSSStyleSheet::LoadPageRule(
    IFDE_CSSSyntaxParser* pSyntax) {
  return SkipRuleSet(pSyntax);
}
FDE_CSSSYNTAXSTATUS CFDE_CSSStyleSheet::SkipRuleSet(
    IFDE_CSSSyntaxParser* pSyntax) {
  for (;;) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSYNTAXSTATUS_Selector:
      case FDE_CSSSYNTAXSTATUS_DeclOpen:
      case FDE_CSSSYNTAXSTATUS_PropertyName:
      case FDE_CSSSYNTAXSTATUS_PropertyValue:
        break;
      case FDE_CSSSYNTAXSTATUS_DeclClose:
        return FDE_CSSSYNTAXSTATUS_None;
        FDE_CSSSWITCHDEFAULTS();
    }
  }
  return FDE_CSSSYNTAXSTATUS_None;
}
void CFDE_CSSStyleRule::SetSelector(IFX_MEMAllocator* pStaticStore,
                                    const CFDE_CSSSelectorArray& list) {
  FXSYS_assert(m_ppSelector == NULL);
  m_iSelectors = list.GetSize();
  m_ppSelector = (IFDE_CSSSelector**)pStaticStore->Alloc(
      m_iSelectors * sizeof(IFDE_CSSSelector*));
  for (int32_t i = 0; i < m_iSelectors; ++i) {
    m_ppSelector[i] = list.GetAt(i);
  }
}
CFDE_CSSMediaRule::~CFDE_CSSMediaRule() {
  for (int32_t i = m_RuleArray.GetSize() - 1; i >= 0; --i) {
    IFDE_CSSRule* pRule = m_RuleArray.GetAt(i);
    switch (pRule->GetType()) {
      case FDE_CSSRULETYPE_Style:
        ((CFDE_CSSStyleRule*)pRule)->~CFDE_CSSStyleRule();
        break;
      default:
        FXSYS_assert(FALSE);
        break;
    }
  }
}
inline FX_BOOL FDE_IsCSSChar(FX_WCHAR wch) {
  return (wch >= 'a' && wch <= 'z') || (wch >= 'A' && wch <= 'Z');
}
int32_t FDE_GetCSSPersudoLen(const FX_WCHAR* psz, const FX_WCHAR* pEnd) {
  FXSYS_assert(*psz == ':');
  const FX_WCHAR* pStart = psz;
  while (psz < pEnd) {
    FX_WCHAR wch = *psz;
    if (FDE_IsCSSChar(wch) || wch == ':') {
      ++psz;
    } else {
      break;
    }
  }
  return psz - pStart;
}
int32_t FDE_GetCSSNameLen(const FX_WCHAR* psz, const FX_WCHAR* pEnd) {
  const FX_WCHAR* pStart = psz;
  while (psz < pEnd) {
    FX_WCHAR wch = *psz;
    if (FDE_IsCSSChar(wch) || (wch >= '0' && wch <= '9') || wch == '_' ||
        wch == '-') {
      ++psz;
    } else {
      break;
    }
  }
  return psz - pStart;
}
IFDE_CSSSelector* CFDE_CSSSelector::FromString(IFX_MEMAllocator* pStaticStore,
                                               const FX_WCHAR* psz,
                                               int32_t iLen) {
  FXSYS_assert(pStaticStore != NULL && psz != NULL && iLen > 0);
  const FX_WCHAR* pStart = psz;
  const FX_WCHAR* pEnd = psz + iLen;
  for (; psz < pEnd; ++psz) {
    switch (*psz) {
      case '>':
      case '[':
      case '+':
        return NULL;
    }
  }
  CFDE_CSSSelector *pFirst = NULL, *pLast = NULL;
  CFDE_CSSSelector *pPersudoFirst = NULL, *pPersudoLast = NULL;
  for (psz = pStart; psz < pEnd;) {
    FX_WCHAR wch = *psz;
    if (wch == '.' || wch == '#') {
      if (psz == pStart || psz[-1] == ' ') {
        CFDE_CSSSelector* p = FDE_NewWith(pStaticStore)
            CFDE_CSSSelector(FDE_CSSSELECTORTYPE_Element, L"*", 1, TRUE);
        if (p == NULL) {
          return NULL;
        }
        if (pFirst != NULL) {
          pFirst->SetType(FDE_CSSSELECTORTYPE_Descendant);
          p->SetNext(pFirst);
        }
        pFirst = pLast = p;
      }
      FXSYS_assert(pLast != NULL);
      int32_t iNameLen = FDE_GetCSSNameLen(++psz, pEnd);
      if (iNameLen == 0) {
        return NULL;
      }
      FDE_CSSSELECTORTYPE eType =
          wch == '.' ? FDE_CSSSELECTORTYPE_Class : FDE_CSSSELECTORTYPE_ID;
      CFDE_CSSSelector* p = FDE_NewWith(pStaticStore)
          CFDE_CSSSelector(eType, psz, iNameLen, FALSE);
      if (p == NULL) {
        return NULL;
      }
      p->SetNext(pLast->GetNextSelector());
      pLast->SetNext(p);
      pLast = p;
      psz += iNameLen;
    } else if (FDE_IsCSSChar(wch) || wch == '*') {
      int32_t iNameLen = wch == '*' ? 1 : FDE_GetCSSNameLen(psz, pEnd);
      if (iNameLen == 0) {
        return NULL;
      }
      CFDE_CSSSelector* p = FDE_NewWith(pStaticStore)
          CFDE_CSSSelector(FDE_CSSSELECTORTYPE_Element, psz, iNameLen, TRUE);
      if (p == NULL) {
        return NULL;
      }
      if (pFirst == NULL) {
        pFirst = pLast = p;
      } else {
        pFirst->SetType(FDE_CSSSELECTORTYPE_Descendant);
        p->SetNext(pFirst);
        pFirst = pLast = p;
      }
      psz += iNameLen;
    } else if (wch == ':') {
      int32_t iNameLen = FDE_GetCSSPersudoLen(psz, pEnd);
      if (iNameLen == 0) {
        return NULL;
      }
      CFDE_CSSSelector* p = FDE_NewWith(pStaticStore)
          CFDE_CSSSelector(FDE_CSSSELECTORTYPE_Persudo, psz, iNameLen, TRUE);
      if (p == NULL) {
        return NULL;
      }
      if (pPersudoFirst == NULL) {
        pPersudoFirst = pPersudoLast = p;
      } else {
        pPersudoLast->SetNext(p);
        pPersudoLast = p;
      }
      psz += iNameLen;
    } else if (wch == ' ') {
      psz++;
    } else {
      return NULL;
    }
  }
  if (pPersudoFirst == NULL) {
    return pFirst;
  } else {
    pPersudoLast->SetNext(pFirst);
    return pPersudoFirst;
  }
}
