// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXGE_SKIA_FX_SKIA_DEVICE_H_
#define CORE_FXGE_SKIA_FX_SKIA_DEVICE_H_

#include <stdint.h>

#include <memory>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/renderdevicedriver_iface.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkRSXform.h"
#include "third_party/skia/include/core/SkRefCnt.h"

class CFX_Font;
class CFX_Matrix;
class SkCanvas;
class SkPaint;
class SkPath;
class SkSurface;
class TextCharPos;
struct CFX_TextRenderOptions;

// Assumes Skia is not going to add non-data members to its fundamental types.
FX_DATA_PARTITION_EXCEPTION(SkPoint);
FX_DATA_PARTITION_EXCEPTION(SkRSXform);

class CFX_SkiaDeviceDriver final : public RenderDeviceDriverIface {
 public:
  static std::unique_ptr<CFX_SkiaDeviceDriver> Create(
      RetainPtr<CFX_DIBitmap> pBitmap,
      bool bRgbByteOrder,
      RetainPtr<CFX_DIBitmap> pBackdropBitmap,
      bool bGroupKnockout);
  static std::unique_ptr<CFX_SkiaDeviceDriver> Create(SkCanvas& canvas);

  ~CFX_SkiaDeviceDriver() override;

  // RenderDeviceDriverIface:
  DeviceType GetDeviceType() const override;
  int GetDeviceCaps(int caps_id) const override;
  void SaveState() override;
  void RestoreState(bool bKeepSaved) override;
  bool SetClip_PathFill(const CFX_Path& path,
                        const CFX_Matrix* pObject2Device,
                        const CFX_FillRenderOptions& fill_options) override;
  bool SetClip_PathStroke(const CFX_Path& path,
                          const CFX_Matrix* pObject2Device,
                          const CFX_GraphStateData* pGraphState) override;
  bool DrawPath(const CFX_Path& path,
                const CFX_Matrix* pObject2Device,
                const CFX_GraphStateData* pGraphState,
                uint32_t fill_color,
                uint32_t stroke_color,
                const CFX_FillRenderOptions& fill_options) override;
  bool FillRect(const FX_RECT& rect, uint32_t fill_color) override;
  FX_RECT GetClipBox() const override;
  bool GetDIBits(RetainPtr<CFX_DIBitmap> bitmap,
                 int left,
                 int top) const override;
  RetainPtr<const CFX_DIBitmap> GetBackDrop() const override;
  bool SetDIBits(RetainPtr<const CFX_DIBBase> bitmap,
                 uint32_t color,
                 const FX_RECT& src_rect,
                 int dest_left,
                 int dest_top,
                 BlendMode blend_type) override;
  bool SetBitsWithMask(RetainPtr<const CFX_DIBBase> bitmap,
                       RetainPtr<const CFX_DIBBase> mask,
                       int dest_left,
                       int dest_top,
                       float alpha,
                       BlendMode blend_type) override;
  void SetGroupKnockout(bool group_knockout) override;
  void SyncInternalBitmaps() override;
  bool StretchDIBits(RetainPtr<const CFX_DIBBase> bitmap,
                     uint32_t color,
                     int dest_left,
                     int dest_top,
                     int dest_width,
                     int dest_height,
                     const FX_RECT* pClipRect,
                     const FXDIB_ResampleOptions& options,
                     BlendMode blend_type) override;
  StartResult StartDIBits(RetainPtr<const CFX_DIBBase> bitmap,
                          float alpha,
                          uint32_t color,
                          const CFX_Matrix& matrix,
                          const FXDIB_ResampleOptions& options,
                          BlendMode blend_type) override;
  bool DrawBitsWithMask(RetainPtr<const CFX_DIBBase> bitmap,
                        RetainPtr<const CFX_DIBBase> mask,
                        float alpha,
                        const CFX_Matrix& matrix,
                        BlendMode blend_type);
  bool DrawDeviceText(pdfium::span<const TextCharPos> pCharPos,
                      CFX_Font* pFont,
                      const CFX_Matrix& mtObject2Device,
                      float font_size,
                      uint32_t color,
                      const CFX_TextRenderOptions& options) override;
  int GetDriverType() const override;
  bool DrawShading(const CPDF_ShadingPattern& pattern,
                   const CFX_Matrix& matrix,
                   const FX_RECT& clip_rect,
                   int alpha) override;

