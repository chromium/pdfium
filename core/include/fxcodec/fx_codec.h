// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCODEC_FX_CODEC_H_
#define CORE_INCLUDE_FXCODEC_FX_CODEC_H_

#include "../fxcrt/fx_ext.h"
#include "fx_codec_def.h"
#include "fx_codec_provider.h"

class CFX_DIBSource;
class ICodec_ScanlineDecoder;
class ICodec_BasicModule;
class ICodec_FaxModule;
class ICodec_JpegModule;
class ICodec_JpxModule;
class ICodec_Jbig2Module;
class ICodec_IccModule;
class ICodec_FlateModule;
class ICodec_Jbig2Encoder;
class ICodec_ScanlineDecoder;
class CCodec_ModuleMgr 
{
public:

    static CCodec_ModuleMgr*	Create();

    void				Destroy();

    void				InitJbig2Decoder();

    void				InitJpxDecoder();


    void				InitIccDecoder();

    ICodec_Jbig2Encoder*		CreateJbig2Encoder();
protected:
    CCodec_ModuleMgr();
    ~CCodec_ModuleMgr();
public:
    ICodec_BasicModule*	GetBasicModule()
    {
        return m_pBasicModule;
    }
    ICodec_FaxModule*	GetFaxModule()
    {
        return m_pFaxModule;
    }
    ICodec_JpegModule*	GetJpegModule()
    {
        return m_pJpegModule;
    }
    ICodec_JpxModule*	GetJpxModule()
    {
        return m_pJpxModule;
    }
    ICodec_Jbig2Module*	GetJbig2Module()
    {
        return m_pJbig2Module;
    }
    ICodec_IccModule*	GetIccModule()
    {
        return m_pIccModule;
    }
    ICodec_FlateModule*	GetFlateModule()
    {
        return m_pFlateModule;
    }
protected:
    ICodec_BasicModule*	m_pBasicModule;
    ICodec_FaxModule*	m_pFaxModule;
    ICodec_JpegModule*	m_pJpegModule;
    ICodec_JpxModule*	m_pJpxModule;
    ICodec_Jbig2Module*	m_pJbig2Module;
    ICodec_IccModule*	m_pIccModule;
    ICodec_FlateModule*	m_pFlateModule;

};
class ICodec_BasicModule 
{
public:

    virtual ~ICodec_BasicModule() {}
    virtual FX_BOOL	RunLengthEncode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf,
                                    FX_DWORD& dest_size) = 0;
    virtual FX_BOOL	A85Encode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf,
                              FX_DWORD& dest_size) = 0;
    virtual ICodec_ScanlineDecoder*	CreateRunLengthDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
            int nComps, int bpc) = 0;
};
class ICodec_ScanlineDecoder 
{
public:

    virtual ~ICodec_ScanlineDecoder() {}

    virtual FX_DWORD	GetSrcOffset() = 0;

    virtual void		DownScale(int dest_width, int dest_height) = 0;

    virtual FX_LPBYTE	GetScanline(int line) = 0;

    virtual FX_BOOL		SkipToScanline(int line, IFX_Pause* pPause) = 0;

    virtual int			GetWidth() = 0;

    virtual int			GetHeight() = 0;

    virtual int			CountComps() = 0;

    virtual int			GetBPC() = 0;

    virtual FX_BOOL		IsColorTransformed() = 0;

    virtual void		ClearImageData() = 0;
};
class ICodec_FlateModule 
{
public:

    virtual ~ICodec_FlateModule() {}
    virtual ICodec_ScanlineDecoder*	CreateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
            int nComps, int bpc, int predictor, int Colors, int BitsPerComponent, int Columns) = 0;
    virtual FX_DWORD	FlateOrLZWDecode(FX_BOOL bLZW, const FX_BYTE* src_buf, FX_DWORD src_size, FX_BOOL bEarlyChange,
                                         int predictor, int Colors, int BitsPerComponent, int Columns,
                                         FX_DWORD estimated_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size) = 0;
    virtual FX_BOOL		Encode(const FX_BYTE* src_buf, FX_DWORD src_size,
                               int predictor, int Colors, int BitsPerComponent, int Columns,
                               FX_LPBYTE& dest_buf, FX_DWORD& dest_size) = 0;
    virtual FX_BOOL		Encode(FX_LPCBYTE src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size) = 0;
};
class ICodec_FaxModule 
{
public:

