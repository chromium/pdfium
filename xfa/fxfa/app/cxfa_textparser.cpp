// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_textparser.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"
#include "xfa/fde/css/cfde_csscomputedstyle.h"
#include "xfa/fde/css/cfde_cssstyleselector.h"
#include "xfa/fde/css/cfde_cssstylesheet.h"
#include "xfa/fde/css/fde_css.h"
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

CXFA_TextParser::CXFA_TextParser()
    : m_bParsed(false), m_cssInitialized(false) {}

CXFA_TextParser::~CXFA_TextParser() {
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

  if (m_cssInitialized)
    return;

  m_cssInitialized = true;
  auto uaSheet = LoadDefaultSheetStyle();
  m_pSelector->SetUAStyleSheet(std::move(uaSheet));
  m_pSelector->UpdateStyleIndex();
}

std::unique_ptr<CFDE_CSSStyleSheet> CXFA_TextParser::LoadDefaultSheetStyle() {
  static const FX_WCHAR s_pStyle[] =
      L"html,body,ol,p,ul{display:block}"
      L"li{display:list-item}"
      L"ol,ul{padding-left:33px;margin:1.12em 0}"
      L"ol{list-style-type:decimal}"
      L"a{color:#0000ff;text-decoration:underline}"
      L"b{font-weight:bolder}"
      L"i{font-style:italic}"
      L"sup{vertical-align:+15em;font-size:.66em}"
      L"sub{vertical-align:-15em;font-size:.66em}";

  auto sheet = pdfium::MakeUnique<CFDE_CSSStyleSheet>();
  return sheet->LoadBuffer(s_pStyle, FXSYS_wcslen(s_pStyle)) ? std::move(sheet)
                                                             : nullptr;
}

CFX_RetainPtr<CFDE_CSSComputedStyle> CXFA_TextParser::CreateRootStyle(
    CXFA_TextProvider* pTextProvider) {
  CXFA_Font font = pTextProvider->GetFontNode();
  CXFA_Para para = pTextProvider->GetParaNode();
  auto pStyle = m_pSelector->CreateComputedStyle(nullptr);
  FX_FLOAT fLineHeight = 0;
  FX_FLOAT fFontSize = 10;

  if (para) {
    fLineHeight = para.GetLineHeight();
    FDE_CSSLength indent;
    indent.Set(FDE_CSSLengthUnit::Point, para.GetTextIndent());
    pStyle->SetTextIndent(indent);
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
    pStyle->SetTextAlign(hAlign);
    FDE_CSSRect rtMarginWidth;
    rtMarginWidth.left.Set(FDE_CSSLengthUnit::Point, para.GetMarginLeft());
    rtMarginWidth.top.Set(FDE_CSSLengthUnit::Point, para.GetSpaceAbove());
    rtMarginWidth.right.Set(FDE_CSSLengthUnit::Point, para.GetMarginRight());
    rtMarginWidth.bottom.Set(FDE_CSSLengthUnit::Point, para.GetSpaceBelow());
    pStyle->SetMarginWidth(rtMarginWidth);
  }

  if (font) {
    pStyle->SetColor(font.GetColor());
    pStyle->SetFontStyle(font.IsItalic() ? FDE_CSSFontStyle::Italic
                                         : FDE_CSSFontStyle::Normal);
    pStyle->SetFontWeight(font.IsBold() ? FXFONT_FW_BOLD : FXFONT_FW_NORMAL);
    pStyle->SetNumberVerticalAlign(-font.GetBaselineShift());
    fFontSize = font.GetFontSize();
    FDE_CSSLength letterSpacing;
    letterSpacing.Set(FDE_CSSLengthUnit::Point, font.GetLetterSpacing());
    pStyle->SetLetterSpacing(letterSpacing);
    uint32_t dwDecoration = 0;
    if (font.GetLineThrough() > 0)
      dwDecoration |= FDE_CSSTEXTDECORATION_LineThrough;
    if (font.GetUnderline() > 1)
      dwDecoration |= FDE_CSSTEXTDECORATION_Double;
    else if (font.GetUnderline() > 0)
      dwDecoration |= FDE_CSSTEXTDECORATION_Underline;

    pStyle->SetTextDecoration(dwDecoration);
  }
  pStyle->SetLineHeight(fLineHeight);
  pStyle->SetFontSize(fFontSize);
  return pStyle;
}

