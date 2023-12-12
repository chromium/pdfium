// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXGE_CFX_FACE_H_
#define CORE_FXGE_CFX_FACE_H_

#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/freetype/fx_freetype.h"
#include "third_party/base/containers/span.h"

class CFX_Face final : public Retainable, public Observable {
 public:
  static RetainPtr<CFX_Face> New(FT_Library library,
                                 RetainPtr<Retainable> pDesc,
                                 pdfium::span<const FT_Byte> data,
                                 FT_Long face_index);

  static RetainPtr<CFX_Face> Open(FT_Library library,
                                  const FT_Open_Args* args,
                                  FT_Long face_index);

  bool HasGlyphNames() const;
  bool IsTtOt() const;
  bool IsTricky() const;
  bool IsFixedWidth() const;

#if defined(PDF_ENABLE_XFA)
  bool IsScalable() const;
  void ClearExternalStream();
#endif

  bool IsItalic() const;
  bool IsBold() const;

  pdfium::span<uint8_t> GetData() const;

  FXFT_FaceRec* GetRec() { return m_pRec.get(); }
  const FXFT_FaceRec* GetRec() const { return m_pRec.get(); }

 private:
  CFX_Face(FXFT_FaceRec* pRec, RetainPtr<Retainable> pDesc);
  ~CFX_Face() override;

  ScopedFXFTFaceRec const m_pRec;
  RetainPtr<Retainable> const m_pDesc;
};

#endif  // CORE_FXGE_CFX_FACE_H_
