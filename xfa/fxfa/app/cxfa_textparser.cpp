// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_textparser.h"

#include <algorithm>

#include "third_party/base/ptr_util.h"
#include "xfa/fde/css/fde_css.h"
#include "xfa/fde/css/fde_cssstyleselector.h"
#include "xfa/fgas/crt/fgas_codepage.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fxfa/app/cxfa_csstagprovider.h"
#include "xfa/fxfa/app/cxfa_textparsecontext.h"
#include "xfa/fxfa/app/cxfa_texttabstopscontext.h"
#include "xfa/fxfa/app/xfa_ffwidgetacc.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/xfa_ffapp.h"
#include "xfa/fxfa/xfa_ffdoc.h"
#include "xfa/fxfa/xfa_fontmgr.h"

namespace {

enum class TabStopStatus {
  Error,
  EOS,
  None,
  Alignment,
  StartLeader,
  Leader,
  Location,
};

}  // namespace

CXFA_TextParser::CXFA_TextParser() : m_pUASheet(nullptr), m_bParsed(false) {}

CXFA_TextParser::~CXFA_TextParser() {
  if (m_pUASheet)
    m_pUASheet->Release();

  for (auto& pair : m_mapXMLNodeToParseContext) {
    if (pair.second)
      delete pair.second;
  }
}

void CXFA_TextParser::Reset() {
  for (auto& pair : m_mapXMLNodeToParseContext) {
    if (pair.second)
      delete pair.second;
  }
  m_mapXMLNodeToParseContext.clear();
  m_bParsed = false;
}
void CXFA_TextParser::InitCSSData(CXFA_TextProvider* pTextProvider) {
  if (!pTextProvider)
    return;

  if (!m_pSelector) {
    CXFA_FFDoc* pDoc = pTextProvider->GetDocNode();
    CFGAS_FontMgr* pFontMgr = pDoc->GetApp()->GetFDEFontMgr();
    ASSERT(pFontMgr);
    m_pSelector = pdfium::MakeUnique<CFDE_CSSStyleSelector>(pFontMgr);
    FX_FLOAT fFontSize = 10;
    CXFA_Font font = pTextProvider->GetFontNode();
    if (font) {
      fFontSize = font.GetFontSize();
    }
    m_pSelector->SetDefFontSize(fFontSize);
  }

  if (!m_pUASheet) {
    m_pUASheet = LoadDefaultSheetStyle();
    m_pSelector->SetStyleSheet(FDE_CSSStyleSheetGroup::UserAgent, m_pUASheet);
    m_pSelector->UpdateStyleIndex(FDE_CSSMEDIATYPE_ALL);
  }
}

IFDE_CSSStyleSheet* CXFA_TextParser::LoadDefaultSheetStyle() {
  static const FX_WCHAR s_pStyle[] =
      L"html,body,ol,p,ul{display:block}"
      L"li{display:list-item}"
      L"ol,ul{padding-left:33px}ol{list-style-type:decimal}ol,ul{margin-top:0;"
      L"margin-bottom:0}ul,ol{margin:1.12em 0}"
      L"a{color:#0000ff;text-decoration:underline}b{font-weight:bolder}i{font-"
      L"style:italic}"
      L"sup{vertical-align:+15em;font-size:.66em}sub{vertical-align:-15em;font-"
      L"size:.66em}";
  return IFDE_CSSStyleSheet::LoadFromBuffer(
      CFX_WideString(), s_pStyle, FXSYS_wcslen(s_pStyle), FX_CODEPAGE_UTF8);
}

