// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_textlayout.h"

#include <algorithm>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_path.h"
#include "xfa/fde/fde_gedevice.h"
#include "xfa/fde/fde_object.h"
#include "xfa/fde/xml/fde_xml_imp.h"
#include "xfa/fxfa/app/cxfa_linkuserdata.h"
#include "xfa/fxfa/app/cxfa_loadercontext.h"
#include "xfa/fxfa/app/cxfa_pieceline.h"
#include "xfa/fxfa/app/cxfa_textparsecontext.h"
#include "xfa/fxfa/app/cxfa_texttabstopscontext.h"
#include "xfa/fxfa/app/cxfa_textuserdata.h"
#include "xfa/fxfa/app/xfa_ffwidgetacc.h"
#include "xfa/fxfa/app/xfa_textpiece.h"
#include "xfa/fxfa/parser/cxfa_font.h"
#include "xfa/fxfa/parser/cxfa_para.h"
#include "xfa/fxfa/parser/xfa_object.h"

#define XFA_LOADERCNTXTFLG_FILTERSPACE 0x001

CXFA_TextLayout::CXFA_TextLayout(CXFA_TextProvider* pTextProvider)
    : m_bHasBlock(false),
      m_pTextProvider(pTextProvider),
      m_pTextDataNode(nullptr),
      m_bRichText(false),
      m_iLines(0),
      m_fMaxWidth(0),
      m_bBlockContinue(true) {
  ASSERT(m_pTextProvider);
}

CXFA_TextLayout::~CXFA_TextLayout() {
  m_textParser.Reset();
  Unload();
}

void CXFA_TextLayout::Unload() {
  for (int32_t i = 0; i < m_pieceLines.GetSize(); i++) {
    CXFA_PieceLine* pLine = m_pieceLines.GetAt(i);
    for (int32_t i = 0; i < pLine->m_textPieces.GetSize(); i++) {
      XFA_TextPiece* pPiece = pLine->m_textPieces.GetAt(i);
      // Release text and widths in a text piece.
      delete pPiece->pszText;
      delete pPiece->pWidths;
      // Release text piece.
      delete pPiece;
    }
    pLine->m_textPieces.RemoveAll();
    // Release line.
    delete pLine;
  }
  m_pieceLines.RemoveAll();
  m_pBreak.reset();
}

const CFX_ArrayTemplate<CXFA_PieceLine*>* CXFA_TextLayout::GetPieceLines() {
  return &m_pieceLines;
}

void CXFA_TextLayout::GetTextDataNode() {
  if (!m_pTextProvider)
    return;

  CXFA_Node* pNode = m_pTextProvider->GetTextNode(m_bRichText);
  if (pNode && m_bRichText)
    m_textParser.Reset();

  m_pTextDataNode = pNode;
}

CFDE_XMLNode* CXFA_TextLayout::GetXMLContainerNode() {
  if (!m_bRichText)
    return nullptr;

  CFDE_XMLNode* pXMLRoot = m_pTextDataNode->GetXMLMappingNode();
  if (!pXMLRoot)
    return nullptr;

  CFDE_XMLNode* pXMLContainer = nullptr;
  for (CFDE_XMLNode* pXMLChild =
           pXMLRoot->GetNodeItem(CFDE_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(CFDE_XMLNode::NextSibling)) {
    if (pXMLChild->GetType() == FDE_XMLNODE_Element) {
      CFDE_XMLElement* pXMLElement = static_cast<CFDE_XMLElement*>(pXMLChild);
      CFX_WideString wsTag;
      pXMLElement->GetLocalTagName(wsTag);
      if (wsTag == FX_WSTRC(L"body") || wsTag == FX_WSTRC(L"html")) {
        pXMLContainer = pXMLChild;
        break;
      }
    }
  }
  return pXMLContainer;
}

CFX_RTFBreak* CXFA_TextLayout::CreateBreak(bool bDefault) {
  uint32_t dwStyle = FX_RTFLAYOUTSTYLE_ExpandTab;
  if (!bDefault)
    dwStyle |= FX_RTFLAYOUTSTYLE_Pagination;

  CFX_RTFBreak* pBreak = new CFX_RTFBreak(0);
  pBreak->SetLayoutStyles(dwStyle);
  pBreak->SetLineBreakChar(L'\n');
  pBreak->SetLineBreakTolerance(1);
  pBreak->SetFont(m_textParser.GetFont(m_pTextProvider, nullptr));
  pBreak->SetFontSize(m_textParser.GetFontSize(m_pTextProvider, nullptr));
  return pBreak;
}

void CXFA_TextLayout::InitBreak(FX_FLOAT fLineWidth) {
  CXFA_Font font = m_pTextProvider->GetFontNode();
  CXFA_Para para = m_pTextProvider->GetParaNode();
  FX_FLOAT fStart = 0;
  FX_FLOAT fStartPos = 0;
  if (para) {
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
      if (iAlign != FX_RTFLINEALIGNMENT_Left)
        fLineWidth -= para.GetMarginRight();
    } else {
      fLineWidth -= para.GetMarginRight();
    }
    if (fLineWidth < 0)
      fLineWidth = fStart;

    fStartPos = fStart;
    FX_FLOAT fIndent = para.GetTextIndent();
    if (fIndent > 0)
      fStartPos += fIndent;
  }

  m_pBreak->SetLineBoundary(fStart, fLineWidth);
  m_pBreak->SetLineStartPos(fStartPos);
  if (font) {
    m_pBreak->SetHorizontalScale((int32_t)font.GetHorizontalScale());
    m_pBreak->SetVerticalScale((int32_t)font.GetVerticalScale());
    m_pBreak->SetCharSpace(font.GetLetterSpacing());
  }

  FX_FLOAT fFontSize = m_textParser.GetFontSize(m_pTextProvider, nullptr);
  m_pBreak->SetFontSize(fFontSize);
  m_pBreak->SetFont(m_textParser.GetFont(m_pTextProvider, nullptr));
  m_pBreak->SetLineBreakTolerance(fFontSize * 0.2f);
}

