// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_TEXTLAYOUT_H_
#define XFA_FXFA_APP_CXFA_TEXTLAYOUT_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "xfa/fde/css/fde_css.h"
#include "xfa/fgas/layout/cfx_rtfbreak.h"
#include "xfa/fxfa/app/cxfa_textparser.h"

class CFDE_Brush;
class CFDE_CSSComputedStyle;
class CFDE_Pen;
class CFDE_RenderDevice;
class CFX_XMLNode;
class CFX_RTFBreak;
class CXFA_LoaderContext;
class CXFA_LinkUserData;
class CXFA_Node;
class CXFA_PieceLine;
class CXFA_TextProvider;
class CXFA_TextTabstopsContext;
class CXFA_TextPiece;

class CXFA_TextLayout {
 public:
  explicit CXFA_TextLayout(CXFA_TextProvider* pTextProvider);
  ~CXFA_TextLayout();

  int32_t GetText(CFX_WideString& wsText);
  float GetLayoutHeight();
  float StartLayout(float fWidth = -1);
  bool DoLayout(int32_t iBlockIndex,
                float& fCalcHeight,
                float fContentAreaHeight = -1,
                float fTextHeight = -1);

  bool CalcSize(const CFX_SizeF& minSize,
                const CFX_SizeF& maxSize,
                CFX_SizeF& defaultSize);
  bool Layout(const CFX_SizeF& size, float* fHeight = nullptr);
  void ItemBlocks(const CFX_RectF& rtText, int32_t iBlockIndex);
  bool DrawString(CFX_RenderDevice* pFxDevice,
                  const CFX_Matrix& tmDoc2Device,
                  const CFX_RectF& rtClip,
                  int32_t iBlock = 0);
  bool IsLoaded() const { return !m_pieceLines.empty(); }
  void Unload();
  const std::vector<std::unique_ptr<CXFA_PieceLine>>* GetPieceLines() const {
    return &m_pieceLines;
  }

  bool m_bHasBlock;
  std::vector<int32_t> m_Blocks;

 private:
  void GetTextDataNode();
  CFX_XMLNode* GetXMLContainerNode();
  std::unique_ptr<CFX_RTFBreak> CreateBreak(bool bDefault);
  void InitBreak(float fLineWidth);
  void InitBreak(CFDE_CSSComputedStyle* pStyle,
                 FDE_CSSDisplay eDisplay,
                 float fLineWidth,
                 CFX_XMLNode* pXMLNode,
                 CFDE_CSSComputedStyle* pParentStyle = nullptr);
  bool Loader(const CFX_SizeF& szText,
              float& fLinePos,
              bool bSavePieces = true);
  void LoadText(CXFA_Node* pNode,
                const CFX_SizeF& szText,
                float& fLinePos,
                bool bSavePieces);
  bool LoadRichText(CFX_XMLNode* pXMLNode,
                    const CFX_SizeF& szText,
                    float& fLinePos,
                    const CFX_RetainPtr<CFDE_CSSComputedStyle>& pParentStyle,
                    bool bSavePieces,
                    CFX_RetainPtr<CXFA_LinkUserData> pLinkData,
                    bool bEndBreak = true,
                    bool bIsOl = false,
                    int32_t iLiCount = 0);
  bool AppendChar(const CFX_WideString& wsText,
                  float& fLinePos,
                  float fSpaceAbove,
                  bool bSavePieces);
  void AppendTextLine(CFX_BreakType dwStatus,
                      float& fLinePos,
                      bool bSavePieces,
                      bool bEndBreak = false);
  void EndBreak(CFX_BreakType dwStatus, float& fLinePos, bool bDefault);
  bool IsEnd(bool bSavePieces);
  void ProcessText(CFX_WideString& wsText);
  void UpdateAlign(float fHeight, float fBottom);
  void RenderString(CFDE_RenderDevice* pDevice,
                    CFDE_Brush* pBrush,
                    CXFA_PieceLine* pPieceLine,
                    int32_t iPiece,
                    FXTEXT_CHARPOS* pCharPos,
                    const CFX_Matrix& tmDoc2Device);
  void RenderPath(CFDE_RenderDevice* pDevice,
                  CFDE_Pen* pPen,
                  CXFA_PieceLine* pPieceLine,
                  int32_t iPiece,
                  FXTEXT_CHARPOS* pCharPos,
                  const CFX_Matrix& tmDoc2Device);
  int32_t GetDisplayPos(const CXFA_TextPiece* pPiece,
                        FXTEXT_CHARPOS* pCharPos,
                        bool bCharCode = false);
  bool ToRun(const CXFA_TextPiece* pPiece, FX_RTFTEXTOBJ* tr);
  void DoTabstops(CFDE_CSSComputedStyle* pStyle, CXFA_PieceLine* pPieceLine);
  bool Layout(int32_t iBlock);
  int32_t CountBlocks() const;

  CXFA_TextProvider* m_pTextProvider;
  CXFA_Node* m_pTextDataNode;
  bool m_bRichText;
  std::unique_ptr<CFX_RTFBreak> m_pBreak;
  std::unique_ptr<CXFA_LoaderContext> m_pLoader;
  int32_t m_iLines;
  float m_fMaxWidth;
  CXFA_TextParser m_textParser;
  std::vector<std::unique_ptr<CXFA_PieceLine>> m_pieceLines;
  std::unique_ptr<CXFA_TextTabstopsContext> m_pTabstopContext;
  bool m_bBlockContinue;
};

#endif  // XFA_FXFA_APP_CXFA_TEXTLAYOUT_H_