IFDE_CSSComputedStyle* CXFA_TextParser::CreateRootStyle(
    CXFA_TextProvider* pTextProvider) {
  CXFA_Font font = pTextProvider->GetFontNode();
  CXFA_Para para = pTextProvider->GetParaNode();
  IFDE_CSSComputedStyle* pStyle = m_pSelector->CreateComputedStyle(nullptr);
  IFDE_CSSFontStyle* pFontStyle = pStyle->GetFontStyles();
  IFDE_CSSParagraphStyle* pParaStyle = pStyle->GetParagraphStyles();
  FX_FLOAT fLineHeight = 0;
  FX_FLOAT fFontSize = 10;

  if (para) {
    fLineHeight = para.GetLineHeight();
    FDE_CSSLENGTH indent;
    indent.Set(FDE_CSSLengthUnit::Point, para.GetTextIndent());
    pParaStyle->SetTextIndent(indent);
    FDE_CSSTextAlign hAlign = FDE_CSSTextAlign::Left;
    switch (para.GetHorizontalAlign()) {
      case XFA_ATTRIBUTEENUM_Center:
        hAlign = FDE_CSSTextAlign::Center;
        break;
      case XFA_ATTRIBUTEENUM_Right:
        hAlign = FDE_CSSTextAlign::Right;
        break;
      case XFA_ATTRIBUTEENUM_Justify:
        hAlign = FDE_CSSTextAlign::Justify;
        break;
      case XFA_ATTRIBUTEENUM_JustifyAll:
        hAlign = FDE_CSSTextAlign::JustifyAll;
        break;
    }
    pParaStyle->SetTextAlign(hAlign);
    FDE_CSSRECT rtMarginWidth;
    rtMarginWidth.left.Set(FDE_CSSLengthUnit::Point, para.GetMarginLeft());
    rtMarginWidth.top.Set(FDE_CSSLengthUnit::Point, para.GetSpaceAbove());
    rtMarginWidth.right.Set(FDE_CSSLengthUnit::Point, para.GetMarginRight());
    rtMarginWidth.bottom.Set(FDE_CSSLengthUnit::Point, para.GetSpaceBelow());
    pStyle->GetBoundaryStyles()->SetMarginWidth(rtMarginWidth);
  }

  if (font) {
    pFontStyle->SetColor(font.GetColor());
    pFontStyle->SetFontStyle(font.IsItalic() ? FDE_CSSFontStyle::Italic
                                             : FDE_CSSFontStyle::Normal);
    pFontStyle->SetFontWeight(font.IsBold() ? FXFONT_FW_BOLD
                                            : FXFONT_FW_NORMAL);
    pParaStyle->SetNumberVerticalAlign(-font.GetBaselineShift());
    fFontSize = font.GetFontSize();
    FDE_CSSLENGTH letterSpacing;
    letterSpacing.Set(FDE_CSSLengthUnit::Point, font.GetLetterSpacing());
    pParaStyle->SetLetterSpacing(letterSpacing);
    uint32_t dwDecoration = 0;
    if (font.GetLineThrough() > 0)
      dwDecoration |= FDE_CSSTEXTDECORATION_LineThrough;
    if (font.GetUnderline() > 1)
      dwDecoration |= FDE_CSSTEXTDECORATION_Double;
    else if (font.GetUnderline() > 0)
      dwDecoration |= FDE_CSSTEXTDECORATION_Underline;

    pParaStyle->SetTextDecoration(dwDecoration);
  }
  pParaStyle->SetLineHeight(fLineHeight);
  pFontStyle->SetFontSize(fFontSize);
  return pStyle;
}

IFDE_CSSComputedStyle* CXFA_TextParser::CreateStyle(
    IFDE_CSSComputedStyle* pParentStyle) {
  IFDE_CSSComputedStyle* pNewStyle =
      m_pSelector->CreateComputedStyle(pParentStyle);
  ASSERT(pNewStyle);
  if (!pParentStyle)
    return pNewStyle;

  IFDE_CSSParagraphStyle* pParaStyle = pParentStyle->GetParagraphStyles();
  uint32_t dwDecoration = pParaStyle->GetTextDecoration();
  FX_FLOAT fBaseLine = 0;
  if (pParaStyle->GetVerticalAlign() == FDE_CSSVerticalAlign::Number)
    fBaseLine = pParaStyle->GetNumberVerticalAlign();

  pParaStyle = pNewStyle->GetParagraphStyles();
  pParaStyle->SetTextDecoration(dwDecoration);
  pParaStyle->SetNumberVerticalAlign(fBaseLine);

  IFDE_CSSBoundaryStyle* pBoundarytyle = pParentStyle->GetBoundaryStyles();
  const FDE_CSSRECT* pRect = pBoundarytyle->GetMarginWidth();
  if (pRect) {
    pBoundarytyle = pNewStyle->GetBoundaryStyles();
    pBoundarytyle->SetMarginWidth(*pRect);
  }
  return pNewStyle;
}

