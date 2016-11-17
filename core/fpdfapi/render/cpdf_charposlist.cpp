// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_charposlist.h"

#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fpdfapi/font/cpdf_font.h"

CPDF_CharPosList::CPDF_CharPosList() {
  m_pCharPos = nullptr;
}

CPDF_CharPosList::~CPDF_CharPosList() {
  FX_Free(m_pCharPos);
}

void CPDF_CharPosList::Load(int nChars,
                            uint32_t* pCharCodes,
                            FX_FLOAT* pCharPos,
                            CPDF_Font* pFont,
                            FX_FLOAT FontSize) {
  m_pCharPos = FX_Alloc(FXTEXT_CHARPOS, nChars);
  m_nChars = 0;
  CPDF_CIDFont* pCIDFont = pFont->AsCIDFont();
  bool bVertWriting = pCIDFont && pCIDFont->IsVertWriting();
  for (int iChar = 0; iChar < nChars; iChar++) {
    uint32_t CharCode =
        nChars == 1 ? (uint32_t)(uintptr_t)pCharCodes : pCharCodes[iChar];
    if (CharCode == (uint32_t)-1) {
      continue;
    }
    bool bVert = false;
    FXTEXT_CHARPOS& charpos = m_pCharPos[m_nChars++];
    if (pCIDFont) {
      charpos.m_bFontStyle = true;
    }
    charpos.m_GlyphIndex = pFont->GlyphFromCharCode(CharCode, &bVert);
    if (charpos.m_GlyphIndex != static_cast<uint32_t>(-1)) {
      charpos.m_FallbackFontPosition = -1;
    } else {
      charpos.m_FallbackFontPosition =
          pFont->FallbackFontFromCharcode(CharCode);
      charpos.m_GlyphIndex = pFont->FallbackGlyphFromCharcode(
          charpos.m_FallbackFontPosition, CharCode);
    }
// TODO(npm): Figure out how this affects m_ExtGID
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
    charpos.m_ExtGID = pFont->GlyphFromCharCodeExt(CharCode);
#endif
    if (!pFont->IsEmbedded() && !pFont->IsCIDFont()) {
      charpos.m_FontCharWidth = pFont->GetCharWidthF(CharCode);
    } else {
      charpos.m_FontCharWidth = 0;
    }
    charpos.m_OriginX = iChar ? pCharPos[iChar - 1] : 0;
    charpos.m_OriginY = 0;
    charpos.m_bGlyphAdjust = false;
    if (!pCIDFont) {
      continue;
    }
    uint16_t CID = pCIDFont->CIDFromCharCode(CharCode);
    if (bVertWriting) {
      charpos.m_OriginY = charpos.m_OriginX;
      charpos.m_OriginX = 0;
      short vx, vy;
      pCIDFont->GetVertOrigin(CID, vx, vy);
      charpos.m_OriginX -= FontSize * vx / 1000;
      charpos.m_OriginY -= FontSize * vy / 1000;
    }
    const uint8_t* pTransform = pCIDFont->GetCIDTransform(CID);
    if (pTransform && !bVert) {
      charpos.m_AdjustMatrix[0] = pCIDFont->CIDTransformToFloat(pTransform[0]);
      charpos.m_AdjustMatrix[2] = pCIDFont->CIDTransformToFloat(pTransform[2]);
      charpos.m_AdjustMatrix[1] = pCIDFont->CIDTransformToFloat(pTransform[1]);
      charpos.m_AdjustMatrix[3] = pCIDFont->CIDTransformToFloat(pTransform[3]);
      charpos.m_OriginX +=
          pCIDFont->CIDTransformToFloat(pTransform[4]) * FontSize;
      charpos.m_OriginY +=
          pCIDFont->CIDTransformToFloat(pTransform[5]) * FontSize;
      charpos.m_bGlyphAdjust = true;
    }
  }
}