CFX_RetainPtr<CFDE_CSSComputedStyle> CXFA_TextParser::CreateStyle(
    CFDE_CSSComputedStyle* pParentStyle) {
  auto pNewStyle = m_pSelector->CreateComputedStyle(pParentStyle);
  ASSERT(pNewStyle);
  if (!pParentStyle)
    return pNewStyle;

  uint32_t dwDecoration = pParentStyle->GetTextDecoration();
  FX_FLOAT fBaseLine = 0;
  if (pParentStyle->GetVerticalAlign() == FDE_CSSVerticalAlign::Number)
    fBaseLine = pParentStyle->GetNumberVerticalAlign();

  pNewStyle->SetTextDecoration(dwDecoration);
  pNewStyle->SetNumberVerticalAlign(fBaseLine);

  const FDE_CSSRect* pRect = pParentStyle->GetMarginWidth();
  if (pRect)
    pNewStyle->SetMarginWidth(*pRect);
  return pNewStyle;
}

CFX_RetainPtr<CFDE_CSSComputedStyle> CXFA_TextParser::ComputeStyle(
    CFDE_XMLNode* pXMLNode,
    CFDE_CSSComputedStyle* pParentStyle) {
  auto it = m_mapXMLNodeToParseContext.find(pXMLNode);
  if (it == m_mapXMLNodeToParseContext.end())
    return nullptr;

  CXFA_TextParseContext* pContext = it->second;
  if (!pContext)
    return nullptr;

  pContext->m_pParentStyle.Reset(pParentStyle);

  auto tagProvider = ParseTagInfo(pXMLNode);
  if (tagProvider->m_bContent)
    return nullptr;

  auto pStyle = CreateStyle(pParentStyle);
  m_pSelector->ComputeStyle(pContext->GetDecls(),
                            tagProvider->GetAttribute(L"style"),
                            tagProvider->GetAttribute(L"align"), pStyle.Get());
  return pStyle;
}

void CXFA_TextParser::DoParse(CFDE_XMLNode* pXMLContainer,
                              CXFA_TextProvider* pTextProvider) {
  if (!pXMLContainer || !pTextProvider || m_bParsed)
    return;

  m_bParsed = true;
  InitCSSData(pTextProvider);
  auto pRootStyle = CreateRootStyle(pTextProvider);
  ParseRichText(pXMLContainer, pRootStyle.Get());
}