    virtual ~ICodec_FaxModule() {}

    virtual ICodec_ScanlineDecoder*	CreateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
            int K, FX_BOOL EndOfLine, FX_BOOL EncodedByteAlign, FX_BOOL BlackIs1, int Columns, int Rows) = 0;


    virtual FX_BOOL		Encode(FX_LPCBYTE src_buf, int width, int height, int pitch,
                               FX_LPBYTE& dest_buf, FX_DWORD& dest_size) = 0;
};
class ICodec_JpegModule 
{
public:

    virtual ~ICodec_JpegModule() {}

    virtual void		SetPovider(IFX_JpegProvider* pJP) = 0;

    virtual ICodec_ScanlineDecoder*	CreateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size,
            int width, int height, int nComps, FX_BOOL ColorTransform) = 0;

    virtual FX_BOOL		LoadInfo(FX_LPCBYTE src_buf, FX_DWORD src_size, int& width, int& height,
                                 int& num_components, int& bits_per_components, FX_BOOL& color_transform,
                                 FX_LPBYTE* icc_buf_ptr = NULL, FX_DWORD* icc_length = NULL) = 0;

    virtual FX_BOOL		Encode(const class CFX_DIBSource* pSource, FX_LPBYTE& dest_buf, FX_STRSIZE& dest_size, int quality = 75,
                               FX_LPCBYTE icc_buf = NULL, FX_DWORD icc_length = 0) = 0;

    virtual void*		Start() = 0;

    virtual void		Finish(void* pContext) = 0;

    virtual void		Input(void* pContext, FX_LPCBYTE src_buf, FX_DWORD src_size) = 0;

    virtual int			ReadHeader(void* pContext, int* width, int* height, int* nComps) = 0;


    virtual int			StartScanline(void* pContext, int down_scale) = 0;


    virtual FX_BOOL		ReadScanline(void* pContext, FX_LPBYTE dest_buf) = 0;


    virtual FX_DWORD	GetAvailInput(void* pContext, FX_LPBYTE* avail_buf_ptr = NULL) = 0;
};
class ICodec_JpxModule 
{
public:

    virtual ~ICodec_JpxModule() {}

    virtual FX_LPVOID 	CreateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, FX_BOOL useColorSpace = FALSE) = 0;

    virtual void		GetImageInfo(FX_LPVOID ctx, FX_DWORD& width, FX_DWORD& height,
                                     FX_DWORD& codestream_nComps, FX_DWORD& output_nComps) = 0;

    virtual FX_BOOL		Decode(FX_LPVOID ctx, FX_LPBYTE dest_data, int pitch,
                               FX_BOOL bTranslateColor, FX_LPBYTE offsets) = 0;

    virtual void		DestroyDecoder(FX_LPVOID ctx) = 0;
};
class ICodec_Jbig2Module 
{
public:

    virtual ~ICodec_Jbig2Module() {}

    virtual FX_BOOL		Decode(FX_DWORD width, FX_DWORD height, FX_LPCBYTE src_buf, FX_DWORD src_size,
                               FX_LPCBYTE global_data, FX_DWORD global_size, FX_LPBYTE dest_buf, FX_DWORD dest_pitch)  = 0;

    virtual FX_BOOL		Decode(IFX_FileRead* file_ptr, FX_DWORD& width, FX_DWORD& height,
                               FX_DWORD& pitch, FX_LPBYTE& dest_buf) = 0;
    virtual void*				CreateJbig2Context() = 0;

    virtual FXCODEC_STATUS		StartDecode(void* pJbig2Context, FX_DWORD width, FX_DWORD height, FX_LPCBYTE src_buf, FX_DWORD src_size,
                                            FX_LPCBYTE global_data, FX_DWORD global_size, FX_LPBYTE dest_buf, FX_DWORD dest_pitch, IFX_Pause* pPause) = 0;