void CXFA_TextLayout::InitBreak(IFDE_CSSComputedStyle* pStyle,
                                FDE_CSSDisplay eDisplay,
                                FX_FLOAT fLineWidth,
                                CFDE_XMLNode* pXMLNode,
                                IFDE_CSSComputedStyle* pParentStyle) {
  if (!pStyle) {
    InitBreak(fLineWidth);
    return;
  }

  IFDE_CSSParagraphStyle* pParaStyle = pStyle->GetParagraphStyles();
  if (eDisplay == FDE_CSSDisplay::Block ||
      eDisplay == FDE_CSSDisplay::ListItem) {
    int32_t iAlign = FX_RTFLINEALIGNMENT_Left;
    switch (pParaStyle->GetTextAlign()) {
      case FDE_CSSTextAlign::Right:
        iAlign = FX_RTFLINEALIGNMENT_Right;
        break;
      case FDE_CSSTextAlign::Center:
        iAlign = FX_RTFLINEALIGNMENT_Center;
        break;
      case FDE_CSSTextAlign::Justify:
        iAlign = FX_RTFLINEALIGNMENT_Justified;
        break;
      case FDE_CSSTextAlign::JustifyAll:
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
    if (pRect) {
      fStart = pRect->left.GetValue();
      fLineWidth -= pRect->right.GetValue();
      if (pPaddingRect) {
        fStart += pPaddingRect->left.GetValue();
        fLineWidth -= pPaddingRect->right.GetValue();
      }
      if (eDisplay == FDE_CSSDisplay::ListItem) {
        const FDE_CSSRECT* pParRect =
            pParentStyle->GetBoundaryStyles()->GetMarginWidth();
        const FDE_CSSRECT* pParPaddingRect =
            pParentStyle->GetBoundaryStyles()->GetPaddingWidth();
        if (pParRect) {
          fStart += pParRect->left.GetValue();
          fLineWidth -= pParRect->right.GetValue();
          if (pParPaddingRect) {
            fStart += pParPaddingRect->left.GetValue();
            fLineWidth -= pParPaddingRect->right.GetValue();
          }
        }
        FDE_CSSRECT pNewRect;
        pNewRect.left.Set(FDE_CSSLengthUnit::Point, fStart);
        pNewRect.right.Set(FDE_CSSLengthUnit::Point, pRect->right.GetValue());
        pNewRect.top.Set(FDE_CSSLengthUnit::Point, pRect->top.GetValue());
        pNewRect.bottom.Set(FDE_CSSLengthUnit::Point, pRect->bottom.GetValue());
        pStyle->GetBoundaryStyles()->SetMarginWidth(pNewRect);
      }
    }
    m_pBreak->SetLineBoundary(fStart, fLineWidth);
    FX_FLOAT fIndent = pParaStyle->GetTextIndent().GetValue();
    if (fIndent > 0)
      fStart += fIndent;

    m_pBreak->SetLineStartPos(fStart);
    m_pBreak->SetTabWidth(m_textParser.GetTabInterval(pStyle));
    if (!m_pTabstopContext)
      m_pTabstopContext = pdfium::MakeUnique<CXFA_TextTabstopsContext>();
    m_textParser.GetTabstops(pStyle, m_pTabstopContext.get());
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
  wsText.clear();
  if (!m_bRichText)
    wsText = m_pTextDataNode->GetContent();
  return wsText.GetLength();
}

FX_FLOAT CXFA_TextLayout::GetLayoutHeight() {
  if (!m_pLoader)
    return 0;

  int32_t iCount = m_pLoader->m_lineHeights.GetSize();
  if (iCount == 0 && m_pLoader->m_fWidth > 0) {
    CFX_SizeF szMax(m_pLoader->m_fWidth, m_pLoader->m_fHeight);
    CFX_SizeF szDef;
    m_pLoader->m_bSaveLineHeight = true;
    m_pLoader->m_fLastPos = 0;
    CalcSize(szMax, szMax, szDef);
    m_pLoader->m_bSaveLineHeight = false;
    return szDef.y;
  }

  FX_FLOAT fHeight = m_pLoader->m_fHeight;
  if (fHeight < 0.1f) {
    fHeight = 0;
    for (int32_t i = 0; i < iCount; i++)
      fHeight += m_pLoader->m_lineHeights.ElementAt(i);
  }
  return fHeight;
}

FX_FLOAT CXFA_TextLayout::StartLayout(FX_FLOAT fWidth) {
  if (!m_pLoader)
    m_pLoader = pdfium::MakeUnique<CXFA_LoaderContext>();

  if (fWidth < 0 || (m_pLoader->m_fWidth > -1 &&
                     FXSYS_fabs(fWidth - m_pLoader->m_fWidth) > 0)) {
    m_pLoader->m_lineHeights.RemoveAll();
    m_Blocks.RemoveAll();
    Unload();
    m_pLoader->m_fStartLineOffset = 0;
  }
  m_pLoader->m_fWidth = fWidth;

  if (fWidth < 0) {
    CFX_SizeF szMax;
    CFX_SizeF szDef;
    m_pLoader->m_bSaveLineHeight = true;
    m_pLoader->m_fLastPos = 0;
    CalcSize(szMax, szMax, szDef);
    m_pLoader->m_bSaveLineHeight = false;
    fWidth = szDef.x;
  }
  return fWidth;
}

bool CXFA_TextLayout::DoLayout(int32_t iBlockIndex,
                               FX_FLOAT& fCalcHeight,
                               FX_FLOAT fContentAreaHeight,
                               FX_FLOAT fTextHeight) {
  if (!m_pLoader)
    return false;

  int32_t iBlockCount = m_Blocks.GetSize();
  FX_FLOAT fHeight = fTextHeight;
  if (fHeight < 0)
    fHeight = GetLayoutHeight();

  m_pLoader->m_fHeight = fHeight;
  if (fContentAreaHeight < 0)
    return false;

  m_bHasBlock = true;
  if (iBlockCount == 0 && fHeight > 0) {
    fHeight = fTextHeight - GetLayoutHeight();
    if (fHeight > 0) {
      int32_t iAlign = m_textParser.GetVAlign(m_pTextProvider);
      if (iAlign == XFA_ATTRIBUTEENUM_Middle)
        fHeight /= 2.0f;
      else if (iAlign != XFA_ATTRIBUTEENUM_Bottom)
        fHeight = 0;
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
    if (!m_pLoader->m_BlocksHeight.empty()) {
      for (int32_t i = 0; i < iBlockIndex; i++)
        fLinePos -= m_pLoader->m_BlocksHeight[i * 2 + 1];
    }
  }

  int32_t iCount = m_pLoader->m_lineHeights.GetSize();
  int32_t i = 0;
  for (i = iLineIndex; i < iCount; i++) {
    FX_FLOAT fLineHeight = m_pLoader->m_lineHeights.ElementAt(i);
    if ((i == iLineIndex) && (fLineHeight - fContentAreaHeight > 0.001)) {
      fCalcHeight = 0;
      return true;
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
          if (pdfium::CollectionSize<int32_t>(m_pLoader->m_BlocksHeight) >
                  iBlockIndex * 2 &&
              (m_pLoader->m_BlocksHeight[iBlockIndex * 2] == iBlockIndex)) {
            m_pLoader->m_BlocksHeight[iBlockIndex * 2 + 1] = fCalcHeight;
          } else {
            m_pLoader->m_BlocksHeight.push_back((FX_FLOAT)iBlockIndex);
            m_pLoader->m_BlocksHeight.push_back(fCalcHeight);
          }
        }
        return true;
      }

      fCalcHeight = fLinePos;
      return true;
    }
    fLinePos += fLineHeight;
  }
  return false;
}

int32_t CXFA_TextLayout::CountBlocks() const {
  int32_t iCount = m_Blocks.GetSize() / 2;
  return iCount > 0 ? iCount : 1;
}

bool CXFA_TextLayout::CalcSize(const CFX_SizeF& minSize,
                               const CFX_SizeF& maxSize,
                               CFX_SizeF& defaultSize) {
  defaultSize.x = maxSize.x;
  if (defaultSize.x < 1)
    defaultSize.x = 0xFFFF;

  m_pBreak.reset(CreateBreak(false));
  FX_FLOAT fLinePos = 0;
  m_iLines = 0;
  m_fMaxWidth = 0;
  Loader(defaultSize, fLinePos, false);
  if (fLinePos < 0.1f)
    fLinePos = m_textParser.GetFontSize(m_pTextProvider, nullptr);

  m_pTabstopContext.reset();
  defaultSize = CFX_SizeF(m_fMaxWidth, fLinePos);
  return true;
}

bool CXFA_TextLayout::Layout(const CFX_SizeF& size, FX_FLOAT* fHeight) {
  if (size.x < 1)
    return false;

  Unload();
  m_pBreak.reset(CreateBreak(true));
  if (m_pLoader) {
    m_pLoader->m_iTotalLines = -1;
    m_pLoader->m_iChar = 0;
  }

  m_iLines = 0;
  FX_FLOAT fLinePos = 0;
  Loader(size, fLinePos, true);
  UpdateAlign(size.y, fLinePos);
  m_pTabstopContext.reset();
  if (fHeight)
    *fHeight = fLinePos;
  return true;
}

bool CXFA_TextLayout::Layout(int32_t iBlock) {
  if (!m_pLoader || iBlock < 0 || iBlock >= CountBlocks())
    return false;
  if (m_pLoader->m_fWidth < 1)
    return false;

  m_pLoader->m_iTotalLines = -1;
  m_iLines = 0;
  FX_FLOAT fLinePos = 0;
  CXFA_Node* pNode = nullptr;
  CFX_SizeF szText(m_pLoader->m_fWidth, m_pLoader->m_fHeight);
  int32_t iCount = m_Blocks.GetSize();
  int32_t iBlocksHeightCount =
      pdfium::CollectionSize<int32_t>(m_pLoader->m_BlocksHeight);
  iBlocksHeightCount /= 2;
  if (iBlock < iBlocksHeightCount)
    return true;
  if (iBlock == iBlocksHeightCount) {
    Unload();
    m_pBreak.reset(CreateBreak(true));
    fLinePos = m_pLoader->m_fStartLineOffset;
    for (int32_t i = 0; i < iBlocksHeightCount; i++)
      fLinePos -= m_pLoader->m_BlocksHeight[i * 2 + 1];

    m_pLoader->m_iChar = 0;
    if (iCount > 1)
      m_pLoader->m_iTotalLines = m_Blocks.ElementAt(iBlock * 2 + 1);

    Loader(szText, fLinePos, true);
    if (iCount == 0 && m_pLoader->m_fStartLineOffset < 0.1f)
      UpdateAlign(szText.y, fLinePos);
  } else if (m_pTextDataNode) {
    iBlock *= 2;
    if (iBlock < iCount - 2)
      m_pLoader->m_iTotalLines = m_Blocks.ElementAt(iBlock + 1);

    m_pBreak->Reset();
    if (m_bRichText) {
      CFDE_XMLNode* pContainerNode = GetXMLContainerNode();
      if (!pContainerNode)
        return true;

      CFDE_XMLNode* pXMLNode = m_pLoader->m_pXMLNode;
      if (!pXMLNode)
        return true;

      CFDE_XMLNode* pSaveXMLNode = m_pLoader->m_pXMLNode;
      for (; pXMLNode;
           pXMLNode = pXMLNode->GetNodeItem(CFDE_XMLNode::NextSibling)) {
        if (!LoadRichText(pXMLNode, szText, fLinePos, m_pLoader->m_pParentStyle,
                          true)) {
          break;
        }
      }
      while (!pXMLNode) {
        pXMLNode = pSaveXMLNode->GetNodeItem(CFDE_XMLNode::Parent);
        if (pXMLNode == pContainerNode)
          break;
        if (!LoadRichText(pXMLNode, szText, fLinePos, m_pLoader->m_pParentStyle,
                          true, nullptr, false)) {
          break;
        }
        pSaveXMLNode = pXMLNode;
        pXMLNode = pXMLNode->GetNodeItem(CFDE_XMLNode::NextSibling);
        if (!pXMLNode)
          continue;
        for (; pXMLNode;
             pXMLNode = pXMLNode->GetNodeItem(CFDE_XMLNode::NextSibling)) {
          if (!LoadRichText(pXMLNode, szText, fLinePos,
                            m_pLoader->m_pParentStyle, true)) {
            break;
          }
        }
      }
    } else {
      pNode = m_pLoader->m_pNode;
      if (!pNode)
        return true;
      LoadText(pNode, szText, fLinePos, true);
    }
  }
  if (iBlock == iCount) {
    m_pTabstopContext.reset();
    m_pLoader.reset();
  }
  return true;
}

void CXFA_TextLayout::ItemBlocks(const CFX_RectF& rtText, int32_t iBlockIndex) {
  if (!m_pLoader)
    return;

  int32_t iCountHeight = m_pLoader->m_lineHeights.GetSize();
  if (iCountHeight == 0)
    return;

  bool bEndItem = true;
  int32_t iBlockCount = m_Blocks.GetSize();
  FX_FLOAT fLinePos = m_pLoader->m_fStartLineOffset;
  int32_t iLineIndex = 0;
  if (iBlockIndex > 0) {
    int32_t iBlockHeightCount =
        pdfium::CollectionSize<int32_t>(m_pLoader->m_BlocksHeight);
    iBlockHeightCount /= 2;
    if (iBlockHeightCount >= iBlockIndex) {
      for (int32_t i = 0; i < iBlockIndex; i++)
        fLinePos -= m_pLoader->m_BlocksHeight[i * 2 + 1];
    } else {
      fLinePos = 0;
    }
    iLineIndex = m_Blocks[iBlockCount - 1] + m_Blocks[iBlockCount - 2];
  }

  int32_t i = 0;
  for (i = iLineIndex; i < iCountHeight; i++) {
    FX_FLOAT fLineHeight = m_pLoader->m_lineHeights.ElementAt(i);
    if (fLinePos + fLineHeight - rtText.height > 0.001) {
      m_Blocks.Add(iLineIndex);
      m_Blocks.Add(i - iLineIndex);
      bEndItem = false;
      break;
    }
    fLinePos += fLineHeight;
  }
  if (iCountHeight > 0 && (i - iLineIndex) > 0 && bEndItem) {
    m_Blocks.Add(iLineIndex);
    m_Blocks.Add(i - iLineIndex);
  }
}

bool CXFA_TextLayout::DrawString(CFX_RenderDevice* pFxDevice,
                                 const CFX_Matrix& tmDoc2Device,
                                 const CFX_RectF& rtClip,
                                 int32_t iBlock) {
  if (!pFxDevice)
    return false;

  std::unique_ptr<CFDE_RenderDevice> pDevice(
      new CFDE_RenderDevice(pFxDevice, false));
  pDevice->SaveState();
  pDevice->SetClipRect(rtClip);

  auto pSolidBrush = pdfium::MakeUnique<CFDE_Brush>();
  auto pPen = pdfium::MakeUnique<CFDE_Pen>();
  if (m_pieceLines.GetSize() == 0) {
    int32_t iBlockCount = CountBlocks();
    for (int32_t i = 0; i < iBlockCount; i++)
      Layout(i);
  }

  FXTEXT_CHARPOS* pCharPos = nullptr;
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
    if (i + iLineStart >= m_pieceLines.GetSize())
      break;

    CXFA_PieceLine* pPieceLine = m_pieceLines.GetAt(i + iLineStart);
    int32_t iPieces = pPieceLine->m_textPieces.GetSize();
    int32_t j = 0;
    for (j = 0; j < iPieces; j++) {
      const XFA_TextPiece* pPiece = pPieceLine->m_textPieces.GetAt(j);
      int32_t iChars = pPiece->iChars;
      if (iCharCount < iChars) {
        FX_Free(pCharPos);
        pCharPos = FX_Alloc(FXTEXT_CHARPOS, iChars);
        iCharCount = iChars;
      }
      FXSYS_memset(pCharPos, 0, iCharCount * sizeof(FXTEXT_CHARPOS));
      RenderString(pDevice.get(), pSolidBrush.get(), pPieceLine, j, pCharPos,
                   tmDoc2Device);
    }
    for (j = 0; j < iPieces; j++) {
      RenderPath(pDevice.get(), pPen.get(), pPieceLine, j, pCharPos,
                 tmDoc2Device);
    }
  }
  pDevice->RestoreState();
  FX_Free(pCharPos);
  return iPieceLines > 0;
}