  bool MultiplyAlpha(float alpha) override;
  bool MultiplyAlphaMask(RetainPtr<const CFX_DIBitmap> mask) override;

  void Clear(uint32_t color);
  void Dump() const;

 private:
  class CharDetail {
   public:
    CharDetail();
    ~CharDetail();

    const DataVector<SkPoint>& GetPositions() const { return m_positions; }
    void SetPositionAt(size_t index, const SkPoint& position) {
      m_positions[index] = position;
    }
    const DataVector<uint16_t>& GetGlyphs() const { return m_glyphs; }
    void SetGlyphAt(size_t index, uint16_t glyph) { m_glyphs[index] = glyph; }
    const DataVector<uint32_t>& GetFontCharWidths() const {
      return m_fontCharWidths;
    }
    void SetFontCharWidthAt(size_t index, uint32_t width) {
      m_fontCharWidths[index] = width;
    }
    size_t Count() const {
      DCHECK_EQ(m_positions.size(), m_glyphs.size());
      return m_glyphs.size();
    }
    void SetCount(size_t count) {
      m_positions.resize(count);
      m_glyphs.resize(count);
      m_fontCharWidths.resize(count);
    }

   private:
    DataVector<SkPoint> m_positions;  // accumulator for text positions
    DataVector<uint16_t> m_glyphs;    // accumulator for text glyphs
    // accumulator for glyphs' width defined in pdf
    DataVector<uint32_t> m_fontCharWidths;
  };

  // Use the public creation methods instead.
  CFX_SkiaDeviceDriver(RetainPtr<CFX_DIBitmap> pBitmap,
                       bool bRgbByteOrder,
                       RetainPtr<CFX_DIBitmap> pBackdropBitmap,
                       bool bGroupKnockout);
  explicit CFX_SkiaDeviceDriver(SkCanvas& canvas);

  bool TryDrawText(pdfium::span<const TextCharPos> char_pos,
                   const CFX_Font* pFont,
                   const CFX_Matrix& matrix,
                   float font_size,
                   uint32_t color,
                   const CFX_TextRenderOptions& options);

  bool StartDIBitsSkia(RetainPtr<const CFX_DIBBase> bitmap,
                       const FX_RECT& src_rect,
                       float alpha,
                       uint32_t color,
                       const CFX_Matrix& matrix,
                       const FXDIB_ResampleOptions& options,
                       BlendMode blend_type);

  // A wrapper around SkCanvas::drawPath() that optionally can show debug data.
  void DrawPathImpl(const SkPath& path, const SkPaint& paint);

  RetainPtr<CFX_DIBitmap> m_pBitmap;
  RetainPtr<CFX_DIBitmap> m_pBackdropBitmap;

  // The input bitmap passed by the render device. Only used when the input
  // bitmap is 24 bpp and cannot be directly used as the back of a SkCanvas.
  RetainPtr<CFX_DIBitmap> m_pOriginalBitmap;

  sk_sp<SkSurface> surface_;
  UnownedPtr<SkCanvas> m_pCanvas;
  CFX_FillRenderOptions m_FillOptions;
  bool m_bRgbByteOrder;
  bool m_bGroupKnockout;

  CharDetail m_charDetails;
  // accumulator for txt rotate/scale/translate
  DataVector<SkRSXform> m_rsxform;
};

#endif  // CORE_FXGE_SKIA_FX_SKIA_DEVICE_H_
