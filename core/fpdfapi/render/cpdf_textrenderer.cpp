// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_textrenderer.h"

#include <algorithm>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/render/cpdf_charposlist.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/text_char_pos.h"

namespace {

CFX_Font* GetFont(CPDF_Font* pFont, int32_t position) {
  return position == -1 ? pFont->GetFont() : pFont->GetFontFallback(position);
}

}  // namespace

// static
bool CPDF_TextRenderer::DrawTextPath(CFX_RenderDevice* pDevice,
                                     const std::vector<uint32_t>& charCodes,
                                     const std::vector<float>& charPos,
                                     CPDF_Font* pFont,
                                     float font_size,
                                     const CFX_Matrix& mtText2User,
                                     const CFX_Matrix* pUser2Device,
                                     const CFX_GraphStateData* pGraphState,
                                     FX_ARGB fill_argb,
                                     FX_ARGB stroke_argb,
                                     CFX_PathData* pClippingPath,
                                     int nFlag) {
  const CPDF_CharPosList CharPosList(charCodes, charPos, pFont, font_size);
  const std::vector<TextCharPos>& pos = CharPosList.Get();
  if (pos.empty())
    return true;

  bool bDraw = true;
  int32_t fontPosition = pos[0].m_FallbackFontPosition;
  size_t startIndex = 0;
  for (size_t i = 0; i < pos.size(); ++i) {
    int32_t curFontPosition = pos[i].m_FallbackFontPosition;
    if (fontPosition == curFontPosition)
      continue;

    CFX_Font* font = GetFont(pFont, fontPosition);
    if (!pDevice->DrawTextPath(i - startIndex, &pos[startIndex], font,
                               font_size, mtText2User, pUser2Device,
                               pGraphState, fill_argb, stroke_argb,
                               pClippingPath, nFlag)) {
      bDraw = false;
    }
    fontPosition = curFontPosition;
    startIndex = i;
  }
  CFX_Font* font = GetFont(pFont, fontPosition);
  if (!pDevice->DrawTextPath(pos.size() - startIndex, &pos[startIndex], font,
                             font_size, mtText2User, pUser2Device, pGraphState,
                             fill_argb, stroke_argb, pClippingPath, nFlag)) {
    bDraw = false;
  }
  return bDraw;
}

// static
void CPDF_TextRenderer::DrawTextString(CFX_RenderDevice* pDevice,
                                       float origin_x,
                                       float origin_y,
                                       CPDF_Font* pFont,
                                       float font_size,
                                       const CFX_Matrix& matrix,
                                       const ByteString& str,
                                       FX_ARGB fill_argb,
                                       const CPDF_RenderOptions& options) {
  if (pFont->IsType3Font())
    return;

  int nChars = pFont->CountChar(str.AsStringView());
  if (nChars <= 0)
    return;

  size_t offset = 0;
  std::vector<uint32_t> codes;
  std::vector<float> positions;
  codes.resize(nChars);
  positions.resize(nChars - 1);
  float cur_pos = 0;
  for (int i = 0; i < nChars; i++) {
    codes[i] = pFont->GetNextChar(str.AsStringView(), &offset);
    if (i)
      positions[i - 1] = cur_pos;
    cur_pos += pFont->GetCharWidthF(codes[i]) * font_size / 1000;
  }
  CFX_Matrix new_matrix = matrix;
  new_matrix.e = origin_x;
  new_matrix.f = origin_y;
  DrawNormalText(pDevice, codes, positions, pFont, font_size, new_matrix,
                 fill_argb, options);
}

// static
bool CPDF_TextRenderer::DrawNormalText(CFX_RenderDevice* pDevice,
                                       const std::vector<uint32_t>& charCodes,
                                       const std::vector<float>& charPos,
                                       CPDF_Font* pFont,
                                       float font_size,
                                       const CFX_Matrix& mtText2Device,
                                       FX_ARGB fill_argb,
                                       const CPDF_RenderOptions& options) {
  const CPDF_CharPosList CharPosList(charCodes, charPos, pFont, font_size);
  const std::vector<TextCharPos>& pos = CharPosList.Get();
  if (pos.empty())
    return true;

  int fxge_flags = 0;
  if (options.GetOptions().bClearType) {
    fxge_flags |= FXTEXT_CLEARTYPE;
    if (options.GetOptions().bBGRStripe)
      fxge_flags |= FXTEXT_BGR_STRIPE;
  }
  if (options.GetOptions().bNoTextSmooth)
    fxge_flags |= FXTEXT_NOSMOOTH;
  if (options.GetOptions().bPrintGraphicText)
    fxge_flags |= FXTEXT_PRINTGRAPHICTEXT;
  if (options.GetOptions().bNoNativeText)
    fxge_flags |= FXTEXT_NO_NATIVETEXT;
  if (options.GetOptions().bPrintImageText)
    fxge_flags |= FXTEXT_PRINTIMAGETEXT;

  if (pFont->IsCIDFont())
    fxge_flags |= FXFONT_CIDFONT;

  bool bDraw = true;
  int32_t fontPosition = pos[0].m_FallbackFontPosition;
  size_t startIndex = 0;
  for (size_t i = 0; i < pos.size(); ++i) {
    int32_t curFontPosition = pos[i].m_FallbackFontPosition;
    if (fontPosition == curFontPosition)
      continue;

    CFX_Font* font = GetFont(pFont, fontPosition);
    if (!pDevice->DrawNormalText(i - startIndex, &pos[startIndex], font,
                                 font_size, mtText2Device, fill_argb,
                                 fxge_flags)) {
      bDraw = false;
    }
    fontPosition = curFontPosition;
    startIndex = i;
  }
  CFX_Font* font = GetFont(pFont, fontPosition);
  if (!pDevice->DrawNormalText(pos.size() - startIndex, &pos[startIndex], font,
                               font_size, mtText2Device, fill_argb,
                               fxge_flags)) {
    bDraw = false;
  }
  return bDraw;
}
