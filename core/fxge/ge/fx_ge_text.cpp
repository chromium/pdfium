// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>
#include <limits>
#include <vector>

#include "core/fxcodec/include/fx_codec.h"
#include "core/fxcrt/include/fx_safe_types.h"
#include "core/fxge/ge/fx_text_int.h"
#include "core/fxge/include/cfx_pathdata.h"
#include "core/fxge/include/fx_freetype.h"
#include "core/fxge/include/ifx_renderdevicedriver.h"

namespace {

void ResetTransform(FT_Face face) {
  FXFT_Matrix matrix;
  matrix.xx = 0x10000L;
  matrix.xy = 0;
  matrix.yx = 0;
  matrix.yy = 0x10000L;
  FXFT_Set_Transform(face, &matrix, 0);
}

}  // namespace

ScopedFontTransform::ScopedFontTransform(FT_Face face, FXFT_Matrix* matrix)
    : m_Face(face) {
  FXFT_Set_Transform(m_Face, matrix, 0);
}

ScopedFontTransform::~ScopedFontTransform() {
  ResetTransform(m_Face);
}

FX_RECT FXGE_GetGlyphsBBox(const std::vector<FXTEXT_GLYPHPOS>& glyphs,
                           int anti_alias,
                           FX_FLOAT retinaScaleX,
                           FX_FLOAT retinaScaleY) {
  FX_RECT rect(0, 0, 0, 0);
  bool bStarted = false;
  for (const FXTEXT_GLYPHPOS& glyph : glyphs) {
    const CFX_GlyphBitmap* pGlyph = glyph.m_pGlyph;
    if (!pGlyph)
      continue;

    FX_SAFE_INT32 char_left = glyph.m_OriginX;
    char_left += pGlyph->m_Left;
    if (!char_left.IsValid())
      continue;

    FX_SAFE_INT32 char_width = pGlyph->m_Bitmap.GetWidth();
    char_width /= retinaScaleX;
    if (anti_alias == FXFT_RENDER_MODE_LCD)
      char_width /= 3;
    if (!char_width.IsValid())
      continue;

    FX_SAFE_INT32 char_right = char_left + char_width;
    if (!char_right.IsValid())
      continue;

    FX_SAFE_INT32 char_top = glyph.m_OriginY;
    char_top -= pGlyph->m_Top;
    if (!char_top.IsValid())
      continue;

    FX_SAFE_INT32 char_height = pGlyph->m_Bitmap.GetHeight();
    char_height /= retinaScaleY;
    if (!char_height.IsValid())
      continue;

    FX_SAFE_INT32 char_bottom = char_top + char_height;
    if (!char_bottom.IsValid())
      continue;

    if (bStarted) {
      rect.left = std::min(rect.left, char_left.ValueOrDie());
      rect.right = std::max(rect.right, char_right.ValueOrDie());
      rect.top = std::min(rect.top, char_top.ValueOrDie());
      rect.bottom = std::max(rect.bottom, char_bottom.ValueOrDie());
      continue;
    }

    rect.left = char_left.ValueOrDie();
    rect.right = char_right.ValueOrDie();
    rect.top = char_top.ValueOrDie();
    rect.bottom = char_bottom.ValueOrDie();
    bStarted = true;
  }
  return rect;
}

CFX_SizeGlyphCache::CFX_SizeGlyphCache() {}

CFX_SizeGlyphCache::~CFX_SizeGlyphCache() {
  for (const auto& pair : m_GlyphMap) {
    delete pair.second;
  }
  m_GlyphMap.clear();
}