void CXFA_TextLayout::UpdateAlign(FX_FLOAT fHeight, FX_FLOAT fBottom) {
  fHeight -= fBottom;
  if (fHeight < 0.1f)
    return;

  switch (m_textParser.GetVAlign(m_pTextProvider)) {
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
      XFA_TextPiece* pPiece = pPieceLine->m_textPieces.GetAt(j);
      CFX_RectF& rect = pPiece->rtPiece;
      rect.top += fHeight;
    }
  }
}

bool CXFA_TextLayout::Loader(const CFX_SizeF& szText,
                             FX_FLOAT& fLinePos,
                             bool bSavePieces) {
  GetTextDataNode();
  if (!m_pTextDataNode)
    return true;

  if (m_bRichText) {
    CFDE_XMLNode* pXMLContainer = GetXMLContainerNode();
    if (pXMLContainer) {
      if (!m_textParser.IsParsed())
        m_textParser.DoParse(pXMLContainer, m_pTextProvider);

      IFDE_CSSComputedStyle* pRootStyle =
          m_textParser.CreateRootStyle(m_pTextProvider);
      LoadRichText(pXMLContainer, szText, fLinePos, pRootStyle, bSavePieces);
      pRootStyle->Release();
    }
  } else {
    LoadText(m_pTextDataNode, szText, fLinePos, bSavePieces);
  }
  return true;
}

