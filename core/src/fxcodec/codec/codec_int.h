// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXCODEC_CODEC_CODEC_INT_H_
#define CORE_SRC_FXCODEC_CODEC_CODEC_INT_H_

#include <limits.h>
#include <list>
#include <map>

#include "../../../../third_party/libopenjpeg20/openjpeg.h"  // For OPJ_SIZE_T.
#include "../../../include/fxcodec/fx_codec.h"
#include "../jbig2/JBig2_Context.h"

class CFX_IccProfileCache;
class CFX_IccTransformCache;

class CCodec_BasicModule : public ICodec_BasicModule {
 public:
  virtual FX_BOOL RunLengthEncode(const uint8_t* src_buf,
                                  FX_DWORD src_size,
                                  uint8_t*& dest_buf,
                                  FX_DWORD& dest_size);
  virtual FX_BOOL A85Encode(const uint8_t* src_buf,
                            FX_DWORD src_size,
                            uint8_t*& dest_buf,
                            FX_DWORD& dest_size);
  virtual ICodec_ScanlineDecoder* CreateRunLengthDecoder(const uint8_t* src_buf,
                                                         FX_DWORD src_size,
                                                         int width,
                                                         int height,
                                                         int nComps,
                                                         int bpc);
};

struct CCodec_ImageDataCache {
  int m_Width, m_Height;
  int m_nCachedLines;
  uint8_t m_Data;
};

class CCodec_ScanlineDecoder : public ICodec_ScanlineDecoder {
 public:
  CCodec_ScanlineDecoder();
  ~CCodec_ScanlineDecoder() override;

  // ICodec_ScanlineDecoder
  FX_DWORD GetSrcOffset() override { return -1; }
  void DownScale(int dest_width, int dest_height) override;
  uint8_t* GetScanline(int line) override;
  FX_BOOL SkipToScanline(int line, IFX_Pause* pPause) override;
  int GetWidth() override { return m_OutputWidth; }
  int GetHeight() override { return m_OutputHeight; }
  int CountComps() override { return m_nComps; }
  int GetBPC() override { return m_bpc; }
  FX_BOOL IsColorTransformed() override { return m_bColorTransformed; }
  void ClearImageData() override {
    FX_Free(m_pDataCache);
    m_pDataCache = NULL;
  }

 protected:
  int m_OrigWidth;

  int m_OrigHeight;

  int m_DownScale;

  int m_OutputWidth;

  int m_OutputHeight;

  int m_nComps;

  int m_bpc;

  int m_Pitch;

  FX_BOOL m_bColorTransformed;

  uint8_t* ReadNextLine();

  virtual FX_BOOL v_Rewind() = 0;

  virtual uint8_t* v_GetNextLine() = 0;

  virtual void v_DownScale(int dest_width, int dest_height) = 0;

  int m_NextLine;

  uint8_t* m_pLastScanline;

  CCodec_ImageDataCache* m_pDataCache;
};

class CCodec_FaxModule : public ICodec_FaxModule {
 public:
  virtual ICodec_ScanlineDecoder* CreateDecoder(const uint8_t* src_buf,
                                                FX_DWORD src_size,
                                                int width,
                                                int height,
                                                int K,
                                                FX_BOOL EndOfLine,
                                                FX_BOOL EncodedByteAlign,
                                                FX_BOOL BlackIs1,
                                                int Columns,
                                                int Rows);
  FX_BOOL Encode(const uint8_t* src_buf,
                 int width,
                 int height,
                 int pitch,
                 uint8_t*& dest_buf,
                 FX_DWORD& dest_size);
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
  CCodec_JpegModule() : m_pExtProvider(NULL) {}
  void SetPovider(IFX_JpegProvider* pJP) { m_pExtProvider = pJP; }
  ICodec_ScanlineDecoder* CreateDecoder(const uint8_t* src_buf,
                                        FX_DWORD src_size,
                                        int width,
                                        int height,
                                        int nComps,
                                        FX_BOOL ColorTransform);
  FX_BOOL LoadInfo(const uint8_t* src_buf,
                   FX_DWORD src_size,
                   int& width,
                   int& height,
                   int& num_components,
                   int& bits_per_components,
                   FX_BOOL& color_transform,
                   uint8_t** icc_buf_ptr,
                   FX_DWORD* icc_length);
  FX_BOOL Encode(const CFX_DIBSource* pSource,
                 uint8_t*& dest_buf,
                 FX_STRSIZE& dest_size,
                 int quality,
                 const uint8_t* icc_buf,
                 FX_DWORD icc_length);
  virtual void* Start();
  virtual void Finish(void* pContext);
  virtual void Input(void* pContext, const uint8_t* src_buf, FX_DWORD src_size);
  virtual int ReadHeader(void* pContext, int* width, int* height, int* nComps);
  virtual int StartScanline(void* pContext, int down_scale);
  virtual FX_BOOL ReadScanline(void* pContext, uint8_t* dest_buf);
  virtual FX_DWORD GetAvailInput(void* pContext, uint8_t** avail_buf_ptr);