IFDE_CSSComputedStyle* CXFA_TextParser::ComputeStyle(
    CFDE_XMLNode* pXMLNode,
    IFDE_CSSComputedStyle* pParentStyle) {
  auto it = m_mapXMLNodeToParseContext.find(pXMLNode);
  if (it == m_mapXMLNodeToParseContext.end())
    return nullptr;

  CXFA_TextParseContext* pContext = it->second;
  if (!pContext)
    return nullptr;

  pContext->m_pParentStyle = pParentStyle;
  pParentStyle->Retain();

  CXFA_CSSTagProvider tagProvider;
  ParseTagInfo(pXMLNode, tagProvider);
  if (tagProvider.m_bContent)
    return nullptr;

  IFDE_CSSComputedStyle* pStyle = CreateStyle(pParentStyle);
  CFDE_CSSAccelerator* pCSSAccel = m_pSelector->InitAccelerator();
  pCSSAccel->OnEnterTag(&tagProvider);
  m_pSelector->ComputeStyle(&tagProvider, pContext->GetDecls(),
                            pContext->CountDecls(), pStyle);
  pCSSAccel->OnLeaveTag(&tagProvider);
  return pStyle;
}

void CXFA_TextParser::DoParse(CFDE_XMLNode* pXMLContainer,
                              CXFA_TextProvider* pTextProvider) {
  if (!pXMLContainer || !pTextProvider || m_bParsed)
    return;

  m_bParsed = true;
  InitCSSData(pTextProvider);
  IFDE_CSSComputedStyle* pRootStyle = CreateRootStyle(pTextProvider);
  ParseRichText(pXMLContainer, pRootStyle);
  pRootStyle->Release();
}