void CXFA_TextLayout::LoadText(CXFA_Node* pNode,
                               const CFX_SizeF& szText,
                               FX_FLOAT& fLinePos,
                               bool bSavePieces) {
  InitBreak(szText.x);

  CXFA_Para para = m_pTextProvider->GetParaNode();
  FX_FLOAT fSpaceAbove = 0;
  if (para) {
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
  bool bRet = AppendChar(wsText, fLinePos, fSpaceAbove, bSavePieces);
  if (bRet && m_pLoader)
    m_pLoader->m_pNode = pNode;
  else
    EndBreak(FX_RTFBREAK_ParagraphBreak, fLinePos, bSavePieces);
}

bool CXFA_TextLayout::LoadRichText(CFDE_XMLNode* pXMLNode,
                                   const CFX_SizeF& szText,
                                   FX_FLOAT& fLinePos,
                                   IFDE_CSSComputedStyle* pParentStyle,
                                   bool bSavePieces,
                                   CXFA_LinkUserData* pLinkData,
                                   bool bEndBreak,
                                   bool bIsOl,
                                   int32_t iLiCount) {
  if (!pXMLNode)
    return false;

  CXFA_TextParseContext* pContext =
      m_textParser.GetParseContextFromMap(pXMLNode);
  FDE_CSSDisplay eDisplay = FDE_CSSDisplay::None;
  bool bContentNode = false;
  FX_FLOAT fSpaceBelow = 0;
  IFDE_CSSComputedStyle* pStyle = nullptr;
  CFX_WideString wsName;
  if (bEndBreak) {
    bool bCurOl = false;
    bool bCurLi = false;
    CFDE_XMLElement* pElement = nullptr;
    if (pContext) {
      if (m_bBlockContinue ||
          (m_pLoader && pXMLNode == m_pLoader->m_pXMLNode)) {
        m_bBlockContinue = true;
      }
      if (pXMLNode->GetType() == FDE_XMLNODE_Text) {
        bContentNode = true;
      } else if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
        pElement = static_cast<CFDE_XMLElement*>(pXMLNode);
        pElement->GetLocalTagName(wsName);
      }
      if (wsName == FX_WSTRC(L"ol")) {
        bIsOl = true;
        bCurOl = true;
      }
      if (m_bBlockContinue || bContentNode == false) {
        eDisplay = pContext->GetDisplay();
        if (eDisplay != FDE_CSSDisplay::Block &&
            eDisplay != FDE_CSSDisplay::Inline &&
            eDisplay != FDE_CSSDisplay::ListItem) {
          return true;
        }

        pStyle = m_textParser.ComputeStyle(pXMLNode, pParentStyle);
        InitBreak(bContentNode ? pParentStyle : pStyle, eDisplay, szText.x,
                  pXMLNode, pParentStyle);
        if ((eDisplay == FDE_CSSDisplay::Block ||
             eDisplay == FDE_CSSDisplay::ListItem) &&
            pStyle &&
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
          ASSERT(pElement);
          pElement->GetString(L"href", wsLinkContent);
          if (!wsLinkContent.IsEmpty()) {
            pLinkData = new CXFA_LinkUserData(
                wsLinkContent.GetBuffer(wsLinkContent.GetLength()));
            wsLinkContent.ReleaseBuffer(wsLinkContent.GetLength());
          }
        }

        int32_t iTabCount =
            m_textParser.CountTabs(bContentNode ? pParentStyle : pStyle);
        bool bSpaceRun =
            m_textParser.IsSpaceRun(bContentNode ? pParentStyle : pStyle);
        CFX_WideString wsText;
        if (bContentNode && iTabCount == 0) {
          static_cast<CFDE_XMLText*>(pXMLNode)->GetText(wsText);
        } else if (wsName == FX_WSTRC(L"br")) {
          wsText = L'\n';
        } else if (wsName == FX_WSTRC(L"li")) {
          bCurLi = true;
          if (bIsOl)
            wsText.Format(L"%d.  ", iLiCount);
          else
            wsText = 0x00B7 + FX_WSTRC(L"  ");
        } else if (!bContentNode) {
          if (iTabCount > 0) {
            while (iTabCount-- > 0)
              wsText += L'\t';
          } else {
            m_textParser.GetEmbbedObj(m_pTextProvider, pXMLNode, wsText);
          }
        }

        int32_t iLength = wsText.GetLength();
        if (iLength > 0 && bContentNode && !bSpaceRun)
          ProcessText(wsText);

        if (m_pLoader) {
          if (wsText.GetLength() > 0 &&
              (m_pLoader->m_dwFlags & XFA_LOADERCNTXTFLG_FILTERSPACE)) {
            wsText.TrimLeft(0x20);
          }
          if (FDE_CSSDisplay::Block == eDisplay) {
            m_pLoader->m_dwFlags |= XFA_LOADERCNTXTFLG_FILTERSPACE;
          } else if (FDE_CSSDisplay::Inline == eDisplay &&
                     (m_pLoader->m_dwFlags & XFA_LOADERCNTXTFLG_FILTERSPACE)) {
            m_pLoader->m_dwFlags &= ~XFA_LOADERCNTXTFLG_FILTERSPACE;
          } else if (wsText.GetLength() > 0 &&
                     (0x20 == wsText.GetAt(wsText.GetLength() - 1))) {
            m_pLoader->m_dwFlags |= XFA_LOADERCNTXTFLG_FILTERSPACE;
          } else if (wsText.GetLength() != 0) {
            m_pLoader->m_dwFlags &= ~XFA_LOADERCNTXTFLG_FILTERSPACE;
          }
        }

        if (wsText.GetLength() > 0) {
          if (!m_pLoader || m_pLoader->m_iChar == 0) {
            if (pLinkData)
              pLinkData->Retain();

            CXFA_TextUserData* pUserData = new CXFA_TextUserData(
                bContentNode ? pParentStyle : pStyle, pLinkData);
            m_pBreak->SetUserData(pUserData);
          }

          if (AppendChar(wsText, fLinePos, 0, bSavePieces)) {
            if (m_pLoader)
              m_pLoader->m_dwFlags &= ~XFA_LOADERCNTXTFLG_FILTERSPACE;
            if (IsEnd(bSavePieces)) {
              if (m_pLoader && m_pLoader->m_iTotalLines > -1) {
                m_pLoader->m_pXMLNode = pXMLNode;
                m_pLoader->m_pParentStyle = pParentStyle;
              }
              if (pStyle)
                pStyle->Release();
              return false;
            }
            return true;
          }
        }
      }
    }

    for (CFDE_XMLNode* pChildNode =
             pXMLNode->GetNodeItem(CFDE_XMLNode::FirstChild);
         pChildNode;
         pChildNode = pChildNode->GetNodeItem(CFDE_XMLNode::NextSibling)) {
      if (bCurOl)
        iLiCount++;

      if (!LoadRichText(pChildNode, szText, fLinePos,
                        pContext ? pStyle : pParentStyle, bSavePieces,
                        pLinkData, true, bIsOl, iLiCount))
        return false;
    }

    if (m_pLoader) {
      if (FDE_CSSDisplay::Block == eDisplay)
        m_pLoader->m_dwFlags |= XFA_LOADERCNTXTFLG_FILTERSPACE;
    }
    if (bCurLi)
      EndBreak(FX_RTFBREAK_LineBreak, fLinePos, bSavePieces);
  } else {
    if (pContext)
      eDisplay = pContext->GetDisplay();
  }

  if (m_bBlockContinue) {
    if (pContext && !bContentNode) {
      uint32_t dwStatus = (eDisplay == FDE_CSSDisplay::Block)
                              ? FX_RTFBREAK_ParagraphBreak
                              : FX_RTFBREAK_PieceBreak;
      EndBreak(dwStatus, fLinePos, bSavePieces);
      if (eDisplay == FDE_CSSDisplay::Block) {
        fLinePos += fSpaceBelow;
        if (m_pTabstopContext)
          m_pTabstopContext->RemoveAll();
      }
      if (wsName == FX_WSTRC(L"a")) {
        if (pLinkData) {
          pLinkData->Release();
          pLinkData = nullptr;
        }
      }
      if (IsEnd(bSavePieces)) {
        if (pStyle)
          pStyle->Release();

        if (m_pLoader && m_pLoader->m_iTotalLines > -1) {
          m_pLoader->m_pXMLNode =
              pXMLNode->GetNodeItem(CFDE_XMLNode::NextSibling);
          m_pLoader->m_pParentStyle = pParentStyle;
        }
        return false;
      }
    }
  }
  if (pStyle)
    pStyle->Release();

  return true;
}

