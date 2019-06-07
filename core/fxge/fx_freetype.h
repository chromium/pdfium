// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_FX_FREETYPE_H_
#define CORE_FXGE_FX_FREETYPE_H_

#include <ft2build.h>

#include <memory>

#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_LCD_FILTER_H
#include FT_MULTIPLE_MASTERS_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H

using FXFT_LibraryRec = struct FT_LibraryRec_;
using FXFT_FaceRec = struct FT_FaceRec_;
using FXFT_StreamRec = struct FT_StreamRec_;
using FXFT_MM_VarPtr = FT_MM_Var*;

struct FXFTFaceRecDeleter {
  inline void operator()(FXFT_FaceRec* pRec) {
    if (pRec)
      FT_Done_Face(pRec);
  }
};

struct FXFTLibraryRecDeleter {
  inline void operator()(FXFT_LibraryRec* pRec) {
    if (pRec)
      FT_Done_FreeType(pRec);
  }
};

using ScopedFXFTFaceRec = std::unique_ptr<FXFT_FaceRec, FXFTFaceRecDeleter>;
using ScopedFXFTLibraryRec =
    std::unique_ptr<FXFT_LibraryRec, FXFTLibraryRecDeleter>;

#define FXFT_Select_Charmap(face, encoding) \
  FT_Select_Charmap(face, static_cast<FT_Encoding>(encoding))
#define FXFT_Get_Name_Index(face, name) \
  FT_Get_Name_Index(face, const_cast<char*>(name))
#define FXFT_Get_Glyph_Outline(face) &((face)->glyph->outline)
#define FXFT_Render_Glyph(face, mode) \
  FT_Render_Glyph((face)->glyph, static_cast<enum FT_Render_Mode_>(mode))

#define FXFT_Has_Glyph_Names(face) \
  (((face)->face_flags) & FT_FACE_FLAG_GLYPH_NAMES)
#define FXFT_Clear_Face_External_Stream(face) \
  ((face)->face_flags &= ~FT_FACE_FLAG_EXTERNAL_STREAM)
#define FXFT_Get_Face_External_Stream(face) \
  (((face)->face_flags) & FT_FACE_FLAG_EXTERNAL_STREAM)
#define FXFT_Is_Face_TT_OT(face) (((face)->face_flags) & FT_FACE_FLAG_SFNT)
#define FXFT_Is_Face_Tricky(face) (((face)->face_flags) & FT_FACE_FLAG_TRICKY)
#define FXFT_Is_Face_fixedwidth(face) \
  (((face)->face_flags) & FT_FACE_FLAG_FIXED_WIDTH)
#define FXFT_Get_Face_Stream_Base(face) (face)->stream->base
#define FXFT_Get_Face_Stream_Size(face) (face)->stream->size
#define FXFT_Get_Face_Family_Name(face) (face)->family_name
#define FXFT_Get_Face_Style_Name(face) (face)->style_name
#define FXFT_Is_Face_Italic(face) (((face)->style_flags) & FT_STYLE_FLAG_ITALIC)
#define FXFT_Is_Face_Bold(face) (((face)->style_flags) & FT_STYLE_FLAG_BOLD)
#define FXFT_Get_Face_Charmaps(face) (face)->charmaps
#define FXFT_Get_Glyph_HoriBearingX(face) (face)->glyph->metrics.horiBearingX
#define FXFT_Get_Glyph_HoriBearingY(face) (face)->glyph->metrics.horiBearingY
#define FXFT_Get_Glyph_Width(face) (face)->glyph->metrics.width
#define FXFT_Get_Glyph_Height(face) (face)->glyph->metrics.height
#define FXFT_Get_Face_CharmapCount(face) (face)->num_charmaps
#define FXFT_Get_Charmap_Encoding(charmap) (charmap)->encoding
#define FXFT_Get_Face_Charmap(face) (face)->charmap
#define FXFT_Get_Charmap_PlatformID(charmap) (charmap)->platform_id
#define FXFT_Get_Charmap_EncodingID(charmap) (charmap)->encoding_id
#define FXFT_Get_Face_UnitsPerEM(face) (face)->units_per_EM
#define FXFT_Get_Face_xMin(face) (face)->bbox.xMin
#define FXFT_Get_Face_xMax(face) (face)->bbox.xMax
#define FXFT_Get_Face_yMin(face) (face)->bbox.yMin
#define FXFT_Get_Face_yMax(face) (face)->bbox.yMax
#define FXFT_Get_Face_Height(face) (face)->height
#define FXFT_Get_Face_Ascender(face) (face)->ascender
#define FXFT_Get_Face_Descender(face) (face)->descender
#define FXFT_Get_Glyph_HoriAdvance(face) (face)->glyph->metrics.horiAdvance
#define FXFT_Get_MM_Axis(var, index) (var)->axis[index]
#define FXFT_Get_MM_Axis_Min(axis) (axis).minimum
#define FXFT_Get_MM_Axis_Max(axis) (axis).maximum
#define FXFT_Get_MM_Axis_Def(axis) (axis).def
#define FXFT_Free(face, p) (face)->memory->free((face)->memory, p)
#define FXFT_Get_Glyph_Outline(face) &((face)->glyph->outline)
#define FXFT_Get_Glyph_Bitmap(face) (face)->glyph->bitmap
#define FXFT_Get_Bitmap_Width(bitmap) (bitmap).width
#define FXFT_Get_Bitmap_Rows(bitmap) (bitmap).rows
#define FXFT_Get_Bitmap_PixelMode(bitmap) (bitmap).pixel_mode
#define FXFT_Get_Bitmap_Pitch(bitmap) (bitmap).pitch
#define FXFT_Get_Bitmap_Buffer(bitmap) (bitmap).buffer
#define FXFT_Get_Glyph_BitmapLeft(face) (face)->glyph->bitmap_left
#define FXFT_Get_Glyph_BitmapTop(face) (face)->glyph->bitmap_top

int FXFT_unicode_from_adobe_name(const char* glyph_name);
void FXFT_adobe_name_from_unicode(char* name, wchar_t unicode);

#endif  // CORE_FXGE_FX_FREETYPE_H_
