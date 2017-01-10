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

class CFDE_CSSStyleSelector;
class CFDE_XMLNode;
class CXFA_CSSTagProvider;
class CXFA_TextParseContext;
class CXFA_TextProvider;
class CXFA_TextTabstopsContext;
class IFDE_CSSComputedStyle;
class IFDE_CSSStyleSheet;

class CXFA_TextParser {
 public:
  CXFA_TextParser();
  virtual ~CXFA_TextParser();

  void Reset();
  void DoParse(CFDE_XMLNode* pXMLContainer, CXFA_TextProvider* pTextProvider);

  IFDE_CSSComputedStyle* CreateRootStyle(CXFA_TextProvider* pTextProvider);
  IFDE_CSSComputedStyle* ComputeStyle(CFDE_XMLNode* pXMLNode,
                                      IFDE_CSSComputedStyle* pParentStyle);

  bool IsParsed() const { return m_bParsed; }

  int32_t GetVAlign(CXFA_TextProvider* pTextProvider) const;

  FX_FLOAT GetTabInterval(IFDE_CSSComputedStyle* pStyle) const;
  int32_t CountTabs(IFDE_CSSComputedStyle* pStyle) const;

  bool IsSpaceRun(IFDE_CSSComputedStyle* pStyle) const;
  bool GetTabstops(IFDE_CSSComputedStyle* pStyle,
                   CXFA_TextTabstopsContext* pTabstopContext);

  CFX_RetainPtr<CFGAS_GEFont> GetFont(CXFA_TextProvider* pTextProvider,
                                      IFDE_CSSComputedStyle* pStyle) const;
  FX_FLOAT GetFontSize(CXFA_TextProvider* pTextProvider,
                       IFDE_CSSComputedStyle* pStyle) const;

  int32_t GetHorScale(CXFA_TextProvider* pTextProvider,
                      IFDE_CSSComputedStyle* pStyle,
                      CFDE_XMLNode* pXMLNode) const;
  int32_t GetVerScale(CXFA_TextProvider* pTextProvider,
                      IFDE_CSSComputedStyle* pStyle) const;

  void GetUnderline(CXFA_TextProvider* pTextProvider,
                    IFDE_CSSComputedStyle* pStyle,
                    int32_t& iUnderline,
                    int32_t& iPeriod) const;
  void GetLinethrough(CXFA_TextProvider* pTextProvider,
                      IFDE_CSSComputedStyle* pStyle,
                      int32_t& iLinethrough) const;
  FX_ARGB GetColor(CXFA_TextProvider* pTextProvider,
                   IFDE_CSSComputedStyle* pStyle) const;
  FX_FLOAT GetBaseline(CXFA_TextProvider* pTextProvider,
                       IFDE_CSSComputedStyle* pStyle) const;
  FX_FLOAT GetLineHeight(CXFA_TextProvider* pTextProvider,
                         IFDE_CSSComputedStyle* pStyle,
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
                     IFDE_CSSComputedStyle* pParentStyle);
  void ParseTagInfo(CFDE_XMLNode* pXMLNode, CXFA_CSSTagProvider& tagProvider);
  IFDE_CSSStyleSheet* LoadDefaultSheetStyle();
  IFDE_CSSComputedStyle* CreateStyle(IFDE_CSSComputedStyle* pParentStyle);

  std::unique_ptr<CFDE_CSSStyleSelector> m_pSelector;
  IFDE_CSSStyleSheet* m_pUASheet;
  std::map<CFDE_XMLNode*, CXFA_TextParseContext*> m_mapXMLNodeToParseContext;
  bool m_bParsed;
};

#endif  // XFA_FXFA_APP_CXFA_TEXTPARSER_H_
