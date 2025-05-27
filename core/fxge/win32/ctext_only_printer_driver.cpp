// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/win32/ctext_only_printer_driver.h"

#include <limits.h>
#include <stddef.h>

#include <algorithm>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/notreached.h"
#include "core/fxge/agg/cfx_agg_imagerenderer.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/text_char_pos.h"

CTextOnlyPrinterDriver::CTextOnlyPrinterDriver(HDC hDC)
    : dc_handle_(hDC),
      width_(INT_MAX),
      height_(INT_MAX),
      horz_size_(INT_MAX),
      vert_size_(INT_MAX),
      origin_y_(0.0f),
      set_origin_(false) {
  bits_per_pixel_ = ::GetDeviceCaps(dc_handle_, BITSPIXEL);
}

CTextOnlyPrinterDriver::~CTextOnlyPrinterDriver() = default;

DeviceType CTextOnlyPrinterDriver::GetDeviceType() const {
  return DeviceType::kPrinter;
}

int CTextOnlyPrinterDriver::GetDeviceCaps(int caps_id) const {
  switch (caps_id) {
    case FXDC_PIXEL_WIDTH:
      return width_;
    case FXDC_PIXEL_HEIGHT:
      return height_;
    case FXDC_BITS_PIXEL:
      return bits_per_pixel_;
    case FXDC_RENDER_CAPS:
      return 0;
    case FXDC_HORZ_SIZE:
      return horz_size_;
    case FXDC_VERT_SIZE:
      return vert_size_;
    default:
      NOTREACHED();
  }
}

void CTextOnlyPrinterDriver::SaveState() {}

void CTextOnlyPrinterDriver::RestoreState(bool bKeepSaved) {}

bool CTextOnlyPrinterDriver::SetClip_PathFill(
    const CFX_Path& path,
    const CFX_Matrix* pObject2Device,
    const CFX_FillRenderOptions& fill_options) {
  return true;
}

bool CTextOnlyPrinterDriver::SetClip_PathStroke(
    const CFX_Path& path,
    const CFX_Matrix* pObject2Device,
    const CFX_GraphStateData* pGraphState) {
  return false;
}

bool CTextOnlyPrinterDriver::DrawPath(
    const CFX_Path& path,
    const CFX_Matrix* pObject2Device,
    const CFX_GraphStateData* pGraphState,
    uint32_t fill_color,
    uint32_t stroke_color,
    const CFX_FillRenderOptions& fill_options) {
  return false;
}

bool CTextOnlyPrinterDriver::SetDIBits(RetainPtr<const CFX_DIBBase> bitmap,
                                       uint32_t color,
                                       const FX_RECT& src_rect,
                                       int left,
                                       int top,
                                       BlendMode blend_type) {
  return false;
}

FX_RECT CTextOnlyPrinterDriver::GetClipBox() const {
  return FX_RECT(0, 0, width_, height_);
}

bool CTextOnlyPrinterDriver::StretchDIBits(RetainPtr<const CFX_DIBBase> bitmap,
                                           uint32_t color,
                                           int dest_left,
                                           int dest_top,
                                           int dest_width,
                                           int dest_height,
                                           const FX_RECT* pClipRect,
                                           const FXDIB_ResampleOptions& options,
                                           BlendMode blend_type) {
  return false;
}

RenderDeviceDriverIface::StartResult CTextOnlyPrinterDriver::StartDIBits(
    RetainPtr<const CFX_DIBBase> bitmap,
    float alpha,
    uint32_t color,
    const CFX_Matrix& matrix,
    const FXDIB_ResampleOptions& options,
    BlendMode blend_type) {
  return {Result::kNotSupported, nullptr};
}

bool CTextOnlyPrinterDriver::DrawDeviceText(
    pdfium::span<const TextCharPos> pCharPos,
    CFX_Font* font,
    const CFX_Matrix& mtObject2Device,
    float font_size,
    uint32_t color,
    const CFX_TextRenderOptions& /*options*/) {
  if (g_pdfium_print_mode != WindowsPrintMode::kTextOnly) {
    return false;
  }
  if (pCharPos.empty() || !font) {
    return false;
  }

  // Scale factor used to minimize the kerning problems caused by rounding
  // errors below. Value chosen based on the title of https://crbug.com/18383
  const double kScaleFactor = 10;

  // Detect new lines and add clrf characters (since this is Windows only).
  // These characters are removed by SkPDF, but the new line information is
  // preserved in the text location. clrf characters seem to be ignored by
  // label printers that use this driver.
  WideString wsText;
  size_t len = pCharPos.size();
  float fOffsetY = mtObject2Device.f * kScaleFactor;
  if (set_origin_ && FXSYS_roundf(origin_y_) != FXSYS_roundf(fOffsetY)) {
    wsText += L"\r\n";
    len += 2;
  }
  wsText.Reserve(len);
  origin_y_ = fOffsetY;
  set_origin_ = true;

  // Text
  for (const auto& charpos : pCharPos) {
    // Only works with PDFs from Skia's PDF generator. Cannot handle arbitrary
    // values from PDFs.
    DCHECK_EQ(charpos.adjust_matrix_[0], 0);
    DCHECK_EQ(charpos.adjust_matrix_[1], 0);
    DCHECK_EQ(charpos.adjust_matrix_[2], 0);
    DCHECK_EQ(charpos.adjust_matrix_[3], 0);
    DCHECK_EQ(charpos.origin_.y, 0);
    wsText += charpos.unicode_;
  }
  ByteString text = wsText.ToDefANSI();
  auto text_span = text.span();
  while (!text_span.empty()) {
    uint8_t buffer[1026];
    size_t send_len = std::min<size_t>(text_span.size(), 1024);
    *(reinterpret_cast<uint16_t*>(buffer)) = static_cast<uint16_t>(send_len);
    UNSAFE_TODO(FXSYS_memcpy(buffer + 2, text_span.data(), send_len));
    ::GdiComment(dc_handle_, static_cast<UINT>(send_len + 2), buffer);
    text_span = text_span.subspan(send_len);
  }
  return true;
}

bool CTextOnlyPrinterDriver::MultiplyAlpha(float alpha) {
  // Not needed. All callers are using `CFX_DIBitmap`-backed raster devices
  // anyway.
  NOTREACHED();
}

bool CTextOnlyPrinterDriver::MultiplyAlphaMask(
    RetainPtr<const CFX_DIBitmap> mask) {
  // Not needed. All callers are using `CFX_DIBitmap`-backed raster devices
  // anyway.
  NOTREACHED();
}
