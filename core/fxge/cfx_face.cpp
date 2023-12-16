// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_face.h"

#include <utility>

#include "core/fxge/fx_fontencoding.h"
#include "third_party/base/check.h"
#include "third_party/base/numerics/safe_conversions.h"

// static
RetainPtr<CFX_Face> CFX_Face::New(FT_Library library,
                                  RetainPtr<Retainable> pDesc,
                                  pdfium::span<const FT_Byte> data,
                                  FT_Long face_index) {
  FXFT_FaceRec* pRec = nullptr;
  if (FT_New_Memory_Face(library, data.data(),
                         pdfium::base::checked_cast<FT_Long>(data.size()),
                         face_index, &pRec) != 0) {
    return nullptr;
  }
  // Private ctor.
  return pdfium::WrapRetain(new CFX_Face(pRec, std::move(pDesc)));
}

// static
RetainPtr<CFX_Face> CFX_Face::Open(FT_Library library,
                                   const FT_Open_Args* args,
                                   FT_Long face_index) {
  FXFT_FaceRec* pRec = nullptr;
  if (FT_Open_Face(library, args, face_index, &pRec) != 0)
    return nullptr;

  // Private ctor.
  return pdfium::WrapRetain(new CFX_Face(pRec, nullptr));
}

bool CFX_Face::HasGlyphNames() const {
  return !!(GetRec()->face_flags & FT_FACE_FLAG_GLYPH_NAMES);
}

bool CFX_Face::IsTtOt() const {
  return !!(GetRec()->face_flags & FT_FACE_FLAG_SFNT);
}

bool CFX_Face::IsTricky() const {
  return !!(GetRec()->face_flags & FT_FACE_FLAG_TRICKY);
}

bool CFX_Face::IsFixedWidth() const {
  return !!(GetRec()->face_flags & FT_FACE_FLAG_FIXED_WIDTH);
}

#if defined(PDF_ENABLE_XFA)
bool CFX_Face::IsScalable() const {
  return !!(GetRec()->face_flags & FT_FACE_FLAG_SCALABLE);
}

void CFX_Face::ClearExternalStream() {
  GetRec()->face_flags &= ~FT_FACE_FLAG_EXTERNAL_STREAM;
}
#endif

bool CFX_Face::IsItalic() const {
  return !!(GetRec()->style_flags & FT_STYLE_FLAG_ITALIC);
}

bool CFX_Face::IsBold() const {
  return !!(GetRec()->style_flags & FT_STYLE_FLAG_BOLD);
}

ByteString CFX_Face::GetFamilyName() const {
  return ByteString(GetRec()->family_name);
}

ByteString CFX_Face::GetStyleName() const {
  return ByteString(GetRec()->style_name);
}

FX_RECT CFX_Face::GetBBox() const {
  return FX_RECT(pdfium::base::checked_cast<int32_t>(GetRec()->bbox.xMin),
                 pdfium::base::checked_cast<int32_t>(GetRec()->bbox.yMin),
                 pdfium::base::checked_cast<int32_t>(GetRec()->bbox.xMax),
                 pdfium::base::checked_cast<int32_t>(GetRec()->bbox.yMax));
}

uint16_t CFX_Face::GetUnitsPerEm() const {
  return pdfium::base::checked_cast<uint16_t>(GetRec()->units_per_EM);
}

int16_t CFX_Face::GetAscender() const {
  return pdfium::base::checked_cast<int16_t>(GetRec()->ascender);
}

int16_t CFX_Face::GetDescender() const {
  return pdfium::base::checked_cast<int16_t>(GetRec()->descender);
}

#if BUILDFLAG(IS_ANDROID)
int16_t CFX_Face::GetHeight() const {
  return pdfium::base::checked_cast<int16_t>(GetRec()->height);
}
#endif

pdfium::span<uint8_t> CFX_Face::GetData() const {
  return {GetRec()->stream->base, GetRec()->stream->size};
}

bool CFX_Face::SelectCharMap(fxge::FontEncoding encoding) {
  FT_Error error =
      FT_Select_Charmap(GetRec(), static_cast<FT_Encoding>(encoding));
  return !error;
}

CFX_Face::CFX_Face(FXFT_FaceRec* rec, RetainPtr<Retainable> pDesc)
    : m_pRec(rec), m_pDesc(std::move(pDesc)) {
  DCHECK(m_pRec);
}

CFX_Face::~CFX_Face() = default;
