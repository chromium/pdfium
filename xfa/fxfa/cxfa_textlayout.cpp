// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_textlayout.h"

#include <math.h>

#include <algorithm>
#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/css/cfx_csscomputedstyle.h"
#include "core/fxcrt/css/cfx_cssstyleselector.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/text_char_pos.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/cfgas_linkuserdata.h"
#include "xfa/fgas/layout/cfgas_rtfbreak.h"
#include "xfa/fgas/layout/cfgas_textuserdata.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_textparser.h"
#include "xfa/fxfa/cxfa_textprovider.h"
#include "xfa/fxfa/cxfa_texttabstopscontext.h"
#include "xfa/fxfa/parser/cxfa_font.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_para.h"

namespace {

constexpr float kHeightTolerance = 0.001f;

void ProcessText(WideString* pText) {
  size_t iLen = pText->GetLength();
  if (iLen == 0) {
    return;
  }

  size_t iTrimFront = 0;
  {
    // Span's lifetime must end before ReleaseBuffer() below.
    pdfium::span<wchar_t> psz = pText->GetBuffer(iLen);
    wchar_t wPrev = 0;
    for (size_t i = 0; i < iLen; i++) {
      wchar_t wch = psz[i];
      if (wch < 0x20) {
        wch = 0x20;
      }
      if (wch == 0x20 && wPrev == 0x20) {
        continue;
      }

      wPrev = wch;
      psz[iTrimFront++] = wch;
    }
  }
  pText->ReleaseBuffer(iTrimFront);
}

}  // namespace

CXFA_TextLayout::TextPiece::TextPiece() = default;

CXFA_TextLayout::TextPiece::~TextPiece() = default;

CXFA_TextLayout::PieceLine::PieceLine() = default;

CXFA_TextLayout::PieceLine::~PieceLine() = default;

CXFA_TextLayout::LoaderContext::LoaderContext() = default;

CXFA_TextLayout::LoaderContext::~LoaderContext() = default;

void CXFA_TextLayout::LoaderContext::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(pNode);
}

CXFA_TextLayout::CXFA_TextLayout(CXFA_FFDoc* doc,
                                 CXFA_TextProvider* pTextProvider)
    : doc_(doc),
      text_provider_(pTextProvider),
      text_parser_(cppgc::MakeGarbageCollected<CXFA_TextParser>(
          doc->GetHeap()->GetAllocationHandle())) {
  DCHECK(text_provider_);
}

CXFA_TextLayout::~CXFA_TextLayout() = default;

void CXFA_TextLayout::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(doc_);
  visitor->Trace(text_provider_);
  visitor->Trace(text_data_node_);
  visitor->Trace(text_parser_);
  visitor->Trace(loader_);
}

void CXFA_TextLayout::Unload() {
  piece_lines_.clear();
  break_.reset();
}

WideString CXFA_TextLayout::GetLinkURLAtPoint(const CFX_PointF& point) {
  for (const auto& pPieceLine : piece_lines_) {
    for (const auto& pPiece : pPieceLine->text_pieces_) {
      if (pPiece->pLinkData && pPiece->rtPiece.Contains(point)) {
        return pPiece->pLinkData->GetLinkURL();
      }
    }
  }
  return WideString();
}

void CXFA_TextLayout::GetTextDataNode() {
  CXFA_Node* pNode = text_provider_->GetTextNode(&rich_text_);
  if (pNode && rich_text_) {
    text_parser_->Reset();
  }

  text_data_node_ = pNode;
}

CFX_XMLNode* CXFA_TextLayout::GetXMLContainerNode() {
  if (!rich_text_) {
    return nullptr;
  }

  CFX_XMLNode* pXMLRoot = text_data_node_->GetXMLMappingNode();
  if (!pXMLRoot) {
    return nullptr;
  }

  for (CFX_XMLNode* pXMLChild = pXMLRoot->GetFirstChild(); pXMLChild;
       pXMLChild = pXMLChild->GetNextSibling()) {
    CFX_XMLElement* pXMLElement = ToXMLElement(pXMLChild);
    if (!pXMLElement) {
      continue;
    }
    WideString wsTag = pXMLElement->GetLocalTagName();
    if (wsTag.EqualsASCII("body") || wsTag.EqualsASCII("html")) {
      return pXMLChild;
    }
  }
  return nullptr;
}

std::unique_ptr<CFGAS_RTFBreak> CXFA_TextLayout::CreateBreak(bool bDefault) {
  Mask<CFGAS_Break::LayoutStyle> dwStyle = CFGAS_Break::LayoutStyle::kExpandTab;
  if (!bDefault) {
    dwStyle |= CFGAS_Break::LayoutStyle::kPagination;
  }

  auto pBreak = std::make_unique<CFGAS_RTFBreak>(dwStyle);
  pBreak->SetLineBreakTolerance(1);
  pBreak->SetFont(text_parser_->GetFont(doc_.Get(), text_provider_, nullptr));
  pBreak->SetFontSize(text_parser_->GetFontSize(text_provider_, nullptr));
  return pBreak;
}

void CXFA_TextLayout::InitBreak(float fLineWidth) {
  CXFA_Para* para = text_provider_->GetParaIfExists();
  float fStart = 0;
  float fStartPos = 0;
  if (para) {
    CFGAS_RTFBreak::LineAlignment iAlign = CFGAS_RTFBreak::LineAlignment::Left;
    switch (para->GetHorizontalAlign()) {
      case XFA_AttributeValue::Center:
        iAlign = CFGAS_RTFBreak::LineAlignment::Center;
        break;
      case XFA_AttributeValue::Right:
        iAlign = CFGAS_RTFBreak::LineAlignment::Right;
        break;
      case XFA_AttributeValue::Justify:
        iAlign = CFGAS_RTFBreak::LineAlignment::Justified;
        break;
      case XFA_AttributeValue::JustifyAll:
        iAlign = CFGAS_RTFBreak::LineAlignment::Distributed;
        break;
      case XFA_AttributeValue::Left:
      case XFA_AttributeValue::Radix:
        break;
      default:
        NOTREACHED();
    }
    break_->SetAlignment(iAlign);

    fStart = para->GetMarginLeft();
    if (text_provider_->IsCheckButtonAndAutoWidth()) {
      if (iAlign != CFGAS_RTFBreak::LineAlignment::Left) {
        fLineWidth -= para->GetMarginRight();
      }
    } else {
      fLineWidth -= para->GetMarginRight();
    }
    if (fLineWidth < 0) {
      fLineWidth = fStart;
    }

    fStartPos = fStart;
    float fIndent = para->GetTextIndent();
    if (fIndent > 0) {
      fStartPos += fIndent;
    }
  }

  break_->SetLineBoundary(fStart, fLineWidth);
  break_->SetLineStartPos(fStartPos);

  CXFA_Font* font = text_provider_->GetFontIfExists();
  if (font) {
    break_->SetHorizontalScale(
        static_cast<int32_t>(font->GetHorizontalScale()));
    break_->SetVerticalScale(static_cast<int32_t>(font->GetVerticalScale()));
    break_->SetCharSpace(font->GetLetterSpacing());
  }

  float fFontSize = text_parser_->GetFontSize(text_provider_, nullptr);
  break_->SetFontSize(fFontSize);
  break_->SetFont(text_parser_->GetFont(doc_.Get(), text_provider_, nullptr));
  break_->SetLineBreakTolerance(fFontSize * 0.2f);
}