void CXFA_TextParser::ParseRichText(CFDE_XMLNode* pXMLNode,
                                    IFDE_CSSComputedStyle* pParentStyle) {
  if (!pXMLNode)
    return;

  CXFA_CSSTagProvider tagProvider;
  ParseTagInfo(pXMLNode, tagProvider);
  if (!tagProvider.m_bTagAvailable)
    return;

  IFDE_CSSComputedStyle* pNewStyle = nullptr;
  if ((tagProvider.GetTagName() != FX_WSTRC(L"body")) ||
      (tagProvider.GetTagName() != FX_WSTRC(L"html"))) {
    CXFA_TextParseContext* pTextContext = new CXFA_TextParseContext;
    FDE_CSSDisplay eDisplay = FDE_CSSDisplay::Inline;
    if (!tagProvider.m_bContent) {
      pNewStyle = CreateStyle(pParentStyle);
      CFDE_CSSAccelerator* pCSSAccel = m_pSelector->InitAccelerator();
      pCSSAccel->OnEnterTag(&tagProvider);
      CFX_ArrayTemplate<CFDE_CSSDeclaration*> DeclArray;
      int32_t iMatchedDecls =
          m_pSelector->MatchDeclarations(&tagProvider, DeclArray);
      const CFDE_CSSDeclaration** ppMatchDecls =
          const_cast<const CFDE_CSSDeclaration**>(DeclArray.GetData());
      m_pSelector->ComputeStyle(&tagProvider, ppMatchDecls, iMatchedDecls,
                                pNewStyle);
      pCSSAccel->OnLeaveTag(&tagProvider);
      if (iMatchedDecls > 0)
        pTextContext->SetDecls(ppMatchDecls, iMatchedDecls);

      eDisplay = pNewStyle->GetPositionStyles()->GetDisplay();
    }
    pTextContext->SetDisplay(eDisplay);
    m_mapXMLNodeToParseContext[pXMLNode] = pTextContext;
  }

  for (CFDE_XMLNode* pXMLChild =
           pXMLNode->GetNodeItem(CFDE_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(CFDE_XMLNode::NextSibling)) {
    ParseRichText(pXMLChild, pNewStyle);
  }
  if (pNewStyle)
    pNewStyle->Release();
}

bool CXFA_TextParser::TagValidate(const CFX_WideString& wsName) const {
  static const uint32_t s_XFATagName[] = {
      0x61,        // a
      0x62,        // b
      0x69,        // i
      0x70,        // p
      0x0001f714,  // br
      0x00022a55,  // li
      0x000239bb,  // ol
      0x00025881,  // ul
      0x0bd37faa,  // sub
      0x0bd37fb8,  // sup
      0xa73e3af2,  // span
      0xb182eaae,  // body
      0xdb8ac455,  // html
  };
  static const int32_t s_iCount = FX_ArraySize(s_XFATagName);

  return std::binary_search(s_XFATagName, s_XFATagName + s_iCount,
                            FX_HashCode_GetW(wsName.AsStringC(), true));
}

void CXFA_TextParser::ParseTagInfo(CFDE_XMLNode* pXMLNode,
                                   CXFA_CSSTagProvider& tagProvider) {
  CFX_WideString wsName;
  if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
    CFDE_XMLElement* pXMLElement = static_cast<CFDE_XMLElement*>(pXMLNode);
    pXMLElement->GetLocalTagName(wsName);
    tagProvider.SetTagNameObj(wsName);
    tagProvider.m_bTagAvailable = TagValidate(wsName);

    CFX_WideString wsValue;
    pXMLElement->GetString(L"style", wsValue);
    if (!wsValue.IsEmpty())
      tagProvider.SetAttribute(L"style", wsValue);
  } else if (pXMLNode->GetType() == FDE_XMLNODE_Text) {
    tagProvider.m_bTagAvailable = true;
    tagProvider.m_bContent = true;
  }
}

int32_t CXFA_TextParser::GetVAlign(CXFA_TextProvider* pTextProvider) const {
  CXFA_Para para = pTextProvider->GetParaNode();
  return para ? para.GetVerticalAlign() : XFA_ATTRIBUTEENUM_Top;
}

FX_FLOAT CXFA_TextParser::GetTabInterval(IFDE_CSSComputedStyle* pStyle) const {
  CFX_WideString wsValue;
  if (pStyle && pStyle->GetCustomStyle(FX_WSTRC(L"tab-interval"), wsValue))
    return CXFA_Measurement(wsValue.AsStringC()).ToUnit(XFA_UNIT_Pt);
  return 36;
}

int32_t CXFA_TextParser::CountTabs(IFDE_CSSComputedStyle* pStyle) const {
  CFX_WideString wsValue;
  if (pStyle && pStyle->GetCustomStyle(FX_WSTRC(L"xfa-tab-count"), wsValue))
    return wsValue.GetInteger();
  return 0;
}

bool CXFA_TextParser::IsSpaceRun(IFDE_CSSComputedStyle* pStyle) const {
  CFX_WideString wsValue;
  if (pStyle && pStyle->GetCustomStyle(FX_WSTRC(L"xfa-spacerun"), wsValue)) {
    wsValue.MakeLower();
    return wsValue == FX_WSTRC(L"yes");
  }
  return false;
}

