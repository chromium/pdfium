// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXCODEC_CODEC_CODEC_INT_H_
#define CORE_SRC_FXCODEC_CODEC_CODEC_INT_H_

#include <limits.h>

#include <list>
#include <map>
#include <memory>

#include "core/include/fxcodec/fx_codec.h"
#include "core/src/fxcodec/jbig2/JBig2_Context.h"
#include "third_party/libopenjpeg20/openjpeg.h"  // For OPJ_SIZE_T.

class CFX_IccProfileCache;
class CFX_IccTransformCache;
class CPDF_ColorSpace;

class CCodec_BasicModule : public ICodec_BasicModule {
 public:
  // ICodec_BasicModule:
  FX_BOOL RunLengthEncode(const uint8_t* src_buf,
                          FX_DWORD src_size,
                          uint8_t*& dest_buf,
                          FX_DWORD& dest_size) override;
  FX_BOOL A85Encode(const uint8_t* src_buf,
                    FX_DWORD src_size,
                    uint8_t*& dest_buf,
                    FX_DWORD& dest_size) override;
  ICodec_ScanlineDecoder* CreateRunLengthDecoder(const uint8_t* src_buf,
                                                 FX_DWORD src_size,
                                                 int width,
                                                 int height,
                                                 int nComps,
                                                 int bpc) override;
};

class CCodec_ScanlineDecoder : public ICodec_ScanlineDecoder {
 public:
  CCodec_ScanlineDecoder();
  ~CCodec_ScanlineDecoder() override;

  // ICodec_ScanlineDecoder
  FX_DWORD GetSrcOffset() override { return -1; }
  void DownScale(int dest_width, int dest_height) override;
  const uint8_t* GetScanline(int line) override;
  FX_BOOL SkipToScanline(int line, IFX_Pause* pPause) override;
  int GetWidth() override { return m_OutputWidth; }
  int GetHeight() override { return m_OutputHeight; }
  int CountComps() override { return m_nComps; }
  int GetBPC() override { return m_bpc; }
  FX_BOOL IsColorTransformed() override { return m_bColorTransformed; }
  void ClearImageData() override { m_pDataCache.reset(); }

 protected:
  class ImageDataCache {
   public:
    ImageDataCache(int width, int height, FX_DWORD pitch);
    ~ImageDataCache();

    bool AllocateCache();
    void AppendLine(const uint8_t* line);

    int NumLines() const { return m_nCachedLines; }
    const uint8_t* GetLine(int line) const;
    bool IsSameDimensions(int width, int height) const {
      return width == m_Width && height == m_Height;
    }

   private:
    bool IsValid() const { return m_Data.get() != nullptr; }

    const int m_Width;
    const int m_Height;
    const FX_DWORD m_Pitch;
    int m_nCachedLines;
    std::unique_ptr<uint8_t, FxFreeDeleter> m_Data;
  };

  virtual FX_BOOL v_Rewind() = 0;
  virtual uint8_t* v_GetNextLine() = 0;
  virtual void v_DownScale(int dest_width, int dest_height) = 0;

  uint8_t* ReadNextLine();

  int m_OrigWidth;
  int m_OrigHeight;
  int m_DownScale;
  int m_OutputWidth;
  int m_OutputHeight;
  int m_nComps;
  int m_bpc;
  FX_DWORD m_Pitch;
  FX_BOOL m_bColorTransformed;
  int m_NextLine;
  uint8_t* m_pLastScanline;
  std::unique_ptr<ImageDataCache> m_pDataCache;
};

class CCodec_FaxModule : public ICodec_FaxModule {
 public:
  // ICodec_FaxModule:
  ICodec_ScanlineDecoder* CreateDecoder(const uint8_t* src_buf,
                                        FX_DWORD src_size,
                                        int width,
                                        int height,
                                        int K,
                                        FX_BOOL EndOfLine,
                                        FX_BOOL EncodedByteAlign,
                                        FX_BOOL BlackIs1,
                                        int Columns,
                                        int Rows) override;
  FX_BOOL Encode(const uint8_t* src_buf,
                 int width,
                 int height,
                 int pitch,
                 uint8_t*& dest_buf,
                 FX_DWORD& dest_size) override;
};

