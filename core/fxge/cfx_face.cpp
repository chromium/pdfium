// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_face.h"

#include <utility>

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

CFX_Face::CFX_Face(FXFT_FaceRec* rec, RetainPtr<Retainable> pDesc)
    : m_pRec(rec), m_pDesc(std::move(pDesc)) {
  DCHECK(m_pRec);
}

CFX_Face::~CFX_Face() = default;