CFX_RetainPtr<CFGAS_GEFont> CXFA_TextParser::GetFont(
    CXFA_TextProvider* pTextProvider,
    IFDE_CSSComputedStyle* pStyle) const {
  CFX_WideStringC wsFamily = FX_WSTRC(L"Courier");
  uint32_t dwStyle = 0;
  CXFA_Font font = pTextProvider->GetFontNode();
  if (font) {
    font.GetTypeface(wsFamily);
    if (font.IsBold())
      dwStyle |= FX_FONTSTYLE_Bold;
    if (font.IsItalic())
      dwStyle |= FX_FONTSTYLE_Italic;
  }

  if (pStyle) {
    IFDE_CSSFontStyle* pFontStyle = pStyle->GetFontStyles();
    int32_t iCount = pFontStyle->CountFontFamilies();
    if (iCount > 0)
      wsFamily = pFontStyle->GetFontFamily(iCount - 1);

    dwStyle = 0;
    if (pFontStyle->GetFontWeight() > FXFONT_FW_NORMAL)
      dwStyle |= FX_FONTSTYLE_Bold;
    if (pFontStyle->GetFontStyle() == FDE_CSSFontStyle::Italic)
      dwStyle |= FX_FONTSTYLE_Italic;
  }

  CXFA_FFDoc* pDoc = pTextProvider->GetDocNode();
  CXFA_FontMgr* pFontMgr = pDoc->GetApp()->GetXFAFontMgr();
  return pFontMgr->GetFont(pDoc, wsFamily, dwStyle);
}

FX_FLOAT CXFA_TextParser::GetFontSize(CXFA_TextProvider* pTextProvider,
                                      IFDE_CSSComputedStyle* pStyle) const {
  if (pStyle)
    return pStyle->GetFontStyles()->GetFontSize();

  CXFA_Font font = pTextProvider->GetFontNode();
  if (font)
    return font.GetFontSize();
  return 10;
}

int32_t CXFA_TextParser::GetHorScale(CXFA_TextProvider* pTextProvider,
                                     IFDE_CSSComputedStyle* pStyle,
                                     CFDE_XMLNode* pXMLNode) const {
  if (pStyle) {
    CFX_WideString wsValue;
    if (pStyle->GetCustomStyle(L"xfa-font-horizontal-scale", wsValue))
      return wsValue.GetInteger();

    while (pXMLNode) {
      auto it = m_mapXMLNodeToParseContext.find(pXMLNode);
      if (it != m_mapXMLNodeToParseContext.end()) {
        CXFA_TextParseContext* pContext = it->second;
        if (pContext && pContext->m_pParentStyle &&
            pContext->m_pParentStyle->GetCustomStyle(
                L"xfa-font-horizontal-scale", wsValue)) {
          return wsValue.GetInteger();
        }
      }
      pXMLNode = pXMLNode->GetNodeItem(CFDE_XMLNode::Parent);
    }
  }

  if (CXFA_Font font = pTextProvider->GetFontNode())
    return static_cast<int32_t>(font.GetHorizontalScale());
  return 100;
}

int32_t CXFA_TextParser::GetVerScale(CXFA_TextProvider* pTextProvider,
                                     IFDE_CSSComputedStyle* pStyle) const {
  if (pStyle) {
    CFX_WideString wsValue;
    if (pStyle->GetCustomStyle(FX_WSTRC(L"xfa-font-vertical-scale"), wsValue))
      return wsValue.GetInteger();
  }

  if (CXFA_Font font = pTextProvider->GetFontNode())
    return (int32_t)font.GetVerticalScale();
  return 100;
}

void CXFA_TextParser::GetUnderline(CXFA_TextProvider* pTextProvider,
                                   IFDE_CSSComputedStyle* pStyle,
                                   int32_t& iUnderline,
                                   int32_t& iPeriod) const {
  iUnderline = 0;
  iPeriod = XFA_ATTRIBUTEENUM_All;
  if (!pStyle) {
    CXFA_Font font = pTextProvider->GetFontNode();
    if (font) {
      iUnderline = font.GetUnderline();
      iPeriod = font.GetUnderlinePeriod();
    }
    return;
  }

  uint32_t dwDecoration = pStyle->GetParagraphStyles()->GetTextDecoration();
  if (dwDecoration & FDE_CSSTEXTDECORATION_Double)
    iUnderline = 2;
  else if (dwDecoration & FDE_CSSTEXTDECORATION_Underline)
    iUnderline = 1;

  CFX_WideString wsValue;
  if (pStyle->GetCustomStyle(FX_WSTRC(L"underlinePeriod"), wsValue)) {
    if (wsValue == FX_WSTRC(L"word"))
      iPeriod = XFA_ATTRIBUTEENUM_Word;
  } else if (CXFA_Font font = pTextProvider->GetFontNode()) {
    iPeriod = font.GetUnderlinePeriod();
  }
}