void CXFA_TextLayout::InitBreak(CFX_CSSComputedStyle* pStyle,
                                CFX_CSSDisplay eDisplay,
                                float fLineWidth,
                                const CFX_XMLNode* pXMLNode,
                                CFX_CSSComputedStyle* pParentStyle) {
  if (!pStyle) {
    InitBreak(fLineWidth);
    return;
  }

  if (eDisplay == CFX_CSSDisplay::Block ||
      eDisplay == CFX_CSSDisplay::ListItem) {
    CFGAS_RTFBreak::LineAlignment iAlign = CFGAS_RTFBreak::LineAlignment::Left;
    switch (pStyle->GetTextAlign()) {
      case CFX_CSSTextAlign::Right:
        iAlign = CFGAS_RTFBreak::LineAlignment::Right;
        break;
      case CFX_CSSTextAlign::Center:
        iAlign = CFGAS_RTFBreak::LineAlignment::Center;
        break;
      case CFX_CSSTextAlign::Justify:
        iAlign = CFGAS_RTFBreak::LineAlignment::Justified;
        break;
      case CFX_CSSTextAlign::JustifyAll:
        iAlign = CFGAS_RTFBreak::LineAlignment::Distributed;
        break;
      default:
        break;
    }
    break_->SetAlignment(iAlign);

    float fStart = 0;
    const CFX_CSSRect* pRect = pStyle->GetMarginWidth();
    const CFX_CSSRect* pPaddingRect = pStyle->GetPaddingWidth();
    if (pRect) {
      fStart = pRect->left.GetValue();
      fLineWidth -= pRect->right.GetValue();
      if (pPaddingRect) {
        fStart += pPaddingRect->left.GetValue();
        fLineWidth -= pPaddingRect->right.GetValue();
      }
      if (eDisplay == CFX_CSSDisplay::ListItem) {
        const CFX_CSSRect* pParRect = pParentStyle->GetMarginWidth();
        const CFX_CSSRect* pParPaddingRect = pParentStyle->GetPaddingWidth();
        if (pParRect) {
          fStart += pParRect->left.GetValue();
          fLineWidth -= pParRect->right.GetValue();
          if (pParPaddingRect) {
            fStart += pParPaddingRect->left.GetValue();
            fLineWidth -= pParPaddingRect->right.GetValue();
          }
        }
        CFX_CSSRect pNewRect;
        pNewRect.left.Set(CFX_CSSLengthUnit::Point, fStart);
        pNewRect.right.Set(CFX_CSSLengthUnit::Point, pRect->right.GetValue());
        pNewRect.top.Set(CFX_CSSLengthUnit::Point, pRect->top.GetValue());
        pNewRect.bottom.Set(CFX_CSSLengthUnit::Point, pRect->bottom.GetValue());
        pStyle->SetMarginWidth(pNewRect);
      }
    }
    break_->SetLineBoundary(fStart, fLineWidth);
    float fIndent = pStyle->GetTextIndent().GetValue();
    if (fIndent > 0) {
      fStart += fIndent;
    }

    break_->SetLineStartPos(fStart);
    break_->SetTabWidth(text_parser_->GetTabInterval(pStyle));
    if (!tabstop_context_) {
      tabstop_context_ = std::make_unique<CXFA_TextTabstopsContext>();
    }
    text_parser_->GetTabstops(pStyle, tabstop_context_.get());
    for (const auto& stop : tabstop_context_->tabstops_) {
      break_->AddPositionedTab(stop.fTabstops);
    }
  }
  float fFontSize = text_parser_->GetFontSize(text_provider_, pStyle);
  break_->SetFontSize(fFontSize);
  break_->SetLineBreakTolerance(fFontSize * 0.2f);
  break_->SetFont(text_parser_->GetFont(doc_.Get(), text_provider_, pStyle));
  break_->SetHorizontalScale(
      text_parser_->GetHorScale(text_provider_, pStyle, pXMLNode));
  break_->SetVerticalScale(text_parser_->GetVerScale(text_provider_, pStyle));
  break_->SetCharSpace(pStyle->GetLetterSpacing().GetValue());
}

float CXFA_TextLayout::GetLayoutHeight() {
  if (!loader_) {
    return 0;
  }

  if (loader_->lineHeights.empty() && loader_->fWidth > 0) {
    CFX_SizeF szMax(loader_->fWidth, loader_->fHeight);
    loader_->bSaveLineHeight = true;
    loader_->fLastPos = 0;
    CFX_SizeF szDef = CalcSize(szMax, szMax);
    loader_->bSaveLineHeight = false;
    return szDef.height;
  }

  float fHeight = loader_->fHeight;
  if (fHeight < 0.1f) {
    fHeight = 0;
    for (float value : loader_->lineHeights) {
      fHeight += value;
    }
  }
  return fHeight;
}

