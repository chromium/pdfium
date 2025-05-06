// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JBIG2_JBIG2_IMAGE_H_
#define CORE_FXCODEC_JBIG2_JBIG2_IMAGE_H_

#include <stdint.h>

#include <memory>

#include "core/fxcodec/jbig2/JBig2_Define.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/maybe_owned.h"
#include "core/fxcrt/span.h"

struct FX_RECT;

enum JBig2ComposeOp {
  JBIG2_COMPOSE_OR = 0,
  JBIG2_COMPOSE_AND = 1,
  JBIG2_COMPOSE_XOR = 2,
  JBIG2_COMPOSE_XNOR = 3,
  JBIG2_COMPOSE_REPLACE = 4
};

class CJBig2_Image {
 public:
  CJBig2_Image(int32_t w, int32_t h);
  CJBig2_Image(int32_t w,
               int32_t h,
               int32_t stride,
               pdfium::span<uint8_t> pBuf);
  CJBig2_Image(const CJBig2_Image& other);
  ~CJBig2_Image();

  static bool IsValidImageSize(int32_t w, int32_t h);

  int32_t width() const { return width_; }
  int32_t height() const { return height_; }
  int32_t stride() const { return stride_; }

  uint8_t* data() const { return data_.Get(); }

  int GetPixel(int32_t x, int32_t y) const;
  void SetPixel(int32_t x, int32_t y, int v);

  // PRECONDITIONS: `y` must refer to a line within the image.
  UNSAFE_BUFFER_USAGE uint8_t* GetLineUnsafe(int32_t y) const {
    // SAFETY: propogated to caller via UNSAFE_BUFFER_USAGE.
    return UNSAFE_BUFFERS(data() + y * stride_);
  }

  uint8_t* GetLine(int32_t y) const {
    // SAFETY: height_ valid lines in image.
    return (y >= 0 && y < height_) ? UNSAFE_BUFFERS(GetLineUnsafe(y)) : nullptr;
  }

  void CopyLine(int32_t hTo, int32_t hFrom);
  void Fill(bool v);

  bool ComposeFrom(int32_t x, int32_t y, CJBig2_Image* pSrc, JBig2ComposeOp op);
  bool ComposeFromWithRect(int32_t x,
                           int32_t y,
                           CJBig2_Image* pSrc,
                           const FX_RECT& rtSrc,
                           JBig2ComposeOp op);

  std::unique_ptr<CJBig2_Image> SubImage(int32_t x,
                                         int32_t y,
                                         int32_t w,
                                         int32_t h);
  void Expand(int32_t h, bool v);

  bool ComposeTo(CJBig2_Image* pDst, int32_t x, int32_t y, JBig2ComposeOp op);
  bool ComposeToWithRect(CJBig2_Image* pDst,
                         int32_t x,
                         int32_t y,
                         const FX_RECT& rtSrc,
                         JBig2ComposeOp op);

 private:
  void SubImageFast(int32_t x,
                    int32_t y,
                    int32_t w,
                    int32_t h,
                    CJBig2_Image* pImage);
  void SubImageSlow(int32_t x,
                    int32_t y,
                    int32_t w,
                    int32_t h,
                    CJBig2_Image* pImage);
  bool ComposeToInternal(CJBig2_Image* pDst,
                         int32_t x,
                         int32_t y,
                         JBig2ComposeOp op,
                         const FX_RECT& rtSrc);

  MaybeOwned<uint8_t, FxFreeDeleter> data_;
  int32_t width_ = 0;   // 1-bit pixels
  int32_t height_ = 0;  // lines
  int32_t stride_ = 0;  // bytes, must be multiple of 4.
};

#endif  // CORE_FXCODEC_JBIG2_JBIG2_IMAGE_H_