    virtual FXCODEC_STATUS		StartDecode(void* pJbig2Context, IFX_FileRead* file_ptr,
                                            FX_DWORD& width, FX_DWORD& height, FX_DWORD& pitch, FX_LPBYTE& dest_buf, IFX_Pause* pPause) = 0;
    virtual FXCODEC_STATUS		ContinueDecode(void* pJbig2Content, IFX_Pause* pPause) = 0;
    virtual void				DestroyJbig2Context(void* pJbig2Content) = 0;
};
class ICodec_Jbig2Encoder 
{
public:

    virtual ~ICodec_Jbig2Encoder() {}
};
class ICodec_IccModule 
{
public:
    typedef enum {
        IccCS_Unknown = 0,
        IccCS_XYZ,
        IccCS_Lab,
        IccCS_Luv,
        IccCS_YCbCr,
        IccCS_Yxy,
        IccCS_Hsv,
        IccCS_Hls,
        IccCS_Gray,
        IccCS_Rgb,
        IccCS_Cmyk,
        IccCS_Cmy
    } IccCS;
    typedef struct _IccParam {
        FX_DWORD	Version;
        IccCS		ColorSpace;
        FX_DWORD	dwProfileType;
        FX_DWORD	dwFormat;
        FX_LPBYTE	pProfileData;
        FX_DWORD	dwProfileSize;
        double		Gamma;
    } IccParam;

    virtual ~ICodec_IccModule() {}

    virtual IccCS			GetProfileCS(FX_LPCBYTE pProfileData, unsigned int dwProfileSize) = 0;

    virtual IccCS			GetProfileCS(IFX_FileRead* pFile) = 0;

    virtual FX_LPVOID	CreateTransform(ICodec_IccModule::IccParam* pInputParam,
                                        ICodec_IccModule::IccParam* pOutputParam,
                                        ICodec_IccModule::IccParam* pProofParam = NULL,
                                        FX_DWORD dwIntent = Icc_INTENT_PERCEPTUAL,
                                        FX_DWORD dwFlag = Icc_FLAGS_DEFAULT,
                                        FX_DWORD dwPrfIntent = Icc_INTENT_ABSOLUTE_COLORIMETRIC,
                                        FX_DWORD dwPrfFlag = Icc_FLAGS_SOFTPROOFING
                                     ) = 0;


    virtual FX_LPVOID	CreateTransform_sRGB(FX_LPCBYTE pProfileData, FX_DWORD dwProfileSize, FX_INT32& nComponents, FX_INT32 intent = 0,
            FX_DWORD dwSrcFormat = Icc_FORMAT_DEFAULT) = 0;

    virtual FX_LPVOID	CreateTransform_CMYK(FX_LPCBYTE pSrcProfileData, FX_DWORD dwSrcProfileSize, FX_INT32& nSrcComponents,
            FX_LPCBYTE pDstProfileData, FX_DWORD dwDstProfileSize, FX_INT32 intent = 0,
            FX_DWORD dwSrcFormat = Icc_FORMAT_DEFAULT,
            FX_DWORD dwDstFormat = Icc_FORMAT_DEFAULT
                                          ) = 0;

    virtual void			DestroyTransform(FX_LPVOID pTransform) = 0;

    virtual void			Translate(FX_LPVOID pTransform, FX_FLOAT* pSrcValues, FX_FLOAT* pDestValues) = 0;

    virtual void			TranslateScanline(FX_LPVOID pTransform, FX_LPBYTE pDest, FX_LPCBYTE pSrc, int pixels) = 0;
    virtual void                        SetComponents(FX_DWORD nComponents) = 0;
};
void AdobeCMYK_to_sRGB(FX_FLOAT c, FX_FLOAT m, FX_FLOAT y, FX_FLOAT k, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B);
void AdobeCMYK_to_sRGB1(FX_BYTE c, FX_BYTE m, FX_BYTE y, FX_BYTE k, FX_BYTE& R, FX_BYTE& G, FX_BYTE& B);
FX_BOOL MD5ComputeID(FX_LPCVOID buf, FX_DWORD dwSize, FX_BYTE ID[16]);

#endif  // CORE_INCLUDE_FXCODEC_FX_CODEC_H_