float CXFA_TextLayout::StartLayout(float fWidth) {
  if (!loader_) {
    loader_ = cppgc::MakeGarbageCollected<LoaderContext>(
        doc_->GetHeap()->GetAllocationHandle());
  }

  if (fWidth < 0 ||
      (loader_->fWidth > -1 && fabs(fWidth - loader_->fWidth) > 0)) {
    loader_->lineHeights.clear();
    blocks_.clear();
    Unload();
    loader_->fStartLineOffset = 0;
  }
  loader_->fWidth = fWidth;

  if (fWidth >= 0) {
    return fWidth;
  }

  CFX_SizeF szMax;

  loader_->bSaveLineHeight = true;
  loader_->fLastPos = 0;
  CFX_SizeF szDef = CalcSize(szMax, szMax);
  loader_->bSaveLineHeight = false;
  return szDef.width;
}

float CXFA_TextLayout::DoLayout(float fTextHeight) {
  if (!loader_) {
    return fTextHeight;
  }

  UpdateLoaderHeight(fTextHeight);
  return fTextHeight;
}

float CXFA_TextLayout::DoSplitLayout(size_t szBlockIndex,
                                     float fCalcHeight,
                                     float fTextHeight) {
  if (!loader_) {
    return fCalcHeight;
  }

  UpdateLoaderHeight(fTextHeight);

  if (fCalcHeight < 0) {
    return fCalcHeight;
  }

  has_block_ = true;
  if (blocks_.empty() && loader_->fHeight > 0) {
    float fHeight = fTextHeight - GetLayoutHeight();
    if (fHeight > 0) {
      XFA_AttributeValue iAlign = text_parser_->GetVAlign(text_provider_);
      if (iAlign == XFA_AttributeValue::Middle) {
        fHeight /= 2.0f;
      } else if (iAlign != XFA_AttributeValue::Bottom) {
        fHeight = 0;
      }
      loader_->fStartLineOffset = fHeight;
    }
  }

  float fLinePos = loader_->fStartLineOffset;
  size_t szLineIndex = 0;
  if (!blocks_.empty()) {
    if (szBlockIndex < blocks_.size()) {
      szLineIndex = blocks_[szBlockIndex].szIndex;
    } else {
      szLineIndex = GetNextIndexFromLastBlockData();
    }
    for (size_t i = 0; i < std::min(szBlockIndex, loader_->blockHeights.size());
         ++i) {
      fLinePos -= loader_->blockHeights[i].fHeight;
    }
  }

  if (szLineIndex >= loader_->lineHeights.size()) {
    return fCalcHeight;
  }

  if (loader_->lineHeights[szLineIndex] - fCalcHeight > kHeightTolerance) {
    return 0;
  }

  for (size_t i = szLineIndex; i < loader_->lineHeights.size(); ++i) {
    float fLineHeight = loader_->lineHeights[i];
    if (fLinePos + fLineHeight - fCalcHeight <= kHeightTolerance) {
      fLinePos += fLineHeight;
      continue;
    }

    if (szBlockIndex < blocks_.size()) {
      blocks_[szBlockIndex] = {szLineIndex, i - szLineIndex};
    } else {
      blocks_.push_back({szLineIndex, i - szLineIndex});
    }

    if (i != szLineIndex) {
      return fLinePos;
    }

    if (fCalcHeight > fLinePos) {
      return fCalcHeight;
    }

    if (szBlockIndex < loader_->blockHeights.size() &&
        loader_->blockHeights[szBlockIndex].szBlockIndex == szBlockIndex) {
      loader_->blockHeights[szBlockIndex].fHeight = fCalcHeight;
    } else {
      loader_->blockHeights.push_back({szBlockIndex, fCalcHeight});
    }
    return fCalcHeight;
  }
  return fCalcHeight;
}

size_t CXFA_TextLayout::CountBlocks() const {
  size_t szCount = blocks_.size();
  return szCount > 0 ? szCount : 1;
}

size_t CXFA_TextLayout::GetNextIndexFromLastBlockData() const {
  return blocks_.back().szIndex + blocks_.back().szLength;
}

void CXFA_TextLayout::UpdateLoaderHeight(float fTextHeight) {
  loader_->fHeight = fTextHeight;
  if (loader_->fHeight < 0) {
    loader_->fHeight = GetLayoutHeight();
  }
}

CFX_SizeF CXFA_TextLayout::CalcSize(const CFX_SizeF& minSize,
                                    const CFX_SizeF& maxSize) {
  float width = maxSize.width;
  if (width < 1) {
    width = 0xFFFF;
  }

  break_ = CreateBreak(false);
  float fLinePos = 0;
  lines_ = 0;
  max_width_ = 0;
  Loader(width, &fLinePos, false);
  if (fLinePos < 0.1f) {
    fLinePos = text_parser_->GetFontSize(text_provider_, nullptr);
  }

  tabstop_context_.reset();
  return CFX_SizeF(max_width_, fLinePos);
}

float CXFA_TextLayout::Layout(const CFX_SizeF& size) {
  if (size.width < 1) {
    return 0.f;
  }

  Unload();
  break_ = CreateBreak(true);
  if (loader_) {
    loader_->iTotalLines = -1;
    loader_->nCharIdx = 0;
  }

  lines_ = 0;
  float fLinePos = 0;
  Loader(size.width, &fLinePos, true);
  UpdateAlign(size.height, fLinePos);
  tabstop_context_.reset();
  return fLinePos;
}