class CCodec_FlateModule : public ICodec_FlateModule {
 public:
  virtual ICodec_ScanlineDecoder* CreateDecoder(const uint8_t* src_buf,
                                                FX_DWORD src_size,
                                                int width,
                                                int height,
                                                int nComps,
                                                int bpc,
                                                int predictor,
                                                int Colors,
                                                int BitsPerComponent,
                                                int Columns);
  virtual FX_DWORD FlateOrLZWDecode(FX_BOOL bLZW,
                                    const uint8_t* src_buf,
                                    FX_DWORD src_size,
                                    FX_BOOL bEarlyChange,
                                    int predictor,
                                    int Colors,
                                    int BitsPerComponent,
                                    int Columns,
                                    FX_DWORD estimated_size,
                                    uint8_t*& dest_buf,
                                    FX_DWORD& dest_size);
  virtual FX_BOOL Encode(const uint8_t* src_buf,
                         FX_DWORD src_size,
                         int predictor,
                         int Colors,
                         int BitsPerComponent,
                         int Columns,
                         uint8_t*& dest_buf,
                         FX_DWORD& dest_size);
  virtual FX_BOOL Encode(const uint8_t* src_buf,
                         FX_DWORD src_size,
                         uint8_t*& dest_buf,
                         FX_DWORD& dest_size);
};

class CCodec_JpegModule : public ICodec_JpegModule {
 public:
  CCodec_JpegModule() {}
  ICodec_ScanlineDecoder* CreateDecoder(const uint8_t* src_buf,
                                        FX_DWORD src_size,
                                        int width,
                                        int height,
                                        int nComps,
                                        FX_BOOL ColorTransform) override;
  FX_BOOL LoadInfo(const uint8_t* src_buf,
                   FX_DWORD src_size,
                   int& width,
                   int& height,
                   int& num_components,
                   int& bits_per_components,
                   FX_BOOL& color_transform,
                   uint8_t** icc_buf_ptr,
                   FX_DWORD* icc_length) override;
  FX_BOOL Encode(const CFX_DIBSource* pSource,
                 uint8_t*& dest_buf,
                 FX_STRSIZE& dest_size,
                 int quality,
                 const uint8_t* icc_buf,
                 FX_DWORD icc_length) override;
  void* Start() override;
  void Finish(void* pContext) override;
  void Input(void* pContext,
             const uint8_t* src_buf,
             FX_DWORD src_size) override;
#ifndef PDF_ENABLE_XFA
  int ReadHeader(void* pContext, int* width, int* height, int* nComps) override;
#else   // PDF_ENABLE_XFA
  int ReadHeader(void* pContext,
                 int* width,
                 int* height,
                 int* nComps,
                 CFX_DIBAttribute* pAttribute) override;
#endif  // PDF_ENABLE_XFA
  int StartScanline(void* pContext, int down_scale) override;
  FX_BOOL ReadScanline(void* pContext, uint8_t* dest_buf) override;
  FX_DWORD GetAvailInput(void* pContext, uint8_t** avail_buf_ptr) override;
};

#ifdef PDF_ENABLE_XFA
#define PNG_ERROR_SIZE 256
class CCodec_PngModule : public ICodec_PngModule {
 public:
  CCodec_PngModule() { FXSYS_memset(m_szLastError, '\0', PNG_ERROR_SIZE); }

  virtual void* Start(void* pModule);
  virtual void Finish(void* pContext);
  virtual FX_BOOL Input(void* pContext,
                        const uint8_t* src_buf,
                        FX_DWORD src_size,
                        CFX_DIBAttribute* pAttribute);

 protected:
  FX_CHAR m_szLastError[PNG_ERROR_SIZE];
};
class CCodec_GifModule : public ICodec_GifModule {
 public:
  CCodec_GifModule() { FXSYS_memset(m_szLastError, '\0', 256); }
  virtual void* Start(void* pModule);
  virtual void Finish(void* pContext);
  virtual FX_DWORD GetAvailInput(void* pContext, uint8_t** avail_buf_ptr);
  virtual void Input(void* pContext, const uint8_t* src_buf, FX_DWORD src_size);

