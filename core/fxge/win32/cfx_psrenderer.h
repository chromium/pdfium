// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_WIN32_CFX_PSRENDERER_H_
#define CORE_FXGE_WIN32_CFX_PSRENDERER_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_graphstatedata.h"

class CFX_DIBBase;
class CFX_GlyphCache;
class CFX_Font;
class CFX_PathData;
class CPSFont;
class TextCharPos;
struct FXDIB_ResampleOptions;

struct EncoderIface {
  bool (*pA85EncodeFunc)(pdfium::span<const uint8_t> src_buf,
                         std::unique_ptr<uint8_t, FxFreeDeleter>* dest_buf,
                         uint32_t* dest_size);
  void (*pFaxEncodeFunc)(const uint8_t* src_buf,
                         int width,
                         int height,
                         int pitch,
                         std::unique_ptr<uint8_t, FxFreeDeleter>* dest_buf,
                         uint32_t* dest_size);
  bool (*pFlateEncodeFunc)(const uint8_t* src_buf,
                           uint32_t src_size,
                           std::unique_ptr<uint8_t, FxFreeDeleter>* dest_buf,
                           uint32_t* dest_size);
  bool (*pJpegEncodeFunc)(const RetainPtr<CFX_DIBBase>& pSource,
                          uint8_t** dest_buf,
                          size_t* dest_size);
  bool (*pRunLengthEncodeFunc)(
      pdfium::span<const uint8_t> src_buf,
      std::unique_ptr<uint8_t, FxFreeDeleter>* dest_buf,
      uint32_t* dest_size);
};

class CFX_PSRenderer {
 public:
  explicit CFX_PSRenderer(const EncoderIface* pEncoderIface);
  ~CFX_PSRenderer();

  void Init(const RetainPtr<IFX_RetainableWriteStream>& stream,
            int pslevel,
            int width,
            int height,
            bool bCmykOutput);
  bool StartRendering();
  void EndRendering();
  void SaveState();
  void RestoreState(bool bKeepSaved);
  void SetClip_PathFill(const CFX_PathData* pPathData,
                        const CFX_Matrix* pObject2Device,
                        int fill_mode);
  void SetClip_PathStroke(const CFX_PathData* pPathData,
                          const CFX_Matrix* pObject2Device,
                          const CFX_GraphStateData* pGraphState);
  FX_RECT GetClipBox() { return m_ClipBox; }
  bool DrawPath(const CFX_PathData* pPathData,
                const CFX_Matrix* pObject2Device,
                const CFX_GraphStateData* pGraphState,
                uint32_t fill_color,
                uint32_t stroke_color,
                int fill_mode);
  bool SetDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                 uint32_t color,
                 int dest_left,
                 int dest_top);
  bool StretchDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                     uint32_t color,
                     int dest_left,
                     int dest_top,
                     int dest_width,
                     int dest_height,
                     const FXDIB_ResampleOptions& options);
  bool DrawDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                  uint32_t color,
                  const CFX_Matrix& matrix,
                  const FXDIB_ResampleOptions& options);
  bool DrawText(int nChars,
                const TextCharPos* pCharPos,
                CFX_Font* pFont,
                const CFX_Matrix& mtObject2Device,
                float font_size,
                uint32_t color);

 private:
  void OutputPath(const CFX_PathData* pPathData,
                  const CFX_Matrix* pObject2Device);
  void SetGraphState(const CFX_GraphStateData* pGraphState);
  void SetColor(uint32_t color);
  void FindPSFontGlyph(CFX_GlyphCache* pGlyphCache,
                       CFX_Font* pFont,
                       const TextCharPos& charpos,
                       int* ps_fontnum,
                       int* ps_glyphindex);
  bool FaxCompressData(std::unique_ptr<uint8_t, FxFreeDeleter> src_buf,
                       int width,
                       int height,
                       std::unique_ptr<uint8_t, FxFreeDeleter>* dest_buf,
                       uint32_t* dest_size) const;
  void PSCompressData(uint8_t* src_buf,
                      uint32_t src_size,
                      uint8_t** output_buf,
                      uint32_t* output_size,
                      const char** filter) const;
  void WritePSBinary(const uint8_t* data, int len);
  void WriteToStream(std::ostringstream* stringStream);

  bool m_bInited = false;
  bool m_bGraphStateSet = false;
  bool m_bCmykOutput;
  bool m_bColorSet = false;
  int m_PSLevel = 0;
  uint32_t m_LastColor = 0;
  FX_RECT m_ClipBox;
  CFX_GraphStateData m_CurGraphState;
  const EncoderIface* const m_pEncoderIface;
  RetainPtr<IFX_RetainableWriteStream> m_pStream;
  std::vector<std::unique_ptr<CPSFont>> m_PSFontList;
  std::vector<FX_RECT> m_ClipBoxStack;
};

#endif  // CORE_FXGE_WIN32_CFX_PSRENDERER_H_