bool CXFA_TextLayout::LayoutInternal(size_t szBlockIndex) {
  DCHECK(szBlockIndex < CountBlocks());

  if (!loader_ || loader_->fWidth < 1) {
    return false;
  }

  loader_->iTotalLines = -1;
  lines_ = 0;
  float fLinePos = 0;
  CXFA_Node* pNode = nullptr;
  CFX_SizeF szText(loader_->fWidth, loader_->fHeight);
  if (szBlockIndex < loader_->blockHeights.size()) {
    return true;
  }
  if (szBlockIndex == loader_->blockHeights.size()) {
    Unload();
    break_ = CreateBreak(true);
    fLinePos = loader_->fStartLineOffset;
    for (size_t i = 0; i < loader_->blockHeights.size(); ++i) {
      fLinePos -= loader_->blockHeights[i].fHeight;
    }

    loader_->nCharIdx = 0;
    if (!blocks_.empty()) {
      loader_->iTotalLines =
          pdfium::checked_cast<int32_t>(blocks_[szBlockIndex].szLength);
    }
    Loader(szText.width, &fLinePos, true);
    if (blocks_.empty() && loader_->fStartLineOffset < 0.1f) {
      UpdateAlign(szText.height, fLinePos);
    }
  } else if (text_data_node_) {
    if (!blocks_.empty() && szBlockIndex < blocks_.size() - 1) {
      loader_->iTotalLines =
          pdfium::checked_cast<int32_t>(blocks_[szBlockIndex].szLength);
    }
    break_->Reset();
    if (rich_text_) {
      CFX_XMLNode* pContainerNode = GetXMLContainerNode();
      if (!pContainerNode) {
        return true;
      }

      const CFX_XMLNode* pXMLNode = loader_->pXMLNode;
      if (!pXMLNode) {
        return true;
      }

      const CFX_XMLNode* pSaveXMLNode = pXMLNode;
      for (; pXMLNode; pXMLNode = pXMLNode->GetNextSibling()) {
        if (!LoadRichText(pXMLNode, szText.width, &fLinePos,
                          loader_->pParentStyle, true, nullptr, true, false,
                          0)) {
          break;
        }
      }
      while (!pXMLNode) {
        pXMLNode = pSaveXMLNode->GetParent();
        if (pXMLNode == pContainerNode) {
          break;
        }
        if (!LoadRichText(pXMLNode, szText.width, &fLinePos,
                          loader_->pParentStyle, true, nullptr, false, false,
                          0)) {
          break;
        }
        pSaveXMLNode = pXMLNode;
        pXMLNode = pXMLNode->GetNextSibling();
        if (!pXMLNode) {
          continue;
        }
        for (; pXMLNode; pXMLNode = pXMLNode->GetNextSibling()) {
          if (!LoadRichText(pXMLNode, szText.width, &fLinePos,
                            loader_->pParentStyle, true, nullptr, true, false,
                            0)) {
            break;
          }
        }
      }
    } else {
      pNode = loader_->pNode.Get();
      if (!pNode) {
        return true;
      }
      LoadText(pNode, szText.width, &fLinePos, true);
    }
  }
  return true;
}

void CXFA_TextLayout::ItemBlocks(const CFX_RectF& rtText, size_t szBlockIndex) {
  if (!loader_) {
    return;
  }

  if (loader_->lineHeights.empty()) {
    return;
  }

  float fLinePos = loader_->fStartLineOffset;
  size_t szLineIndex = 0;
  if (szBlockIndex > 0) {
    if (szBlockIndex <= loader_->blockHeights.size()) {
      for (size_t i = 0; i < szBlockIndex; ++i) {
        fLinePos -= loader_->blockHeights[i].fHeight;
      }
    } else {
      fLinePos = 0;
    }
    szLineIndex = GetNextIndexFromLastBlockData();
  }

  size_t i;
  for (i = szLineIndex; i < loader_->lineHeights.size(); ++i) {
    float fLineHeight = loader_->lineHeights[i];
    if (fLinePos + fLineHeight - rtText.height > kHeightTolerance) {
      blocks_.push_back({szLineIndex, i - szLineIndex});
      return;
    }
    fLinePos += fLineHeight;
  }
  if (i > szLineIndex) {
    blocks_.push_back({szLineIndex, i - szLineIndex});
  }
}

bool CXFA_TextLayout::DrawString(CFX_RenderDevice* pFxDevice,
                                 const CFX_Matrix& mtDoc2Device,
                                 const CFX_RectF& rtClip,
                                 size_t szBlockIndex) {
  if (!pFxDevice) {
    return false;
  }

  pFxDevice->SaveState();
  pFxDevice->SetClip_Rect(rtClip.GetOuterRect());

  if (piece_lines_.empty()) {
    size_t szBlockCount = CountBlocks();
    for (size_t i = 0; i < szBlockCount; ++i) {
      LayoutInternal(i);
    }
    tabstop_context_.reset();
    loader_.Clear();
  }

  std::vector<TextCharPos> char_pos(1);
  size_t szLineStart = 0;
  size_t szPieceLines = piece_lines_.size();
  if (!blocks_.empty()) {
    if (szBlockIndex < blocks_.size()) {
      szLineStart = blocks_[szBlockIndex].szIndex;
      szPieceLines = blocks_[szBlockIndex].szLength;
    } else {
      szPieceLines = 0;
    }
  }

  for (size_t i = 0; i < szPieceLines; ++i) {
    if (i + szLineStart >= piece_lines_.size()) {
      break;
    }

    PieceLine* pPieceLine = piece_lines_[i + szLineStart].get();
    for (size_t j = 0; j < pPieceLine->text_pieces_.size(); ++j) {
      const TextPiece* pPiece = pPieceLine->text_pieces_[j].get();
      int32_t iChars = pPiece->iChars;
      if (fxcrt::CollectionSize<int32_t>(char_pos) < iChars) {
        char_pos.resize(iChars);
      }
      RenderString(pFxDevice, pPieceLine, j, char_pos, mtDoc2Device);
    }
    for (size_t j = 0; j < pPieceLine->text_pieces_.size(); ++j) {
      RenderPath(pFxDevice, pPieceLine, j, char_pos, mtDoc2Device);
    }
  }
  pFxDevice->RestoreState(false);
  return szPieceLines > 0;
}

void CXFA_TextLayout::UpdateAlign(float fHeight, float fBottom) {
  fHeight -= fBottom;
  if (fHeight < 0.1f) {
    return;
  }

  switch (text_parser_->GetVAlign(text_provider_)) {
    case XFA_AttributeValue::Middle:
      fHeight /= 2.0f;
      break;
    case XFA_AttributeValue::Bottom:
      break;
    default:
      return;
  }

  for (const auto& pPieceLine : piece_lines_) {
    for (const auto& pPiece : pPieceLine->text_pieces_) {
      pPiece->rtPiece.top += fHeight;
    }
  }
}

void CXFA_TextLayout::Loader(float textWidth,
                             float* pLinePos,
                             bool bSavePieces) {
  GetTextDataNode();
  if (!text_data_node_) {
    return;
  }

  if (!rich_text_) {
    LoadText(text_data_node_, textWidth, pLinePos, bSavePieces);
    return;
  }

  const CFX_XMLNode* pXMLContainer = GetXMLContainerNode();
  if (!pXMLContainer) {
    return;
  }

  if (!text_parser_->IsParsed()) {
    text_parser_->DoParse(pXMLContainer, text_provider_);
  }

  auto pRootStyle = text_parser_->CreateRootStyle(text_provider_);
  LoadRichText(pXMLContainer, textWidth, pLinePos, std::move(pRootStyle),
               bSavePieces, nullptr, true, false, 0);
}