void CXFA_TextParser::ParseRichText(CFDE_XMLNode* pXMLNode,
                                    CFDE_CSSComputedStyle* pParentStyle) {
  if (!pXMLNode)
    return;

  auto tagProvider = ParseTagInfo(pXMLNode);
  if (!tagProvider->m_bTagAvailable)
    return;

  CFX_RetainPtr<CFDE_CSSComputedStyle> pNewStyle;
  if ((tagProvider->GetTagName() != L"body") ||
      (tagProvider->GetTagName() != L"html")) {
    CXFA_TextParseContext* pTextContext = new CXFA_TextParseContext;
    FDE_CSSDisplay eDisplay = FDE_CSSDisplay::Inline;
    if (!tagProvider->m_bContent) {
      auto declArray =
          m_pSelector->MatchDeclarations(tagProvider->GetTagName());
      pNewStyle = CreateStyle(pParentStyle);
      m_pSelector->ComputeStyle(declArray, tagProvider->GetAttribute(L"style"),
                                tagProvider->GetAttribute(L"align"),
                                pNewStyle.Get());

      if (!declArray.empty())
        pTextContext->SetDecls(std::move(declArray));

      eDisplay = pNewStyle->GetDisplay();
    }
    pTextContext->SetDisplay(eDisplay);
    m_mapXMLNodeToParseContext[pXMLNode] = pTextContext;
  }

  for (CFDE_XMLNode* pXMLChild =
           pXMLNode->GetNodeItem(CFDE_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(CFDE_XMLNode::NextSibling)) {
    ParseRichText(pXMLChild, pNewStyle.Get());
  }
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

std::unique_ptr<CXFA_CSSTagProvider> CXFA_TextParser::ParseTagInfo(
    CFDE_XMLNode* pXMLNode) {
  auto tagProvider = pdfium::MakeUnique<CXFA_CSSTagProvider>();

  CFX_WideString wsName;
  if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
    CFDE_XMLElement* pXMLElement = static_cast<CFDE_XMLElement*>(pXMLNode);
    pXMLElement->GetLocalTagName(wsName);
    tagProvider->SetTagName(wsName);
    tagProvider->m_bTagAvailable = TagValidate(wsName);

    CFX_WideString wsValue;
    pXMLElement->GetString(L"style", wsValue);
    if (!wsValue.IsEmpty())
      tagProvider->SetAttribute(L"style", wsValue);
  } else if (pXMLNode->GetType() == FDE_XMLNODE_Text) {
    tagProvider->m_bTagAvailable = true;
    tagProvider->m_bContent = true;
  }
  return tagProvider;
}

int32_t CXFA_TextParser::GetVAlign(CXFA_TextProvider* pTextProvider) const {
  CXFA_Para para = pTextProvider->GetParaNode();
  return para ? para.GetVerticalAlign() : XFA_ATTRIBUTEENUM_Top;
}

FX_FLOAT CXFA_TextParser::GetTabInterval(CFDE_CSSComputedStyle* pStyle) const {
  CFX_WideString wsValue;
  if (pStyle && pStyle->GetCustomStyle(L"tab-interval", wsValue))
    return CXFA_Measurement(wsValue.AsStringC()).ToUnit(XFA_UNIT_Pt);
  return 36;
}

int32_t CXFA_TextParser::CountTabs(CFDE_CSSComputedStyle* pStyle) const {
  CFX_WideString wsValue;
  if (pStyle && pStyle->GetCustomStyle(L"xfa-tab-count", wsValue))
    return wsValue.GetInteger();
  return 0;
}

bool CXFA_TextParser::IsSpaceRun(CFDE_CSSComputedStyle* pStyle) const {
  CFX_WideString wsValue;
  if (pStyle && pStyle->GetCustomStyle(L"xfa-spacerun", wsValue)) {
    wsValue.MakeLower();
    return wsValue == L"yes";
  }
  return false;
}

CFX_RetainPtr<CFGAS_GEFont> CXFA_TextParser::GetFont(
    CXFA_TextProvider* pTextProvider,
    CFDE_CSSComputedStyle* pStyle) const {
  CFX_WideStringC wsFamily = L"Courier";
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
    int32_t iCount = pStyle->CountFontFamilies();
    if (iCount > 0)
      wsFamily = pStyle->GetFontFamily(iCount - 1).AsStringC();

    dwStyle = 0;
    if (pStyle->GetFontWeight() > FXFONT_FW_NORMAL)
      dwStyle |= FX_FONTSTYLE_Bold;
    if (pStyle->GetFontStyle() == FDE_CSSFontStyle::Italic)
      dwStyle |= FX_FONTSTYLE_Italic;
  }

  CXFA_FFDoc* pDoc = pTextProvider->GetDocNode();
  CXFA_FontMgr* pFontMgr = pDoc->GetApp()->GetXFAFontMgr();
  return pFontMgr->GetFont(pDoc, wsFamily, dwStyle);
}

FX_FLOAT CXFA_TextParser::GetFontSize(CXFA_TextProvider* pTextProvider,
                                      CFDE_CSSComputedStyle* pStyle) const {
  if (pStyle)
    return pStyle->GetFontSize();

  CXFA_Font font = pTextProvider->GetFontNode();
  if (font)
    return font.GetFontSize();
  return 10;
}

