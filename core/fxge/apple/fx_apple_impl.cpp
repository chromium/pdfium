// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <stdint.h>

#include <memory>
#include <vector>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/span.h"
#include "core/fxge/agg/cfx_agg_cliprgn.h"
#include "core/fxge/agg/cfx_agg_devicedriver.h"
#include "core/fxge/apple/fx_apple_platform.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_glyphbitmap.h"
#include "core/fxge/cfx_glyphcache.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/text_char_pos.h"

namespace {

void DoNothing(void* info, const void* data, size_t size) {}

bool CGDrawGlyphRun(CGContextRef context,
                    pdfium::span<const TextCharPos> pCharPos,
                    CFX_Font* font,
                    const CFX_Matrix& mtObject2Device,
                    float font_size,
                    uint32_t argb) {
  if (pCharPos.empty()) {
    return true;
  }

  bool bNegSize = font_size < 0;
  if (bNegSize) {
    font_size = -font_size;
  }

  CFX_Matrix new_matrix = mtObject2Device;
  CQuartz2D& quartz2d =
      static_cast<CApplePlatform*>(CFX_GEModule::Get()->GetPlatform())
          ->quartz_2d_;
  if (!font->GetPlatformFont()) {
    if (font->GetPsName() == "DFHeiStd-W5") {
      return false;
    }

    font->SetPlatformFont(quartz2d.CreateFont(font->GetFontSpan()));
    if (!font->GetPlatformFont()) {
      return false;
    }
  }
  DataVector<uint16_t> glyph_indices(pCharPos.size());
  std::vector<CGPoint> glyph_positions(pCharPos.size());
  for (size_t i = 0; i < pCharPos.size(); i++) {
    glyph_indices[i] =
        pCharPos[i].ext_gid_ ? pCharPos[i].ext_gid_ : pCharPos[i].glyph_index_;
    if (bNegSize) {
      glyph_positions[i].x = -pCharPos[i].origin_.x;
    } else {
      glyph_positions[i].x = pCharPos[i].origin_.x;
    }
    glyph_positions[i].y = pCharPos[i].origin_.y;
  }
  if (bNegSize) {
    new_matrix.a = -new_matrix.a;
    new_matrix.c = -new_matrix.c;
  } else {
    new_matrix.b = -new_matrix.b;
    new_matrix.d = -new_matrix.d;
  }
  quartz2d.SetGraphicsTextMatrix(context, new_matrix);
  return quartz2d.DrawGraphicsString(context, font->GetPlatformFont(),
                                     font_size, glyph_indices, glyph_positions,
                                     argb);
}

}  // namespace

namespace pdfium {

void CFX_AggDeviceDriver::InitPlatform() {
  CQuartz2D& quartz2d =
      static_cast<CApplePlatform*>(CFX_GEModule::Get()->GetPlatform())
          ->quartz_2d_;
  platform_graphics_ = quartz2d.CreateGraphics(bitmap_);
}

void CFX_AggDeviceDriver::DestroyPlatform() {
  CQuartz2D& quartz2d =
      static_cast<CApplePlatform*>(CFX_GEModule::Get()->GetPlatform())
          ->quartz_2d_;
  if (platform_graphics_) {
    quartz2d.DestroyGraphics(platform_graphics_);
    platform_graphics_ = nullptr;
  }
}

bool CFX_AggDeviceDriver::DrawDeviceText(
    pdfium::span<const TextCharPos> pCharPos,
    CFX_Font* font,
    const CFX_Matrix& mtObject2Device,
    float font_size,
    uint32_t color,
    const CFX_TextRenderOptions& /*options*/) {
  if (!font) {
    return false;
  }

  bool bBold = font->IsBold();
  if (!bBold && font->GetSubstFont() && font->GetSubstFont()->weight_ >= 500 &&
      font->GetSubstFont()->weight_ <= 600) {
    return false;
  }
  for (const auto& cp : pCharPos) {
    if (cp.glyph_adjust_) {
      return false;
    }
  }
  CGContextRef ctx = CGContextRef(platform_graphics_);
  if (!ctx) {
    return false;
  }

  CGContextSaveGState(ctx);
  CGContextSetTextDrawingMode(ctx, kCGTextFillClip);
  CGRect rect_cg;
  CGImageRef pImageCG = nullptr;
  if (clip_rgn_) {
    rect_cg =
        CGRectMake(clip_rgn_->GetBox().left, clip_rgn_->GetBox().top,
                   clip_rgn_->GetBox().Width(), clip_rgn_->GetBox().Height());
    RetainPtr<CFX_DIBitmap> pClipMask = clip_rgn_->GetMask();
    if (pClipMask) {
      CGDataProviderRef pClipMaskDataProvider = CGDataProviderCreateWithData(
          nullptr, pClipMask->GetBuffer().data(),
          pClipMask->GetPitch() * pClipMask->GetHeight(), DoNothing);
      CGFloat decode_f[2] = {255.f, 0.f};
      pImageCG = CGImageMaskCreate(
          pClipMask->GetWidth(), pClipMask->GetHeight(), 8, 8,
          pClipMask->GetPitch(), pClipMaskDataProvider, decode_f, false);
      CGDataProviderRelease(pClipMaskDataProvider);
    }
  } else {
    rect_cg = CGRectMake(0, 0, bitmap_->GetWidth(), bitmap_->GetHeight());
  }
  rect_cg = CGContextConvertRectToDeviceSpace(ctx, rect_cg);
  if (pImageCG) {
    CGContextClipToMask(ctx, rect_cg, pImageCG);
  } else {
    CGContextClipToRect(ctx, rect_cg);
  }

  if (rgb_byte_order_) {
    uint8_t a = FXARGB_A(color);
    uint8_t r = FXARGB_R(color);
    uint8_t g = FXARGB_G(color);
    uint8_t b = FXARGB_B(color);
    color = ArgbEncode(a, b, g, r);
  }
  bool ret =
      CGDrawGlyphRun(ctx, pCharPos, font, mtObject2Device, font_size, color);
  if (pImageCG) {
    CGImageRelease(pImageCG);
  }
  CGContextRestoreGState(ctx);
  return ret;
}

}  // namespace pdfium

std::unique_ptr<CFX_GlyphBitmap> CFX_GlyphCache::RenderGlyph_Nativetext(
    const CFX_Font* font,
    uint32_t glyph_index,
    const CFX_Matrix& matrix,
    int dest_width,
    int anti_alias) {
  return nullptr;
}

void CFX_Font::ReleasePlatformResource() {
  if (platform_font_) {
    CQuartz2D& quartz2d =
        static_cast<CApplePlatform*>(CFX_GEModule::Get()->GetPlatform())
            ->quartz_2d_;
    quartz2d.DestroyFont(platform_font_);
    platform_font_ = nullptr;
  }
}