bool CXFA_TextLayout::AppendChar(const CFX_WideString& wsText,
                                 FX_FLOAT& fLinePos,
                                 FX_FLOAT fSpaceAbove,
                                 bool bSavePieces) {
  uint32_t dwStatus = 0;
  int32_t iChar = 0;
  if (m_pLoader)
    iChar = m_pLoader->m_iChar;

  int32_t iLength = wsText.GetLength();
  for (int32_t i = iChar; i < iLength; i++) {
    FX_WCHAR wch = wsText.GetAt(i);
    if (wch == 0xA0)
      wch = 0x20;

    if ((dwStatus = m_pBreak->AppendChar(wch)) > FX_RTFBREAK_PieceBreak) {
      AppendTextLine(dwStatus, fLinePos, bSavePieces);
      if (IsEnd(bSavePieces)) {
        if (m_pLoader)
          m_pLoader->m_iChar = i;
        return true;
      }
      if (dwStatus == FX_RTFBREAK_ParagraphBreak && m_bRichText)
        fLinePos += fSpaceAbove;
    }
  }
  if (m_pLoader)
    m_pLoader->m_iChar = 0;

  return false;
}

bool CXFA_TextLayout::IsEnd(bool bSavePieces) {
  if (!bSavePieces)
    return false;
  if (m_pLoader && m_pLoader->m_iTotalLines > 0)
    return m_iLines >= m_pLoader->m_iTotalLines;
  return false;
}

