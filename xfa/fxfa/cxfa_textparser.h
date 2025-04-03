// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_TEXTPARSER_H_
#define XFA_FXFA_CXFA_TEXTPARSER_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "core/fxcrt/css/cfx_css.h"
#include "core/fxcrt/css/cfx_csscomputedstyle.h"
#include "core/fxcrt/css/cfx_cssdeclaration.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/widestring.h"
#include "core/fxge/dib/fx_dib.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFGAS_GEFont;
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

    void SetParentStyle(RetainPtr<const CFX_CSSComputedStyle> style);
    RetainPtr<const CFX_CSSComputedStyle> GetParentStyle() const {
      return parent_style_;
    }

    void SetDisplay(CFX_CSSDisplay eDisplay) { display_ = eDisplay; }
    CFX_CSSDisplay GetDisplay() const { return display_; }

    void SetDecls(std::vector<const CFX_CSSDeclaration*>&& decl);
    const std::vector<const CFX_CSSDeclaration*>& GetDecls() const {
      return decls_;
    }

   private:
    RetainPtr<const CFX_CSSComputedStyle> parent_style_;
    CFX_CSSDisplay display_ = CFX_CSSDisplay::None;
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
      RetainPtr<const CFX_CSSComputedStyle> pParentStyle);

  bool IsParsed() const { return parsed_; }

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

  std::optional<WideString> GetEmbeddedObj(
      const CXFA_TextProvider* pTextProvider,
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

    WideString GetTagName() { return tag_name_; }

    void SetTagName(const WideString& wsName) { tag_name_ = wsName; }
    void SetAttribute(const WideString& wsAttr, const WideString& wsValue) {
      attributes_.insert({wsAttr, wsValue});
    }

    WideString GetAttribute(const WideString& wsAttr) {
      return attributes_[wsAttr];
    }

    bool tag_available_ = false;
    bool content_ = false;

   private:
    WideString tag_name_;
    std::map<WideString, WideString> attributes_;
  };

  // static
  std::unique_ptr<TagProvider> ParseTagInfo(const CFX_XMLNode* pXMLNode);

  void InitCSSData(CXFA_TextProvider* pTextProvider);
  void ParseRichText(const CFX_XMLNode* pXMLNode,
                     const CFX_CSSComputedStyle* pParentStyle);
  std::unique_ptr<CFX_CSSStyleSheet> LoadDefaultSheetStyle();
  RetainPtr<CFX_CSSComputedStyle> CreateStyle(
      const CFX_CSSComputedStyle* pParentStyle);

  bool parsed_ = false;
  bool css_initialized_ = false;
  std::unique_ptr<CFX_CSSStyleSelector> selector_;
  std::map<const CFX_XMLNode*, std::unique_ptr<Context>>
      map_xmlnode_to_parse_context_;
};

#endif  // XFA_FXFA_CXFA_TEXTPARSER_H_
