// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jbig2/JBig2_Image.h"

#include <limits.h>
#include <stddef.h>

#include <algorithm>
#include <memory>

#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_safe_types.h"

#define JBIG2_GETDWORD(buf)                  \
  ((static_cast<uint32_t>((buf)[0]) << 24) | \
   (static_cast<uint32_t>((buf)[1]) << 16) | \
   (static_cast<uint32_t>((buf)[2]) << 8) |  \
   (static_cast<uint32_t>((buf)[3]) << 0))

#define JBIG2_PUTDWORD(buf, val)                 \
  ((buf)[0] = static_cast<uint8_t>((val) >> 24), \
   (buf)[1] = static_cast<uint8_t>((val) >> 16), \
   (buf)[2] = static_cast<uint8_t>((val) >> 8),  \
   (buf)[3] = static_cast<uint8_t>((val) >> 0))

namespace {

const int kMaxImagePixels = INT_MAX - 31;
const int kMaxImageBytes = kMaxImagePixels / 8;

int BitIndexToByte(int index) {
  return index / 8;
}

int BitIndexToAlignedByte(int index) {
  return index / 32 * 4;
}

}  // namespace

CJBig2_Image::CJBig2_Image(int32_t w, int32_t h) {
  if (w <= 0 || h <= 0 || w > kMaxImagePixels) {
    return;
  }

  int32_t stride_pixels = FxAlignToBoundary<32>(w);
  if (h > kMaxImagePixels / stride_pixels) {
    return;
  }

  width_ = w;
  height_ = h;
  stride_ = stride_pixels / 8;
  data_.Reset(std::unique_ptr<uint8_t, FxFreeDeleter>(
      FX_Alloc2D(uint8_t, stride_, height_)));
}

CJBig2_Image::CJBig2_Image(int32_t w,
                           int32_t h,
                           int32_t stride,
                           pdfium::span<uint8_t> pBuf) {
  if (w < 0 || h < 0) {
    return;
  }

  // Stride must be word-aligned.
  if (stride < 0 || stride > kMaxImageBytes || stride % 4 != 0) {
    return;
  }

  int32_t stride_pixels = 8 * stride;
  if (stride_pixels < w || h > kMaxImagePixels / stride_pixels) {
    return;
  }

  width_ = w;
  height_ = h;
  stride_ = stride;
  data_.Reset(pBuf.data());
}

CJBig2_Image::CJBig2_Image(const CJBig2_Image& other)
    : width_(other.width_), height_(other.height_), stride_(other.stride_) {
  if (other.data_) {
    data_.Reset(std::unique_ptr<uint8_t, FxFreeDeleter>(
        FX_Alloc2D(uint8_t, stride_, height_)));
    UNSAFE_TODO(FXSYS_memcpy(data(), other.data(), stride_ * height_));
  }
}

CJBig2_Image::~CJBig2_Image() = default;

// static
bool CJBig2_Image::IsValidImageSize(int32_t w, int32_t h) {
  return w > 0 && w <= kJBig2MaxImageSize && h > 0 && h <= kJBig2MaxImageSize;
}

int CJBig2_Image::GetPixel(int32_t x, int32_t y) const {
  if (!data_) {
    return 0;
  }

  if (x < 0 || x >= width_) {
    return 0;
  }

  const uint8_t* pLine = GetLine(y);
  if (!pLine) {
    return 0;
  }

  int32_t m = BitIndexToByte(x);
  int32_t n = x & 7;
  return UNSAFE_TODO((pLine[m] >> (7 - n)) & 1);
}

void CJBig2_Image::SetPixel(int32_t x, int32_t y, int v) {
  if (!data_) {
    return;
  }

  if (x < 0 || x >= width_) {
    return;
  }

  uint8_t* pLine = GetLine(y);
  if (!pLine) {
    return;
  }

  int32_t m = BitIndexToByte(x);
  int32_t n = 1 << (7 - (x & 7));
  if (v) {
    UNSAFE_TODO(pLine[m]) |= n;
  } else {
    UNSAFE_TODO(pLine[m]) &= ~n;
  }
}

