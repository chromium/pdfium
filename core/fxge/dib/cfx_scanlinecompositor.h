// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CFX_SCANLINECOMPOSITOR_H_
#define CORE_FXGE_DIB_CFX_SCANLINECOMPOSITOR_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <variant>

#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/span.h"
#include "core/fxge/dib/fx_dib.h"

class CFX_ScanlineCompositor {
 public:
  struct GrayWithAlpha {
    uint8_t gray;
    uint8_t alpha;
  };

  CFX_ScanlineCompositor();
  ~CFX_ScanlineCompositor();

  bool Init(FXDIB_Format dest_format,
            FXDIB_Format src_format,
            pdfium::span<const uint32_t> src_palette,
            uint32_t mask_color,
            BlendMode blend_type,
            bool bRgbByteOrder);

  void CompositeRgbBitmapLine(pdfium::span<uint8_t> dest_scan,
                              pdfium::span<const uint8_t> src_scan,
                              int width,
                              pdfium::span<const uint8_t> clip_scan) const;

  void CompositePalBitmapLine(pdfium::span<uint8_t> dest_scan,
                              pdfium::span<const uint8_t> src_scan,
                              int src_left,
                              int width,
                              pdfium::span<const uint8_t> clip_scan) const;

  void CompositeByteMaskLine(pdfium::span<uint8_t> dest_scan,
                             pdfium::span<const uint8_t> src_scan,
                             int width,
                             pdfium::span<const uint8_t> clip_scan) const;

  void CompositeBitMaskLine(pdfium::span<uint8_t> dest_scan,
                            pdfium::span<const uint8_t> src_scan,
                            int src_left,
                            int width,
                            pdfium::span<const uint8_t> clip_scan) const;

 private:
  class Palette {
   public:
    Palette();
    ~Palette();

    void Reset();
    pdfium::span<uint8_t> Make8BitPalette(size_t nElements);
    pdfium::span<uint32_t> Make32BitPalette(size_t nElements);

    // Hard CHECK() if mismatch between created and requested widths.
    pdfium::span<const uint8_t> Get8BitPalette() const;
    pdfium::span<const uint32_t> Get32BitPalette() const;

   private:
    // If 0, then no |data_|.
    // If 1, then |data_| is really uint8_t* instead.
    // If 4, then |data_| is uint32_t* as expected.
    size_t width_ = 0;
    size_t elements_ = 0;

    // TODO(tsepez): convert to variant of FixedArray.
    std::unique_ptr<uint32_t, FxFreeDeleter> data_;
  };

  void InitSourcePalette(pdfium::span<const uint32_t> src_palette);

  void InitSourceMask(FX_ARGB mask_color);

  void CompositeRgbBitmapLineSrcBgrx(
      pdfium::span<uint8_t> dest_scan,
      pdfium::span<const uint8_t> src_scan,
      int width,
      pdfium::span<const uint8_t> clip_scan) const;
  void CompositeRgbBitmapLineSrcBgra(
      pdfium::span<uint8_t> dest_scan,
      pdfium::span<const uint8_t> src_scan,
      int width,
      pdfium::span<const uint8_t> clip_scan) const;
#if defined(PDF_USE_SKIA)
  void CompositeRgbBitmapLineSrcBgraPremul(pdfium::span<uint8_t> dest_scan,
                                           pdfium::span<const uint8_t> src_scan,
                                           int width) const;
#endif

  void CompositePalBitmapLineSrcBpp1(
      pdfium::span<uint8_t> dest_scan,
      pdfium::span<const uint8_t> src_scan,
      int src_left,
      int width,
      pdfium::span<const uint8_t> clip_scan) const;
  void CompositePalBitmapLineSrcBpp8(
      pdfium::span<uint8_t> dest_scan,
      pdfium::span<const uint8_t> src_scan,
      int src_left,
      int width,
      pdfium::span<const uint8_t> clip_scan) const;

  FXDIB_Format src_format_;
  FXDIB_Format dest_format_;
  Palette src_palette_;
  std::variant<FX_BGRA_STRUCT<uint8_t>, GrayWithAlpha, uint8_t> mask_color_;
  BlendMode blend_type_ = BlendMode::kNormal;
  bool rgb_byte_order_ = false;
};

#endif  // CORE_FXGE_DIB_CFX_SCANLINECOMPOSITOR_H_