void CXFA_TextLayout::ProcessText(CFX_WideString& wsText) {
  int32_t iLen = wsText.GetLength();
  if (iLen == 0)
    return;

  FX_WCHAR* psz = wsText.GetBuffer(iLen);
  int32_t iTrimLeft = 0;
  FX_WCHAR wch = 0, wPrev = 0;
  for (int32_t i = 0; i < iLen; i++) {
    wch = psz[i];
    if (wch < 0x20)
      wch = 0x20;
    if (wch == 0x20 && wPrev == 0x20)
      continue;

    wPrev = wch;
    psz[iTrimLeft++] = wch;
  }
  wsText.ReleaseBuffer(iLen);
  wsText = wsText.Left(iTrimLeft);
}

void CXFA_TextLayout::EndBreak(uint32_t dwStatus,
                               FX_FLOAT& fLinePos,
                               bool bSavePieces) {
  dwStatus = m_pBreak->EndBreak(dwStatus);
  if (dwStatus > FX_RTFBREAK_PieceBreak)
    AppendTextLine(dwStatus, fLinePos, bSavePieces, true);
}

void CXFA_TextLayout::DoTabstops(IFDE_CSSComputedStyle* pStyle,
                                 CXFA_PieceLine* pPieceLine) {
  if (!m_pTabstopContext || m_pTabstopContext->m_iTabCount == 0)
    return;
  if (!pStyle || !pPieceLine)
    return;

  int32_t iPieces = pPieceLine->m_textPieces.GetSize();
  if (iPieces == 0)
    return;

  XFA_TextPiece* pPiece = pPieceLine->m_textPieces.GetAt(iPieces - 1);
  int32_t& iTabstopsIndex = m_pTabstopContext->m_iTabIndex;
  int32_t iCount = m_textParser.CountTabs(pStyle);
  if (iTabstopsIndex > m_pTabstopContext->m_iTabCount - 1)
    return;

  if (iCount > 0) {
    iTabstopsIndex++;
    m_pTabstopContext->m_bTabstops = true;
    FX_FLOAT fRight = 0;
    if (iPieces > 1) {
      XFA_TextPiece* p = pPieceLine->m_textPieces.GetAt(iPieces - 2);
      fRight = p->rtPiece.right();
    }
    m_pTabstopContext->m_fTabWidth =
        pPiece->rtPiece.width + pPiece->rtPiece.left - fRight;
  } else if (iTabstopsIndex > -1) {
    FX_FLOAT fLeft = 0;
    if (m_pTabstopContext->m_bTabstops) {
      XFA_TABSTOPS* pTabstops =
          m_pTabstopContext->m_tabstops.GetDataPtr(iTabstopsIndex);
      uint32_t dwAlign = pTabstops->dwAlign;
      if (dwAlign == FX_HashCode_GetW(L"center", false)) {
        fLeft = pPiece->rtPiece.width / 2.0f;
      } else if (dwAlign == FX_HashCode_GetW(L"right", false) ||
                 dwAlign == FX_HashCode_GetW(L"before", false)) {
        fLeft = pPiece->rtPiece.width;
      } else if (dwAlign == FX_HashCode_GetW(L"decimal", false)) {
        int32_t iChars = pPiece->iChars;
        for (int32_t i = 0; i < iChars; i++) {
          if (pPiece->pszText[i] == L'.')
            break;

          fLeft += pPiece->pWidths[i] / 20000.0f;
        }
      }
      m_pTabstopContext->m_fLeft =
          std::min(fLeft, m_pTabstopContext->m_fTabWidth);
      m_pTabstopContext->m_bTabstops = false;
      m_pTabstopContext->m_fTabWidth = 0;
    }
    pPiece->rtPiece.left -= m_pTabstopContext->m_fLeft;
  }
}