void CJBig2_Image::CopyLine(int32_t hTo, int32_t hFrom) {
  if (!data_) {
    return;
  }

  uint8_t* pDst = GetLine(hTo);
  if (!pDst) {
    return;
  }

  const uint8_t* pSrc = GetLine(hFrom);
  UNSAFE_TODO({
    if (!pSrc) {
      FXSYS_memset(pDst, 0, stride_);
      return;
    }
    FXSYS_memcpy(pDst, pSrc, stride_);
  });
}

void CJBig2_Image::Fill(bool v) {
  if (!data_) {
    return;
  }

  UNSAFE_TODO(
      FXSYS_memset(data(), v ? 0xff : 0, Fx2DSizeOrDie(stride_, height_)));
}

bool CJBig2_Image::ComposeTo(CJBig2_Image* pDst,
                             int64_t x,
                             int64_t y,
                             JBig2ComposeOp op) {
  return data_ &&
         ComposeToInternal(pDst, x, y, op, FX_RECT(0, 0, width_, height_));
}

bool CJBig2_Image::ComposeToWithRect(CJBig2_Image* pDst,
                                     int64_t x,
                                     int64_t y,
                                     const FX_RECT& rtSrc,
                                     JBig2ComposeOp op) {
  return data_ && ComposeToInternal(pDst, x, y, op, rtSrc);
}

bool CJBig2_Image::ComposeFrom(int64_t x,
                               int64_t y,
                               CJBig2_Image* pSrc,
                               JBig2ComposeOp op) {
  return data_ && pSrc->ComposeTo(this, x, y, op);
}

bool CJBig2_Image::ComposeFromWithRect(int64_t x,
                                       int64_t y,
                                       CJBig2_Image* pSrc,
                                       const FX_RECT& rtSrc,
                                       JBig2ComposeOp op) {
  return data_ && pSrc->ComposeToWithRect(this, x, y, rtSrc, op);
}

std::unique_ptr<CJBig2_Image> CJBig2_Image::SubImage(int32_t x,
                                                     int32_t y,
                                                     int32_t w,
                                                     int32_t h) {
  auto pImage = std::make_unique<CJBig2_Image>(w, h);
  if (!pImage->data() || !data_) {
    return pImage;
  }

  if (x < 0 || x >= width_ || y < 0 || y >= height_) {
    return pImage;
  }

  // Fast case when byte-aligned, normal slow case otherwise.
  if ((x & 7) == 0) {
    SubImageFast(x, y, w, h, pImage.get());
  } else {
    SubImageSlow(x, y, w, h, pImage.get());
  }

  return pImage;
}

void CJBig2_Image::SubImageFast(int32_t x,
                                int32_t y,
                                int32_t w,
                                int32_t h,
                                CJBig2_Image* pImage) {
  int32_t m = BitIndexToByte(x);
  int32_t bytes_to_copy = std::min(pImage->stride_, stride_ - m);
  int32_t lines_to_copy = std::min(pImage->height_, height_ - y);
  for (int32_t j = 0; j < lines_to_copy; j++) {
    UNSAFE_TODO(FXSYS_memcpy(pImage->GetLineUnsafe(j), GetLineUnsafe(y + j) + m,
                             bytes_to_copy));
  }
}

void CJBig2_Image::SubImageSlow(int32_t x,
                                int32_t y,
                                int32_t w,
                                int32_t h,
                                CJBig2_Image* pImage) {
  int32_t m = BitIndexToAlignedByte(x);
  int32_t n = x & 31;
  int32_t bytes_to_copy = std::min(pImage->stride_, stride_ - m);
  int32_t lines_to_copy = std::min(pImage->height_, height_ - y);
  UNSAFE_TODO({
    for (int32_t j = 0; j < lines_to_copy; j++) {
      const uint8_t* pLineSrc = GetLineUnsafe(y + j);
      uint8_t* pLineDst = pImage->GetLineUnsafe(j);
      const uint8_t* pSrc = pLineSrc + m;
      const uint8_t* pSrcEnd = pLineSrc + stride_;
      uint8_t* pDstEnd = pLineDst + bytes_to_copy;
      for (uint8_t* pDst = pLineDst; pDst < pDstEnd; pSrc += 4, pDst += 4) {
        uint32_t wTmp = JBIG2_GETDWORD(pSrc) << n;
        if (pSrc + 4 < pSrcEnd) {
          wTmp |= (JBIG2_GETDWORD(pSrc + 4) >> (32 - n));
        }
        JBIG2_PUTDWORD(pDst, wTmp);
      }
    }
  });
}