void CXFA_TextLayout::LoadText(CXFA_Node* pNode,
                               float textWidth,
                               float* pLinePos,
                               bool bSavePieces) {
  InitBreak(textWidth);

  CXFA_Para* para = text_provider_->GetParaIfExists();
  float fSpaceAbove = 0;
  if (para) {
    fSpaceAbove = para->GetSpaceAbove();
    if (fSpaceAbove < 0.1f) {
      fSpaceAbove = 0;
    }

    switch (para->GetVerticalAlign()) {
      case XFA_AttributeValue::Top:
      case XFA_AttributeValue::Middle:
      case XFA_AttributeValue::Bottom: {
        *pLinePos += fSpaceAbove;
        break;
      }
      default:
        NOTREACHED();
    }
  }

  WideString wsText = pNode->JSObject()->GetContent(false);
  wsText.TrimBack(L" ");
  bool bRet = AppendChar(wsText, pLinePos, fSpaceAbove, bSavePieces);
  if (bRet && loader_) {
    loader_->pNode = pNode;
  } else {
    EndBreak(CFGAS_Char::BreakType::kParagraph, pLinePos, bSavePieces);
  }
}

bool CXFA_TextLayout::LoadRichText(const CFX_XMLNode* pXMLNode,
                                   float textWidth,
                                   float* pLinePos,
                                   RetainPtr<CFX_CSSComputedStyle> pParentStyle,
                                   bool bSavePieces,
                                   RetainPtr<CFGAS_LinkUserData> pLinkData,
                                   bool bEndBreak,
                                   bool bIsOl,
                                   int32_t iLiCount) {
  if (!pXMLNode) {
    return false;
  }

  CXFA_TextParser::Context* pContext =
      text_parser_->GetParseContextFromMap(pXMLNode);
  CFX_CSSDisplay eDisplay = CFX_CSSDisplay::None;
  bool bContentNode = false;
  float fSpaceBelow = 0;
  RetainPtr<CFX_CSSComputedStyle> pStyle;
  WideString wsName;
  if (bEndBreak) {
    bool bCurOl = false;
    bool bCurLi = false;
    const CFX_XMLElement* pElement = nullptr;
    if (pContext) {
      if (pXMLNode->GetType() == CFX_XMLNode::Type::kText) {
        bContentNode = true;
      } else if (pXMLNode->GetType() == CFX_XMLNode::Type::kElement) {
        pElement = static_cast<const CFX_XMLElement*>(pXMLNode);
        wsName = pElement->GetLocalTagName();
      }
      if (wsName.EqualsASCII("ol")) {
        bIsOl = true;
        bCurOl = true;
      }

      eDisplay = pContext->GetDisplay();
      if (eDisplay != CFX_CSSDisplay::Block &&
          eDisplay != CFX_CSSDisplay::Inline &&
          eDisplay != CFX_CSSDisplay::ListItem) {
        return true;
      }

      pStyle = text_parser_->ComputeStyle(pXMLNode, pParentStyle);
      InitBreak(bContentNode ? pParentStyle.Get() : pStyle.Get(), eDisplay,
                textWidth, pXMLNode, pParentStyle.Get());
      if ((eDisplay == CFX_CSSDisplay::Block ||
           eDisplay == CFX_CSSDisplay::ListItem) &&
          pStyle &&
          (wsName.IsEmpty() ||
           !(wsName.EqualsASCII("body") || wsName.EqualsASCII("html") ||
             wsName.EqualsASCII("ol") || wsName.EqualsASCII("ul")))) {
        const CFX_CSSRect* pRect = pStyle->GetMarginWidth();
        if (pRect) {
          *pLinePos += pRect->top.GetValue();
          fSpaceBelow = pRect->bottom.GetValue();
        }
      }

      if (wsName.EqualsASCII("a")) {
        WideString wsLinkContent = pElement->GetAttribute(L"href");
        if (!wsLinkContent.IsEmpty()) {
          pLinkData = pdfium::MakeRetain<CFGAS_LinkUserData>(wsLinkContent);
        }
      }

      int32_t iTabCount = text_parser_->CountTabs(
          bContentNode ? pParentStyle.Get() : pStyle.Get());
      bool bSpaceRun = text_parser_->IsSpaceRun(
          bContentNode ? pParentStyle.Get() : pStyle.Get());
      WideString wsText;
      if (bContentNode && iTabCount == 0) {
        wsText = ToXMLText(pXMLNode)->GetText();
      } else if (wsName.EqualsASCII("br")) {
        wsText = WideString(L'\n');
      } else if (wsName.EqualsASCII("li")) {
        bCurLi = true;
        if (bIsOl) {
          wsText = WideString::Format(L"%d.  ", iLiCount);
        } else {
          wsText = 0x00B7 + WideStringView(L" ");
        }
      } else if (!bContentNode) {
        if (iTabCount > 0) {
          while (iTabCount-- > 0) {
            wsText += L'\t';
          }
        } else {
          std::optional<WideString> obj =
              text_parser_->GetEmbeddedObj(text_provider_, pXMLNode);
          if (obj.has_value()) {
            wsText = obj.value();
          }
        }
      }

      if (!wsText.IsEmpty() && bContentNode && !bSpaceRun) {
        ProcessText(&wsText);
      }

      if (loader_) {
        if (wsText.GetLength() > 0 && loader_->bFilterSpace) {
          wsText.TrimFront(L" ");
        }
        if (CFX_CSSDisplay::Block == eDisplay) {
          loader_->bFilterSpace = true;
        } else if (CFX_CSSDisplay::Inline == eDisplay &&
                   loader_->bFilterSpace) {
          loader_->bFilterSpace = false;
        } else if (wsText.GetLength() > 0 && wsText.Back() == 0x20) {
          loader_->bFilterSpace = true;
        } else if (wsText.GetLength() != 0) {
          loader_->bFilterSpace = false;
        }
      }

      if (wsText.GetLength() > 0) {
        if (!loader_ || loader_->nCharIdx == 0) {
          auto pUserData = pdfium::MakeRetain<CFGAS_TextUserData>(
              bContentNode ? pParentStyle : pStyle, pLinkData);
          break_->SetUserData(pUserData);
        }

        if (AppendChar(wsText, pLinePos, 0, bSavePieces)) {
          if (loader_) {
            loader_->bFilterSpace = false;
          }
          if (!IsEnd(bSavePieces)) {
            return true;
          }
          if (loader_ && loader_->iTotalLines > -1) {
            loader_->pXMLNode = pXMLNode;
            loader_->pParentStyle = pParentStyle;
          }
          return false;
        }
      }
    }

    for (CFX_XMLNode* pChildNode = pXMLNode->GetFirstChild(); pChildNode;
         pChildNode = pChildNode->GetNextSibling()) {
      if (bCurOl) {
        iLiCount++;
      }

      if (!LoadRichText(pChildNode, textWidth, pLinePos,
                        pContext ? pStyle : pParentStyle, bSavePieces,
                        pLinkData, true, bIsOl, iLiCount)) {
        return false;
      }
    }

    if (loader_) {
      if (CFX_CSSDisplay::Block == eDisplay) {
        loader_->bFilterSpace = true;
      }
    }
    if (bCurLi) {
      EndBreak(CFGAS_Char::BreakType::kLine, pLinePos, bSavePieces);
    }
  } else {
    if (pContext) {
      eDisplay = pContext->GetDisplay();
    }
  }

  if (!pContext || bContentNode) {
    return true;
  }

  CFGAS_Char::BreakType dwStatus = (eDisplay == CFX_CSSDisplay::Block)
                                       ? CFGAS_Char::BreakType::kParagraph
                                       : CFGAS_Char::BreakType::kPiece;
  EndBreak(dwStatus, pLinePos, bSavePieces);
  if (eDisplay == CFX_CSSDisplay::Block) {
    *pLinePos += fSpaceBelow;
    if (tabstop_context_) {
      tabstop_context_->RemoveAll();
    }
  }
  if (!IsEnd(bSavePieces)) {
    return true;
  }

  if (loader_ && loader_->iTotalLines > -1) {
    loader_->pXMLNode = pXMLNode->GetNextSibling();
    loader_->pParentStyle = pParentStyle;
  }
  return false;
}

