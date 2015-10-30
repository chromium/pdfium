// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_CODEC_PROGRESS_H_
#define _FX_CODEC_PROGRESS_H_
#define FXCODEC_BLOCK_SIZE 4096
#define FXCODEC_PNG_GAMMA 2.2
#if _FX_OS_ == _FX_MACOSX_ || _FX_OS_ == _FX_IOS_
#undef FXCODEC_PNG_GAMMA
#define FXCODEC_PNG_GAMMA 1.7
#endif
struct PixelWeight {
  int m_SrcStart;
  int m_SrcEnd;
  int m_Weights[1];
};
class CFXCODEC_WeightTable {
 public:
  CFXCODEC_WeightTable() { m_pWeightTables = NULL; }
  ~CFXCODEC_WeightTable() {
    if (m_pWeightTables != NULL) {
      FX_Free(m_pWeightTables);
    }
  }

  void Calc(int dest_len,
            int dest_min,
            int dest_max,
            int src_len,
            int src_min,
            int src_max,
            FX_BOOL bInterpol);
  PixelWeight* GetPixelWeight(int pixel) {
    return (PixelWeight*)(m_pWeightTables + (pixel - m_DestMin) * m_ItemSize);
  }

  int m_DestMin, m_ItemSize;
  uint8_t* m_pWeightTables;
};
class CFXCODEC_HorzTable {
 public:
  CFXCODEC_HorzTable() { m_pWeightTables = NULL; }
  ~CFXCODEC_HorzTable() {
    if (m_pWeightTables != NULL) {
      FX_Free(m_pWeightTables);
    }
  }

  void Calc(int dest_len, int src_len, FX_BOOL bInterpol);
  PixelWeight* GetPixelWeight(int pixel) {
    return (PixelWeight*)(m_pWeightTables + pixel * m_ItemSize);
  }

  int m_ItemSize;
  uint8_t* m_pWeightTables;
};
class CFXCODEC_VertTable {
 public:
  CFXCODEC_VertTable() { m_pWeightTables = NULL; }
  ~CFXCODEC_VertTable() {
    if (m_pWeightTables != NULL) {
      FX_Free(m_pWeightTables);
    }
  }
  void Calc(int dest_len, int src_len);
  PixelWeight* GetPixelWeight(int pixel) {
    return (PixelWeight*)(m_pWeightTables + pixel * m_ItemSize);
  }
  int m_ItemSize;
  uint8_t* m_pWeightTables;
};
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
class CCodec_ProgressiveDecoder : public ICodec_ProgressiveDecoder {
 public:
  CCodec_ProgressiveDecoder(CCodec_ModuleMgr* pCodecMgr);
  ~CCodec_ProgressiveDecoder() override;

  FXCODEC_STATUS LoadImageInfo(IFX_FileRead* pFile,
                               FXCODEC_IMAGE_TYPE imageType,
                               CFX_DIBAttribute* pAttribute) override;

  FXCODEC_IMAGE_TYPE GetType() const override { return m_imagType; }
  int32_t GetWidth() const override { return m_SrcWidth; }
  int32_t GetHeight() const override { return m_SrcHeight; }
  int32_t GetNumComponents() const override { return m_SrcComponents; }
  int32_t GetBPC() const override { return m_SrcBPC; }
  void SetClipBox(FX_RECT* clip) override;

  FXCODEC_STATUS GetFrames(int32_t& frames, IFX_Pause* pPause) override;
  FXCODEC_STATUS StartDecode(CFX_DIBitmap* pDIBitmap,
                             int start_x,
                             int start_y,
                             int size_x,
                             int size_y,
                             int32_t frames,
                             FX_BOOL bInterpol) override;

  FXCODEC_STATUS ContinueDecode(IFX_Pause* pPause) override;