void CXFA_TextParser::GetLinethrough(CXFA_TextProvider* pTextProvider,
                                     IFDE_CSSComputedStyle* pStyle,
                                     int32_t& iLinethrough) const {
  if (pStyle) {
    uint32_t dwDecoration = pStyle->GetParagraphStyles()->GetTextDecoration();
    iLinethrough = (dwDecoration & FDE_CSSTEXTDECORATION_LineThrough) ? 1 : 0;
    return;
  }

  CXFA_Font font = pTextProvider->GetFontNode();
  if (font)
    iLinethrough = font.GetLineThrough();
}

FX_ARGB CXFA_TextParser::GetColor(CXFA_TextProvider* pTextProvider,
                                  IFDE_CSSComputedStyle* pStyle) const {
  if (pStyle)
    return pStyle->GetFontStyles()->GetColor();
  if (CXFA_Font font = pTextProvider->GetFontNode())
    return font.GetColor();

  return 0xFF000000;
}

FX_FLOAT CXFA_TextParser::GetBaseline(CXFA_TextProvider* pTextProvider,
                                      IFDE_CSSComputedStyle* pStyle) const {
  if (pStyle) {
    IFDE_CSSParagraphStyle* pParaStyle = pStyle->GetParagraphStyles();
    if (pParaStyle->GetVerticalAlign() == FDE_CSSVerticalAlign::Number)
      return pParaStyle->GetNumberVerticalAlign();
  } else if (CXFA_Font font = pTextProvider->GetFontNode()) {
    return font.GetBaselineShift();
  }
  return 0;
}

FX_FLOAT CXFA_TextParser::GetLineHeight(CXFA_TextProvider* pTextProvider,
                                        IFDE_CSSComputedStyle* pStyle,
                                        bool bFirst,
                                        FX_FLOAT fVerScale) const {
  FX_FLOAT fLineHeight = 0;
  if (pStyle)
    fLineHeight = pStyle->GetParagraphStyles()->GetLineHeight();
  else if (CXFA_Para para = pTextProvider->GetParaNode())
    fLineHeight = para.GetLineHeight();

  if (bFirst) {
    FX_FLOAT fFontSize = GetFontSize(pTextProvider, pStyle);
    if (fLineHeight < 0.1f)
      fLineHeight = fFontSize;
    else
      fLineHeight = std::min(fLineHeight, fFontSize);
  } else if (fLineHeight < 0.1f) {
    fLineHeight = GetFontSize(pTextProvider, pStyle) * 1.2f;
  }
  fLineHeight *= fVerScale;
  return fLineHeight;
}

bool CXFA_TextParser::GetEmbbedObj(CXFA_TextProvider* pTextProvider,
                                   CFDE_XMLNode* pXMLNode,
                                   CFX_WideString& wsValue) {
  wsValue.clear();
  if (!pXMLNode)
    return false;

  bool bRet = false;
  if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
    CFDE_XMLElement* pElement = static_cast<CFDE_XMLElement*>(pXMLNode);
    CFX_WideString wsAttr;
    pElement->GetString(L"xfa:embed", wsAttr);
    if (wsAttr.IsEmpty())
      return false;
    if (wsAttr.GetAt(0) == L'#')
      wsAttr.Delete(0);

    CFX_WideString ws;
    pElement->GetString(L"xfa:embedType", ws);
    if (ws.IsEmpty())
      ws = L"som";
    else
      ws.MakeLower();

    bool bURI = (ws == FX_WSTRC(L"uri"));
    if (!bURI && ws != FX_WSTRC(L"som"))
      return false;

    ws.clear();
    pElement->GetString(L"xfa:embedMode", ws);
    if (ws.IsEmpty())
      ws = L"formatted";
    else
      ws.MakeLower();

    bool bRaw = (ws == FX_WSTRC(L"raw"));
    if (!bRaw && ws != FX_WSTRC(L"formatted"))
      return false;

    bRet = pTextProvider->GetEmbbedObj(bURI, bRaw, wsAttr, wsValue);
  }
  return bRet;
}