bool CXFA_TextLayout::AppendChar(const WideString& wsText,
                                 float* pLinePos,
                                 float fSpaceAbove,
                                 bool bSavePieces) {
  CFGAS_Char::BreakType dwStatus = CFGAS_Char::BreakType::kNone;
  size_t iChar = loader_ ? loader_->nCharIdx : 0;
  size_t iLength = wsText.GetLength();
  for (size_t i = iChar; i < iLength; i++) {
    wchar_t wch = wsText[i];
    if (wch == 0xA0) {
      wch = 0x20;
    }

    dwStatus = break_->AppendChar(wch);
    if (dwStatus != CFGAS_Char::BreakType::kNone &&
        dwStatus != CFGAS_Char::BreakType::kPiece) {
      AppendTextLine(dwStatus, pLinePos, bSavePieces, false);
      if (IsEnd(bSavePieces)) {
        if (loader_) {
          loader_->nCharIdx = i;
        }
        return true;
      }
      if (dwStatus == CFGAS_Char::BreakType::kParagraph && rich_text_) {
        *pLinePos += fSpaceAbove;
      }
    }
  }
  if (loader_) {
    loader_->nCharIdx = 0;
  }

  return false;
}

bool CXFA_TextLayout::IsEnd(bool bSavePieces) {
  if (!bSavePieces) {
    return false;
  }
  if (loader_ && loader_->iTotalLines > 0) {
    return lines_ >= loader_->iTotalLines;
  }
  return false;
}

void CXFA_TextLayout::EndBreak(CFGAS_Char::BreakType dwStatus,
                               float* pLinePos,
                               bool bSavePieces) {
  dwStatus = break_->EndBreak(dwStatus);
  if (dwStatus != CFGAS_Char::BreakType::kNone &&
      dwStatus != CFGAS_Char::BreakType::kPiece) {
    AppendTextLine(dwStatus, pLinePos, bSavePieces, true);
  }
}

void CXFA_TextLayout::DoTabstops(CFX_CSSComputedStyle* pStyle,
                                 PieceLine* pPieceLine) {
  if (!pStyle || !pPieceLine) {
    return;
  }

  if (!tabstop_context_ || tabstop_context_->tabstops_.empty()) {
    return;
  }

  int32_t iPieces = fxcrt::CollectionSize<int32_t>(pPieceLine->text_pieces_);
  if (iPieces == 0) {
    return;
  }

  TextPiece* pPiece = pPieceLine->text_pieces_[iPieces - 1].get();
  int32_t& iTabstopsIndex = tabstop_context_->tab_index_;
  int32_t iCount = text_parser_->CountTabs(pStyle);
  if (!fxcrt::IndexInBounds(tabstop_context_->tabstops_, iTabstopsIndex)) {
    return;
  }

  if (iCount > 0) {
    iTabstopsIndex++;
    tabstop_context_->has_tabstops_ = true;
    float fRight = 0;
    if (iPieces > 1) {
      const TextPiece* p = pPieceLine->text_pieces_[iPieces - 2].get();
      fRight = p->rtPiece.right();
    }
    tabstop_context_->tab_width_ =
        pPiece->rtPiece.width + pPiece->rtPiece.left - fRight;
  } else if (iTabstopsIndex > -1) {
    float fLeft = 0;
    if (tabstop_context_->has_tabstops_) {
      uint32_t dwAlign = tabstop_context_->tabstops_[iTabstopsIndex].dwAlign;
      if (dwAlign == FX_HashCode_GetW(L"center")) {
        fLeft = pPiece->rtPiece.width / 2.0f;
      } else if (dwAlign == FX_HashCode_GetW(L"right") ||
                 dwAlign == FX_HashCode_GetW(L"before")) {
        fLeft = pPiece->rtPiece.width;
      } else if (dwAlign == FX_HashCode_GetW(L"decimal")) {
        int32_t iChars = pPiece->iChars;
        for (int32_t i = 0; i < iChars; i++) {
          if (pPiece->szText[i] == L'.') {
            break;
          }

          fLeft += pPiece->Widths[i] / 20000.0f;
        }
      }
      tabstop_context_->left_ = std::min(fLeft, tabstop_context_->tab_width_);
      tabstop_context_->has_tabstops_ = false;
      tabstop_context_->tab_width_ = 0;
    }
    pPiece->rtPiece.left -= tabstop_context_->left_;
  }
}

