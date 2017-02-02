// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_TEXTPARSER_H_
#define XFA_FXFA_APP_CXFA_TEXTPARSER_H_

#include <map>
#include <memory>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fgas/font/cfgas_gefont.h"

class CFDE_CSSComputedStyle;
class CFDE_CSSStyleSelector;
class CFDE_CSSStyleSheet;
class CFDE_XMLNode;
class CXFA_CSSTagProvider;
class CXFA_TextParseContext;
class CXFA_TextProvider;
class CXFA_TextTabstopsContext;

class CXFA_TextParser {
 public:
  CXFA_TextParser();
  virtual ~CXFA_TextParser();

  void Reset();
  void DoParse(CFDE_XMLNode* pXMLContainer, CXFA_TextProvider* pTextProvider);

  CFX_RetainPtr<CFDE_CSSComputedStyle> CreateRootStyle(
      CXFA_TextProvider* pTextProvider);
  CFX_RetainPtr<CFDE_CSSComputedStyle> ComputeStyle(
      CFDE_XMLNode* pXMLNode,
      CFDE_CSSComputedStyle* pParentStyle);

  bool IsParsed() const { return m_bParsed; }

  int32_t GetVAlign(CXFA_TextProvider* pTextProvider) const;

  FX_FLOAT GetTabInterval(CFDE_CSSComputedStyle* pStyle) const;
  int32_t CountTabs(CFDE_CSSComputedStyle* pStyle) const;

  bool IsSpaceRun(CFDE_CSSComputedStyle* pStyle) const;
  bool GetTabstops(CFDE_CSSComputedStyle* pStyle,
                   CXFA_TextTabstopsContext* pTabstopContext);

  CFX_RetainPtr<CFGAS_GEFont> GetFont(CXFA_TextProvider* pTextProvider,
                                      CFDE_CSSComputedStyle* pStyle) const;
  FX_FLOAT GetFontSize(CXFA_TextProvider* pTextProvider,
                       CFDE_CSSComputedStyle* pStyle) const;

  int32_t GetHorScale(CXFA_TextProvider* pTextProvider,
                      CFDE_CSSComputedStyle* pStyle,
                      CFDE_XMLNode* pXMLNode) const;
  int32_t GetVerScale(CXFA_TextProvider* pTextProvider,
                      CFDE_CSSComputedStyle* pStyle) const;

  void GetUnderline(CXFA_TextProvider* pTextProvider,
                    CFDE_CSSComputedStyle* pStyle,
                    int32_t& iUnderline,
                    int32_t& iPeriod) const;
  void GetLinethrough(CXFA_TextProvider* pTextProvider,
                      CFDE_CSSComputedStyle* pStyle,
                      int32_t& iLinethrough) const;
  FX_ARGB GetColor(CXFA_TextProvider* pTextProvider,
                   CFDE_CSSComputedStyle* pStyle) const;
  FX_FLOAT GetBaseline(CXFA_TextProvider* pTextProvider,
                       CFDE_CSSComputedStyle* pStyle) const;
  FX_FLOAT GetLineHeight(CXFA_TextProvider* pTextProvider,
                         CFDE_CSSComputedStyle* pStyle,
                         bool bFirst,
                         FX_FLOAT fVerScale) const;

  bool GetEmbbedObj(CXFA_TextProvider* pTextProvider,
                    CFDE_XMLNode* pXMLNode,
                    CFX_WideString& wsValue);
  CXFA_TextParseContext* GetParseContextFromMap(CFDE_XMLNode* pXMLNode);

 protected:
  bool TagValidate(const CFX_WideString& str) const;

 private:
  void InitCSSData(CXFA_TextProvider* pTextProvider);
  void ParseRichText(CFDE_XMLNode* pXMLNode,
                     CFDE_CSSComputedStyle* pParentStyle);
  std::unique_ptr<CXFA_CSSTagProvider> ParseTagInfo(CFDE_XMLNode* pXMLNode);
  std::unique_ptr<CFDE_CSSStyleSheet> LoadDefaultSheetStyle();
  CFX_RetainPtr<CFDE_CSSComputedStyle> CreateStyle(
      CFDE_CSSComputedStyle* pParentStyle);

  std::unique_ptr<CFDE_CSSStyleSelector> m_pSelector;
  std::map<CFDE_XMLNode*, CXFA_TextParseContext*> m_mapXMLNodeToParseContext;
  bool m_bParsed;
  bool m_cssInitialized;
};

#endif  // XFA_FXFA_APP_CXFA_TEXTPARSER_H_