void CJBig2_Image::Expand(int32_t h, bool v) {
  if (!data_ || h <= height_ || h > kMaxImageBytes / stride_) {
    return;
  }

  // Won't die unless kMaxImageBytes were to be increased someday.
  const size_t current_size = Fx2DSizeOrDie(height_, stride_);
  const size_t desired_size = Fx2DSizeOrDie(h, stride_);

  if (data_.IsOwned()) {
    data_.Reset(std::unique_ptr<uint8_t, FxFreeDeleter>(
        FX_Realloc(uint8_t, data_.ReleaseAndClear().release(), desired_size)));
  } else {
    uint8_t* pExternalBuffer = data();
    data_.Reset(std::unique_ptr<uint8_t, FxFreeDeleter>(
        FX_Alloc(uint8_t, desired_size)));
    UNSAFE_TODO(FXSYS_memcpy(data(), pExternalBuffer, current_size));
  }
  UNSAFE_TODO(FXSYS_memset(data() + current_size, v ? 0xff : 0,
                           desired_size - current_size));
  height_ = h;
}

bool CJBig2_Image::ComposeToInternal(CJBig2_Image* pDst,
                                     int64_t x_in,
                                     int64_t y_in,
                                     JBig2ComposeOp op,
                                     const FX_RECT& rtSrc) {
  DCHECK(data_);

  // TODO(weili): Check whether the range check is correct. Should x>=1048576?
  if (x_in < -1048576 || x_in > 1048576 || y_in < -1048576 || y_in > 1048576) {
    return false;
  }
  int32_t x = static_cast<int32_t>(x_in);
  int32_t y = static_cast<int32_t>(y_in);

  int32_t sw = rtSrc.Width();
  int32_t sh = rtSrc.Height();

  int32_t xs0 = x < 0 ? -x : 0;
  int32_t xs1;
  FX_SAFE_INT32 iChecked = pDst->width_;
  iChecked -= x;
  if (iChecked.IsValid() && sw > iChecked.ValueOrDie()) {
    xs1 = iChecked.ValueOrDie();
  } else {
    xs1 = sw;
  }

  int32_t ys0 = y < 0 ? -y : 0;
  int32_t ys1;
  iChecked = pDst->height_;
  iChecked -= y;
  if (iChecked.IsValid() && sh > iChecked.ValueOrDie()) {
    ys1 = iChecked.ValueOrDie();
  } else {
    ys1 = sh;
  }

  if (ys0 >= ys1 || xs0 >= xs1) {
    return false;
  }

  int32_t xd0 = std::max(x, 0);
  int32_t yd0 = std::max(y, 0);
  int32_t w = xs1 - xs0;
  int32_t h = ys1 - ys0;
  int32_t xd1 = xd0 + w;
  int32_t yd1 = yd0 + h;
  uint32_t d1 = xd0 & 31;
  uint32_t d2 = xd1 & 31;
  uint32_t s1 = xs0 & 31;
  uint32_t maskL = 0xffffffff >> d1;
  uint32_t maskR = 0xffffffff << ((32 - (xd1 & 31)) % 32);
  uint32_t maskM = maskL & maskR;
  UNSAFE_TODO({
    const uint8_t* lineSrc = GetLineUnsafe(rtSrc.top + ys0) +
                             BitIndexToAlignedByte(xs0 + rtSrc.left);
    const uint8_t* lineSrcEnd = data() + Fx2DSizeOrDie(height_, stride_);
    int32_t lineLeft = stride_ - BitIndexToAlignedByte(xs0);
    uint8_t* lineDst = pDst->GetLineUnsafe(yd0) + BitIndexToAlignedByte(xd0);
    if ((xd0 & ~31) == ((xd1 - 1) & ~31)) {
      if ((xs0 & ~31) == ((xs1 - 1) & ~31)) {
        if (s1 > d1) {
          uint32_t shift = s1 - d1;
          for (int32_t yy = yd0; yy < yd1; yy++) {
            if (lineSrc >= lineSrcEnd) {
              return false;
            }
            uint32_t tmp1 = JBIG2_GETDWORD(lineSrc) << shift;
            uint32_t tmp2 = JBIG2_GETDWORD(lineDst);
            uint32_t tmp = 0;
            switch (op) {
              case JBIG2_COMPOSE_OR:
                tmp = (tmp2 & ~maskM) | ((tmp1 | tmp2) & maskM);
                break;
              case JBIG2_COMPOSE_AND:
                tmp = (tmp2 & ~maskM) | ((tmp1 & tmp2) & maskM);
                break;
              case JBIG2_COMPOSE_XOR:
                tmp = (tmp2 & ~maskM) | ((tmp1 ^ tmp2) & maskM);
                break;
              case JBIG2_COMPOSE_XNOR:
                tmp = (tmp2 & ~maskM) | ((~(tmp1 ^ tmp2)) & maskM);
                break;
              case JBIG2_COMPOSE_REPLACE:
                tmp = (tmp2 & ~maskM) | (tmp1 & maskM);
                break;
            }
            JBIG2_PUTDWORD(lineDst, tmp);
            lineSrc += stride_;
            lineDst += pDst->stride_;
          }
        } else {
          uint32_t shift = d1 - s1;
          for (int32_t yy = yd0; yy < yd1; yy++) {
            if (lineSrc >= lineSrcEnd) {
              return false;
            }
            uint32_t tmp1 = JBIG2_GETDWORD(lineSrc) >> shift;
            uint32_t tmp2 = JBIG2_GETDWORD(lineDst);
            uint32_t tmp = 0;
            switch (op) {
              case JBIG2_COMPOSE_OR:
                tmp = (tmp2 & ~maskM) | ((tmp1 | tmp2) & maskM);
                break;
              case JBIG2_COMPOSE_AND:
                tmp = (tmp2 & ~maskM) | ((tmp1 & tmp2) & maskM);
                break;
              case JBIG2_COMPOSE_XOR:
                tmp = (tmp2 & ~maskM) | ((tmp1 ^ tmp2) & maskM);
                break;
              case JBIG2_COMPOSE_XNOR:
                tmp = (tmp2 & ~maskM) | ((~(tmp1 ^ tmp2)) & maskM);
                break;
              case JBIG2_COMPOSE_REPLACE:
                tmp = (tmp2 & ~maskM) | (tmp1 & maskM);
                break;
            }
            JBIG2_PUTDWORD(lineDst, tmp);
            lineSrc += stride_;
            lineDst += pDst->stride_;
          }
        }
      } else {
        uint32_t shift1 = s1 - d1;
        uint32_t shift2 = 32 - shift1;
        for (int32_t yy = yd0; yy < yd1; yy++) {
          if (lineSrc >= lineSrcEnd) {
            return false;
          }
          uint32_t tmp1 = (JBIG2_GETDWORD(lineSrc) << shift1) |
                          (JBIG2_GETDWORD(lineSrc + 4) >> shift2);
          uint32_t tmp2 = JBIG2_GETDWORD(lineDst);
          uint32_t tmp = 0;
          switch (op) {
            case JBIG2_COMPOSE_OR:
              tmp = (tmp2 & ~maskM) | ((tmp1 | tmp2) & maskM);
              break;
            case JBIG2_COMPOSE_AND:
              tmp = (tmp2 & ~maskM) | ((tmp1 & tmp2) & maskM);
              break;
            case JBIG2_COMPOSE_XOR:
              tmp = (tmp2 & ~maskM) | ((tmp1 ^ tmp2) & maskM);
              break;
            case JBIG2_COMPOSE_XNOR:
              tmp = (tmp2 & ~maskM) | ((~(tmp1 ^ tmp2)) & maskM);
              break;
            case JBIG2_COMPOSE_REPLACE:
              tmp = (tmp2 & ~maskM) | (tmp1 & maskM);
              break;
          }
          JBIG2_PUTDWORD(lineDst, tmp);
          lineSrc += stride_;
          lineDst += pDst->stride_;
        }
      }
    } else {
      if (s1 > d1) {
        uint32_t shift1 = s1 - d1;
        uint32_t shift2 = 32 - shift1;
        int32_t middleDwords = (xd1 >> 5) - ((xd0 + 31) >> 5);
        for (int32_t yy = yd0; yy < yd1; yy++) {
          if (lineSrc >= lineSrcEnd) {
            return false;
          }
          const uint8_t* sp = lineSrc;
          uint8_t* dp = lineDst;
          if (d1 != 0) {
            uint32_t tmp1 = (JBIG2_GETDWORD(sp) << shift1) |
                            (JBIG2_GETDWORD(sp + 4) >> shift2);
            uint32_t tmp2 = JBIG2_GETDWORD(dp);
            uint32_t tmp = 0;
            switch (op) {
              case JBIG2_COMPOSE_OR:
                tmp = (tmp2 & ~maskL) | ((tmp1 | tmp2) & maskL);
                break;
              case JBIG2_COMPOSE_AND:
                tmp = (tmp2 & ~maskL) | ((tmp1 & tmp2) & maskL);
                break;
              case JBIG2_COMPOSE_XOR:
                tmp = (tmp2 & ~maskL) | ((tmp1 ^ tmp2) & maskL);
                break;
              case JBIG2_COMPOSE_XNOR:
                tmp = (tmp2 & ~maskL) | ((~(tmp1 ^ tmp2)) & maskL);
                break;
              case JBIG2_COMPOSE_REPLACE:
                tmp = (tmp2 & ~maskL) | (tmp1 & maskL);
                break;
            }
            JBIG2_PUTDWORD(dp, tmp);
            sp += 4;
            dp += 4;
          }
          for (int32_t xx = 0; xx < middleDwords; xx++) {
            uint32_t tmp1 = (JBIG2_GETDWORD(sp) << shift1) |
                            (JBIG2_GETDWORD(sp + 4) >> shift2);
            uint32_t tmp2 = JBIG2_GETDWORD(dp);
            uint32_t tmp = 0;
            switch (op) {
              case JBIG2_COMPOSE_OR:
                tmp = tmp1 | tmp2;
                break;
              case JBIG2_COMPOSE_AND:
                tmp = tmp1 & tmp2;
                break;
              case JBIG2_COMPOSE_XOR:
                tmp = tmp1 ^ tmp2;
                break;
              case JBIG2_COMPOSE_XNOR:
                tmp = ~(tmp1 ^ tmp2);
                break;
              case JBIG2_COMPOSE_REPLACE:
                tmp = tmp1;
                break;
            }
            JBIG2_PUTDWORD(dp, tmp);
            sp += 4;
            dp += 4;
          }
          if (d2 != 0) {
            uint32_t tmp1 =
                (JBIG2_GETDWORD(sp) << shift1) |
                (((sp + 4) < lineSrc + lineLeft ? JBIG2_GETDWORD(sp + 4) : 0) >>
                 shift2);
            uint32_t tmp2 = JBIG2_GETDWORD(dp);
            uint32_t tmp = 0;
            switch (op) {
              case JBIG2_COMPOSE_OR:
                tmp = (tmp2 & ~maskR) | ((tmp1 | tmp2) & maskR);
                break;
              case JBIG2_COMPOSE_AND:
                tmp = (tmp2 & ~maskR) | ((tmp1 & tmp2) & maskR);
                break;
              case JBIG2_COMPOSE_XOR:
                tmp = (tmp2 & ~maskR) | ((tmp1 ^ tmp2) & maskR);
                break;
              case JBIG2_COMPOSE_XNOR:
                tmp = (tmp2 & ~maskR) | ((~(tmp1 ^ tmp2)) & maskR);
                break;
              case JBIG2_COMPOSE_REPLACE:
                tmp = (tmp2 & ~maskR) | (tmp1 & maskR);
                break;
            }
            JBIG2_PUTDWORD(dp, tmp);
          }
          lineSrc += stride_;
          lineDst += pDst->stride_;
        }
      } else if (s1 == d1) {
        int32_t middleDwords = (xd1 >> 5) - ((xd0 + 31) >> 5);
        for (int32_t yy = yd0; yy < yd1; yy++) {
          if (lineSrc >= lineSrcEnd) {
            return false;
          }
          const uint8_t* sp = lineSrc;
          uint8_t* dp = lineDst;
          if (d1 != 0) {
            uint32_t tmp1 = JBIG2_GETDWORD(sp);
            uint32_t tmp2 = JBIG2_GETDWORD(dp);
            uint32_t tmp = 0;
            switch (op) {
              case JBIG2_COMPOSE_OR:
                tmp = (tmp2 & ~maskL) | ((tmp1 | tmp2) & maskL);
                break;
              case JBIG2_COMPOSE_AND:
                tmp = (tmp2 & ~maskL) | ((tmp1 & tmp2) & maskL);
                break;
              case JBIG2_COMPOSE_XOR:
                tmp = (tmp2 & ~maskL) | ((tmp1 ^ tmp2) & maskL);
                break;
              case JBIG2_COMPOSE_XNOR:
                tmp = (tmp2 & ~maskL) | ((~(tmp1 ^ tmp2)) & maskL);
                break;
              case JBIG2_COMPOSE_REPLACE:
                tmp = (tmp2 & ~maskL) | (tmp1 & maskL);
                break;
            }
            JBIG2_PUTDWORD(dp, tmp);
            sp += 4;
            dp += 4;
          }
          for (int32_t xx = 0; xx < middleDwords; xx++) {
            uint32_t tmp1 = JBIG2_GETDWORD(sp);
            uint32_t tmp2 = JBIG2_GETDWORD(dp);
            uint32_t tmp = 0;
            switch (op) {
              case JBIG2_COMPOSE_OR:
                tmp = tmp1 | tmp2;
                break;
              case JBIG2_COMPOSE_AND:
                tmp = tmp1 & tmp2;
                break;
              case JBIG2_COMPOSE_XOR:
                tmp = tmp1 ^ tmp2;
                break;
              case JBIG2_COMPOSE_XNOR:
                tmp = ~(tmp1 ^ tmp2);
                break;
              case JBIG2_COMPOSE_REPLACE:
                tmp = tmp1;
                break;
            }
            JBIG2_PUTDWORD(dp, tmp);
            sp += 4;
            dp += 4;
          }
          if (d2 != 0) {
            uint32_t tmp1 = JBIG2_GETDWORD(sp);
            uint32_t tmp2 = JBIG2_GETDWORD(dp);
            uint32_t tmp = 0;
            switch (op) {
              case JBIG2_COMPOSE_OR:
                tmp = (tmp2 & ~maskR) | ((tmp1 | tmp2) & maskR);
                break;
              case JBIG2_COMPOSE_AND:
                tmp = (tmp2 & ~maskR) | ((tmp1 & tmp2) & maskR);
                break;
              case JBIG2_COMPOSE_XOR:
                tmp = (tmp2 & ~maskR) | ((tmp1 ^ tmp2) & maskR);
                break;
              case JBIG2_COMPOSE_XNOR:
                tmp = (tmp2 & ~maskR) | ((~(tmp1 ^ tmp2)) & maskR);
                break;
              case JBIG2_COMPOSE_REPLACE:
                tmp = (tmp2 & ~maskR) | (tmp1 & maskR);
                break;
            }
            JBIG2_PUTDWORD(dp, tmp);
          }
          lineSrc += stride_;
          lineDst += pDst->stride_;
        }
      } else {
        uint32_t shift1 = d1 - s1;
        uint32_t shift2 = 32 - shift1;
        int32_t middleDwords = (xd1 >> 5) - ((xd0 + 31) >> 5);
        for (int32_t yy = yd0; yy < yd1; yy++) {
          if (lineSrc >= lineSrcEnd) {
            return false;
          }
          const uint8_t* sp = lineSrc;
          uint8_t* dp = lineDst;
          if (d1 != 0) {
            uint32_t tmp1 = JBIG2_GETDWORD(sp) >> shift1;
            uint32_t tmp2 = JBIG2_GETDWORD(dp);
            uint32_t tmp = 0;
            switch (op) {
              case JBIG2_COMPOSE_OR:
                tmp = (tmp2 & ~maskL) | ((tmp1 | tmp2) & maskL);
                break;
              case JBIG2_COMPOSE_AND:
                tmp = (tmp2 & ~maskL) | ((tmp1 & tmp2) & maskL);
                break;
              case JBIG2_COMPOSE_XOR:
                tmp = (tmp2 & ~maskL) | ((tmp1 ^ tmp2) & maskL);
                break;
              case JBIG2_COMPOSE_XNOR:
                tmp = (tmp2 & ~maskL) | ((~(tmp1 ^ tmp2)) & maskL);
                break;
              case JBIG2_COMPOSE_REPLACE:
                tmp = (tmp2 & ~maskL) | (tmp1 & maskL);
                break;
            }
            JBIG2_PUTDWORD(dp, tmp);
            dp += 4;
          }
          for (int32_t xx = 0; xx < middleDwords; xx++) {
            uint32_t tmp1 = (JBIG2_GETDWORD(sp) << shift2) |
                            ((JBIG2_GETDWORD(sp + 4)) >> shift1);
            uint32_t tmp2 = JBIG2_GETDWORD(dp);
            uint32_t tmp = 0;
            switch (op) {
              case JBIG2_COMPOSE_OR:
                tmp = tmp1 | tmp2;
                break;
              case JBIG2_COMPOSE_AND:
                tmp = tmp1 & tmp2;
                break;
              case JBIG2_COMPOSE_XOR:
                tmp = tmp1 ^ tmp2;
                break;
              case JBIG2_COMPOSE_XNOR:
                tmp = ~(tmp1 ^ tmp2);
                break;
              case JBIG2_COMPOSE_REPLACE:
                tmp = tmp1;
                break;
            }
            JBIG2_PUTDWORD(dp, tmp);
            sp += 4;
            dp += 4;
          }
          if (d2 != 0) {
            uint32_t tmp1 =
                (JBIG2_GETDWORD(sp) << shift2) |
                (((sp + 4) < lineSrc + lineLeft ? JBIG2_GETDWORD(sp + 4) : 0) >>
                 shift1);
            uint32_t tmp2 = JBIG2_GETDWORD(dp);
            uint32_t tmp = 0;
            switch (op) {
              case JBIG2_COMPOSE_OR:
                tmp = (tmp2 & ~maskR) | ((tmp1 | tmp2) & maskR);
                break;
              case JBIG2_COMPOSE_AND:
                tmp = (tmp2 & ~maskR) | ((tmp1 & tmp2) & maskR);
                break;
              case JBIG2_COMPOSE_XOR:
                tmp = (tmp2 & ~maskR) | ((tmp1 ^ tmp2) & maskR);
                break;
              case JBIG2_COMPOSE_XNOR:
                tmp = (tmp2 & ~maskR) | ((~(tmp1 ^ tmp2)) & maskR);
                break;
              case JBIG2_COMPOSE_REPLACE:
                tmp = (tmp2 & ~maskR) | (tmp1 & maskR);
                break;
            }
            JBIG2_PUTDWORD(dp, tmp);
          }
          lineSrc += stride_;
          lineDst += pDst->stride_;
        }
      }
    }
  });
  return true;
}