void CXFA_TextLayout::AppendTextLine(CFGAS_Char::BreakType dwStatus,
                                     float* pLinePos,
                                     bool bSavePieces,
                                     bool bEndBreak) {
  int32_t iPieces = break_->CountBreakPieces();
  if (iPieces < 1) {
    return;
  }

  RetainPtr<CFX_CSSComputedStyle> pStyle;
  if (bSavePieces) {
    auto pNew = std::make_unique<PieceLine>();
    PieceLine* pPieceLine = pNew.get();
    piece_lines_.push_back(std::move(pNew));
    if (tabstop_context_) {
      tabstop_context_->Reset();
    }

    float fLineStep = 0;
    float fBaseLine = 0;
    int32_t i = 0;
    for (i = 0; i < iPieces; i++) {
      const CFGAS_BreakPiece* pPiece = break_->GetBreakPieceUnstable(i);
      const CFGAS_TextUserData* pUserData = pPiece->GetUserData();
      if (pUserData) {
        pStyle = pUserData->style_;
      }
      float fVerScale = pPiece->GetVerticalScale() / 100.0f;

      auto pTP = std::make_unique<TextPiece>();
      pTP->iChars = pPiece->GetCharCount();
      pTP->szText = pPiece->GetString();
      pTP->Widths = pPiece->GetWidths();
      pTP->iBidiLevel = pPiece->GetBidiLevel();
      pTP->iHorScale = pPiece->GetHorizontalScale();
      pTP->iVerScale = pPiece->GetVerticalScale();
      pTP->iUnderline =
          text_parser_->GetUnderline(text_provider_, pStyle.Get());
      pTP->iPeriod =
          text_parser_->GetUnderlinePeriod(text_provider_, pStyle.Get());
      pTP->iLineThrough =
          text_parser_->GetLinethrough(text_provider_, pStyle.Get());
      pTP->dwColor = text_parser_->GetColor(text_provider_, pStyle.Get());
      pTP->pFont =
          text_parser_->GetFont(doc_.Get(), text_provider_, pStyle.Get());
      pTP->fFontSize = text_parser_->GetFontSize(text_provider_, pStyle.Get());
      pTP->rtPiece.left = pPiece->GetStartPos() / 20000.0f;
      pTP->rtPiece.width = pPiece->GetWidth() / 20000.0f;
      pTP->rtPiece.height =
          static_cast<float>(pPiece->GetFontSize()) * fVerScale / 20.0f;
      float fBaseLineTemp =
          text_parser_->GetBaseline(text_provider_, pStyle.Get());
      pTP->rtPiece.top = fBaseLineTemp;

      float fLineHeight = text_parser_->GetLineHeight(
          text_provider_, pStyle.Get(), lines_ == 0, fVerScale);
      if (fBaseLineTemp > 0) {
        float fLineHeightTmp = fBaseLineTemp + pTP->rtPiece.height;
        if (fLineHeight < fLineHeightTmp) {
          fLineHeight = fLineHeightTmp;
        }
      }
      fLineStep = std::max(fLineStep, fLineHeight);
      pTP->pLinkData = pUserData ? pUserData->link_data_ : nullptr;
      pPieceLine->text_pieces_.push_back(std::move(pTP));
      DoTabstops(pStyle.Get(), pPieceLine);
    }
    for (const auto& pTP : pPieceLine->text_pieces_) {
      float& fTop = pTP->rtPiece.top;
      float fBaseLineTemp = fTop;
      fTop = *pLinePos + fLineStep - pTP->rtPiece.height - fBaseLineTemp;
      fTop = std::max(0.0f, fTop);
    }
    *pLinePos += fLineStep + fBaseLine;
  } else {
    float fLineStep = 0;
    float fLineWidth = 0;
    for (int32_t i = 0; i < iPieces; i++) {
      const CFGAS_BreakPiece* pPiece = break_->GetBreakPieceUnstable(i);
      const CFGAS_TextUserData* pUserData = pPiece->GetUserData();
      if (pUserData) {
        pStyle = pUserData->style_;
      }
      float fVerScale = pPiece->GetVerticalScale() / 100.0f;
      float fBaseLine = text_parser_->GetBaseline(text_provider_, pStyle.Get());
      float fLineHeight = text_parser_->GetLineHeight(
          text_provider_, pStyle.Get(), lines_ == 0, fVerScale);
      if (fBaseLine > 0) {
        float fLineHeightTmp =
            fBaseLine +
            static_cast<float>(pPiece->GetFontSize()) * fVerScale / 20.0f;
        if (fLineHeight < fLineHeightTmp) {
          fLineHeight = fLineHeightTmp;
        }
      }
      fLineStep = std::max(fLineStep, fLineHeight);
      fLineWidth += pPiece->GetWidth() / 20000.0f;
    }
    *pLinePos += fLineStep;
    max_width_ = std::max(max_width_, fLineWidth);
    if (loader_ && loader_->bSaveLineHeight) {
      float fHeight = *pLinePos - loader_->fLastPos;
      loader_->fLastPos = *pLinePos;
      loader_->lineHeights.push_back(fHeight);
    }
  }

  break_->ClearBreakPieces();
  if (dwStatus == CFGAS_Char::BreakType::kParagraph) {
    break_->Reset();
    if (!pStyle && bEndBreak) {
      CXFA_Para* para = text_provider_->GetParaIfExists();
      if (para) {
        float fStartPos = para->GetMarginLeft();
        float fIndent = para->GetTextIndent();
        if (fIndent > 0) {
          fStartPos += fIndent;
        }

        float fSpaceBelow = para->GetSpaceBelow();
        if (fSpaceBelow < 0.1f) {
          fSpaceBelow = 0;
        }

        break_->SetLineStartPos(fStartPos);
        *pLinePos += fSpaceBelow;
      }
    }
  }

  if (pStyle) {
    float fStart = 0;
    const CFX_CSSRect* pRect = pStyle->GetMarginWidth();
    if (pRect) {
      fStart = pRect->left.GetValue();
    }

    float fTextIndent = pStyle->GetTextIndent().GetValue();
    if (fTextIndent < 0) {
      fStart -= fTextIndent;
    }

    break_->SetLineStartPos(fStart);
  }
  lines_++;
}

