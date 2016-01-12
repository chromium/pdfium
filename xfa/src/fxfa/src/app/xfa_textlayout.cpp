// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_textlayout.h"
#include "xfa_ffapp.h"
#include "xfa_ffdoc.h"
#include "xfa_fontmgr.h"
CXFA_CSSTagProvider::~CXFA_CSSTagProvider() {
  FX_POSITION pos = m_Attributes.GetStartPosition();
  while (pos) {
    CFX_WideString *pName = NULL, *pValue = NULL;
    m_Attributes.GetNextAssoc(pos, (void*&)pName, (void*&)pValue);
    if (pName != NULL) {
      delete pName;
    }
    if (pValue != NULL) {
      delete pValue;
    }
  }
}
void CXFA_CSSTagProvider::GetNextAttribute(FX_POSITION& pos,
                                           CFX_WideStringC& wsAttr,
                                           CFX_WideStringC& wsValue) {
  if (pos == NULL) {
    return;
  }
  CFX_WideString* pName = NULL;
  CFX_WideString* pValue = NULL;
  m_Attributes.GetNextAssoc(pos, (void*&)pName, (void*&)pValue);
  wsAttr = *pName;
  wsValue = *pValue;
}
void CXFA_CSSTagProvider::SetAttribute(const CFX_WideString& wsAttr,
                                       const CFX_WideString& wsValue) {
  CFX_WideString* pName = new CFX_WideString();
  CFX_WideString* pValue = new CFX_WideString();
  *pName = wsAttr;
  *pValue = wsValue;
  m_Attributes.SetAt(pName, pValue);
}
void CXFA_TextParseContext::SetDecls(const IFDE_CSSDeclaration** ppDeclArray,
                                     int32_t iDeclCount) {
  if (iDeclCount <= 0 || ppDeclArray == NULL) {
    return;
  }
  m_dwMatchedDecls = iDeclCount;
  m_ppMatchedDecls = FX_Alloc(IFDE_CSSDeclaration*, iDeclCount);
  FX_memcpy(m_ppMatchedDecls, ppDeclArray,
            iDeclCount * sizeof(IFDE_CSSDeclaration*));
}
CXFA_TextParser::~CXFA_TextParser() {
  if (m_pUASheet != NULL) {
    m_pUASheet->Release();
  }
  if (m_pSelector != NULL) {
    m_pSelector->Release();
  }
  if (m_pAllocator != NULL) {
    m_pAllocator->Release();
  }
  FX_POSITION ps = m_mapXMLNodeToParseContext.GetStartPosition();
  while (ps) {
    IFDE_XMLNode* pXMLNode;
    CXFA_TextParseContext* pParseContext;
    m_mapXMLNodeToParseContext.GetNextAssoc(ps, pXMLNode, pParseContext);
    if (pParseContext != NULL) {
      FDE_DeleteWith(CXFA_TextParseContext, m_pAllocator, pParseContext);
    }
  }
  m_mapXMLNodeToParseContext.RemoveAll();
}
void CXFA_TextParser::Reset() {
  FX_POSITION ps = m_mapXMLNodeToParseContext.GetStartPosition();
  while (ps) {
    IFDE_XMLNode* pXMLNode;
    CXFA_TextParseContext* pParseContext;
    m_mapXMLNodeToParseContext.GetNextAssoc(ps, pXMLNode, pParseContext);
    if (pParseContext != NULL) {
      FDE_DeleteWith(CXFA_TextParseContext, m_pAllocator, pParseContext);
    }
  }
  m_mapXMLNodeToParseContext.RemoveAll();
  if (m_pAllocator != NULL) {
    m_pAllocator->Release();
    m_pAllocator = NULL;
  }
}
void CXFA_TextParser::InitCSSData(IXFA_TextProvider* pTextProvider) {
  if (pTextProvider == NULL) {
    return;
  }
  if (m_pSelector == NULL) {
    CXFA_FFDoc* pDoc = pTextProvider->GetDocNode();
    IFX_FontMgr* pFontMgr = pDoc->GetApp()->GetFDEFontMgr();
    FXSYS_assert(pFontMgr != NULL);
    m_pSelector = IFDE_CSSStyleSelector::Create();
    m_pSelector->SetFontMgr(pFontMgr);
    FX_FLOAT fFontSize = 10;
    CXFA_Font font = pTextProvider->GetFontNode();
    if (font.IsExistInXML()) {
      fFontSize = font.GetFontSize();
    }
    m_pSelector->SetDefFontSize(fFontSize);
  }
  if (m_pUASheet == NULL) {
    m_pUASheet = LoadDefaultSheetStyle();
    m_pSelector->SetStyleSheet(FDE_CSSSTYLESHEETGROUP_UserAgent, m_pUASheet);
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
    IXFA_TextProvider* pTextProvider) {
  CXFA_Font font = pTextProvider->GetFontNode();
  CXFA_Para para = pTextProvider->GetParaNode();
  IFDE_CSSComputedStyle* pStyle = m_pSelector->CreateComputedStyle(NULL);
  IFDE_CSSFontStyle* pFontStyle = pStyle->GetFontStyles();
  IFDE_CSSParagraphStyle* pParaStyle = pStyle->GetParagraphStyles();
  FX_FLOAT fLineHeight = 0, fFontSize = 10;
  if (para.IsExistInXML()) {
    fLineHeight = para.GetLineHeight();
    FDE_CSSLENGTH indent;
    indent.Set(FDE_CSSLENGTHUNIT_Point, para.GetTextIndent());
    pParaStyle->SetTextIndent(indent);
    FDE_CSSTEXTALIGN hAlgin = FDE_CSSTEXTALIGN_Left;
    switch (para.GetHorizontalAlign()) {
      case XFA_ATTRIBUTEENUM_Center:
        hAlgin = FDE_CSSTEXTALIGN_Center;
        break;
      case XFA_ATTRIBUTEENUM_Right:
        hAlgin = FDE_CSSTEXTALIGN_Right;
        break;
      case XFA_ATTRIBUTEENUM_Justify:
        hAlgin = FDE_CSSTEXTALIGN_Justify;
        break;
      case XFA_ATTRIBUTEENUM_JustifyAll:
        hAlgin = FDE_CSSTEXTALIGN_JustifyAll;
        break;
    }
    pParaStyle->SetTextAlign(hAlgin);
    FDE_CSSRECT rtMarginWidth;
    rtMarginWidth.left.Set(FDE_CSSLENGTHUNIT_Point, para.GetMarginLeft());
    rtMarginWidth.top.Set(FDE_CSSLENGTHUNIT_Point, para.GetSpaceAbove());
    rtMarginWidth.right.Set(FDE_CSSLENGTHUNIT_Point, para.GetMarginRight());
    rtMarginWidth.bottom.Set(FDE_CSSLENGTHUNIT_Point, para.GetSpaceBelow());
    pStyle->GetBoundaryStyles()->SetMarginWidth(rtMarginWidth);
  }
  if (font.IsExistInXML()) {
    pFontStyle->SetColor(font.GetColor());
    pFontStyle->SetFontStyle(font.IsItalic() ? FDE_CSSFONTSTYLE_Italic
                                             : FDE_CSSFONTSTYLE_Normal);
    pFontStyle->SetFontWeight(font.IsBold() ? FXFONT_FW_BOLD
                                            : FXFONT_FW_NORMAL);
    pParaStyle->SetNumberVerticalAlign(-font.GetBaselineShift());
    fFontSize = font.GetFontSize();
    FDE_CSSLENGTH letterSpacing;
    letterSpacing.Set(FDE_CSSLENGTHUNIT_Point, font.GetLetterSpacing());
    pParaStyle->SetLetterSpacing(letterSpacing);
    FX_DWORD dwDecoration = 0;
    if (font.GetLineThrough() > 0) {
      dwDecoration |= FDE_CSSTEXTDECORATION_LineThrough;
    }
    if (font.GetUnderline() > 1) {
      dwDecoration |= FDE_CSSTEXTDECORATION_Double;
    } else if (font.GetUnderline() > 0) {
      dwDecoration |= FDE_CSSTEXTDECORATION_Underline;
    }
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
  FXSYS_assert(pNewStyle != NULL);
  if (pParentStyle) {
    IFDE_CSSParagraphStyle* pParaStyle = pParentStyle->GetParagraphStyles();
    FX_DWORD dwDecoration = pParaStyle->GetTextDecoration();
    FX_FLOAT fBaseLine = 0;
    if (pParaStyle->GetVerticalAlign() == FDE_CSSVERTICALALIGN_Number) {
      fBaseLine = pParaStyle->GetNumberVerticalAlign();
    }
    pParaStyle = pNewStyle->GetParagraphStyles();
    pParaStyle->SetTextDecoration(dwDecoration);
    pParaStyle->SetNumberVerticalAlign(fBaseLine);
    IFDE_CSSBoundaryStyle* pBoundarytyle = pParentStyle->GetBoundaryStyles();
    const FDE_CSSRECT* pRect = pBoundarytyle->GetMarginWidth();
    if (pRect != NULL) {
      pBoundarytyle = pNewStyle->GetBoundaryStyles();
      pBoundarytyle->SetMarginWidth(*pRect);
    }
  }
  return pNewStyle;
}
IFDE_CSSComputedStyle* CXFA_TextParser::ComputeStyle(
    IFDE_XMLNode* pXMLNode,
    IFDE_CSSComputedStyle* pParentStyle) {
  CXFA_TextParseContext* pContext = static_cast<CXFA_TextParseContext*>(
      m_mapXMLNodeToParseContext.GetValueAt(pXMLNode));
  if (!pContext)
    return nullptr;
  pContext->m_pParentStyle = pParentStyle;
  pParentStyle->AddRef();
  CXFA_CSSTagProvider tagProvider;
  ParseTagInfo(pXMLNode, tagProvider);
  if (tagProvider.m_bContent)
    return nullptr;
  IFDE_CSSComputedStyle* pStyle = CreateStyle(pParentStyle);
  IFDE_CSSAccelerator* pCSSAccel = m_pSelector->InitAccelerator();
  pCSSAccel->OnEnterTag(&tagProvider);
  m_pSelector->ComputeStyle(&tagProvider, pContext->GetDecls(),
                            pContext->CountDecls(), pStyle);
  pCSSAccel->OnLeaveTag(&tagProvider);
  return pStyle;
}
void CXFA_TextParser::DoParse(IFDE_XMLNode* pXMLContainer,
                              IXFA_TextProvider* pTextProvider) {
  if (pXMLContainer == NULL || pTextProvider == NULL || m_pAllocator != NULL) {
    return;
  }
  m_pAllocator =
      FX_CreateAllocator(FX_ALLOCTYPE_Fixed, 32, sizeof(CXFA_CSSTagProvider));
  InitCSSData(pTextProvider);
  IFDE_CSSComputedStyle* pRootStyle = CreateRootStyle(pTextProvider);
  ParseRichText(pXMLContainer, pRootStyle);
  pRootStyle->Release();
}
void CXFA_TextParser::ParseRichText(IFDE_XMLNode* pXMLNode,
                                    IFDE_CSSComputedStyle* pParentStyle) {
  if (pXMLNode == NULL) {
    return;
  }
  CXFA_CSSTagProvider tagProvider;
  ParseTagInfo(pXMLNode, tagProvider);
  if (!tagProvider.m_bTagAviliable) {
    return;
  }
  IFDE_CSSComputedStyle* pNewStyle = NULL;
  if ((tagProvider.GetTagName() != FX_WSTRC(L"body")) ||
      (tagProvider.GetTagName() != FX_WSTRC(L"html"))) {
    CXFA_TextParseContext* pTextContext =
        FDE_NewWith(m_pAllocator) CXFA_TextParseContext;
    FDE_CSSDISPLAY eDisplay = FDE_CSSDISPLAY_Inline;
    if (!tagProvider.m_bContent) {
      pNewStyle = CreateStyle(pParentStyle);
      IFDE_CSSAccelerator* pCSSAccel = m_pSelector->InitAccelerator();
      pCSSAccel->OnEnterTag(&tagProvider);
      CFDE_CSSDeclarationArray DeclArray;
      int32_t iMatchedDecls =
          m_pSelector->MatchDeclarations(&tagProvider, DeclArray);
      const IFDE_CSSDeclaration** ppMatchDecls =
          (const IFDE_CSSDeclaration**)DeclArray.GetData();
      m_pSelector->ComputeStyle(&tagProvider, ppMatchDecls, iMatchedDecls,
                                pNewStyle);
      pCSSAccel->OnLeaveTag(&tagProvider);
      if (iMatchedDecls > 0) {
        pTextContext->SetDecls(ppMatchDecls, iMatchedDecls);
      }
      eDisplay = pNewStyle->GetPositionStyles()->GetDisplay();
    }
    pTextContext->SetDisplay(eDisplay);
    m_mapXMLNodeToParseContext.SetAt(pXMLNode, pTextContext);
  }
  for (IFDE_XMLNode* pXMLChild =
           pXMLNode->GetNodeItem(IFDE_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(IFDE_XMLNode::NextSibling)) {
    ParseRichText(pXMLChild, pNewStyle);
  }
  if (pNewStyle != NULL) {
    pNewStyle->Release();
  }
}
void CXFA_TextParser::ParseTagInfo(IFDE_XMLNode* pXMLNode,
                                   CXFA_CSSTagProvider& tagProvider) {
  static const FX_DWORD s_XFATagName[] = {
      0x61,       0x62,       0x69,       0x70,       0x0001f714,
      0x00022a55, 0x000239bb, 0x00025881, 0x0bd37faa, 0x0bd37fb8,
      0xa73e3af2, 0xb182eaae, 0xdb8ac455,
  };
  CFX_WideString wsName;
  if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
    IFDE_XMLElement* pXMLElement = (IFDE_XMLElement*)pXMLNode;
    pXMLElement->GetLocalTagName(wsName);
    tagProvider.SetTagNameObj(wsName);
    FX_DWORD dwHashCode =
        FX_HashCode_String_GetW(wsName, wsName.GetLength(), TRUE);
    static const int32_t s_iCount = sizeof(s_XFATagName) / sizeof(FX_DWORD);
    CFX_DSPATemplate<FX_DWORD> lookup;
    tagProvider.m_bTagAviliable =
        lookup.Lookup(dwHashCode, s_XFATagName, s_iCount) > -1;
    CFX_WideString wsValue;
    pXMLElement->GetString(FX_WSTRC(L"style").GetPtr(), wsValue);
    if (!wsValue.IsEmpty()) {
      tagProvider.SetAttribute(FX_WSTRC(L"style"), wsValue);
    }
  } else if (pXMLNode->GetType() == FDE_XMLNODE_Text) {
    tagProvider.m_bTagAviliable = TRUE;
    tagProvider.m_bContent = TRUE;
  }
}
int32_t CXFA_TextParser::GetVAlgin(IXFA_TextProvider* pTextProvider) const {
  int32_t iAlign = XFA_ATTRIBUTEENUM_Top;
  CXFA_Para para = pTextProvider->GetParaNode();
  if (para.IsExistInXML()) {
    iAlign = para.GetVerticalAlign();
  }
  return iAlign;
}
FX_FLOAT CXFA_TextParser::GetTabInterval(IFDE_CSSComputedStyle* pStyle) const {
  CFX_WideString wsValue;
  if (pStyle && pStyle->GetCustomStyle(FX_WSTRC(L"tab-interval"), wsValue)) {
    CXFA_Measurement ms(wsValue);
    return ms.ToUnit(XFA_UNIT_Pt);
  }
  return 36;
}
int32_t CXFA_TextParser::CountTabs(IFDE_CSSComputedStyle* pStyle) const {
  CFX_WideString wsValue;
  if (pStyle && pStyle->GetCustomStyle(FX_WSTRC(L"xfa-tab-count"), wsValue)) {
    return wsValue.GetInteger();
  }
  return 0;
}
FX_BOOL CXFA_TextParser::IsSpaceRun(IFDE_CSSComputedStyle* pStyle) const {
  CFX_WideString wsValue;
  if (pStyle && pStyle->GetCustomStyle(FX_WSTRC(L"xfa-spacerun"), wsValue)) {
    wsValue.MakeLower();
    return wsValue == FX_WSTRC(L"yes");
  }
  return FALSE;
}
IFX_Font* CXFA_TextParser::GetFont(IXFA_TextProvider* pTextProvider,
                                   IFDE_CSSComputedStyle* pStyle) const {
  CFX_WideStringC wsFamily = FX_WSTRC(L"Courier");
  FX_DWORD dwStyle = 0;
  CXFA_Font font = pTextProvider->GetFontNode();
  if (font.IsExistInXML()) {
    font.GetTypeface(wsFamily);
    if (font.IsBold()) {
      dwStyle |= FX_FONTSTYLE_Bold;
    }
    if (font.IsItalic()) {
      dwStyle |= FX_FONTSTYLE_Italic;
    }
  }
  if (pStyle) {
    IFDE_CSSFontStyle* pFontStyle = pStyle->GetFontStyles();
    int32_t iCount = pFontStyle->CountFontFamilies();
    if (iCount > 0) {
      wsFamily = pFontStyle->GetFontFamily(iCount - 1);
    }
    dwStyle = 0;
    if (pFontStyle->GetFontWeight() > FXFONT_FW_NORMAL) {
      dwStyle |= FX_FONTSTYLE_Bold;
    }
    if (pFontStyle->GetFontStyle() == FDE_CSSFONTSTYLE_Italic) {
      dwStyle |= FX_FONTSTYLE_Italic;
    }
  }
  CXFA_FFDoc* pDoc = pTextProvider->GetDocNode();
  FXSYS_assert(pDoc != NULL);
  CXFA_FontMgr* pFontMgr = pDoc->GetApp()->GetXFAFontMgr();
  return pFontMgr->GetFont(pDoc, wsFamily, dwStyle);
}
FX_FLOAT CXFA_TextParser::GetFontSize(IXFA_TextProvider* pTextProvider,
                                      IFDE_CSSComputedStyle* pStyle) const {
  if (pStyle != NULL) {
    return pStyle->GetFontStyles()->GetFontSize();
  }
  CXFA_Font font = pTextProvider->GetFontNode();
  if (font.IsExistInXML()) {
    return font.GetFontSize();
  }
  return 10;
}
int32_t CXFA_TextParser::GetHorScale(IXFA_TextProvider* pTextProvider,
                                     IFDE_CSSComputedStyle* pStyle,
                                     IFDE_XMLNode* pXMLNode) const {
  if (pStyle) {
    CFX_WideString wsValue;
    if (pStyle->GetCustomStyle(FX_WSTRC(L"xfa-font-horizontal-scale"),
                               wsValue)) {
      return wsValue.GetInteger();
    }
    while (pXMLNode) {
      CXFA_TextParseContext* pContext = static_cast<CXFA_TextParseContext*>(
          m_mapXMLNodeToParseContext.GetValueAt(pXMLNode));
      if (pContext && pContext->m_pParentStyle &&
          pContext->m_pParentStyle->GetCustomStyle(
              FX_WSTRC(L"xfa-font-horizontal-scale"), wsValue)) {
        return wsValue.GetInteger();
      }
      pXMLNode = pXMLNode->GetNodeItem(IFDE_XMLNode::Parent);
    }
  }
  if (CXFA_Font font = pTextProvider->GetFontNode()) {
    return static_cast<int32_t>(font.GetHorizontalScale());
  }
  return 100;
}
int32_t CXFA_TextParser::GetVerScale(IXFA_TextProvider* pTextProvider,
                                     IFDE_CSSComputedStyle* pStyle) const {
  if (pStyle != NULL) {
    CFX_WideString wsValue;
    if (pStyle->GetCustomStyle(FX_WSTRC(L"xfa-font-vertical-scale"), wsValue)) {
      return wsValue.GetInteger();
    }
  }
  if (CXFA_Font font = pTextProvider->GetFontNode()) {
    return (int32_t)font.GetVerticalScale();
  }
  return 100;
}
void CXFA_TextParser::GetUnderline(IXFA_TextProvider* pTextProvider,
                                   IFDE_CSSComputedStyle* pStyle,
                                   int32_t& iUnderline,
                                   int32_t& iPeriod) const {
  iUnderline = 0;
  iPeriod = XFA_ATTRIBUTEENUM_All;
  if (pStyle) {
    FX_DWORD dwDecoration = pStyle->GetParagraphStyles()->GetTextDecoration();
    if (dwDecoration & FDE_CSSTEXTDECORATION_Double) {
      iUnderline = 2;
    } else if (dwDecoration & FDE_CSSTEXTDECORATION_Underline) {
      iUnderline = 1;
    }
    CFX_WideString wsValue;
    if (pStyle->GetCustomStyle(FX_WSTRC(L"underlinePeriod"), wsValue)) {
      if (wsValue == FX_WSTRC(L"word")) {
        iPeriod = XFA_ATTRIBUTEENUM_Word;
      }
    } else if (CXFA_Font font = pTextProvider->GetFontNode()) {
      iPeriod = font.GetUnderlinePeriod();
    }
  } else {
    CXFA_Font font = pTextProvider->GetFontNode();
    if (font.IsExistInXML()) {
      iUnderline = font.GetUnderline();
      iPeriod = font.GetUnderlinePeriod();
    }
  }
}
void CXFA_TextParser::GetLinethrough(IXFA_TextProvider* pTextProvider,
                                     IFDE_CSSComputedStyle* pStyle,
                                     int32_t& iLinethrough) const {
  if (pStyle) {
    FX_DWORD dwDecoration = pStyle->GetParagraphStyles()->GetTextDecoration();
    iLinethrough = (dwDecoration & FDE_CSSTEXTDECORATION_LineThrough) ? 1 : 0;
  } else {
    CXFA_Font font = pTextProvider->GetFontNode();
    if (font.IsExistInXML()) {
      iLinethrough = font.GetLineThrough();
    }
  }
}
FX_ARGB CXFA_TextParser::GetColor(IXFA_TextProvider* pTextProvider,
                                  IFDE_CSSComputedStyle* pStyle) const {
  if (pStyle != NULL) {
    return pStyle->GetFontStyles()->GetColor();
  }
  if (CXFA_Font font = pTextProvider->GetFontNode()) {
    return font.GetColor();
  }
  return 0xFF000000;
}
FX_FLOAT CXFA_TextParser::GetBaseline(IXFA_TextProvider* pTextProvider,
                                      IFDE_CSSComputedStyle* pStyle) const {
  if (pStyle != NULL) {
    IFDE_CSSParagraphStyle* pParaStyle = pStyle->GetParagraphStyles();
    if (pParaStyle->GetVerticalAlign() == FDE_CSSVERTICALALIGN_Number) {
      return pParaStyle->GetNumberVerticalAlign();
    }
  } else if (CXFA_Font font = pTextProvider->GetFontNode()) {
    return font.GetBaselineShift();
  }
  return 0;
}
FX_FLOAT CXFA_TextParser::GetLineHeight(IXFA_TextProvider* pTextProvider,
                                        IFDE_CSSComputedStyle* pStyle,
                                        FX_BOOL bFirst,
                                        FX_FLOAT fVerScale) const {
  FX_FLOAT fLineHeight = 0;
  if (pStyle != NULL) {
    fLineHeight = pStyle->GetParagraphStyles()->GetLineHeight();
  } else if (CXFA_Para para = pTextProvider->GetParaNode()) {
    fLineHeight = para.GetLineHeight();
  }
  if (bFirst) {
    FX_FLOAT fFontSize = GetFontSize(pTextProvider, pStyle);
    if (fLineHeight < 0.1f) {
      fLineHeight = fFontSize;
    } else {
      fLineHeight = std::min(fLineHeight, fFontSize);
    }
  } else if (fLineHeight < 0.1f) {
    fLineHeight = GetFontSize(pTextProvider, pStyle) * 1.2f;
  }
  fLineHeight *= fVerScale;
  return fLineHeight;
}
FX_BOOL CXFA_TextParser::GetEmbbedObj(IXFA_TextProvider* pTextProvider,
                                      IFDE_XMLNode* pXMLNode,
                                      CFX_WideString& wsValue) {
  wsValue.Empty();
  if (pXMLNode == NULL) {
    return FALSE;
  }
  FX_BOOL bRet = FALSE;
  if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
    IFDE_XMLElement* pElement = (IFDE_XMLElement*)pXMLNode;
    CFX_WideString wsAttr;
    pElement->GetString(FX_WSTRC(L"xfa:embed").GetPtr(), wsAttr);
    if (wsAttr.IsEmpty()) {
      return FALSE;
    }
    if (wsAttr.GetAt(0) == L'#') {
      wsAttr.Delete(0);
    }
    CFX_WideString ws;
    pElement->GetString(FX_WSTRC(L"xfa:embedType").GetPtr(), ws);
    if (ws.IsEmpty()) {
      ws = L"som";
    } else {
      ws.MakeLower();
    }
    FX_BOOL bURI = (ws == FX_WSTRC(L"uri"));
    if (!bURI && ws != FX_WSTRC(L"som")) {
      return FALSE;
    }
    ws.Empty();
    pElement->GetString(FX_WSTRC(L"xfa:embedMode").GetPtr(), ws);
    if (ws.IsEmpty()) {
      ws = L"formatted";
    } else {
      ws.MakeLower();
    }
    FX_BOOL bRaw = (ws == FX_WSTRC(L"raw"));
    if (!bRaw && ws != FX_WSTRC(L"formatted")) {
      return FALSE;
    }
    bRet = pTextProvider->GetEmbbedObj(bURI, bRaw, wsAttr, wsValue);
  }
  return bRet;
}
CXFA_TextParseContext* CXFA_TextParser::GetParseContextFromMap(
    IFDE_XMLNode* pXMLNode) {
  return (CXFA_TextParseContext*)m_mapXMLNodeToParseContext.GetValueAt(
      pXMLNode);
}
enum XFA_TABSTOPSSTATUS {
  XFA_TABSTOPSSTATUS_Error,
  XFA_TABSTOPSSTATUS_EOS,
  XFA_TABSTOPSSTATUS_None,
  XFA_TABSTOPSSTATUS_Alignment,
  XFA_TABSTOPSSTATUS_StartLeader,
  XFA_TABSTOPSSTATUS_Leader,
  XFA_TABSTOPSSTATUS_Location,
};
FX_BOOL CXFA_TextParser::GetTabstops(
    IFDE_CSSComputedStyle* pStyle,
    CXFA_TextTabstopsContext* pTabstopContext) {
  if (pStyle == NULL || pTabstopContext == NULL) {
    return FALSE;
  }
  CFX_WideString wsValue;
  if (!pStyle->GetCustomStyle(FX_WSTRC(L"xfa-tab-stops"), wsValue) &&
      !pStyle->GetCustomStyle(FX_WSTRC(L"tab-stops"), wsValue)) {
    return FALSE;
  }
  int32_t iLength = wsValue.GetLength();
  const FX_WCHAR* pTabStops = wsValue;
  int32_t iCur = 0;
  int32_t iLast = 0;
  CFX_WideString wsAlign;
  XFA_TABSTOPSSTATUS eStatus = XFA_TABSTOPSSTATUS_None;
  FX_WCHAR ch;
  while (iCur < iLength) {
    ch = pTabStops[iCur];
    switch (eStatus) {
      case XFA_TABSTOPSSTATUS_None:
        if (ch <= ' ') {
          iCur++;
        } else {
          eStatus = XFA_TABSTOPSSTATUS_Alignment;
          iLast = iCur;
        }
        break;
      case XFA_TABSTOPSSTATUS_Alignment:
        if (ch == ' ') {
          wsAlign = CFX_WideStringC(pTabStops + iLast, iCur - iLast);
          eStatus = XFA_TABSTOPSSTATUS_StartLeader;
          iCur++;
          while (iCur < iLength && pTabStops[iCur] <= ' ') {
            iCur++;
          }
          iLast = iCur;
        } else {
          iCur++;
        }
        break;
      case XFA_TABSTOPSSTATUS_StartLeader:
        if (ch != 'l') {
          eStatus = XFA_TABSTOPSSTATUS_Location;
        } else {
          int32_t iCount = 0;
          while (iCur < iLength) {
            ch = pTabStops[iCur];
            iCur++;
            if (ch == '(') {
              iCount++;
            } else if (ch == ')') {
              iCount--;
              if (iCount == 0) {
                break;
              }
            }
          }
          while (iCur < iLength && pTabStops[iCur] <= ' ') {
            iCur++;
          }
          iLast = iCur;
          eStatus = XFA_TABSTOPSSTATUS_Location;
        }
        break;
      case XFA_TABSTOPSSTATUS_Location:
        if (ch == ' ') {
          FX_DWORD dwHashCode =
              FX_HashCode_String_GetW(wsAlign, wsAlign.GetLength(), TRUE);
          CXFA_Measurement ms(CFX_WideStringC(pTabStops + iLast, iCur - iLast));
          FX_FLOAT fPos = ms.ToUnit(XFA_UNIT_Pt);
          pTabstopContext->Append(dwHashCode, fPos);
          wsAlign.Empty();
          eStatus = XFA_TABSTOPSSTATUS_None;
        }
        iCur++;
        break;
      default:
        break;
    }
  }
  if (!wsAlign.IsEmpty()) {
    FX_DWORD dwHashCode =
        FX_HashCode_String_GetW(wsAlign, wsAlign.GetLength(), TRUE);
    CXFA_Measurement ms(CFX_WideStringC(pTabStops + iLast, iCur - iLast));
    FX_FLOAT fPos = ms.ToUnit(XFA_UNIT_Pt);
    pTabstopContext->Append(dwHashCode, fPos);
  }
  return TRUE;
}
CXFA_TextLayout::CXFA_TextLayout(IXFA_TextProvider* pTextProvider)
    : m_bHasBlock(FALSE),
      m_pTextProvider(pTextProvider),
      m_pTextDataNode(nullptr),
      m_bRichText(FALSE),
      m_pAllocator(nullptr),
      m_pBreak(nullptr),
      m_pLoader(nullptr),
      m_iLines(0),
      m_fMaxWidth(0),
      m_pTabstopContext(nullptr),
      m_bBlockContinue(TRUE) {
  FXSYS_assert(m_pTextProvider);
}
CXFA_TextLayout::~CXFA_TextLayout() {
  m_textParser.Reset();
  delete m_pLoader;
  delete m_pTabstopContext;
  Unload();
}
void CXFA_TextLayout::Unload() {
  int32_t iCount = m_pieceLines.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CXFA_PieceLine* pLine = m_pieceLines.GetAt(i);
    FDE_DeleteWith(CXFA_PieceLine, m_pAllocator, pLine);
  }
  m_pieceLines.RemoveAll();
  if (m_pBreak != NULL) {
    m_pBreak->Release();
    m_pBreak = NULL;
  }
  if (m_pAllocator != NULL) {
    m_pAllocator->Release();
    m_pAllocator = NULL;
  }
}
const CXFA_PieceLineArray* CXFA_TextLayout::GetPieceLines() {
  return &m_pieceLines;
}
void CXFA_TextLayout::GetTextDataNode() {
  if (m_pTextProvider == NULL) {
    return;
  }
  CXFA_Node* pNode = m_pTextProvider->GetTextNode(m_bRichText);
  if (pNode && m_bRichText) {
    m_textParser.Reset();
  }
  m_pTextDataNode = pNode;
}
IFDE_XMLNode* CXFA_TextLayout::GetXMLContainerNode() {
  IFDE_XMLNode* pXMLContainer = NULL;
  if (m_bRichText) {
    IFDE_XMLNode* pXMLRoot = m_pTextDataNode->GetXMLMappingNode();
    if (!pXMLRoot) {
      return pXMLContainer;
    }
    for (IFDE_XMLNode* pXMLChild =
             pXMLRoot->GetNodeItem(IFDE_XMLNode::FirstChild);
         pXMLChild;
         pXMLChild = pXMLChild->GetNodeItem(IFDE_XMLNode::NextSibling)) {
      if (pXMLChild->GetType() == FDE_XMLNODE_Element) {
        IFDE_XMLElement* pXMLElement = (IFDE_XMLElement*)pXMLChild;
        CFX_WideString wsTag;
        pXMLElement->GetLocalTagName(wsTag);
        if (wsTag.Equal(FX_WSTRC(L"body")) || wsTag.Equal(FX_WSTRC(L"html"))) {
          pXMLContainer = pXMLChild;
          break;
        }
      }
    }
  }
  return pXMLContainer;
}
IFX_RTFBreak* CXFA_TextLayout::CreateBreak(FX_BOOL bDefault) {
  FX_DWORD dwStyle = FX_RTFLAYOUTSTYLE_ExpandTab;
  if (!bDefault) {
    dwStyle |= FX_RTFLAYOUTSTYLE_Pagination;
  }
  IFX_RTFBreak* pBreak = IFX_RTFBreak::Create(0);
  pBreak->SetLayoutStyles(dwStyle);
  pBreak->SetLineBreakChar(L'\n');
  pBreak->SetLineBreakTolerance(1);
  pBreak->SetFont(m_textParser.GetFont(m_pTextProvider, NULL));
  pBreak->SetFontSize(m_textParser.GetFontSize(m_pTextProvider, NULL));
  return pBreak;
}
void CXFA_TextLayout::InitBreak(FX_FLOAT fLineWidth) {
  CXFA_Font font = m_pTextProvider->GetFontNode();
  CXFA_Para para = m_pTextProvider->GetParaNode();
  FX_FLOAT fStart = 0;
  FX_FLOAT fStartPos = 0;
  if (para.IsExistInXML()) {
    int32_t iAlign = FX_RTFLINEALIGNMENT_Left;
    switch (para.GetHorizontalAlign()) {
      case XFA_ATTRIBUTEENUM_Center:
        iAlign = FX_RTFLINEALIGNMENT_Center;
        break;
      case XFA_ATTRIBUTEENUM_Right:
        iAlign = FX_RTFLINEALIGNMENT_Right;
        break;
      case XFA_ATTRIBUTEENUM_Justify:
        iAlign = FX_RTFLINEALIGNMENT_Justified;
        break;
      case XFA_ATTRIBUTEENUM_JustifyAll:
        iAlign = FX_RTFLINEALIGNMENT_Distributed;
        break;
    }
    m_pBreak->SetAlignment(iAlign);
    fStart = para.GetMarginLeft();
    if (m_pTextProvider->IsCheckButtonAndAutoWidth()) {
      if (iAlign != FX_RTFLINEALIGNMENT_Left) {
        fLineWidth -= para.GetMarginRight();
      }
    } else {
      fLineWidth -= para.GetMarginRight();
    }
    if (fLineWidth < 0) {
      fLineWidth = fStart;
    }
    fStartPos = fStart;
    FX_FLOAT fIndent = para.GetTextIndent();
    if (fIndent > 0) {
      fStartPos += fIndent;
    }
  }
  m_pBreak->SetLineBoundary(fStart, fLineWidth);
  m_pBreak->SetLineStartPos(fStartPos);
  if (font.IsExistInXML()) {
    m_pBreak->SetHorizontalScale((int32_t)font.GetHorizontalScale());
    m_pBreak->SetVerticalScale((int32_t)font.GetVerticalScale());
    m_pBreak->SetCharSpace(font.GetLetterSpacing());
  }
  FX_FLOAT fFontSize = m_textParser.GetFontSize(m_pTextProvider, NULL);
  m_pBreak->SetFontSize(fFontSize);
  m_pBreak->SetFont(m_textParser.GetFont(m_pTextProvider, NULL));
  m_pBreak->SetLineBreakTolerance(fFontSize * 0.2f);
}
void CXFA_TextLayout::InitBreak(IFDE_CSSComputedStyle* pStyle,
                                FDE_CSSDISPLAY eDisplay,
                                FX_FLOAT fLineWidth,
                                IFDE_XMLNode* pXMLNode,
                                IFDE_CSSComputedStyle* pParentStyle) {
  if (pStyle == NULL) {
    InitBreak(fLineWidth);
    return;
  }
  IFDE_CSSParagraphStyle* pParaStyle = pStyle->GetParagraphStyles();
  if (eDisplay == FDE_CSSDISPLAY_Block || eDisplay == FDE_CSSDISPLAY_ListItem) {
    int32_t iAlign = FX_RTFLINEALIGNMENT_Left;
    switch (pParaStyle->GetTextAlign()) {
      case FDE_CSSTEXTALIGN_Right:
        iAlign = FX_RTFLINEALIGNMENT_Right;
        break;
      case FDE_CSSTEXTALIGN_Center:
        iAlign = FX_RTFLINEALIGNMENT_Center;
        break;
      case FDE_CSSTEXTALIGN_Justify:
        iAlign = FX_RTFLINEALIGNMENT_Justified;
        break;
      case FDE_CSSTEXTALIGN_JustifyAll:
        iAlign = FX_RTFLINEALIGNMENT_Distributed;
        break;
      default:
        break;
    }
    m_pBreak->SetAlignment(iAlign);
    FX_FLOAT fStart = 0;
    const FDE_CSSRECT* pRect = pStyle->GetBoundaryStyles()->GetMarginWidth();
    const FDE_CSSRECT* pPaddingRect =
        pStyle->GetBoundaryStyles()->GetPaddingWidth();
    if (pRect != NULL) {
      fStart = pRect->left.GetValue();
      fLineWidth -= pRect->right.GetValue();
      if (pPaddingRect != NULL) {
        fStart += pPaddingRect->left.GetValue();
        fLineWidth -= pPaddingRect->right.GetValue();
      }
      if (eDisplay == FDE_CSSDISPLAY_ListItem) {
        const FDE_CSSRECT* pParRect =
            pParentStyle->GetBoundaryStyles()->GetMarginWidth();
        const FDE_CSSRECT* pParPaddingRect =
            pParentStyle->GetBoundaryStyles()->GetPaddingWidth();
        if (pParRect != NULL) {
          fStart += pParRect->left.GetValue();
          fLineWidth -= pParRect->right.GetValue();
          if (pParPaddingRect != NULL) {
            fStart += pParPaddingRect->left.GetValue();
            fLineWidth -= pParPaddingRect->right.GetValue();
          }
        }
        FDE_CSSRECT pNewRect;
        pNewRect.left.Set(FDE_CSSLENGTHUNIT_Point, fStart);
        pNewRect.right.Set(FDE_CSSLENGTHUNIT_Point, pRect->right.GetValue());
        pNewRect.top.Set(FDE_CSSLENGTHUNIT_Point, pRect->top.GetValue());
        pNewRect.bottom.Set(FDE_CSSLENGTHUNIT_Point, pRect->bottom.GetValue());
        pStyle->GetBoundaryStyles()->SetMarginWidth(pNewRect);
      }
    }
    m_pBreak->SetLineBoundary(fStart, fLineWidth);
    FX_FLOAT fIndent = pParaStyle->GetTextIndent().GetValue();
    if (fIndent > 0) {
      fStart += fIndent;
    }
    m_pBreak->SetLineStartPos(fStart);
    m_pBreak->SetTabWidth(m_textParser.GetTabInterval(pStyle));
    if (m_pTabstopContext == NULL) {
      m_pTabstopContext = new CXFA_TextTabstopsContext;
    }
    m_textParser.GetTabstops(pStyle, m_pTabstopContext);
    for (int32_t i = 0; i < m_pTabstopContext->m_iTabCount; i++) {
      XFA_TABSTOPS* pTab = m_pTabstopContext->m_tabstops.GetDataPtr(i);
      m_pBreak->AddPositionedTab(pTab->fTabstops);
    }
  }
  FX_FLOAT fFontSize = m_textParser.GetFontSize(m_pTextProvider, pStyle);
  m_pBreak->SetFontSize(fFontSize);
  m_pBreak->SetLineBreakTolerance(fFontSize * 0.2f);
  m_pBreak->SetFont(m_textParser.GetFont(m_pTextProvider, pStyle));
  m_pBreak->SetHorizontalScale(
      m_textParser.GetHorScale(m_pTextProvider, pStyle, pXMLNode));
  m_pBreak->SetVerticalScale(m_textParser.GetVerScale(m_pTextProvider, pStyle));
  m_pBreak->SetCharSpace(pParaStyle->GetLetterSpacing().GetValue());
}
int32_t CXFA_TextLayout::GetText(CFX_WideString& wsText) {
  GetTextDataNode();
  wsText.Empty();
  if (m_bRichText) {
  } else {
    wsText = m_pTextDataNode->GetContent();
  }
  return wsText.GetLength();
}
FX_FLOAT CXFA_TextLayout::GetLayoutHeight() {
  if (m_pLoader == NULL) {
    return 0;
  }
  int32_t iCount = m_pLoader->m_lineHeights.GetSize();
  if (iCount == 0 && m_pLoader->m_fWidth > 0) {
    CFX_SizeF szMax, szDef;
    szMax.Set(m_pLoader->m_fWidth, m_pLoader->m_fHeight);
    szDef.Set(0, 0);
    m_pLoader->m_bSaveLineHeight = TRUE;
    m_pLoader->m_fLastPos = 0;
    CalcSize(szMax, szMax, szDef);
    m_pLoader->m_bSaveLineHeight = FALSE;
    return szDef.y;
  }
  FX_FLOAT fHeight = m_pLoader->m_fHeight;
  if (fHeight < 0.1f) {
    fHeight = 0;
    for (int32_t i = 0; i < iCount; i++) {
      fHeight += m_pLoader->m_lineHeights.ElementAt(i);
    }
  }
  return fHeight;
}
FX_FLOAT CXFA_TextLayout::StartLayout(FX_FLOAT fWidth) {
  if (m_pLoader == NULL) {
    m_pLoader = new CXFA_LoaderContext;
  }
  if (fWidth < 0 || (m_pLoader->m_fWidth > -1 &&
                     FXSYS_fabs(fWidth - m_pLoader->m_fWidth) > 0)) {
    m_pLoader->m_lineHeights.RemoveAll();
    m_Blocks.RemoveAll();
    Unload();
    m_pLoader->m_fStartLineOffset = 0;
  }
  m_pLoader->m_fWidth = fWidth;
  if (fWidth < 0) {
    CFX_SizeF szMax, szDef;
    szMax.Set(0, 0);
    szDef.Set(0, 0);
    m_pLoader->m_bSaveLineHeight = TRUE;
    m_pLoader->m_fLastPos = 0;
    CalcSize(szMax, szMax, szDef);
    m_pLoader->m_bSaveLineHeight = FALSE;
    fWidth = szDef.x;
  }
  return fWidth;
}
FX_BOOL CXFA_TextLayout::DoLayout(int32_t iBlockIndex,
                                  FX_FLOAT& fCalcHeight,
                                  FX_FLOAT fContentAreaHeight,
                                  FX_FLOAT fTextHeight) {
  if (m_pLoader == NULL) {
    return FALSE;
  }
  int32_t iBlockCount = m_Blocks.GetSize();
  FX_FLOAT fHeight = fTextHeight;
  if (fHeight < 0) {
    fHeight = GetLayoutHeight();
  }
  m_pLoader->m_fHeight = fHeight;
  if (fContentAreaHeight < 0) {
    return FALSE;
  }
  m_bHasBlock = TRUE;
  if (iBlockCount == 0 && fHeight > 0) {
    fHeight = fTextHeight - GetLayoutHeight();
    if (fHeight > 0) {
      int32_t iAlign = m_textParser.GetVAlgin(m_pTextProvider);
      if (iAlign == XFA_ATTRIBUTEENUM_Middle) {
        fHeight /= 2.0f;
      } else if (iAlign != XFA_ATTRIBUTEENUM_Bottom) {
        fHeight = 0;
      }
      m_pLoader->m_fStartLineOffset = fHeight;
    }
  }
  FX_FLOAT fLinePos = m_pLoader->m_fStartLineOffset;
  int32_t iLineIndex = 0;
  if (iBlockCount > 1) {
    if (iBlockCount >= (iBlockIndex + 1) * 2) {
      iLineIndex = m_Blocks.ElementAt(iBlockIndex * 2);
    } else {
      iLineIndex = m_Blocks.ElementAt(iBlockCount - 1) +
                   m_Blocks.ElementAt(iBlockCount - 2);
    }
    if (m_pLoader->m_BlocksHeight.GetSize() > 0) {
      for (int32_t i = 0; i < iBlockIndex; i++) {
        fLinePos -= m_pLoader->m_BlocksHeight.ElementAt(i * 2 + 1);
      }
    }
  }
  int32_t iCount = m_pLoader->m_lineHeights.GetSize();
  int32_t i = 0;
  for (i = iLineIndex; i < iCount; i++) {
    FX_FLOAT fLineHeight = m_pLoader->m_lineHeights.ElementAt(i);
    if ((i == iLineIndex) && (fLineHeight - fContentAreaHeight > 0.001)) {
      fCalcHeight = 0;
      return TRUE;
    }
    if (fLinePos + fLineHeight - fContentAreaHeight > 0.001) {
      if (iBlockCount >= (iBlockIndex + 1) * 2) {
        m_Blocks.SetAt(iBlockIndex * 2, iLineIndex);
        m_Blocks.SetAt(iBlockIndex * 2 + 1, i - iLineIndex);
      } else {
        m_Blocks.Add(iLineIndex);
        m_Blocks.Add(i - iLineIndex);
      }
      if (i == iLineIndex) {
        if (fCalcHeight <= fLinePos) {
          if (m_pLoader->m_BlocksHeight.GetSize() > iBlockIndex * 2 &&
              (m_pLoader->m_BlocksHeight.GetAt(iBlockIndex * 2) ==
               iBlockIndex)) {
            m_pLoader->m_BlocksHeight.SetAt(iBlockIndex * 2 + 1, fCalcHeight);
          } else {
            m_pLoader->m_BlocksHeight.Add((FX_FLOAT)iBlockIndex);
            m_pLoader->m_BlocksHeight.Add(fCalcHeight);
          }
        }
        return TRUE;
      }
      fCalcHeight = fLinePos;
      return TRUE;
    }
    fLinePos += fLineHeight;
  }
  return FALSE;
}
int32_t CXFA_TextLayout::CountBlocks() const {
  int32_t iCount = m_Blocks.GetSize() / 2;
  return iCount > 0 ? iCount : 1;
}
FX_BOOL CXFA_TextLayout::CalcSize(const CFX_SizeF& minSize,
                                  const CFX_SizeF& maxSize,
                                  CFX_SizeF& defaultSize) {
  defaultSize.x = maxSize.x;
  if (defaultSize.x < 1) {
    defaultSize.x = 0xFFFF;
  }
  if (m_pBreak != NULL) {
    m_pBreak->Release();
  }
  m_pBreak = CreateBreak(FALSE);
  FX_FLOAT fLinePos = 0;
  m_iLines = 0;
  m_fMaxWidth = 0;
  Loader(defaultSize, fLinePos, FALSE);
  if (fLinePos < 0.1f) {
    fLinePos = m_textParser.GetFontSize(m_pTextProvider, NULL);
  }
  if (m_pTabstopContext) {
    delete m_pTabstopContext;
    m_pTabstopContext = NULL;
  }
  defaultSize.Set(m_fMaxWidth, fLinePos);
  return TRUE;
}
FX_BOOL CXFA_TextLayout::Layout(const CFX_SizeF& size, FX_FLOAT* fHeight) {
  if (size.x < 1) {
    return FALSE;
  }
  Unload();
  m_pBreak = CreateBreak(TRUE);
  if (m_pLoader != NULL) {
    m_pLoader->m_iTotalLines = -1;
    m_pLoader->m_iChar = 0;
  }
  m_iLines = 0;
  FX_FLOAT fLinePos = 0;
  Loader(size, fLinePos, TRUE);
  UpdateAlign(size.y, fLinePos);
  if (m_pTabstopContext) {
    delete m_pTabstopContext;
    m_pTabstopContext = NULL;
  }
  if (fHeight) {
    *fHeight = fLinePos;
  }
  return TRUE;
}
FX_BOOL CXFA_TextLayout::Layout(int32_t iBlock) {
  if (m_pLoader == NULL || iBlock < 0 || iBlock >= CountBlocks()) {
    return FALSE;
  }
  if (m_pLoader->m_fWidth < 1) {
    return FALSE;
  }
  m_pLoader->m_iTotalLines = -1;
  m_iLines = 0;
  FX_FLOAT fLinePos = 0;
  CXFA_Node* pNode = NULL;
  CFX_SizeF szText;
  szText.Set(m_pLoader->m_fWidth, m_pLoader->m_fHeight);
  int32_t iCount = m_Blocks.GetSize();
  int32_t iBlocksHeightCount = m_pLoader->m_BlocksHeight.GetSize();
  iBlocksHeightCount /= 2;
  if (iBlock < iBlocksHeightCount) {
    return TRUE;
  }
  if (iBlock == iBlocksHeightCount) {
    Unload();
    m_pBreak = CreateBreak(TRUE);
    fLinePos = m_pLoader->m_fStartLineOffset;
    for (int32_t i = 0; i < iBlocksHeightCount; i++) {
      fLinePos -= m_pLoader->m_BlocksHeight.ElementAt(i * 2 + 1);
    }
    m_pLoader->m_iChar = 0;
    if (iCount > 1) {
      m_pLoader->m_iTotalLines = m_Blocks.ElementAt(iBlock * 2 + 1);
    }
    Loader(szText, fLinePos, TRUE);
    if (iCount == 0 && m_pLoader->m_fStartLineOffset < 0.1f) {
      UpdateAlign(szText.y, fLinePos);
    }
  } else if (m_pTextDataNode != NULL) {
    iBlock *= 2;
    if (iBlock < iCount - 2) {
      m_pLoader->m_iTotalLines = m_Blocks.ElementAt(iBlock + 1);
    }
    m_pBreak->Reset();
    if (m_bRichText) {
      IFDE_XMLNode* pContainerNode = GetXMLContainerNode();
      if (!pContainerNode) {
        return TRUE;
      }
      IFDE_XMLNode* pXMLNode = m_pLoader->m_pXMLNode;
      if (pXMLNode == NULL) {
        return TRUE;
      }
      IFDE_XMLNode* pSaveXMLNode = m_pLoader->m_pXMLNode;
      for (; pXMLNode;
           pXMLNode = pXMLNode->GetNodeItem(IFDE_XMLNode::NextSibling)) {
        FX_BOOL bFlag = LoadRichText(pXMLNode, szText, fLinePos,
                                     m_pLoader->m_pParentStyle, TRUE);
        if (!bFlag) {
          break;
        }
      }
      while (pXMLNode == NULL) {
        pXMLNode = pSaveXMLNode->GetNodeItem(IFDE_XMLNode::Parent);
        if (pXMLNode == pContainerNode) {
          break;
        }
        FX_BOOL bFlag =
            LoadRichText(pXMLNode, szText, fLinePos, m_pLoader->m_pParentStyle,
                         TRUE, NULL, FALSE);
        if (!bFlag) {
          break;
        }
        pSaveXMLNode = pXMLNode;
        pXMLNode = pXMLNode->GetNodeItem(IFDE_XMLNode::NextSibling);
        if (!pXMLNode) {
          continue;
        }
        for (; pXMLNode;
             pXMLNode = pXMLNode->GetNodeItem(IFDE_XMLNode::NextSibling)) {
          FX_BOOL bFlag = LoadRichText(pXMLNode, szText, fLinePos,
                                       m_pLoader->m_pParentStyle, TRUE);
          if (!bFlag) {
            break;
          }
        }
      }
    } else {
      pNode = m_pLoader->m_pNode;
      if (pNode == NULL) {
        return TRUE;
      }
      LoadText(pNode, szText, fLinePos, TRUE);
    }
  }
  if (iBlock == iCount) {
    if (m_pTabstopContext != NULL) {
      delete m_pTabstopContext;
      m_pTabstopContext = NULL;
    }
    if (m_pLoader != NULL) {
      delete m_pLoader;
      m_pLoader = NULL;
    }
  }
  return TRUE;
}
void CXFA_TextLayout::ItemBlocks(const CFX_RectF& rtText, int32_t iBlockIndex) {
  if (!m_pLoader) {
    return;
  }
  int32_t iCountHeight = m_pLoader->m_lineHeights.GetSize();
  if (iCountHeight == 0) {
    return;
  }
  FX_BOOL bEndItem = TRUE;
  int32_t iBlockCount = m_Blocks.GetSize();
  FX_FLOAT fLinePos = m_pLoader->m_fStartLineOffset;
  int32_t iLineIndex = 0;
  if (iBlockIndex > 0) {
    int32_t iBlockHeightCount = m_pLoader->m_BlocksHeight.GetSize();
    iBlockHeightCount /= 2;
    if (iBlockHeightCount >= iBlockIndex) {
      for (int32_t i = 0; i < iBlockIndex; i++) {
        fLinePos -= m_pLoader->m_BlocksHeight.ElementAt(i * 2 + 1);
      }
    } else {
      fLinePos = 0;
    }
    iLineIndex = m_Blocks.ElementAt(iBlockCount - 1) +
                 m_Blocks.ElementAt(iBlockCount - 2);
  }
  int32_t i = 0;
  for (i = iLineIndex; i < iCountHeight; i++) {
    FX_FLOAT fLineHeight = m_pLoader->m_lineHeights.ElementAt(i);
    if (fLinePos + fLineHeight - rtText.height > 0.001) {
      m_Blocks.Add(iLineIndex);
      m_Blocks.Add(i - iLineIndex);
      bEndItem = FALSE;
      break;
    }
    fLinePos += fLineHeight;
  }
  if (iCountHeight > 0 && (i - iLineIndex) > 0 && bEndItem) {
    m_Blocks.Add(iLineIndex);
    m_Blocks.Add(i - iLineIndex);
  }
}
FX_BOOL CXFA_TextLayout::DrawString(CFX_RenderDevice* pFxDevice,
                                    const CFX_Matrix& tmDoc2Device,
                                    const CFX_RectF& rtClip,
                                    int32_t iBlock) {
  IFDE_RenderDevice* pDevice = IFDE_RenderDevice::Create(pFxDevice);
  if (pDevice == NULL) {
    return FALSE;
  }
  FDE_HDEVICESTATE state = pDevice->SaveState();
  pDevice->SetClipRect(rtClip);
  IFDE_SolidBrush* pSolidBrush =
      (IFDE_SolidBrush*)IFDE_Brush::Create(FDE_BRUSHTYPE_Solid);
  IFDE_Pen* pPen = IFDE_Pen::Create();
  FXSYS_assert(pDevice != NULL && pSolidBrush != NULL && pPen != NULL);
  if (m_pieceLines.GetSize() == 0) {
    int32_t iBlockCount = CountBlocks();
    for (int32_t i = 0; i < iBlockCount; i++) {
      Layout(i);
    }
  }
  FXTEXT_CHARPOS* pCharPos = NULL;
  int32_t iCharCount = 0;
  int32_t iLineStart = 0;
  int32_t iPieceLines = m_pieceLines.GetSize();
  int32_t iCount = m_Blocks.GetSize();
  if (iCount > 0) {
    iBlock *= 2;
    if (iBlock < iCount) {
      iLineStart = m_Blocks.ElementAt(iBlock);
      iPieceLines = m_Blocks.ElementAt(iBlock + 1);
    } else {
      iPieceLines = 0;
    }
  }
  for (int32_t i = 0; i < iPieceLines; i++) {
    if (i + iLineStart >= m_pieceLines.GetSize()) {
      break;
    }
    CXFA_PieceLine* pPieceLine = m_pieceLines.GetAt(i + iLineStart);
    int32_t iPieces = pPieceLine->m_textPieces.GetSize();
    int32_t j = 0;
    for (j = 0; j < iPieces; j++) {
      XFA_LPCTEXTPIECE pPiece = pPieceLine->m_textPieces.GetAt(j);
      int32_t iChars = pPiece->iChars;
      if (iCharCount < iChars) {
        FX_Free(pCharPos);
        pCharPos = FX_Alloc(FXTEXT_CHARPOS, iChars);
        iCharCount = iChars;
      }
      FXSYS_memset(pCharPos, 0, iCharCount * sizeof(FXTEXT_CHARPOS));
      RenderString(pDevice, pSolidBrush, pPieceLine, j, pCharPos, tmDoc2Device);
    }
    for (j = 0; j < iPieces; j++) {
      RenderPath(pDevice, pPen, pPieceLine, j, pCharPos, tmDoc2Device);
    }
  }
  pDevice->RestoreState(state);
  FX_Free(pCharPos);
  pSolidBrush->Release();
  pPen->Release();
  pDevice->Release();
  return iPieceLines;
}
void CXFA_TextLayout::UpdateAlign(FX_FLOAT fHeight, FX_FLOAT fBottom) {
  fHeight -= fBottom;
  if (fHeight < 0.1f) {
    return;
  }
  switch (m_textParser.GetVAlgin(m_pTextProvider)) {
    case XFA_ATTRIBUTEENUM_Middle:
      fHeight /= 2.0f;
      break;
    case XFA_ATTRIBUTEENUM_Bottom:
      break;
    default:
      return;
  }
  int32_t iCount = m_pieceLines.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CXFA_PieceLine* pPieceLine = m_pieceLines.GetAt(i);
    int32_t iPieces = pPieceLine->m_textPieces.GetSize();
    for (int32_t j = 0; j < iPieces; j++) {
      XFA_LPTEXTPIECE pPiece = pPieceLine->m_textPieces.GetAt(j);
      CFX_RectF& rect = pPiece->rtPiece;
      rect.top += fHeight;
    }
  }
}
FX_BOOL CXFA_TextLayout::Loader(const CFX_SizeF& szText,
                                FX_FLOAT& fLinePos,
                                FX_BOOL bSavePieces) {
  if (m_pAllocator == NULL) {
    m_pAllocator = FX_CreateAllocator(FX_ALLOCTYPE_Static, 256, 0);
  }
  GetTextDataNode();
  if (m_pTextDataNode == NULL) {
    return TRUE;
  }
  if (m_bRichText) {
    IFDE_XMLNode* pXMLContainer = GetXMLContainerNode();
    if (pXMLContainer) {
      if (!m_textParser.IsParsed()) {
        m_textParser.DoParse(pXMLContainer, m_pTextProvider);
      }
      IFDE_CSSComputedStyle* pRootStyle =
          m_textParser.CreateRootStyle(m_pTextProvider);
      LoadRichText(pXMLContainer, szText, fLinePos, pRootStyle, bSavePieces);
      pRootStyle->Release();
    }
  } else {
    LoadText(m_pTextDataNode, szText, fLinePos, bSavePieces);
  }
  return TRUE;
}
void CXFA_TextLayout::LoadText(CXFA_Node* pNode,
                               const CFX_SizeF& szText,
                               FX_FLOAT& fLinePos,
                               FX_BOOL bSavePieces) {
  InitBreak(szText.x);
  CXFA_Para para = m_pTextProvider->GetParaNode();
  FX_FLOAT fSpaceAbove = 0;
  if (para.IsExistInXML()) {
    fSpaceAbove = para.GetSpaceAbove();
    if (fSpaceAbove < 0.1f) {
      fSpaceAbove = 0;
    }
    int32_t verAlign = para.GetVerticalAlign();
    switch (verAlign) {
      case XFA_ATTRIBUTEENUM_Top:
      case XFA_ATTRIBUTEENUM_Middle:
      case XFA_ATTRIBUTEENUM_Bottom: {
        fLinePos += fSpaceAbove;
        break;
      }
    }
  }
  CFX_WideString wsText = pNode->GetContent();
  wsText.TrimRight(L" ");
  FX_BOOL bRet = AppendChar(wsText, fLinePos, fSpaceAbove, bSavePieces);
  if (bRet && m_pLoader != NULL) {
    m_pLoader->m_pNode = pNode;
  } else {
    EndBreak(FX_RTFBREAK_ParagraphBreak, fLinePos, bSavePieces);
  }
}
FX_BOOL CXFA_TextLayout::LoadRichText(IFDE_XMLNode* pXMLNode,
                                      const CFX_SizeF& szText,
                                      FX_FLOAT& fLinePos,
                                      IFDE_CSSComputedStyle* pParentStyle,
                                      FX_BOOL bSavePieces,
                                      CXFA_LinkUserData* pLinkData,
                                      FX_BOOL bEndBreak,
                                      FX_BOOL bIsOl,
                                      int32_t iLiCount) {
  if (pXMLNode == NULL) {
    return FALSE;
  }
  CXFA_TextParseContext* pContext =
      m_textParser.GetParseContextFromMap(pXMLNode);
  FDE_CSSDISPLAY eDisplay = FDE_CSSDISPLAY_None;
  FX_BOOL bContentNode = FALSE;
  FX_FLOAT fSpaceBelow = 0;
  IFDE_CSSComputedStyle* pStyle = NULL;
  CFX_WideString wsName;
  if (bEndBreak) {
    FX_BOOL bCurOl = FALSE;
    FX_BOOL bCurLi = FALSE;
    IFDE_XMLElement* pElement = NULL;
    if (pContext != NULL) {
      if (m_bBlockContinue ||
          (m_pLoader && pXMLNode == m_pLoader->m_pXMLNode)) {
        m_bBlockContinue = TRUE;
      }
      if (pXMLNode->GetType() == FDE_XMLNODE_Text) {
        bContentNode = TRUE;
      } else if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
        pElement = (IFDE_XMLElement*)pXMLNode;
        pElement->GetLocalTagName(wsName);
      }
      if (wsName == FX_WSTRC(L"ol")) {
        bIsOl = TRUE;
        bCurOl = TRUE;
      }
      if (m_bBlockContinue || bContentNode == FALSE) {
        eDisplay = pContext->GetDisplay();
        if (eDisplay != FDE_CSSDISPLAY_Block &&
            eDisplay != FDE_CSSDISPLAY_Inline &&
            eDisplay != FDE_CSSDISPLAY_ListItem) {
          return TRUE;
        }
        pStyle = m_textParser.ComputeStyle(pXMLNode, pParentStyle);
        InitBreak(bContentNode ? pParentStyle : pStyle, eDisplay, szText.x,
                  pXMLNode, pParentStyle);
        if ((eDisplay == FDE_CSSDISPLAY_Block ||
             eDisplay == FDE_CSSDISPLAY_ListItem) &&
            (pStyle != NULL) &&
            (wsName.IsEmpty() ||
             (wsName != FX_WSTRC(L"body") && wsName != FX_WSTRC(L"html") &&
              wsName != FX_WSTRC(L"ol") && wsName != FX_WSTRC(L"ul")))) {
          const FDE_CSSRECT* pRect =
              pStyle->GetBoundaryStyles()->GetMarginWidth();
          if (pRect) {
            fLinePos += pRect->top.GetValue();
            fSpaceBelow = pRect->bottom.GetValue();
          }
        }
        if (wsName == FX_WSTRC(L"a")) {
          CFX_WideString wsLinkContent;
          FXSYS_assert(pElement);
          pElement->GetString(FX_WSTRC(L"href").GetPtr(), wsLinkContent);
          if (!wsLinkContent.IsEmpty()) {
            pLinkData = FDE_NewWith(m_pAllocator) CXFA_LinkUserData(
                m_pAllocator,
                wsLinkContent.GetBuffer(wsLinkContent.GetLength()));
            wsLinkContent.ReleaseBuffer(wsLinkContent.GetLength());
          }
        }
        int32_t iTabCount =
            m_textParser.CountTabs(bContentNode ? pParentStyle : pStyle);
        FX_BOOL bSpaceRun =
            m_textParser.IsSpaceRun(bContentNode ? pParentStyle : pStyle);
        CFX_WideString wsText;
        if (bContentNode && iTabCount == 0) {
          ((IFDE_XMLText*)pXMLNode)->GetText(wsText);
        } else if (wsName == FX_WSTRC(L"br")) {
          wsText = L'\n';
        } else if (wsName == FX_WSTRC(L"li")) {
          bCurLi = TRUE;
          if (bIsOl) {
            wsText.Format(L"%d.  ", iLiCount);
          } else {
            wsText = 0x00B7 + FX_WSTRC(L"  ");
          }
        } else if (!bContentNode) {
          if (iTabCount > 0)
            while (iTabCount-- > 0) {
              wsText += L'\t';
            }
          else {
            m_textParser.GetEmbbedObj(m_pTextProvider, pXMLNode, wsText);
          }
        }
        int32_t iLength = wsText.GetLength();
        if (iLength > 0 && bContentNode && !bSpaceRun) {
          ProcessText(wsText);
        }
        if (m_pLoader) {
          if (wsText.GetLength() > 0 &&
              (m_pLoader->m_dwFlags & XFA_LOADERCNTXTFLG_FILTERSPACE)) {
            wsText.TrimLeft(0x20);
          }
          if (FDE_CSSDISPLAY_Block == eDisplay) {
            m_pLoader->m_dwFlags |= XFA_LOADERCNTXTFLG_FILTERSPACE;
          } else if (FDE_CSSDISPLAY_Inline == eDisplay &&
                     (m_pLoader->m_dwFlags & XFA_LOADERCNTXTFLG_FILTERSPACE)) {
            m_pLoader->m_dwFlags &= ~XFA_LOADERCNTXTFLG_FILTERSPACE;
          } else if (wsText.GetLength() > 0 &&
                     (0x20 == wsText.GetAt(wsText.GetLength() - 1))) {
            m_pLoader->m_dwFlags |= XFA_LOADERCNTXTFLG_FILTERSPACE;
          } else if (wsText.GetLength() == 0)
            ;
          else {
            m_pLoader->m_dwFlags &= ~XFA_LOADERCNTXTFLG_FILTERSPACE;
          }
        }
        if (wsText.GetLength() > 0) {
          if (m_pLoader == NULL || m_pLoader->m_iChar == 0) {
            if (pLinkData) {
              pLinkData->AddRef();
            }
            CXFA_TextUserData* pUserData = FDE_NewWith(m_pAllocator)
                CXFA_TextUserData(m_pAllocator,
                                  bContentNode ? pParentStyle : pStyle,
                                  pLinkData);
            m_pBreak->SetUserData(pUserData);
          }
          if (AppendChar(wsText, fLinePos, 0, bSavePieces)) {
            if (m_pLoader) {
              m_pLoader->m_dwFlags &= ~XFA_LOADERCNTXTFLG_FILTERSPACE;
            }
            if (IsEnd(bSavePieces)) {
              if (m_pLoader && m_pLoader->m_iTotalLines > -1) {
                m_pLoader->m_pXMLNode = pXMLNode;
                m_pLoader->m_pParentStyle = pParentStyle;
              }
              if (pStyle != NULL) {
                pStyle->Release();
              }
              return FALSE;
            }
            return TRUE;
          }
        }
      }
    }
    FX_BOOL ret = TRUE;
    for (IFDE_XMLNode* pChildNode =
             pXMLNode->GetNodeItem(IFDE_XMLNode::FirstChild);
         pChildNode;
         pChildNode = pChildNode->GetNodeItem(IFDE_XMLNode::NextSibling)) {
      if (bCurOl) {
        iLiCount++;
      }
      ret = LoadRichText(pChildNode, szText, fLinePos,
                         pContext ? pStyle : pParentStyle, bSavePieces,
                         pLinkData, TRUE, bIsOl, iLiCount);
      if (ret == FALSE) {
        return FALSE;
      }
    }
    if (m_pLoader) {
      if (FDE_CSSDISPLAY_Block == eDisplay) {
        m_pLoader->m_dwFlags |= XFA_LOADERCNTXTFLG_FILTERSPACE;
      }
    }
    if (bCurLi) {
      EndBreak(FX_RTFBREAK_LineBreak, fLinePos, bSavePieces);
    }
  } else {
    if (pContext != NULL) {
      eDisplay = pContext->GetDisplay();
    }
  }
  if (m_bBlockContinue) {
    if (pContext != NULL && !bContentNode) {
      FX_DWORD dwStatus = (eDisplay == FDE_CSSDISPLAY_Block)
                              ? FX_RTFBREAK_ParagraphBreak
                              : FX_RTFBREAK_PieceBreak;
      EndBreak(dwStatus, fLinePos, bSavePieces);
      if (eDisplay == FDE_CSSDISPLAY_Block) {
        fLinePos += fSpaceBelow;
        if (m_pTabstopContext) {
          m_pTabstopContext->RemoveAll();
        }
      }
      if (wsName == FX_WSTRC(L"a")) {
        if (pLinkData) {
          pLinkData->Release();
          pLinkData = nullptr;
        }
      }
      if (IsEnd(bSavePieces)) {
        if (pStyle) {
          pStyle->Release();
        }
        if (m_pLoader && m_pLoader->m_iTotalLines > -1) {
          m_pLoader->m_pXMLNode =
              pXMLNode->GetNodeItem(IFDE_XMLNode::NextSibling);
          m_pLoader->m_pParentStyle = pParentStyle;
        }
        return FALSE;
      }
    }
  }
  if (pStyle != NULL) {
    pStyle->Release();
  }
  return TRUE;
}
FX_BOOL CXFA_TextLayout::AppendChar(const CFX_WideString& wsText,
                                    FX_FLOAT& fLinePos,
                                    FX_FLOAT fSpaceAbove,
                                    FX_BOOL bSavePieces) {
  FX_DWORD dwStatus = 0;
  int32_t iChar = 0;
  if (m_pLoader) {
    iChar = m_pLoader->m_iChar;
  }
  int32_t iLength = wsText.GetLength();
  for (int32_t i = iChar; i < iLength; i++) {
    FX_WCHAR wch = wsText.GetAt(i);
    if (wch == 0xA0) {
      wch = 0x20;
    }
    if ((dwStatus = m_pBreak->AppendChar(wch)) > FX_RTFBREAK_PieceBreak) {
      AppendTextLine(dwStatus, fLinePos, bSavePieces);
      if (IsEnd(bSavePieces)) {
        if (m_pLoader != NULL) {
          m_pLoader->m_iChar = i;
        }
        return TRUE;
      }
      if (dwStatus == FX_RTFBREAK_ParagraphBreak && m_bRichText) {
        fLinePos += fSpaceAbove;
      }
    }
  }
  if (m_pLoader) {
    m_pLoader->m_iChar = 0;
  }
  return FALSE;
}
FX_BOOL CXFA_TextLayout::IsEnd(FX_BOOL bSavePieces) {
  if (!bSavePieces) {
    return FALSE;
  }
  if (m_pLoader && m_pLoader->m_iTotalLines > 0) {
    return m_iLines >= m_pLoader->m_iTotalLines;
  }
  return FALSE;
}
void CXFA_TextLayout::ProcessText(CFX_WideString& wsText) {
  int32_t iLen = wsText.GetLength();
  if (iLen == 0) {
    return;
  }
  FX_WCHAR* psz = wsText.GetBuffer(iLen);
  int32_t iTrimLeft = 0;
  FX_WCHAR wch = 0, wPrev = 0;
  for (int32_t i = 0; i < iLen; i++) {
    wch = psz[i];
    if (wch < 0x20) {
      wch = 0x20;
    }
    if (wch == 0x20 && wPrev == 0x20) {
      continue;
    }
    wPrev = wch;
    psz[iTrimLeft++] = wch;
  }
  wsText.ReleaseBuffer(iLen);
  wsText = wsText.Left(iTrimLeft);
}
void CXFA_TextLayout::EndBreak(FX_DWORD dwStatus,
                               FX_FLOAT& fLinePos,
                               FX_BOOL bSavePieces) {
  dwStatus = m_pBreak->EndBreak(dwStatus);
  if (dwStatus > FX_RTFBREAK_PieceBreak) {
    AppendTextLine(dwStatus, fLinePos, bSavePieces, TRUE);
  }
}
void CXFA_TextLayout::DoTabstops(IFDE_CSSComputedStyle* pStyle,
                                 CXFA_PieceLine* pPieceLine) {
  if (m_pTabstopContext == NULL || m_pTabstopContext->m_iTabCount == 0) {
    return;
  }
  if (pStyle == NULL || pPieceLine == NULL) {
    return;
  }
  int32_t iPieces = pPieceLine->m_textPieces.GetSize();
  if (iPieces == 0) {
    return;
  }
  XFA_LPTEXTPIECE pPiece = pPieceLine->m_textPieces.GetAt(iPieces - 1);
  int32_t& iTabstopsIndex = m_pTabstopContext->m_iTabIndex;
  int32_t iCount = m_textParser.CountTabs(pStyle);
  if (iTabstopsIndex > m_pTabstopContext->m_iTabCount - 1) {
    return;
  }
  if (iCount > 0) {
    iTabstopsIndex++;
    m_pTabstopContext->m_bTabstops = TRUE;
    FX_FLOAT fRight = 0;
    if (iPieces > 1) {
      XFA_LPTEXTPIECE p = pPieceLine->m_textPieces.GetAt(iPieces - 2);
      fRight = p->rtPiece.right();
    }
    m_pTabstopContext->m_fTabWidth =
        pPiece->rtPiece.width + pPiece->rtPiece.left - fRight;
  } else if (iTabstopsIndex > -1) {
    FX_FLOAT fLeft = 0;
    if (m_pTabstopContext->m_bTabstops) {
      XFA_TABSTOPS* pTabstops =
          m_pTabstopContext->m_tabstops.GetDataPtr(iTabstopsIndex);
      FX_DWORD dwAlgin = pTabstops->dwAlign;
      if (dwAlgin == FX_HashCode_String_GetW(L"center", 6)) {
        fLeft = pPiece->rtPiece.width / 2.0f;
      } else if (dwAlgin == FX_HashCode_String_GetW(L"right", 5) ||
                 dwAlgin == FX_HashCode_String_GetW(L"before", 6)) {
        fLeft = pPiece->rtPiece.width;
      } else if (dwAlgin == FX_HashCode_String_GetW(L"decimal", 7)) {
        int32_t iChars = pPiece->iChars;
        for (int32_t i = 0; i < iChars; i++) {
          if (pPiece->pszText[i] == L'.') {
            break;
          }
          fLeft += pPiece->pWidths[i] / 20000.0f;
        }
      }
      m_pTabstopContext->m_fLeft =
          std::min(fLeft, m_pTabstopContext->m_fTabWidth);
      m_pTabstopContext->m_bTabstops = FALSE;
      m_pTabstopContext->m_fTabWidth = 0;
    }
    pPiece->rtPiece.left -= m_pTabstopContext->m_fLeft;
  }
}
void CXFA_TextLayout::AppendTextLine(FX_DWORD dwStatus,
                                     FX_FLOAT& fLinePos,
                                     FX_BOOL bSavePieces,
                                     FX_BOOL bEndBreak) {
  int32_t iPieces = m_pBreak->CountBreakPieces();
  if (iPieces < 1) {
    return;
  }
  IFDE_CSSComputedStyle* pStyle = NULL;
  if (bSavePieces) {
    CXFA_PieceLine* pPieceLine = FDE_NewWith(m_pAllocator) CXFA_PieceLine;
    m_pieceLines.Add(pPieceLine);
    if (m_pTabstopContext) {
      m_pTabstopContext->Reset();
    }
    FX_FLOAT fLineStep = 0, fBaseLine = 0;
    int32_t i = 0;
    for (i = 0; i < iPieces; i++) {
      const CFX_RTFPiece* pPiece = m_pBreak->GetBreakPiece(i);
      CXFA_TextUserData* pUserData = (CXFA_TextUserData*)pPiece->m_pUserData;
      if (pUserData != NULL) {
        pStyle = pUserData->m_pStyle;
      }
      FX_FLOAT fVerScale = pPiece->m_iVerticalScale / 100.0f;
      XFA_LPTEXTPIECE pTP =
          (XFA_LPTEXTPIECE)m_pAllocator->Alloc(sizeof(XFA_TEXTPIECE));
      pTP->pszText =
          (FX_WCHAR*)m_pAllocator->Alloc(pPiece->m_iChars * sizeof(FX_WCHAR));
      pTP->pWidths =
          (int32_t*)m_pAllocator->Alloc(pPiece->m_iChars * sizeof(int32_t));
      pTP->iChars = pPiece->m_iChars;
      pPiece->GetString(pTP->pszText);
      pPiece->GetWidths(pTP->pWidths);
      pTP->iBidiLevel = pPiece->m_iBidiLevel;
      pTP->iHorScale = pPiece->m_iHorizontalScale;
      pTP->iVerScale = pPiece->m_iVerticalScale;
      m_textParser.GetUnderline(m_pTextProvider, pStyle, pTP->iUnderline,
                                pTP->iPeriod);
      m_textParser.GetLinethrough(m_pTextProvider, pStyle, pTP->iLineThrough);
      pTP->dwColor = m_textParser.GetColor(m_pTextProvider, pStyle);
      pTP->pFont = m_textParser.GetFont(m_pTextProvider, pStyle);
      pTP->fFontSize = m_textParser.GetFontSize(m_pTextProvider, pStyle);
      pTP->rtPiece.left = pPiece->m_iStartPos / 20000.0f;
      pTP->rtPiece.width = pPiece->m_iWidth / 20000.0f;
      pTP->rtPiece.height = (FX_FLOAT)pPiece->m_iFontSize * fVerScale / 20.0f;
      FX_FLOAT fBaseLineTemp =
          m_textParser.GetBaseline(m_pTextProvider, pStyle);
      pTP->rtPiece.top = fBaseLineTemp;
      pPieceLine->m_textPieces.Add(pTP);
      FX_FLOAT fLineHeight = m_textParser.GetLineHeight(
          m_pTextProvider, pStyle, m_iLines == 0, fVerScale);
      if (fBaseLineTemp > 0) {
        FX_FLOAT fLineHeightTmp = fBaseLineTemp + pTP->rtPiece.height;
        if (fLineHeight < fLineHeightTmp) {
          fLineHeight = fLineHeightTmp;
        } else {
          fBaseLineTemp = 0;
        }
      } else if (fBaseLine < -fBaseLineTemp) {
        fBaseLine = -fBaseLineTemp;
      }
      fLineStep = std::max(fLineStep, fLineHeight);
      if (pUserData != NULL && pUserData->m_pLinkData != NULL) {
        pUserData->m_pLinkData->AddRef();
        pTP->pLinkData = pUserData->m_pLinkData;
      } else {
        pTP->pLinkData = NULL;
      }
      DoTabstops(pStyle, pPieceLine);
    }
    for (i = 0; i < iPieces; i++) {
      XFA_LPTEXTPIECE pTP = pPieceLine->m_textPieces.GetAt(i);
      FX_FLOAT& fTop = pTP->rtPiece.top;
      FX_FLOAT fBaseLineTemp = fTop;
      fTop = fLinePos + fLineStep - pTP->rtPiece.height - fBaseLineTemp;
      fTop = std::max(0.0f, fTop);
    }
    fLinePos += fLineStep + fBaseLine;
  } else {
    FX_FLOAT fLineStep = 0;
    FX_FLOAT fLineWidth = 0;
    for (int32_t i = 0; i < iPieces; i++) {
      const CFX_RTFPiece* pPiece = m_pBreak->GetBreakPiece(i);
      CXFA_TextUserData* pUserData = (CXFA_TextUserData*)pPiece->m_pUserData;
      if (pUserData != NULL) {
        pStyle = pUserData->m_pStyle;
      }
      FX_FLOAT fVerScale = pPiece->m_iVerticalScale / 100.0f;
      FX_FLOAT fBaseLine = m_textParser.GetBaseline(m_pTextProvider, pStyle);
      FX_FLOAT fLineHeight = m_textParser.GetLineHeight(
          m_pTextProvider, pStyle, m_iLines == 0, fVerScale);
      if (fBaseLine > 0) {
        FX_FLOAT fLineHeightTmp =
            fBaseLine + (FX_FLOAT)pPiece->m_iFontSize * fVerScale / 20.0f;
        if (fLineHeight < fLineHeightTmp) {
          fLineHeight = fLineHeightTmp;
        }
      }
      fLineStep = std::max(fLineStep, fLineHeight);
      fLineWidth += pPiece->m_iWidth / 20000.0f;
    }
    fLinePos += fLineStep;
    m_fMaxWidth = std::max(m_fMaxWidth, fLineWidth);
    if (m_pLoader && m_pLoader->m_bSaveLineHeight) {
      FX_FLOAT fHeight = fLinePos - m_pLoader->m_fLastPos;
      m_pLoader->m_fLastPos = fLinePos;
      m_pLoader->m_lineHeights.Add(fHeight);
    }
  }
  if (pStyle) {
    pStyle->AddRef();
  }
  m_pBreak->ClearBreakPieces();
  if (dwStatus == FX_RTFBREAK_ParagraphBreak) {
    m_pBreak->Reset();
    if (!pStyle && bEndBreak) {
      CXFA_Para para = m_pTextProvider->GetParaNode();
      if (para.IsExistInXML()) {
        FX_FLOAT fStartPos = para.GetMarginLeft();
        FX_FLOAT fIndent = para.GetTextIndent();
        if (fIndent > 0) {
          fStartPos += fIndent;
        }
        FX_FLOAT fSpaceBelow = para.GetSpaceBelow();
        if (fSpaceBelow < 0.1f) {
          fSpaceBelow = 0;
        }
        m_pBreak->SetLineStartPos(fStartPos);
        fLinePos += fSpaceBelow;
      }
    }
  }
  if (pStyle) {
    FX_FLOAT fStart = 0;
    const FDE_CSSRECT* pRect = pStyle->GetBoundaryStyles()->GetMarginWidth();
    if (pRect) {
      fStart = pRect->left.GetValue();
    }
    FX_FLOAT fTextIndent =
        pStyle->GetParagraphStyles()->GetTextIndent().GetValue();
    if (fTextIndent < 0) {
      fStart -= fTextIndent;
    }
    m_pBreak->SetLineStartPos(fStart);
    pStyle->Release();
  }
  m_iLines++;
}
void CXFA_TextLayout::RenderString(IFDE_RenderDevice* pDevice,
                                   IFDE_SolidBrush* pBrush,
                                   CXFA_PieceLine* pPieceLine,
                                   int32_t iPiece,
                                   FXTEXT_CHARPOS* pCharPos,
                                   const CFX_Matrix& tmDoc2Device) {
  XFA_LPCTEXTPIECE pPiece = pPieceLine->m_textPieces.GetAt(iPiece);
  int32_t iCount = GetDisplayPos(pPiece, pCharPos);
  if (iCount > 0) {
    pBrush->SetColor(pPiece->dwColor);
    pDevice->DrawString(pBrush, pPiece->pFont, pCharPos, iCount,
                        pPiece->fFontSize, &tmDoc2Device);
  }
  pPieceLine->m_charCounts.Add(iCount);
}
void CXFA_TextLayout::RenderPath(IFDE_RenderDevice* pDevice,
                                 IFDE_Pen* pPen,
                                 CXFA_PieceLine* pPieceLine,
                                 int32_t iPiece,
                                 FXTEXT_CHARPOS* pCharPos,
                                 const CFX_Matrix& tmDoc2Device) {
  XFA_TEXTPIECE* pPiece = pPieceLine->m_textPieces.GetAt(iPiece);
  FX_BOOL bNoUnderline = pPiece->iUnderline < 1 || pPiece->iUnderline > 2;
  FX_BOOL bNoLineThrough = pPiece->iLineThrough < 1 || pPiece->iLineThrough > 2;
  if (bNoUnderline && bNoLineThrough) {
    return;
  }
  pPen->SetColor(pPiece->dwColor);
  IFDE_Path* pPath = IFDE_Path::Create();
  int32_t iChars = GetDisplayPos(pPiece, pCharPos);
  if (iChars > 0) {
    CFX_PointF pt1, pt2;
    FX_FLOAT fEndY = pCharPos[0].m_OriginY + 1.05f;
    int32_t i = 0;
    if (pPiece->iPeriod == XFA_ATTRIBUTEENUM_Word) {
      for (int32_t i = 0; i < pPiece->iUnderline; i++) {
        for (int32_t j = 0; j < iChars; j++) {
          pt1.x = pCharPos[j].m_OriginX;
          pt2.x =
              pt1.x + pCharPos[j].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
          pt1.y = pt2.y = fEndY;
          pPath->AddLine(pt1, pt2);
        }
        fEndY += 2.0f;
      }
    } else {
      pt1.x = pCharPos[0].m_OriginX;
      pt2.x =
          pCharPos[iChars - 1].m_OriginX +
          pCharPos[iChars - 1].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
      for (int32_t i = 0; i < pPiece->iUnderline; i++) {
        pt1.y = pt2.y = fEndY;
        pPath->AddLine(pt1, pt2);
        fEndY += 2.0f;
      }
    }
    fEndY = pCharPos[0].m_OriginY - pPiece->rtPiece.height * 0.25f;
    pt1.x = pCharPos[0].m_OriginX;
    pt2.x = pCharPos[iChars - 1].m_OriginX +
            pCharPos[iChars - 1].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
    for (i = 0; i < pPiece->iLineThrough; i++) {
      pt1.y = pt2.y = fEndY;
      pPath->AddLine(pt1, pt2);
      fEndY += 2.0f;
    }
  } else {
    if (bNoLineThrough &&
        (bNoUnderline || pPiece->iPeriod != XFA_ATTRIBUTEENUM_All)) {
      goto XFA_RenderPathRet;
    }
    int32_t iCharsTmp = 0;
    int32_t iPiecePrev = iPiece, iPieceNext = iPiece;
    while (iPiecePrev > 0) {
      iPiecePrev--;
      iCharsTmp = pPieceLine->m_charCounts.GetAt(iPiecePrev);
      if (iCharsTmp > 0) {
        break;
      }
    }
    if (iCharsTmp == 0) {
      goto XFA_RenderPathRet;
    }
    iCharsTmp = 0;
    int32_t iPieces = pPieceLine->m_textPieces.GetSize();
    while (iPieceNext < iPieces - 1) {
      iPieceNext++;
      iCharsTmp = pPieceLine->m_charCounts.GetAt(iPieceNext);
      if (iCharsTmp > 0) {
        break;
      }
    }
    if (iCharsTmp == 0) {
      goto XFA_RenderPathRet;
    }
    FX_FLOAT fOrgX = 0.0f, fEndX = 0.0f;
    pPiece = pPieceLine->m_textPieces.GetAt(iPiecePrev);
    iChars = GetDisplayPos(pPiece, pCharPos);
    if (iChars < 1) {
      goto XFA_RenderPathRet;
    }
    fOrgX = pCharPos[iChars - 1].m_OriginX +
            pCharPos[iChars - 1].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
    pPiece = pPieceLine->m_textPieces.GetAt(iPieceNext);
    iChars = GetDisplayPos(pPiece, pCharPos);
    if (iChars < 1) {
      goto XFA_RenderPathRet;
    }
    fEndX = pCharPos[0].m_OriginX;
    CFX_PointF pt1, pt2;
    pt1.x = fOrgX, pt2.x = fEndX;
    FX_FLOAT fEndY = pCharPos[0].m_OriginY + 1.05f;
    int32_t i = 0;
    for (i = 0; i < pPiece->iUnderline; i++) {
      pt1.y = pt2.y = fEndY;
      pPath->AddLine(pt1, pt2);
      fEndY += 2.0f;
    }
    fEndY = pCharPos[0].m_OriginY - pPiece->rtPiece.height * 0.25f;
    for (i = 0; i < pPiece->iLineThrough; i++) {
      pt1.y = pt2.y = fEndY;
      pPath->AddLine(pt1, pt2);
      fEndY += 2.0f;
    }
  }
  pDevice->DrawPath(pPen, 1, pPath, &tmDoc2Device);
XFA_RenderPathRet:
  pPath->Release();
}
int32_t CXFA_TextLayout::GetDisplayPos(XFA_LPCTEXTPIECE pPiece,
                                       FXTEXT_CHARPOS* pCharPos,
                                       FX_BOOL bCharCode) {
  if (pPiece == NULL) {
    return 0;
  }
  FX_RTFTEXTOBJ tr;
  if (!ToRun(pPiece, tr)) {
    return 0;
  }
  return m_pBreak->GetDisplayPos(&tr, pCharPos, bCharCode);
}
FX_BOOL CXFA_TextLayout::ToRun(XFA_LPCTEXTPIECE pPiece, FX_RTFTEXTOBJ& tr) {
  int32_t iLength = pPiece->iChars;
  if (iLength < 1) {
    return FALSE;
  }
  tr.pStr = pPiece->pszText;
  tr.pFont = pPiece->pFont;
  tr.pRect = &pPiece->rtPiece;
  tr.pWidths = pPiece->pWidths;
  tr.iLength = iLength;
  tr.fFontSize = pPiece->fFontSize;
  tr.iBidiLevel = pPiece->iBidiLevel;
  tr.iCharRotation = 0;
  tr.wLineBreakChar = L'\n';
  tr.iVerticalScale = pPiece->iVerScale;
  tr.dwLayoutStyles = FX_RTFLAYOUTSTYLE_ExpandTab;
  tr.iHorizontalScale = pPiece->iHorScale;
  return TRUE;
}