int32_t CXFA_TextParser::GetHorScale(CXFA_TextProvider* pTextProvider,
                                     CFDE_CSSComputedStyle* pStyle,
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
                                     CFDE_CSSComputedStyle* pStyle) const {
  if (pStyle) {
    CFX_WideString wsValue;
    if (pStyle->GetCustomStyle(L"xfa-font-vertical-scale", wsValue))
      return wsValue.GetInteger();
  }

  if (CXFA_Font font = pTextProvider->GetFontNode())
    return (int32_t)font.GetVerticalScale();
  return 100;
}

void CXFA_TextParser::GetUnderline(CXFA_TextProvider* pTextProvider,
                                   CFDE_CSSComputedStyle* pStyle,
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

  uint32_t dwDecoration = pStyle->GetTextDecoration();
  if (dwDecoration & FDE_CSSTEXTDECORATION_Double)
    iUnderline = 2;
  else if (dwDecoration & FDE_CSSTEXTDECORATION_Underline)
    iUnderline = 1;

  CFX_WideString wsValue;
  if (pStyle->GetCustomStyle(L"underlinePeriod", wsValue)) {
    if (wsValue == L"word")
      iPeriod = XFA_ATTRIBUTEENUM_Word;
  } else if (CXFA_Font font = pTextProvider->GetFontNode()) {
    iPeriod = font.GetUnderlinePeriod();
  }
}

void CXFA_TextParser::GetLinethrough(CXFA_TextProvider* pTextProvider,
                                     CFDE_CSSComputedStyle* pStyle,
                                     int32_t& iLinethrough) const {
  if (pStyle) {
    uint32_t dwDecoration = pStyle->GetTextDecoration();
    iLinethrough = (dwDecoration & FDE_CSSTEXTDECORATION_LineThrough) ? 1 : 0;
    return;
  }

  CXFA_Font font = pTextProvider->GetFontNode();
  if (font)
    iLinethrough = font.GetLineThrough();
}

FX_ARGB CXFA_TextParser::GetColor(CXFA_TextProvider* pTextProvider,
                                  CFDE_CSSComputedStyle* pStyle) const {
  if (pStyle)
    return pStyle->GetColor();
  if (CXFA_Font font = pTextProvider->GetFontNode())
    return font.GetColor();

  return 0xFF000000;
}

FX_FLOAT CXFA_TextParser::GetBaseline(CXFA_TextProvider* pTextProvider,
                                      CFDE_CSSComputedStyle* pStyle) const {
  if (pStyle) {
    if (pStyle->GetVerticalAlign() == FDE_CSSVerticalAlign::Number)
      return pStyle->GetNumberVerticalAlign();
  } else if (CXFA_Font font = pTextProvider->GetFontNode()) {
    return font.GetBaselineShift();
  }
  return 0;
}

FX_FLOAT CXFA_TextParser::GetLineHeight(CXFA_TextProvider* pTextProvider,
                                        CFDE_CSSComputedStyle* pStyle,
                                        bool bFirst,
                                        FX_FLOAT fVerScale) const {
  FX_FLOAT fLineHeight = 0;
  if (pStyle)
    fLineHeight = pStyle->GetLineHeight();
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

    bool bURI = (ws == L"uri");
    if (!bURI && ws != L"som")
      return false;

    ws.clear();
    pElement->GetString(L"xfa:embedMode", ws);
    if (ws.IsEmpty())
      ws = L"formatted";
    else
      ws.MakeLower();

    bool bRaw = (ws == L"raw");
    if (!bRaw && ws != L"formatted")
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

bool CXFA_TextParser::GetTabstops(CFDE_CSSComputedStyle* pStyle,
                                  CXFA_TextTabstopsContext* pTabstopContext) {
  if (!pStyle || !pTabstopContext)
    return false;

  CFX_WideString wsValue;
  if (!pStyle->GetCustomStyle(L"xfa-tab-stops", wsValue) &&
      !pStyle->GetCustomStyle(L"tab-stops", wsValue)) {
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
