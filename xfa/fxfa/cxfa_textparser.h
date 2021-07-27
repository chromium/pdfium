// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_TEXTPARSER_H_
#define XFA_FXFA_CXFA_TEXTPARSER_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/css/cfx_css.h"
#include "core/fxcrt/css/cfx_cssdeclaration.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/fx_dib.h"
#include "fxjs/gc/heap.h"
#include "third_party/base/optional.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFGAS_GEFont;
class CFX_CSSComputedStyle;
class CFX_CSSStyleSelector;
class CFX_CSSStyleSheet;
class CFX_XMLNode;
class CXFA_FFDoc;
class CXFA_TextProvider;
class CXFA_TextTabstopsContext;

class CXFA_TextParser : public cppgc::GarbageCollected<CXFA_TextParser> {
 public:
  class Context {
   public:
    Context();
    ~Context();

    void SetParentStyle(const CFX_CSSComputedStyle* style);
    const CFX_CSSComputedStyle* GetParentStyle() const {
      return m_pParentStyle.Get();
    }

    void SetDisplay(CFX_CSSDisplay eDisplay) { m_eDisplay = eDisplay; }
    CFX_CSSDisplay GetDisplay() const { return m_eDisplay; }

    void SetDecls(std::vector<const CFX_CSSDeclaration*>&& decl);
    const std::vector<const CFX_CSSDeclaration*>& GetDecls() const {
      return decls_;
    }

   private:
    RetainPtr<const CFX_CSSComputedStyle> m_pParentStyle;
    CFX_CSSDisplay m_eDisplay = CFX_CSSDisplay::None;
    std::vector<const CFX_CSSDeclaration*> decls_;
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  virtual ~CXFA_TextParser();

  void Trace(cppgc::Visitor* visitor) const {}

  void Reset();
  void DoParse(const CFX_XMLNode* pXMLContainer,
               CXFA_TextProvider* pTextProvider);

  RetainPtr<CFX_CSSComputedStyle> CreateRootStyle(
      CXFA_TextProvider* pTextProvider);
  RetainPtr<CFX_CSSComputedStyle> ComputeStyle(
      const CFX_XMLNode* pXMLNode,
      const CFX_CSSComputedStyle* pParentStyle);

  bool IsParsed() const { return m_bParsed; }

  XFA_AttributeValue GetVAlign(CXFA_TextProvider* pTextProvider) const;

  float GetTabInterval(const CFX_CSSComputedStyle* pStyle) const;
  int32_t CountTabs(const CFX_CSSComputedStyle* pStyle) const;

  bool IsSpaceRun(const CFX_CSSComputedStyle* pStyle) const;
  bool GetTabstops(const CFX_CSSComputedStyle* pStyle,
                   CXFA_TextTabstopsContext* pTabstopContext);

  RetainPtr<CFGAS_GEFont> GetFont(CXFA_FFDoc* doc,
                                  CXFA_TextProvider* pTextProvider,
                                  const CFX_CSSComputedStyle* pStyle) const;
  float GetFontSize(CXFA_TextProvider* pTextProvider,
                    const CFX_CSSComputedStyle* pStyle) const;
  int32_t GetHorScale(CXFA_TextProvider* pTextProvider,
                      const CFX_CSSComputedStyle* pStyle,
                      const CFX_XMLNode* pXMLNode) const;
  int32_t GetVerScale(CXFA_TextProvider* pTextProvider,
                      const CFX_CSSComputedStyle* pStyle) const;
  int32_t GetUnderline(CXFA_TextProvider* pTextProvider,
                       const CFX_CSSComputedStyle* pStyle) const;
  XFA_AttributeValue GetUnderlinePeriod(
      CXFA_TextProvider* pTextProvider,
      const CFX_CSSComputedStyle* pStyle) const;
  int32_t GetLinethrough(CXFA_TextProvider* pTextProvider,
                         const CFX_CSSComputedStyle* pStyle) const;
  FX_ARGB GetColor(CXFA_TextProvider* pTextProvider,
                   const CFX_CSSComputedStyle* pStyle) const;
  float GetBaseline(CXFA_TextProvider* pTextProvider,
                    const CFX_CSSComputedStyle* pStyle) const;
  float GetLineHeight(CXFA_TextProvider* pTextProvider,
                      const CFX_CSSComputedStyle* pStyle,
                      bool bFirst,
                      float fVerScale) const;

  Optional<WideString> GetEmbeddedObj(const CXFA_TextProvider* pTextProvider,
                                      const CFX_XMLNode* pXMLNode);
  Context* GetParseContextFromMap(const CFX_XMLNode* pXMLNode);

 protected:
  CXFA_TextParser();

  bool TagValidate(const WideString& str) const;

 private:
  class TagProvider {
   public:
    TagProvider();
    ~TagProvider();

    WideString GetTagName() { return m_wsTagName; }

    void SetTagName(const WideString& wsName) { m_wsTagName = wsName; }
    void SetAttribute(const WideString& wsAttr, const WideString& wsValue) {
      m_Attributes.insert({wsAttr, wsValue});
    }

    WideString GetAttribute(const WideString& wsAttr) {
      return m_Attributes[wsAttr];
    }

    bool m_bTagAvailable = false;
    bool m_bContent = false;

   private:
    WideString m_wsTagName;
    std::map<WideString, WideString> m_Attributes;
  };

  // static
  std::unique_ptr<TagProvider> ParseTagInfo(const CFX_XMLNode* pXMLNode);

  void InitCSSData(CXFA_TextProvider* pTextProvider);
  void ParseRichText(const CFX_XMLNode* pXMLNode,
                     const CFX_CSSComputedStyle* pParentStyle);
  std::unique_ptr<CFX_CSSStyleSheet> LoadDefaultSheetStyle();
  RetainPtr<CFX_CSSComputedStyle> CreateStyle(
      const CFX_CSSComputedStyle* pParentStyle);

  bool m_bParsed = false;
  bool m_cssInitialized = false;
  std::unique_ptr<CFX_CSSStyleSelector> m_pSelector;
  std::map<const CFX_XMLNode*, std::unique_ptr<Context>>
      m_mapXMLNodeToParseContext;
};

#endif  // XFA_FXFA_CXFA_TEXTPARSER_H_