  virtual int32_t ReadHeader(void* pContext,
                             int* width,
                             int* height,
                             int* pal_num,
                             void** pal_pp,
                             int* bg_index,
                             CFX_DIBAttribute* pAttribute);

  virtual int32_t LoadFrameInfo(void* pContext, int* frame_num);

  virtual int32_t LoadFrame(void* pContext,
                            int frame_num,
                            CFX_DIBAttribute* pAttribute);

 protected:
  FX_CHAR m_szLastError[256];
};
class CCodec_BmpModule : public ICodec_BmpModule {
 public:
  CCodec_BmpModule() { FXSYS_memset(m_szLastError, 0, sizeof(m_szLastError)); }
  void* Start(void* pModule) override;
  void Finish(void* pContext) override;
  FX_DWORD GetAvailInput(void* pContext, uint8_t** avail_buf_ptr) override;
  void Input(void* pContext,
             const uint8_t* src_buf,
             FX_DWORD src_size) override;
  int32_t ReadHeader(void* pContext,
                     int32_t* width,
                     int32_t* height,
                     FX_BOOL* tb_flag,
                     int32_t* components,
                     int32_t* pal_num,
                     FX_DWORD** pal_pp,
                     CFX_DIBAttribute* pAttribute) override;
  int32_t LoadImage(void* pContext) override;

 protected:
  FX_CHAR m_szLastError[256];
};
#endif  // PDF_ENABLE_XFA

class CCodec_IccModule : public ICodec_IccModule {
 public:
  ~CCodec_IccModule() override;

  // ICodec_IccModule:
  IccCS GetProfileCS(const uint8_t* pProfileData,
                     unsigned int dwProfileSize) override;
  IccCS GetProfileCS(IFX_FileRead* pFile) override;
  void* CreateTransform(ICodec_IccModule::IccParam* pInputParam,
                        ICodec_IccModule::IccParam* pOutputParam,
                        ICodec_IccModule::IccParam* pProofParam = NULL,
                        FX_DWORD dwIntent = Icc_INTENT_PERCEPTUAL,
                        FX_DWORD dwFlag = Icc_FLAGS_DEFAULT,
                        FX_DWORD dwPrfIntent = Icc_INTENT_ABSOLUTE_COLORIMETRIC,
                        FX_DWORD dwPrfFlag = Icc_FLAGS_SOFTPROOFING) override;
  void* CreateTransform_sRGB(
      const uint8_t* pProfileData,
      FX_DWORD dwProfileSize,
      int32_t& nComponents,
      int32_t intent = 0,
      FX_DWORD dwSrcFormat = Icc_FORMAT_DEFAULT) override;
  void* CreateTransform_CMYK(
      const uint8_t* pSrcProfileData,
      FX_DWORD dwSrcProfileSize,
      int32_t& nSrcComponents,
      const uint8_t* pDstProfileData,
      FX_DWORD dwDstProfileSize,
      int32_t intent = 0,
      FX_DWORD dwSrcFormat = Icc_FORMAT_DEFAULT,
      FX_DWORD dwDstFormat = Icc_FORMAT_DEFAULT) override;
  void DestroyTransform(void* pTransform) override;
  void Translate(void* pTransform,
                 FX_FLOAT* pSrcValues,
                 FX_FLOAT* pDestValues) override;
  void TranslateScanline(void* pTransform,
                         uint8_t* pDest,
                         const uint8_t* pSrc,
                         int pixels) override;
  void SetComponents(FX_DWORD nComponents) override {
    m_nComponents = nComponents;
  }

 protected:
  enum Icc_CLASS {
    Icc_CLASS_INPUT = 0,
    Icc_CLASS_OUTPUT,
    Icc_CLASS_PROOF,
    Icc_CLASS_MAX
  };
  void* CreateProfile(ICodec_IccModule::IccParam* pIccParam,
                      Icc_CLASS ic,
                      CFX_BinaryBuf* pTransformKey);

  FX_DWORD m_nComponents;
  std::map<CFX_ByteString, CFX_IccTransformCache*> m_MapTranform;
  std::map<CFX_ByteString, CFX_IccProfileCache*> m_MapProfile;
};