 protected:
  static FX_BOOL PngReadHeaderFunc(void* pModule,
                                   int width,
                                   int height,
                                   int bpc,
                                   int pass,
                                   int* color_type,
                                   double* gamma);
  static FX_BOOL PngAskScanlineBufFunc(void* pModule,
                                       int line,
                                       uint8_t*& src_buf);
  static void PngFillScanlineBufCompletedFunc(void* pModule,
                                              int pass,
                                              int line);
  static void GifRecordCurrentPositionCallback(void* pModule,
                                               FX_DWORD& cur_pos);
  static uint8_t* GifAskLocalPaletteBufCallback(void* pModule,
                                                int32_t frame_num,
                                                int32_t pal_size);
  static FX_BOOL GifInputRecordPositionBufCallback(void* pModule,
                                                   FX_DWORD rcd_pos,
                                                   const FX_RECT& img_rc,
                                                   int32_t pal_num,
                                                   void* pal_ptr,
                                                   int32_t delay_time,
                                                   FX_BOOL user_input,
                                                   int32_t trans_index,
                                                   int32_t disposal_method,
                                                   FX_BOOL interlace);
  static void GifReadScanlineCallback(void* pModule,
                                      int32_t row_num,
                                      uint8_t* row_buf);
  static FX_BOOL BmpInputImagePositionBufCallback(void* pModule,
                                                  FX_DWORD rcd_pos);
  static void BmpReadScanlineCallback(void* pModule,
                                      int32_t row_num,
                                      uint8_t* row_buf);

  FX_BOOL DetectImageType(FXCODEC_IMAGE_TYPE imageType,
                          CFX_DIBAttribute* pAttribute);
  void GetDownScale(int& down_scale);
  void GetTransMethod(FXDIB_Format des_format, FXCodec_Format src_format);
  void ReSampleScanline(CFX_DIBitmap* pDeviceBitmap,
                        int32_t des_line,
                        uint8_t* src_scan,
                        FXCodec_Format src_format);
  void Resample(CFX_DIBitmap* pDeviceBitmap,
                int32_t src_line,
                uint8_t* src_scan,
                FXCodec_Format src_format);
  void ResampleVert(CFX_DIBitmap* pDeviceBitmap, double scale_y, int des_row);
  FX_BOOL JpegReadMoreData(ICodec_JpegModule* pJpegModule,
                           FXCODEC_STATUS& err_status);
  void PngOneOneMapResampleHorz(CFX_DIBitmap* pDeviceBitmap,
                                int32_t des_line,
                                uint8_t* src_scan,
                                FXCodec_Format src_format);
  FX_BOOL GifReadMoreData(ICodec_GifModule* pGifModule,
                          FXCODEC_STATUS& err_status);
  void GifDoubleLineResampleVert(CFX_DIBitmap* pDeviceBitmap,
                                 double scale_y,
                                 int des_row);
  FX_BOOL BmpReadMoreData(ICodec_BmpModule* pBmpModule,
                          FXCODEC_STATUS& err_status);
  void ResampleVertBT(CFX_DIBitmap* pDeviceBitmap, double scale_y, int des_row);

 public:
  IFX_FileRead* m_pFile;
  CCodec_ModuleMgr* m_pCodecMgr;
  void* m_pJpegContext;
  void* m_pPngContext;
  void* m_pGifContext;
  void* m_pBmpContext;
  void* m_pTiffContext;
  FXCODEC_IMAGE_TYPE m_imagType;
  FX_DWORD m_offSet;
  uint8_t* m_pSrcBuf;
  FX_DWORD m_SrcSize;
  uint8_t* m_pDecodeBuf;
  int m_ScanlineSize;
  CFX_DIBitmap* m_pDeviceBitmap;
  FX_BOOL m_bInterpol;
  CFXCODEC_WeightTable m_WeightHorz;
  CFXCODEC_VertTable m_WeightVert;
  CFXCODEC_HorzTable m_WeightHorzOO;
  int m_SrcWidth;
  int m_SrcHeight;
  int m_SrcComponents;
  int m_SrcBPC;
  FX_RECT m_clipBox;
  int m_startX;
  int m_startY;
  int m_sizeX;
  int m_sizeY;
  int m_TransMethod;
  FX_ARGB* m_pSrcPalette;
  int m_SrcPaletteNumber;
  int m_SrcRow;
  FXCodec_Format m_SrcFormat;
  int m_SrcPassNumber;
  int m_FrameNumber;
  int m_FrameCur;
  int m_GifBgIndex;
  uint8_t* m_pGifPalette;
  int32_t m_GifPltNumber;
  int m_GifTransIndex;
  FX_RECT m_GifFrameRect;
  FX_BOOL m_BmpIsTopBottom;
  FXCODEC_STATUS m_status;
};
#endif