CXFA_TextParseContext* CXFA_TextParser::GetParseContextFromMap(
    CFDE_XMLNode* pXMLNode) {
  auto it = m_mapXMLNodeToParseContext.find(pXMLNode);
  return it != m_mapXMLNodeToParseContext.end() ? it->second : nullptr;
}

bool CXFA_TextParser::GetTabstops(IFDE_CSSComputedStyle* pStyle,
                                  CXFA_TextTabstopsContext* pTabstopContext) {
  if (!pStyle || !pTabstopContext)
    return false;

  CFX_WideString wsValue;
  if (!pStyle->GetCustomStyle(FX_WSTRC(L"xfa-tab-stops"), wsValue) &&
      !pStyle->GetCustomStyle(FX_WSTRC(L"tab-stops"), wsValue)) {
    return false;
  }

  int32_t iLength = wsValue.GetLength();
  const FX_WCHAR* pTabStops = wsValue.c_str();
  int32_t iCur = 0;
  int32_t iLast = 0;
  CFX_WideString wsAlign;
  TabStopStatus eStatus = TabStopStatus::None;
  FX_WCHAR ch;
  while (iCur < iLength) {
    ch = pTabStops[iCur];
    switch (eStatus) {
      case TabStopStatus::None:
        if (ch <= ' ') {
          iCur++;
        } else {
          eStatus = TabStopStatus::Alignment;
          iLast = iCur;
        }
        break;
      case TabStopStatus::Alignment:
        if (ch == ' ') {
          wsAlign = CFX_WideStringC(pTabStops + iLast, iCur - iLast);
          eStatus = TabStopStatus::StartLeader;
          iCur++;
          while (iCur < iLength && pTabStops[iCur] <= ' ')
            iCur++;
          iLast = iCur;
        } else {
          iCur++;
        }
        break;
      case TabStopStatus::StartLeader:
        if (ch != 'l') {
          eStatus = TabStopStatus::Location;
        } else {
          int32_t iCount = 0;
          while (iCur < iLength) {
            ch = pTabStops[iCur];
            iCur++;
            if (ch == '(') {
              iCount++;
            } else if (ch == ')') {
              iCount--;
              if (iCount == 0)
                break;
            }
          }
          while (iCur < iLength && pTabStops[iCur] <= ' ')
            iCur++;

          iLast = iCur;
          eStatus = TabStopStatus::Location;
        }
        break;
      case TabStopStatus::Location:
        if (ch == ' ') {
          uint32_t dwHashCode = FX_HashCode_GetW(wsAlign.AsStringC(), true);
          CXFA_Measurement ms(CFX_WideStringC(pTabStops + iLast, iCur - iLast));
          FX_FLOAT fPos = ms.ToUnit(XFA_UNIT_Pt);
          pTabstopContext->Append(dwHashCode, fPos);
          wsAlign.clear();
          eStatus = TabStopStatus::None;
        }
        iCur++;
        break;
      default:
        break;
    }
  }

  if (!wsAlign.IsEmpty()) {
    uint32_t dwHashCode = FX_HashCode_GetW(wsAlign.AsStringC(), true);
    CXFA_Measurement ms(CFX_WideStringC(pTabStops + iLast, iCur - iLast));
    FX_FLOAT fPos = ms.ToUnit(XFA_UNIT_Pt);
    pTabstopContext->Append(dwHashCode, fPos);
  }
  return true;
}