 protected:
  IFX_JpegProvider* m_pExtProvider;
};
class CCodec_IccModule : public ICodec_IccModule {
 public:
  virtual IccCS GetProfileCS(const uint8_t* pProfileData,
                             unsigned int dwProfileSize);
  virtual IccCS GetProfileCS(IFX_FileRead* pFile);
  virtual void* CreateTransform(
      ICodec_IccModule::IccParam* pInputParam,
      ICodec_IccModule::IccParam* pOutputParam,
      ICodec_IccModule::IccParam* pProofParam = NULL,
      FX_DWORD dwIntent = Icc_INTENT_PERCEPTUAL,
      FX_DWORD dwFlag = Icc_FLAGS_DEFAULT,
      FX_DWORD dwPrfIntent = Icc_INTENT_ABSOLUTE_COLORIMETRIC,
      FX_DWORD dwPrfFlag = Icc_FLAGS_SOFTPROOFING);
  virtual void* CreateTransform_sRGB(const uint8_t* pProfileData,
                                     FX_DWORD dwProfileSize,
                                     int32_t& nComponents,
                                     int32_t intent = 0,
                                     FX_DWORD dwSrcFormat = Icc_FORMAT_DEFAULT);
  virtual void* CreateTransform_CMYK(const uint8_t* pSrcProfileData,
                                     FX_DWORD dwSrcProfileSize,
                                     int32_t& nSrcComponents,
                                     const uint8_t* pDstProfileData,
                                     FX_DWORD dwDstProfileSize,
                                     int32_t intent = 0,
                                     FX_DWORD dwSrcFormat = Icc_FORMAT_DEFAULT,
                                     FX_DWORD dwDstFormat = Icc_FORMAT_DEFAULT);
  virtual void DestroyTransform(void* pTransform);
  virtual void Translate(void* pTransform,
                         FX_FLOAT* pSrcValues,
                         FX_FLOAT* pDestValues);
  virtual void TranslateScanline(void* pTransform,
                                 uint8_t* pDest,
                                 const uint8_t* pSrc,
                                 int pixels);
  virtual void SetComponents(FX_DWORD nComponents) {
    m_nComponents = nComponents;
  }
  virtual ~CCodec_IccModule();

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
                              bool use_colorspace) override;
  void GetImageInfo(CJPX_Decoder* pDecoder,
                    FX_DWORD* width,
                    FX_DWORD* height,
                    FX_DWORD* components) override;
  FX_BOOL Decode(CJPX_Decoder* pDecoder,
                 uint8_t* dest_data,
                 int pitch,
                 uint8_t* offsets) override;
  void DestroyDecoder(CJPX_Decoder* pDecoder) override;
};

class CCodec_Jbig2Context {
 public:
  CCodec_Jbig2Context();
  ~CCodec_Jbig2Context() {}

  FX_DWORD m_width;
  FX_DWORD m_height;
  uint8_t* m_src_buf;
  FX_DWORD m_src_size;
  const uint8_t* m_global_data;
  FX_DWORD m_global_size;
  uint8_t* m_dest_buf;
  FX_DWORD m_dest_pitch;
  FX_BOOL m_bFileReader;
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
                             FX_DWORD width,
                             FX_DWORD height,
                             const uint8_t* src_buf,
                             FX_DWORD src_size,
                             const uint8_t* global_data,
                             FX_DWORD global_size,
                             uint8_t* dest_buf,
                             FX_DWORD dest_pitch,
                             IFX_Pause* pPause) override;
  FXCODEC_STATUS ContinueDecode(void* pJbig2Context,
                                IFX_Pause* pPause) override;
  void DestroyJbig2Context(void* pJbig2Context) override;

 private:
  std::list<CJBig2_CachePair> m_SymbolDictCache;
};

struct DecodeData {
 public:
  DecodeData(unsigned char* src_data, OPJ_SIZE_T src_size)
      : src_data(src_data), src_size(src_size), offset(0) {}
  unsigned char* src_data;
  OPJ_SIZE_T src_size;
  OPJ_SIZE_T offset;
};

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