void CXFA_TextLayout::AppendTextLine(uint32_t dwStatus,
                                     FX_FLOAT& fLinePos,
                                     bool bSavePieces,
                                     bool bEndBreak) {
  int32_t iPieces = m_pBreak->CountBreakPieces();
  if (iPieces < 1)
    return;

  IFDE_CSSComputedStyle* pStyle = nullptr;
  if (bSavePieces) {
    CXFA_PieceLine* pPieceLine = new CXFA_PieceLine;
    m_pieceLines.Add(pPieceLine);
    if (m_pTabstopContext)
      m_pTabstopContext->Reset();

    FX_FLOAT fLineStep = 0, fBaseLine = 0;
    int32_t i = 0;
    for (i = 0; i < iPieces; i++) {
      const CFX_RTFPiece* pPiece = m_pBreak->GetBreakPiece(i);
      CXFA_TextUserData* pUserData = (CXFA_TextUserData*)pPiece->m_pUserData;
      if (pUserData)
        pStyle = pUserData->m_pStyle;
      FX_FLOAT fVerScale = pPiece->m_iVerticalScale / 100.0f;

      XFA_TextPiece* pTP = new XFA_TextPiece();
      pTP->pszText =
          (FX_WCHAR*)FX_Alloc(uint8_t, pPiece->m_iChars * sizeof(FX_WCHAR));
      pTP->pWidths =
          (int32_t*)FX_Alloc(uint8_t, pPiece->m_iChars * sizeof(int32_t));
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
        if (fLineHeight < fLineHeightTmp)
          fLineHeight = fLineHeightTmp;
        else
          fBaseLineTemp = 0;
      } else if (fBaseLine < -fBaseLineTemp) {
        fBaseLine = -fBaseLineTemp;
      }
      fLineStep = std::max(fLineStep, fLineHeight);
      if (pUserData && pUserData->m_pLinkData) {
        pUserData->m_pLinkData->Retain();
        pTP->pLinkData = pUserData->m_pLinkData;
      } else {
        pTP->pLinkData = nullptr;
      }
      DoTabstops(pStyle, pPieceLine);
    }
    for (i = 0; i < iPieces; i++) {
      XFA_TextPiece* pTP = pPieceLine->m_textPieces.GetAt(i);
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
      if (pUserData)
        pStyle = pUserData->m_pStyle;
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
  if (pStyle)
    pStyle->Retain();

  m_pBreak->ClearBreakPieces();
  if (dwStatus == FX_RTFBREAK_ParagraphBreak) {
    m_pBreak->Reset();
    if (!pStyle && bEndBreak) {
      CXFA_Para para = m_pTextProvider->GetParaNode();
      if (para) {
        FX_FLOAT fStartPos = para.GetMarginLeft();
        FX_FLOAT fIndent = para.GetTextIndent();
        if (fIndent > 0)
          fStartPos += fIndent;

        FX_FLOAT fSpaceBelow = para.GetSpaceBelow();
        if (fSpaceBelow < 0.1f)
          fSpaceBelow = 0;

        m_pBreak->SetLineStartPos(fStartPos);
        fLinePos += fSpaceBelow;
      }
    }
  }

  if (pStyle) {
    FX_FLOAT fStart = 0;
    const FDE_CSSRECT* pRect = pStyle->GetBoundaryStyles()->GetMarginWidth();
    if (pRect)
      fStart = pRect->left.GetValue();

    FX_FLOAT fTextIndent =
        pStyle->GetParagraphStyles()->GetTextIndent().GetValue();
    if (fTextIndent < 0)
      fStart -= fTextIndent;

    m_pBreak->SetLineStartPos(fStart);
    pStyle->Release();
  }
  m_iLines++;
}

void CXFA_TextLayout::RenderString(CFDE_RenderDevice* pDevice,
                                   CFDE_Brush* pBrush,
                                   CXFA_PieceLine* pPieceLine,
                                   int32_t iPiece,
                                   FXTEXT_CHARPOS* pCharPos,
                                   const CFX_Matrix& tmDoc2Device) {
  const XFA_TextPiece* pPiece = pPieceLine->m_textPieces.GetAt(iPiece);
  int32_t iCount = GetDisplayPos(pPiece, pCharPos);
  if (iCount > 0) {
    pBrush->SetColor(pPiece->dwColor);
    pDevice->DrawString(pBrush, pPiece->pFont, pCharPos, iCount,
                        pPiece->fFontSize, &tmDoc2Device);
  }
  pPieceLine->m_charCounts.Add(iCount);
}

void CXFA_TextLayout::RenderPath(CFDE_RenderDevice* pDevice,
                                 CFDE_Pen* pPen,
                                 CXFA_PieceLine* pPieceLine,
                                 int32_t iPiece,
                                 FXTEXT_CHARPOS* pCharPos,
                                 const CFX_Matrix& tmDoc2Device) {
  XFA_TextPiece* pPiece = pPieceLine->m_textPieces.GetAt(iPiece);
  bool bNoUnderline = pPiece->iUnderline < 1 || pPiece->iUnderline > 2;
  bool bNoLineThrough = pPiece->iLineThrough < 1 || pPiece->iLineThrough > 2;
  if (bNoUnderline && bNoLineThrough)
    return;

  pPen->SetColor(pPiece->dwColor);
  std::unique_ptr<CFDE_Path> pPath(new CFDE_Path);
  int32_t iChars = GetDisplayPos(pPiece, pCharPos);
  if (iChars > 0) {
    CFX_PointF pt1, pt2;
    FX_FLOAT fEndY = pCharPos[0].m_OriginY + 1.05f;
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
    for (int32_t i = 0; i < pPiece->iLineThrough; i++) {
      pt1.y = pt2.y = fEndY;
      pPath->AddLine(pt1, pt2);
      fEndY += 2.0f;
    }
  } else {
    if (bNoLineThrough &&
        (bNoUnderline || pPiece->iPeriod != XFA_ATTRIBUTEENUM_All)) {
      return;
    }
    int32_t iCharsTmp = 0;
    int32_t iPiecePrev = iPiece, iPieceNext = iPiece;
    while (iPiecePrev > 0) {
      iPiecePrev--;
      iCharsTmp = pPieceLine->m_charCounts.GetAt(iPiecePrev);
      if (iCharsTmp > 0)
        break;
    }
    if (iCharsTmp == 0)
      return;

    iCharsTmp = 0;
    int32_t iPieces = pPieceLine->m_textPieces.GetSize();
    while (iPieceNext < iPieces - 1) {
      iPieceNext++;
      iCharsTmp = pPieceLine->m_charCounts.GetAt(iPieceNext);
      if (iCharsTmp > 0)
        break;
    }
    if (iCharsTmp == 0)
      return;

    FX_FLOAT fOrgX = 0.0f;
    FX_FLOAT fEndX = 0.0f;
    pPiece = pPieceLine->m_textPieces.GetAt(iPiecePrev);
    iChars = GetDisplayPos(pPiece, pCharPos);
    if (iChars < 1)
      return;

    fOrgX = pCharPos[iChars - 1].m_OriginX +
            pCharPos[iChars - 1].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
    pPiece = pPieceLine->m_textPieces.GetAt(iPieceNext);
    iChars = GetDisplayPos(pPiece, pCharPos);
    if (iChars < 1)
      return;

    fEndX = pCharPos[0].m_OriginX;
    CFX_PointF pt1, pt2;
    pt1.x = fOrgX, pt2.x = fEndX;
    FX_FLOAT fEndY = pCharPos[0].m_OriginY + 1.05f;
    for (int32_t i = 0; i < pPiece->iUnderline; i++) {
      pt1.y = pt2.y = fEndY;
      pPath->AddLine(pt1, pt2);
      fEndY += 2.0f;
    }
    fEndY = pCharPos[0].m_OriginY - pPiece->rtPiece.height * 0.25f;
    for (int32_t i = 0; i < pPiece->iLineThrough; i++) {
      pt1.y = pt2.y = fEndY;
      pPath->AddLine(pt1, pt2);
      fEndY += 2.0f;
    }
  }
  pDevice->DrawPath(pPen, 1, pPath.get(), &tmDoc2Device);
}

int32_t CXFA_TextLayout::GetDisplayPos(const XFA_TextPiece* pPiece,
                                       FXTEXT_CHARPOS* pCharPos,
                                       bool bCharCode) {
  if (!pPiece)
    return 0;

  FX_RTFTEXTOBJ tr;
  if (!ToRun(pPiece, tr))
    return 0;
  return m_pBreak->GetDisplayPos(&tr, pCharPos, bCharCode);
}

bool CXFA_TextLayout::ToRun(const XFA_TextPiece* pPiece, FX_RTFTEXTOBJ& tr) {
  int32_t iLength = pPiece->iChars;
  if (iLength < 1)
    return false;

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
  return true;
}