void CXFA_TextLayout::RenderString(CFX_RenderDevice* pDevice,
                                   PieceLine* pPieceLine,
                                   size_t szPiece,
                                   pdfium::span<TextCharPos> pCharPos,
                                   const CFX_Matrix& mtDoc2Device) {
  const TextPiece* pPiece = pPieceLine->text_pieces_[szPiece].get();
  size_t szCount = GetDisplayPos(pPiece, pCharPos);
  if (szCount > 0) {
    CFDE_TextOut::DrawString(pDevice, pPiece->dwColor, pPiece->pFont,
                             pCharPos.first(szCount), pPiece->fFontSize,
                             mtDoc2Device);
  }
  pPieceLine->char_counts_.push_back(szCount);
}

void CXFA_TextLayout::RenderPath(CFX_RenderDevice* pDevice,
                                 const PieceLine* pPieceLine,
                                 size_t szPiece,
                                 pdfium::span<TextCharPos> pCharPos,
                                 const CFX_Matrix& mtDoc2Device) {
  const TextPiece* pPiece = pPieceLine->text_pieces_[szPiece].get();
  bool bNoUnderline = pPiece->iUnderline < 1 || pPiece->iUnderline > 2;
  bool bNoLineThrough = pPiece->iLineThrough < 1 || pPiece->iLineThrough > 2;
  if (bNoUnderline && bNoLineThrough) {
    return;
  }

  CFX_Path path;
  size_t szChars = GetDisplayPos(pPiece, pCharPos);
  if (szChars > 0) {
    CFX_PointF pt1;
    CFX_PointF pt2;
    float fEndY = pCharPos[0].m_Origin.y + 1.05f;
    if (pPiece->iPeriod == XFA_AttributeValue::Word) {
      for (int32_t i = 0; i < pPiece->iUnderline; i++) {
        for (size_t j = 0; j < szChars; j++) {
          pt1.x = pCharPos[j].m_Origin.x;
          pt2.x =
              pt1.x + pCharPos[j].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
          pt1.y = pt2.y = fEndY;
          path.AppendLine(pt1, pt2);
        }
        fEndY += 2.0f;
      }
    } else {
      pt1.x = pCharPos[0].m_Origin.x;
      pt2.x =
          pCharPos[szChars - 1].m_Origin.x +
          pCharPos[szChars - 1].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
      for (int32_t i = 0; i < pPiece->iUnderline; i++) {
        pt1.y = pt2.y = fEndY;
        path.AppendLine(pt1, pt2);
        fEndY += 2.0f;
      }
    }
    fEndY = pCharPos[0].m_Origin.y - pPiece->rtPiece.height * 0.25f;
    pt1.x = pCharPos[0].m_Origin.x;
    pt2.x = pCharPos[szChars - 1].m_Origin.x +
            pCharPos[szChars - 1].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
    for (int32_t i = 0; i < pPiece->iLineThrough; i++) {
      pt1.y = pt2.y = fEndY;
      path.AppendLine(pt1, pt2);
      fEndY += 2.0f;
    }
  } else {
    if (bNoLineThrough &&
        (bNoUnderline || pPiece->iPeriod != XFA_AttributeValue::All)) {
      return;
    }
    bool bHasCount = false;
    size_t szPiecePrev = szPiece;
    size_t szPieceNext = szPiece;
    while (szPiecePrev > 0) {
      szPiecePrev--;
      if (pPieceLine->char_counts_[szPiecePrev] > 0) {
        bHasCount = true;
        break;
      }
    }
    if (!bHasCount) {
      return;
    }

    bHasCount = false;
    while (szPieceNext + 1 < pPieceLine->text_pieces_.size()) {
      ++szPieceNext;
      if (pPieceLine->char_counts_[szPieceNext] > 0) {
        bHasCount = true;
        break;
      }
    }
    if (!bHasCount) {
      return;
    }

    float fOrgX = 0.0f;
    float fEndX = 0.0f;
    pPiece = pPieceLine->text_pieces_[szPiecePrev].get();
    szChars = GetDisplayPos(pPiece, pCharPos);
    if (szChars < 1) {
      return;
    }

    fOrgX = pCharPos[szChars - 1].m_Origin.x +
            pCharPos[szChars - 1].m_FontCharWidth * pPiece->fFontSize / 1000.0f;
    pPiece = pPieceLine->text_pieces_[szPieceNext].get();
    szChars = GetDisplayPos(pPiece, pCharPos);
    if (szChars < 1) {
      return;
    }

    fEndX = pCharPos[0].m_Origin.x;
    CFX_PointF pt1;
    CFX_PointF pt2;
    pt1.x = fOrgX;
    pt2.x = fEndX;
    float fEndY = pCharPos[0].m_Origin.y + 1.05f;
    for (int32_t i = 0; i < pPiece->iUnderline; i++) {
      pt1.y = fEndY;
      pt2.y = fEndY;
      path.AppendLine(pt1, pt2);
      fEndY += 2.0f;
    }
    fEndY = pCharPos[0].m_Origin.y - pPiece->rtPiece.height * 0.25f;
    for (int32_t i = 0; i < pPiece->iLineThrough; i++) {
      pt1.y = fEndY;
      pt2.y = fEndY;
      path.AppendLine(pt1, pt2);
      fEndY += 2.0f;
    }
  }

  const CFX_GraphStateData graph_state;
  pDevice->DrawPath(path, &mtDoc2Device, &graph_state, 0, pPiece->dwColor,
                    CFX_FillRenderOptions());
}

size_t CXFA_TextLayout::GetDisplayPos(const TextPiece* pPiece,
                                      pdfium::span<TextCharPos> pCharPos) {
  if (!pPiece || pPiece->iChars < 1) {
    return 0;
  }
  return break_->GetDisplayPos(pPiece, pCharPos);
}