#define CONTRAST_RAMP_STEP 1
void CFX_Font::AdjustMMParams(int glyph_index, int dest_width, int weight) {
  FXFT_MM_Var pMasters = nullptr;
  FXFT_Get_MM_Var(m_Face, &pMasters);
  if (!pMasters) {
    return;
  }
  long coords[2];
  if (weight == 0) {
    coords[0] = FXFT_Get_MM_Axis_Def(FXFT_Get_MM_Axis(pMasters, 0)) / 65536;
  } else {
    coords[0] = weight;
  }
  if (dest_width == 0) {
    coords[1] = FXFT_Get_MM_Axis_Def(FXFT_Get_MM_Axis(pMasters, 1)) / 65536;
  } else {
    int min_param = FXFT_Get_MM_Axis_Min(FXFT_Get_MM_Axis(pMasters, 1)) / 65536;
    int max_param = FXFT_Get_MM_Axis_Max(FXFT_Get_MM_Axis(pMasters, 1)) / 65536;
    coords[1] = min_param;
    (void)FXFT_Set_MM_Design_Coordinates(m_Face, 2, coords);
    (void)FXFT_Load_Glyph(
        m_Face, glyph_index,
        FXFT_LOAD_NO_SCALE | FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
    int min_width = FXFT_Get_Glyph_HoriAdvance(m_Face) * 1000 /
                    FXFT_Get_Face_UnitsPerEM(m_Face);
    coords[1] = max_param;
    (void)FXFT_Set_MM_Design_Coordinates(m_Face, 2, coords);
    (void)FXFT_Load_Glyph(
        m_Face, glyph_index,
        FXFT_LOAD_NO_SCALE | FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
    int max_width = FXFT_Get_Glyph_HoriAdvance(m_Face) * 1000 /
                    FXFT_Get_Face_UnitsPerEM(m_Face);
    if (max_width == min_width) {
      FXFT_Free(m_Face, pMasters);
      return;
    }
    int param = min_param +
                (max_param - min_param) * (dest_width - min_width) /
                    (max_width - min_width);
    coords[1] = param;
  }
  FXFT_Free(m_Face, pMasters);
  FXFT_Set_MM_Design_Coordinates(m_Face, 2, coords);
}
const char CFX_Font::s_AngleSkew[] = {
    0,  2,  3,  5,  7,  9,  11, 12, 14, 16, 18, 19, 21, 23, 25,
    27, 29, 31, 32, 34, 36, 38, 40, 42, 45, 47, 49, 51, 53, 55,
};
const uint8_t CFX_Font::s_WeightPow[] = {
    0,  3,  6,  7,  8,  9,  11, 12, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 35, 36, 36, 37,
    37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42,
    42, 43, 43, 43, 44, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 46, 47,
    47, 47, 47, 48, 48, 48, 48, 48, 49, 49, 49, 49, 50, 50, 50, 50, 50,
    51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53,
};
const uint8_t CFX_Font::s_WeightPow_11[] = {
    0,  4,  7,  8,  9,  10, 12, 13, 15, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 39, 39, 40, 40, 41,
    41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 46, 46, 46,
    46, 43, 47, 47, 48, 48, 48, 48, 45, 50, 50, 50, 46, 51, 51, 51, 52,
    52, 52, 52, 53, 53, 53, 53, 53, 54, 54, 54, 54, 55, 55, 55, 55, 55,
    56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58,
};
const uint8_t CFX_Font::s_WeightPow_SHIFTJIS[] = {
    0,  0,  1,  2,  3,  4,  5,  7,  8,  10, 11, 13, 14, 16, 17, 19, 21,
    22, 24, 26, 28, 30, 32, 33, 35, 37, 39, 41, 43, 45, 48, 48, 48, 48,
    49, 49, 49, 50, 50, 50, 50, 51, 51, 51, 51, 52, 52, 52, 52, 52, 53,
    53, 53, 53, 53, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 56, 56, 56,
    56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 58, 58,
    59, 59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 60,
};
typedef struct {
  FX_BOOL m_bCount;
  int m_PointCount;
  FX_PATHPOINT* m_pPoints;
  int m_CurX;
  int m_CurY;
  FX_FLOAT m_CoordUnit;
} OUTLINE_PARAMS;
void _Outline_CheckEmptyContour(OUTLINE_PARAMS* param) {
  if (param->m_PointCount >= 2 &&
      param->m_pPoints[param->m_PointCount - 2].m_Flag == FXPT_MOVETO &&
      param->m_pPoints[param->m_PointCount - 2].m_PointX ==
          param->m_pPoints[param->m_PointCount - 1].m_PointX &&
      param->m_pPoints[param->m_PointCount - 2].m_PointY ==
          param->m_pPoints[param->m_PointCount - 1].m_PointY) {
    param->m_PointCount -= 2;
  }
  if (param->m_PointCount >= 4 &&
      param->m_pPoints[param->m_PointCount - 4].m_Flag == FXPT_MOVETO &&
      param->m_pPoints[param->m_PointCount - 3].m_Flag == FXPT_BEZIERTO &&
      param->m_pPoints[param->m_PointCount - 3].m_PointX ==
          param->m_pPoints[param->m_PointCount - 4].m_PointX &&
      param->m_pPoints[param->m_PointCount - 3].m_PointY ==
          param->m_pPoints[param->m_PointCount - 4].m_PointY &&
      param->m_pPoints[param->m_PointCount - 2].m_PointX ==
          param->m_pPoints[param->m_PointCount - 4].m_PointX &&
      param->m_pPoints[param->m_PointCount - 2].m_PointY ==
          param->m_pPoints[param->m_PointCount - 4].m_PointY &&
      param->m_pPoints[param->m_PointCount - 1].m_PointX ==
          param->m_pPoints[param->m_PointCount - 4].m_PointX &&
      param->m_pPoints[param->m_PointCount - 1].m_PointY ==
          param->m_pPoints[param->m_PointCount - 4].m_PointY) {
    param->m_PointCount -= 4;
  }
}
extern "C" {
static int _Outline_MoveTo(const FXFT_Vector* to, void* user) {
  OUTLINE_PARAMS* param = (OUTLINE_PARAMS*)user;
  if (!param->m_bCount) {
    _Outline_CheckEmptyContour(param);
    param->m_pPoints[param->m_PointCount].m_PointX = to->x / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount].m_PointY = to->y / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount].m_Flag = FXPT_MOVETO;
    param->m_CurX = to->x;
    param->m_CurY = to->y;
    if (param->m_PointCount) {
      param->m_pPoints[param->m_PointCount - 1].m_Flag |= FXPT_CLOSEFIGURE;
    }
  }
  param->m_PointCount++;
  return 0;
}
};
extern "C" {
static int _Outline_LineTo(const FXFT_Vector* to, void* user) {
  OUTLINE_PARAMS* param = (OUTLINE_PARAMS*)user;
  if (!param->m_bCount) {
    param->m_pPoints[param->m_PointCount].m_PointX = to->x / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount].m_PointY = to->y / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount].m_Flag = FXPT_LINETO;
    param->m_CurX = to->x;
    param->m_CurY = to->y;
  }
  param->m_PointCount++;
  return 0;
}
};
extern "C" {
static int _Outline_ConicTo(const FXFT_Vector* control,
                            const FXFT_Vector* to,
                            void* user) {
  OUTLINE_PARAMS* param = (OUTLINE_PARAMS*)user;
  if (!param->m_bCount) {
    param->m_pPoints[param->m_PointCount].m_PointX =
        (param->m_CurX + (control->x - param->m_CurX) * 2 / 3) /
        param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount].m_PointY =
        (param->m_CurY + (control->y - param->m_CurY) * 2 / 3) /
        param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount].m_Flag = FXPT_BEZIERTO;
    param->m_pPoints[param->m_PointCount + 1].m_PointX =
        (control->x + (to->x - control->x) / 3) / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount + 1].m_PointY =
        (control->y + (to->y - control->y) / 3) / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount + 1].m_Flag = FXPT_BEZIERTO;
    param->m_pPoints[param->m_PointCount + 2].m_PointX =
        to->x / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount + 2].m_PointY =
        to->y / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount + 2].m_Flag = FXPT_BEZIERTO;
    param->m_CurX = to->x;
    param->m_CurY = to->y;
  }
  param->m_PointCount += 3;
  return 0;
}
};
extern "C" {
static int _Outline_CubicTo(const FXFT_Vector* control1,
                            const FXFT_Vector* control2,
                            const FXFT_Vector* to,
                            void* user) {
  OUTLINE_PARAMS* param = (OUTLINE_PARAMS*)user;
  if (!param->m_bCount) {
    param->m_pPoints[param->m_PointCount].m_PointX =
        control1->x / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount].m_PointY =
        control1->y / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount].m_Flag = FXPT_BEZIERTO;
    param->m_pPoints[param->m_PointCount + 1].m_PointX =
        control2->x / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount + 1].m_PointY =
        control2->y / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount + 1].m_Flag = FXPT_BEZIERTO;
    param->m_pPoints[param->m_PointCount + 2].m_PointX =
        to->x / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount + 2].m_PointY =
        to->y / param->m_CoordUnit;
    param->m_pPoints[param->m_PointCount + 2].m_Flag = FXPT_BEZIERTO;
    param->m_CurX = to->x;
    param->m_CurY = to->y;
  }
  param->m_PointCount += 3;
  return 0;
}
};
CFX_PathData* CFX_Font::LoadGlyphPath(uint32_t glyph_index, int dest_width) {
  if (!m_Face) {
    return nullptr;
  }
  FXFT_Set_Pixel_Sizes(m_Face, 0, 64);
  FXFT_Matrix ft_matrix = {65536, 0, 0, 65536};
  if (m_pSubstFont) {
    if (m_pSubstFont->m_ItalicAngle) {
      int skew = m_pSubstFont->m_ItalicAngle;
      // |skew| is nonpositive so |-skew| is used as the index. We need to make
      // sure |skew| != INT_MIN since -INT_MIN is undefined.
      if (skew <= 0 && skew != std::numeric_limits<int>::min() &&
          static_cast<size_t>(-skew) < kAngleSkewArraySize) {
        skew = -s_AngleSkew[-skew];
      } else {
        skew = -58;
      }
      if (m_bVertical)
        ft_matrix.yx += ft_matrix.yy * skew / 100;
      else
        ft_matrix.xy -= ft_matrix.xx * skew / 100;
    }
    if (m_pSubstFont->m_SubstFlags & FXFONT_SUBST_MM) {
      AdjustMMParams(glyph_index, dest_width, m_pSubstFont->m_Weight);
    }
  }
  ScopedFontTransform scoped_transform(m_Face, &ft_matrix);
  int load_flags = FXFT_LOAD_NO_BITMAP;
  if (!(m_Face->face_flags & FT_FACE_FLAG_SFNT) || !FT_IS_TRICKY(m_Face)) {
    load_flags |= FT_LOAD_NO_HINTING;
  }
  if (FXFT_Load_Glyph(m_Face, glyph_index, load_flags))
    return nullptr;
  if (m_pSubstFont && !(m_pSubstFont->m_SubstFlags & FXFONT_SUBST_MM) &&
      m_pSubstFont->m_Weight > 400) {
    uint32_t index = (m_pSubstFont->m_Weight - 400) / 10;
    index = std::min(index, static_cast<uint32_t>(kWeightPowArraySize - 1));
    int level = 0;
    if (m_pSubstFont->m_Charset == FXFONT_SHIFTJIS_CHARSET)
      level = s_WeightPow_SHIFTJIS[index] * 2 * 65536 / 36655;
    else
      level = s_WeightPow[index] * 2;
    FXFT_Outline_Embolden(FXFT_Get_Glyph_Outline(m_Face), level);
  }
  FXFT_Outline_Funcs funcs;
  funcs.move_to = _Outline_MoveTo;
  funcs.line_to = _Outline_LineTo;
  funcs.conic_to = _Outline_ConicTo;
  funcs.cubic_to = _Outline_CubicTo;
  funcs.shift = 0;
  funcs.delta = 0;
  OUTLINE_PARAMS params;
  params.m_bCount = TRUE;
  params.m_PointCount = 0;
  FXFT_Outline_Decompose(FXFT_Get_Glyph_Outline(m_Face), &funcs, &params);
  if (params.m_PointCount == 0) {
    return nullptr;
  }
  CFX_PathData* pPath = new CFX_PathData;
  pPath->SetPointCount(params.m_PointCount);
  params.m_bCount = FALSE;
  params.m_PointCount = 0;
  params.m_pPoints = pPath->GetPoints();
  params.m_CurX = params.m_CurY = 0;
  params.m_CoordUnit = 64 * 64.0;
  FXFT_Outline_Decompose(FXFT_Get_Glyph_Outline(m_Face), &funcs, &params);
  _Outline_CheckEmptyContour(&params);
  pPath->TrimPoints(params.m_PointCount);
  if (params.m_PointCount) {
    pPath->GetPoints()[params.m_PointCount - 1].m_Flag |= FXPT_CLOSEFIGURE;
  }
  return pPath;
}
void _CFX_UniqueKeyGen::Generate(int count, ...) {
  va_list argList;
  va_start(argList, count);
  for (int i = 0; i < count; i++) {
    int p = va_arg(argList, int);
    ((uint32_t*)m_Key)[i] = p;
  }
  va_end(argList);
  m_KeyLen = count * sizeof(uint32_t);
}
