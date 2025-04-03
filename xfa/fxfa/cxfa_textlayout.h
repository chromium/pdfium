// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_TEXTLAYOUT_H_
#define XFA_FXFA_CXFA_TEXTLAYOUT_H_

#include <memory>
#include <vector>

#include "core/fxcrt/css/cfx_css.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "core/fxge/dib/fx_dib.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fgas/layout/cfgas_char.h"
#include "xfa/fgas/layout/cfgas_textpiece.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFGAS_LinkUserData;
class CFGAS_RTFBreak;
class CFX_CSSComputedStyle;
class CFX_RenderDevice;
class CFX_XMLNode;
class CXFA_FFDoc;
class CXFA_Node;
class CXFA_TextParser;
class CXFA_TextProvider;
class CXFA_TextTabstopsContext;
class TextCharPos;

class CXFA_TextLayout final : public cppgc::GarbageCollected<CXFA_TextLayout> {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_TextLayout();

  void Trace(cppgc::Visitor* visitor) const;

  float GetLayoutHeight();
  float StartLayout(float fWidth);
  float DoLayout(float fTextHeight);
  float DoSplitLayout(size_t szBlockIndex,
                      float fCalcHeight,
                      float fTextHeight);
  float Layout(const CFX_SizeF& size);

  CFX_SizeF CalcSize(const CFX_SizeF& minSize, const CFX_SizeF& maxSize);
  void ItemBlocks(const CFX_RectF& rtText, size_t szBlockIndex);
  bool DrawString(CFX_RenderDevice* pFxDevice,
                  const CFX_Matrix& mtDoc2Device,
                  const CFX_RectF& rtClip,
                  size_t szBlockIndex);
  bool IsLoaded() const { return !piece_lines_.empty(); }
  void Unload();
  bool HasBlock() const { return has_block_; }
  void ClearBlocks() { blocks_.clear(); }
  void ResetHasBlock() { has_block_ = false; }

  // Returns empty string when no link is present.
  WideString GetLinkURLAtPoint(const CFX_PointF& point);

 private:
  class TextPiece : public CFGAS_TextPiece {
   public:
    TextPiece();
    ~TextPiece();

    int32_t iUnderline = 0;
    int32_t iLineThrough = 0;
    XFA_AttributeValue iPeriod = XFA_AttributeValue::All;
    FX_ARGB dwColor = 0;
    RetainPtr<CFGAS_LinkUserData> pLinkData;
  };

  class PieceLine {
   public:
    PieceLine();
    ~PieceLine();

    std::vector<std::unique_ptr<TextPiece>> text_pieces_;
    std::vector<size_t> char_counts_;
  };

  struct BlockData {
    size_t szIndex;
    size_t szLength;
  };

  struct BlockHeight {
    size_t szBlockIndex;
    float fHeight;
  };

  struct LoaderContext : public cppgc::GarbageCollected<LoaderContext> {
    LoaderContext();
    ~LoaderContext();

    void Trace(cppgc::Visitor* visitor) const;

    bool bSaveLineHeight = false;
    bool bFilterSpace = false;
    float fWidth = 0;
    float fHeight = 0;
    float fLastPos = 0;
    float fStartLineOffset = 0;
    size_t nCharIdx = 0;
    // TODO(thestig): Make this size_t?
    int32_t iTotalLines = -1;
    UnownedPtr<const CFX_XMLNode> pXMLNode;
    RetainPtr<CFX_CSSComputedStyle> pParentStyle;
    cppgc::Member<CXFA_Node> pNode;
    std::vector<float> lineHeights;
    std::vector<BlockHeight> blockHeights;
  };

  CXFA_TextLayout(CXFA_FFDoc* doc, CXFA_TextProvider* pTextProvider);

  void GetTextDataNode();
  CFX_XMLNode* GetXMLContainerNode();
  std::unique_ptr<CFGAS_RTFBreak> CreateBreak(bool bDefault);
  void InitBreak(float fLineWidth);
  void InitBreak(CFX_CSSComputedStyle* pStyle,
                 CFX_CSSDisplay eDisplay,
                 float fLineWidth,
                 const CFX_XMLNode* pXMLNode,
                 CFX_CSSComputedStyle* pParentStyle);
  void Loader(float textWidth, float* pLinePos, bool bSavePieces);
  void LoadText(CXFA_Node* pNode,
                float textWidth,
                float* pLinePos,
                bool bSavePieces);
  bool LoadRichText(const CFX_XMLNode* pXMLNode,
                    float textWidth,
                    float* pLinePos,
                    RetainPtr<CFX_CSSComputedStyle> pParentStyle,
                    bool bSavePieces,
                    RetainPtr<CFGAS_LinkUserData> pLinkData,
                    bool bEndBreak,
                    bool bIsOl,
                    int32_t iLiCount);
  bool AppendChar(const WideString& wsText,
                  float* pLinePos,
                  float fSpaceAbove,
                  bool bSavePieces);
  void AppendTextLine(CFGAS_Char::BreakType dwStatus,
                      float* pLinePos,
                      bool bSavePieces,
                      bool bEndBreak);
  void EndBreak(CFGAS_Char::BreakType dwStatus, float* pLinePos, bool bDefault);
  bool IsEnd(bool bSavePieces);
  void UpdateAlign(float fHeight, float fBottom);
  void RenderString(CFX_RenderDevice* pDevice,
                    PieceLine* pPieceLine,
                    size_t szPiece,
                    pdfium::span<TextCharPos> pCharPos,
                    const CFX_Matrix& mtDoc2Device);
  void RenderPath(CFX_RenderDevice* pDevice,
                  const PieceLine* pPieceLine,
                  size_t szPiece,
                  pdfium::span<TextCharPos> pCharPos,
                  const CFX_Matrix& mtDoc2Device);
  size_t GetDisplayPos(const TextPiece* pPiece,
                       pdfium::span<TextCharPos> pCharPos);
  void DoTabstops(CFX_CSSComputedStyle* pStyle, PieceLine* pPieceLine);
  bool LayoutInternal(size_t szBlockIndex);
  size_t CountBlocks() const;
  size_t GetNextIndexFromLastBlockData() const;
  void UpdateLoaderHeight(float fTextHeight);

  bool has_block_ = false;
  bool rich_text_ = false;
  int32_t lines_ = 0;
  float max_width_ = 0;
  std::vector<BlockData> blocks_;
  cppgc::Member<CXFA_FFDoc> const doc_;
  cppgc::Member<CXFA_TextProvider> const text_provider_;
  cppgc::Member<CXFA_Node> text_data_node_;
  cppgc::Member<CXFA_TextParser> text_parser_;
  cppgc::Member<LoaderContext> loader_;
  std::unique_ptr<CFGAS_RTFBreak> break_;
  std::vector<std::unique_ptr<PieceLine>> piece_lines_;
  std::unique_ptr<CXFA_TextTabstopsContext> tabstop_context_;
};

#endif  // XFA_FXFA_CXFA_TEXTLAYOUT_H_
