// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_PROGRESSIVE_DECODER_H_
#define CORE_FXCODEC_PROGRESSIVE_DECODER_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <utility>

#include "core/fxcodec/fx_codec_def.h"
#include "core/fxcodec/jpeg/jpegmodule.h"
#include "core/fxcodec/progressive_decoder_iface.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/cstretchengine.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/span.h"

#ifdef PDF_ENABLE_XFA_BMP
#include "core/fxcodec/bmp/bmp_decoder.h"
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
#include "core/fxcodec/gif/gif_decoder.h"
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_PNG
#include "core/fxcodec/png/png_decoder.h"
#endif  // PDF_ENABLE_XFA_PNG

class CFX_DIBitmap;
class IFX_SeekableReadStream;

namespace fxcodec {

class CFX_DIBAttribute;

class Dummy {};  // Placeholder to work around C++ syntax issues

class ProgressiveDecoder final :
#ifdef PDF_ENABLE_XFA_BMP
    public BmpDecoder::Delegate,
#endif  // PDF_ENABLE_XFA_BMP
#ifdef PDF_ENABLE_XFA_GIF
    public GifDecoder::Delegate,
#endif  // PDF_ENABLE_XFA_GIF
#ifdef PDF_ENABLE_XFA_PNG
    public PngDecoder::Delegate,
#endif  // PDF_ENABLE_XFA_PNG
    public Dummy {
 public:
  enum FXCodec_Format {
    FXCodec_Invalid = 0,
    FXCodec_1bppGray = 0x101,
    FXCodec_1bppRgb = 0x001,
    FXCodec_8bppGray = 0x108,
    FXCodec_8bppRgb = 0x008,
    FXCodec_Rgb = 0x018,
    FXCodec_Rgb32 = 0x020,
    FXCodec_Argb = 0x220,
    FXCodec_Cmyk = 0x120
  };

  ProgressiveDecoder();
  virtual ~ProgressiveDecoder();

  FXCODEC_STATUS LoadImageInfo(RetainPtr<IFX_SeekableReadStream> pFile,
                               FXCODEC_IMAGE_TYPE imageType,
                               CFX_DIBAttribute* pAttribute,
                               bool bSkipImageTypeCheck);

  FXCODEC_IMAGE_TYPE GetType() const { return m_imageType; }
  int32_t GetWidth() const { return m_SrcWidth; }
  int32_t GetHeight() const { return m_SrcHeight; }
  int32_t GetNumComponents() const { return m_SrcComponents; }
  int32_t GetBPC() const { return m_SrcBPC; }
  void SetClipBox(FX_RECT* clip);

  std::pair<FXCODEC_STATUS, size_t> GetFrames();
  FXCODEC_STATUS StartDecode(const RetainPtr<CFX_DIBitmap>& pDIBitmap,
                             int start_x,
                             int start_y,
                             int size_x,
                             int size_y);

  FXCODEC_STATUS ContinueDecode();

#ifdef PDF_ENABLE_XFA_PNG
  // PngDecoder::Delegate
  bool PngReadHeader(int width,
                     int height,
                     int bpc,
                     int pass,
                     int* color_type,
                     double* gamma) override;
  bool PngAskScanlineBuf(int line, uint8_t** pSrcBuf) override;
  void PngFillScanlineBufCompleted(int pass, int line) override;
#endif  // PDF_ENABLE_XFA_PNG

#ifdef PDF_ENABLE_XFA_GIF
  // GifDecoder::Delegate
  uint32_t GifCurrentPosition() const override;
  bool GifInputRecordPositionBuf(uint32_t rcd_pos,
                                 const FX_RECT& img_rc,
                                 int32_t pal_num,
                                 CFX_GifPalette* pal_ptr,
                                 int32_t trans_index,
                                 bool interlace) override;
  void GifReadScanline(int32_t row_num, pdfium::span<uint8_t> row_buf) override;
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_BMP
  // BmpDecoder::Delegate
  bool BmpInputImagePositionBuf(uint32_t rcd_pos) override;
  void BmpReadScanline(uint32_t row_num,
                       pdfium::span<const uint8_t> row_buf) override;
#endif  // PDF_ENABLE_XFA_BMP

 private:
  using WeightTable = CStretchEngine::WeightTable;
  using PixelWeight = CStretchEngine::PixelWeight;

  class HorzTable {
   public:
    HorzTable();
    ~HorzTable();

    void CalculateWeights(int dest_len, int src_len);
    PixelWeight* GetPixelWeight(int pixel) {
      return reinterpret_cast<PixelWeight*>(m_pWeightTables.data() +
                                            pixel * m_ItemSize);
    }

   private:
    int m_ItemSize = 0;
    DataVector<uint8_t> m_pWeightTables;
  };

  class VertTable {
   public:
    VertTable();
    ~VertTable();

    void CalculateWeights(int dest_len, int src_len);
    PixelWeight* GetPixelWeight(int pixel) {
      return reinterpret_cast<PixelWeight*>(m_pWeightTables.data() +
                                            pixel * m_ItemSize);
    }

   private:
    int m_ItemSize = 0;
    DataVector<uint8_t> m_pWeightTables;
  };

#ifdef PDF_ENABLE_XFA_BMP
  bool BmpReadMoreData(ProgressiveDecoderIface::Context* pBmpContext,
                       FXCODEC_STATUS* err_status);
  bool BmpDetectImageTypeInBuffer(CFX_DIBAttribute* pAttribute);
  FXCODEC_STATUS BmpStartDecode();
  FXCODEC_STATUS BmpContinueDecode();
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
  bool GifReadMoreData(FXCODEC_STATUS* err_status);
  bool GifDetectImageTypeInBuffer();
  FXCODEC_STATUS GifStartDecode();
  FXCODEC_STATUS GifContinueDecode();
  void GifDoubleLineResampleVert(const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
                                 double scale_y,
                                 int dest_row);
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_PNG
  void PngOneOneMapResampleHorz(const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
                                int32_t dest_line,
                                pdfium::span<uint8_t> src_span,
                                FXCodec_Format src_format);
  bool PngDetectImageTypeInBuffer(CFX_DIBAttribute* pAttribute);
  FXCODEC_STATUS PngStartDecode();
  FXCODEC_STATUS PngContinueDecode();
#endif  // PDF_ENABLE_XFA_PNG

#ifdef PDF_ENABLE_XFA_TIFF
  bool TiffDetectImageTypeFromFile(CFX_DIBAttribute* pAttribute);
  FXCODEC_STATUS TiffContinueDecode();
#endif  // PDF_ENABLE_XFA_TIFF

  bool JpegReadMoreData(FXCODEC_STATUS* err_status);
  bool JpegDetectImageTypeInBuffer(CFX_DIBAttribute* pAttribute);
  FXCODEC_STATUS JpegStartDecode(FXDIB_Format format);
  FXCODEC_STATUS JpegContinueDecode();

  bool DetectImageType(FXCODEC_IMAGE_TYPE imageType,
                       CFX_DIBAttribute* pAttribute);
  bool ReadMoreData(ProgressiveDecoderIface* pModule,
                    ProgressiveDecoderIface::Context* pContext,
                    FXCODEC_STATUS* err_status);

  int GetDownScale();
  void GetTransMethod(FXDIB_Format dest_format, FXCodec_Format src_format);

  void ResampleScanline(const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
                        int32_t dest_line,
                        pdfium::span<uint8_t> src_span,
                        FXCodec_Format src_format);
  void Resample(const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
                int32_t src_line,
                uint8_t* src_scan,
                FXCodec_Format src_format);
  void ResampleVert(const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
                    double scale_y,
                    int dest_row);
  void ResampleVertBT(const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
                      double scale_y,
                      int dest_row);

  FXCODEC_STATUS m_status = FXCODEC_STATUS::kDecodeFinished;
  FXCODEC_IMAGE_TYPE m_imageType = FXCODEC_IMAGE_UNKNOWN;
  RetainPtr<IFX_SeekableReadStream> m_pFile;
  RetainPtr<CFX_DIBitmap> m_pDeviceBitmap;
  RetainPtr<CFX_CodecMemory> m_pCodecMemory;
  DataVector<uint8_t> m_DecodeBuf;
  DataVector<FX_ARGB> m_SrcPalette;
  std::unique_ptr<ProgressiveDecoderIface::Context> m_pJpegContext;
#ifdef PDF_ENABLE_XFA_BMP
  std::unique_ptr<ProgressiveDecoderIface::Context> m_pBmpContext;
#endif  // PDF_ENABLE_XFA_BMP
#ifdef PDF_ENABLE_XFA_GIF
  std::unique_ptr<ProgressiveDecoderIface::Context> m_pGifContext;
#endif  // PDF_ENABLE_XFA_GIF
#ifdef PDF_ENABLE_XFA_PNG
  std::unique_ptr<ProgressiveDecoderIface::Context> m_pPngContext;
#endif  // PDF_ENABLE_XFA_PNG
#ifdef PDF_ENABLE_XFA_TIFF
  std::unique_ptr<ProgressiveDecoderIface::Context> m_pTiffContext;
#endif  // PDF_ENABLE_XFA_TIFF
  uint32_t m_offSet = 0;
  int m_ScanlineSize = 0;
  WeightTable m_WeightHorz;
  VertTable m_WeightVert;
  HorzTable m_WeightHorzOO;
  int m_SrcWidth = 0;
  int m_SrcHeight = 0;
  int m_SrcComponents = 0;
  int m_SrcBPC = 0;
  FX_RECT m_clipBox;
  int m_startX = 0;
  int m_startY = 0;
  int m_sizeX = 0;
  int m_sizeY = 0;
  int m_TransMethod = -1;
  int m_SrcPaletteNumber = 0;
  int m_SrcRow = 0;
  FXCodec_Format m_SrcFormat = FXCodec_Invalid;
  int m_SrcPassNumber = 0;
  size_t m_FrameNumber = 0;
  size_t m_FrameCur = 0;
#ifdef PDF_ENABLE_XFA_GIF
  int m_GifBgIndex = 0;
  CFX_GifPalette* m_pGifPalette = nullptr;
  int32_t m_GifPltNumber = 0;
  int m_GifTransIndex = -1;
  FX_RECT m_GifFrameRect;
#endif  // PDF_ENABLE_XFA_GIF
#ifdef PDF_ENABLE_XFA_BMP
  bool m_BmpIsTopBottom = false;
#endif  // PDF_ENABLE_XFA_BMP
};

}  // namespace fxcodec

using ProgressiveDecoder = fxcodec::ProgressiveDecoder;

#endif  // CORE_FXCODEC_PROGRESSIVE_DECODER_H_