class CCodec_JpxModule : public ICodec_JpxModule {
 public:
  CCodec_JpxModule();
  ~CCodec_JpxModule() override;

  // ICodec_JpxModule:
  CJPX_Decoder* CreateDecoder(const uint8_t* src_buf,
                              FX_DWORD src_size,
                              CPDF_ColorSpace* cs) override;
  void GetImageInfo(CJPX_Decoder* pDecoder,
                    FX_DWORD* width,
                    FX_DWORD* height,
                    FX_DWORD* components) override;
  bool Decode(CJPX_Decoder* pDecoder,
              uint8_t* dest_data,
              int pitch,
              const std::vector<uint8_t>& offsets) override;
  void DestroyDecoder(CJPX_Decoder* pDecoder) override;
};

#ifdef PDF_ENABLE_XFA
class CCodec_TiffModule : public ICodec_TiffModule {
 public:
  // ICodec_TiffModule
  void* CreateDecoder(IFX_FileRead* file_ptr) override;
  void GetFrames(void* ctx, int32_t& frames) override;
  FX_BOOL LoadFrameInfo(void* ctx,
                        int32_t frame,
                        FX_DWORD& width,
                        FX_DWORD& height,
                        FX_DWORD& comps,
                        FX_DWORD& bpc,
                        CFX_DIBAttribute* pAttribute) override;
  FX_BOOL Decode(void* ctx, class CFX_DIBitmap* pDIBitmap) override;
  void DestroyDecoder(void* ctx) override;

 protected:
  ~CCodec_TiffModule() override {}
};
#endif  // PDF_ENABLE_XFA

class CCodec_Jbig2Context {
 public:
  CCodec_Jbig2Context();
  ~CCodec_Jbig2Context() {}

  FX_DWORD m_width;
  FX_DWORD m_height;
  CPDF_StreamAcc* m_pGlobalStream;
  CPDF_StreamAcc* m_pSrcStream;
  uint8_t* m_dest_buf;
  FX_DWORD m_dest_pitch;
  IFX_Pause* m_pPause;
  CJBig2_Context* m_pContext;
  CJBig2_Image* m_dest_image;
};
class CCodec_Jbig2Module : public ICodec_Jbig2Module {
 public:
  CCodec_Jbig2Module() {}
  ~CCodec_Jbig2Module() override;

  // ICodec_Jbig2Module
  void* CreateJbig2Context() override;
  FXCODEC_STATUS StartDecode(void* pJbig2Context,
                             CFX_PrivateData* pPrivateData,
                             FX_DWORD width,
                             FX_DWORD height,
                             CPDF_StreamAcc* src_stream,
                             CPDF_StreamAcc* global_stream,
                             uint8_t* dest_buf,
                             FX_DWORD dest_pitch,
                             IFX_Pause* pPause) override;
  FXCODEC_STATUS ContinueDecode(void* pJbig2Context,
                                IFX_Pause* pPause) override;
  void DestroyJbig2Context(void* pJbig2Context) override;
};

struct DecodeData {
 public:
  DecodeData(unsigned char* src_data, OPJ_SIZE_T src_size)
      : src_data(src_data), src_size(src_size), offset(0) {}
  unsigned char* src_data;
  OPJ_SIZE_T src_size;
  OPJ_SIZE_T offset;
};

void sycc420_to_rgb(opj_image_t* img);

/* Wrappers for C-style callbacks. */
OPJ_SIZE_T opj_read_from_memory(void* p_buffer,
                                OPJ_SIZE_T nb_bytes,
                                void* p_user_data);
OPJ_SIZE_T opj_write_from_memory(void* p_buffer,
                                 OPJ_SIZE_T nb_bytes,
                                 void* p_user_data);
OPJ_OFF_T opj_skip_from_memory(OPJ_OFF_T nb_bytes, void* p_user_data);
OPJ_BOOL opj_seek_from_memory(OPJ_OFF_T nb_bytes, void* p_user_data);

#endif  // CORE_SRC_FXCODEC_CODEC_CODEC_INT_H_
