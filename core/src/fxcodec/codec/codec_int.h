// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <limits.h>
class CCodec_BasicModule : public ICodec_BasicModule
{
public:
    virtual FX_BOOL	RunLengthEncode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf,
                                    FX_DWORD& dest_size);
    virtual FX_BOOL	A85Encode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf,
                              FX_DWORD& dest_size);
    virtual ICodec_ScanlineDecoder*	CreateRunLengthDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
            int nComps, int bpc);
};
struct CCodec_ImageDataCache {
    int			m_Width, m_Height;
    int			m_nCachedLines;
    FX_BYTE		m_Data;
};
class CCodec_ScanlineDecoder : public ICodec_ScanlineDecoder
{
public:

    CCodec_ScanlineDecoder();

    virtual ~CCodec_ScanlineDecoder();

    virtual FX_DWORD	GetSrcOffset()
    {
        return -1;
    }

    virtual void		DownScale(int dest_width, int dest_height);

    FX_LPBYTE			GetScanline(int line);

    FX_BOOL				SkipToScanline(int line, IFX_Pause* pPause);

    int					GetWidth()
    {
        return m_OutputWidth;
    }

    int					GetHeight()
    {
        return m_OutputHeight;
    }

    int					CountComps()
    {
        return m_nComps;
    }

    int					GetBPC()
    {
        return m_bpc;
    }

    FX_BOOL				IsColorTransformed()
    {
        return m_bColorTransformed;
    }

    void				ClearImageData()
    {
        if (m_pDataCache) {
            FX_Free(m_pDataCache);
        }
        m_pDataCache = NULL;
    }
protected:

    int					m_OrigWidth;

    int					m_OrigHeight;

    int					m_DownScale;

    int					m_OutputWidth;

    int					m_OutputHeight;

    int					m_nComps;

    int					m_bpc;

    int					m_Pitch;

    FX_BOOL				m_bColorTransformed;

    FX_LPBYTE			ReadNextLine();

    virtual FX_BOOL		v_Rewind() = 0;

    virtual FX_LPBYTE	v_GetNextLine() = 0;

    virtual void		v_DownScale(int dest_width, int dest_height) = 0;

    int					m_NextLine;

    FX_LPBYTE			m_pLastScanline;

    CCodec_ImageDataCache*	m_pDataCache;
};
class CCodec_FaxModule : public ICodec_FaxModule
{
public:
    virtual ICodec_ScanlineDecoder*	CreateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
            int K, FX_BOOL EndOfLine, FX_BOOL EncodedByteAlign, FX_BOOL BlackIs1, int Columns, int Rows);
    FX_BOOL		Encode(FX_LPCBYTE src_buf, int width, int height, int pitch, FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
};
class CCodec_FlateModule : public ICodec_FlateModule
{
public:
    virtual ICodec_ScanlineDecoder*	CreateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
            int nComps, int bpc, int predictor, int Colors, int BitsPerComponent, int Columns);
    virtual FX_DWORD FlateOrLZWDecode(FX_BOOL bLZW, const FX_BYTE* src_buf, FX_DWORD src_size, FX_BOOL bEarlyChange,
                                      int predictor, int Colors, int BitsPerComponent, int Columns,
                                      FX_DWORD estimated_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
    virtual FX_BOOL Encode(const FX_BYTE* src_buf, FX_DWORD src_size,
                           int predictor, int Colors, int BitsPerComponent, int Columns,
                           FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
    virtual FX_BOOL		Encode(FX_LPCBYTE src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
};
class CCodec_JpegModule : public ICodec_JpegModule
{
public:
    CCodec_JpegModule() : m_pExtProvider(NULL) {}
    void SetPovider(IFX_JpegProvider* pJP)
    {
        m_pExtProvider = pJP;
    }
    ICodec_ScanlineDecoder*	CreateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size,
                                          int width, int height, int nComps, FX_BOOL ColorTransform);
    FX_BOOL		LoadInfo(FX_LPCBYTE src_buf, FX_DWORD src_size, int& width, int& height,
                         int& num_components, int& bits_per_components, FX_BOOL& color_transform,
                         FX_LPBYTE* icc_buf_ptr, FX_DWORD* icc_length);
    FX_BOOL		Encode(const CFX_DIBSource* pSource, FX_LPBYTE& dest_buf, FX_STRSIZE& dest_size, int quality, FX_LPCBYTE icc_buf, FX_DWORD icc_length);
    virtual void*		Start();
    virtual void		Finish(void* pContext);
    virtual void		Input(void* pContext, FX_LPCBYTE src_buf, FX_DWORD src_size);
    virtual int			ReadHeader(void* pContext, int* width, int* height, int* nComps, CFX_DIBAttribute* pAttribute = NULL);
    virtual FX_BOOL		StartScanline(void* pContext, int down_scale);
    virtual FX_BOOL		ReadScanline(void* pContext, FX_LPBYTE dest_buf);
    virtual FX_DWORD	GetAvailInput(void* pContext, FX_LPBYTE* avail_buf_ptr);
protected:
    IFX_JpegProvider* m_pExtProvider;
};
#define PNG_ERROR_SIZE 256
class CCodec_PngModule : public ICodec_PngModule
{
public:
    CCodec_PngModule()
    {
        FXSYS_memset8(m_szLastError, '\0', PNG_ERROR_SIZE);
    }

    virtual void*		Start(void* pModule);
    virtual void		Finish(void* pContext);
    virtual FX_BOOL		Input(void* pContext, FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_DIBAttribute* pAttribute);
protected:
    FX_CHAR				m_szLastError[PNG_ERROR_SIZE];
};
class CCodec_GifModule : public ICodec_GifModule
{
public:
    CCodec_GifModule()
    {
        FXSYS_memset8(m_szLastError, '\0', 256);
    }
    virtual void*		Start(void* pModule);
    virtual void		Finish(void* pContext);
    virtual FX_DWORD	GetAvailInput(void* pContext, FX_LPBYTE* avail_buf_ptr);
    virtual void		Input(void* pContext, FX_LPCBYTE src_buf, FX_DWORD src_size);

    virtual FX_INT32	ReadHeader(void* pContext, int* width, int* height,
                                   int* pal_num, void** pal_pp, int* bg_index, CFX_DIBAttribute* pAttribute);

    virtual FX_INT32	LoadFrameInfo(void* pContext, int* frame_num);

    virtual FX_INT32	LoadFrame(void* pContext, int frame_num, CFX_DIBAttribute* pAttribute);

protected:
    FX_CHAR				m_szLastError[256];
};
class CCodec_BmpModule : public ICodec_BmpModule
{
public:
    CCodec_BmpModule()
    {
        FXSYS_memset8(m_szLastError, '\0', 256);
    }
    virtual void*		Start(void* pModule);
    virtual void		Finish(void* pContext);
    virtual FX_DWORD	GetAvailInput(void* pContext, FX_LPBYTE* avail_buf_ptr);
    virtual void		Input(void* pContext, FX_LPCBYTE src_buf, FX_DWORD src_size);
    virtual FX_INT32	ReadHeader(void* pContext, FX_INT32* width, FX_INT32* height, FX_BOOL* tb_flag, FX_INT32* components, FX_INT32* pal_num, FX_DWORD** pal_pp, CFX_DIBAttribute* pAttribute);
    virtual FX_INT32	LoadImage(void* pContext);

protected:
    FX_CHAR				m_szLastError[256];
};
class CCodec_IccModule : public ICodec_IccModule
{
public:
    virtual IccCS			GetProfileCS(FX_LPCBYTE pProfileData, unsigned int dwProfileSize);
    virtual IccCS			GetProfileCS(IFX_FileRead* pFile);
    virtual FX_LPVOID		CreateTransform(ICodec_IccModule::IccParam* pInputParam,
                                            ICodec_IccModule::IccParam* pOutputParam,
                                            ICodec_IccModule::IccParam* pProofParam = NULL,
                                            FX_DWORD dwIntent = Icc_INTENT_PERCEPTUAL,
                                            FX_DWORD dwFlag = Icc_FLAGS_DEFAULT,
                                            FX_DWORD dwPrfIntent = Icc_INTENT_ABSOLUTE_COLORIMETRIC,
                                            FX_DWORD dwPrfFlag = Icc_FLAGS_SOFTPROOFING
                                      );
    virtual FX_LPVOID		CreateTransform_sRGB(FX_LPCBYTE pProfileData, FX_DWORD dwProfileSize, FX_INT32& nComponents, FX_INT32 intent = 0,
            FX_DWORD dwSrcFormat = Icc_FORMAT_DEFAULT);
    virtual FX_LPVOID		CreateTransform_CMYK(FX_LPCBYTE pSrcProfileData, FX_DWORD dwSrcProfileSize, FX_INT32& nSrcComponents,
            FX_LPCBYTE pDstProfileData, FX_DWORD dwDstProfileSize, FX_INT32 intent = 0,
            FX_DWORD dwSrcFormat = Icc_FORMAT_DEFAULT,
            FX_DWORD dwDstFormat = Icc_FORMAT_DEFAULT
                                           );
    virtual void			DestroyTransform(FX_LPVOID pTransform);
    virtual void			Translate(FX_LPVOID pTransform, FX_FLOAT* pSrcValues, FX_FLOAT* pDestValues);
    virtual void			TranslateScanline(FX_LPVOID pTransform, FX_LPBYTE pDest, FX_LPCBYTE pSrc, int pixels);
    virtual void                        SetComponents(FX_DWORD nComponents) {m_nComponents = nComponents;}
    virtual ~CCodec_IccModule();
protected:
    CFX_MapByteStringToPtr		m_MapTranform;
    CFX_MapByteStringToPtr		m_MapProfile;
    FX_DWORD                            m_nComponents;
    typedef enum {
        Icc_CLASS_INPUT = 0,
        Icc_CLASS_OUTPUT,
        Icc_CLASS_PROOF,
        Icc_CLASS_MAX
    } Icc_CLASS;
    FX_LPVOID		CreateProfile(ICodec_IccModule::IccParam* pIccParam, Icc_CLASS ic, CFX_BinaryBuf* pTransformKey);
};
class CCodec_JpxModule : public ICodec_JpxModule
{
public:
    CCodec_JpxModule();
    void*		CreateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, FX_BOOL useColorSpace = FALSE);
    void		GetImageInfo(FX_LPVOID ctx, FX_DWORD& width, FX_DWORD& height,
                             FX_DWORD& codestream_nComps, FX_DWORD& output_nComps);
    FX_BOOL		Decode(void* ctx, FX_LPBYTE dest_data, int pitch, FX_BOOL bTranslateColor, FX_LPBYTE offsets);
    void		DestroyDecoder(void* ctx);
};
class CCodec_TiffModule : public ICodec_TiffModule
{
public:
    virtual FX_LPVOID 	CreateDecoder(IFX_FileRead* file_ptr);
    virtual void		GetFrames(FX_LPVOID ctx, FX_INT32& frames);
    virtual FX_BOOL		LoadFrameInfo(FX_LPVOID ctx, FX_INT32 frame, FX_DWORD& width, FX_DWORD& height, FX_DWORD& comps, FX_DWORD& bpc, CFX_DIBAttribute* pAttribute = NULL);
    virtual FX_BOOL		Decode(FX_LPVOID ctx, class CFX_DIBitmap* pDIBitmap);
    virtual void		DestroyDecoder(FX_LPVOID ctx);
};
#include "../jbig2/JBig2_Context.h"
class CPDF_Jbig2Interface : public CFX_Object, public CJBig2_Module
{
public:
    virtual void *JBig2_Malloc(FX_DWORD dwSize)
    {
        return FX_Alloc(FX_BYTE, dwSize);
    }
    virtual void *JBig2_Malloc2(FX_DWORD num, FX_DWORD dwSize)
    {
        if (dwSize && num >= UINT_MAX / dwSize) {
            return NULL;
        }
        return FX_Alloc(FX_BYTE, num * dwSize);
    }
    virtual void *JBig2_Malloc3(FX_DWORD num, FX_DWORD dwSize, FX_DWORD dwSize2)
    {
        if (dwSize2 && dwSize >= UINT_MAX / dwSize2) {
            return NULL;
        }
        FX_DWORD size = dwSize2 * dwSize;
        if (size && num >= UINT_MAX / size) {
            return NULL;
        }
        return FX_Alloc(FX_BYTE, num * size);
    }
    virtual void *JBig2_Realloc(FX_LPVOID pMem, FX_DWORD dwSize)
    {
        return FX_Realloc(FX_BYTE, pMem, dwSize);
    }
    virtual void JBig2_Free(FX_LPVOID pMem)
    {
        FX_Free(pMem);
    }
};
class CCodec_Jbig2Context : public CFX_Object
{
public:
    CCodec_Jbig2Context();
    ~CCodec_Jbig2Context() {};
    IFX_FileRead* m_file_ptr;
    FX_DWORD m_width;
    FX_DWORD m_height;
    FX_LPBYTE m_src_buf;
    FX_DWORD m_src_size;
    FX_LPCBYTE m_global_data;
    FX_DWORD m_global_size;
    FX_LPBYTE m_dest_buf;
    FX_DWORD m_dest_pitch;
    FX_BOOL	m_bFileReader;
    IFX_Pause* m_pPause;
    CJBig2_Context* m_pContext;
    CJBig2_Image* m_dest_image;
};
class CCodec_Jbig2Module : public ICodec_Jbig2Module
{
public:
    CCodec_Jbig2Module() {};
    ~CCodec_Jbig2Module();
    FX_BOOL		Decode(FX_DWORD width, FX_DWORD height, FX_LPCBYTE src_buf, FX_DWORD src_size,
                       FX_LPCBYTE global_data, FX_DWORD global_size, FX_LPBYTE dest_buf, FX_DWORD dest_pitch);
    FX_BOOL		Decode(IFX_FileRead* file_ptr,
                       FX_DWORD& width, FX_DWORD& height, FX_DWORD& pitch, FX_LPBYTE& dest_buf);
    void*				CreateJbig2Context();
    FXCODEC_STATUS		StartDecode(void* pJbig2Context, FX_DWORD width, FX_DWORD height, FX_LPCBYTE src_buf, FX_DWORD src_size,
                                    FX_LPCBYTE global_data, FX_DWORD global_size, FX_LPBYTE dest_buf, FX_DWORD dest_pitch, IFX_Pause* pPause);

    FXCODEC_STATUS		StartDecode(void* pJbig2Context, IFX_FileRead* file_ptr,
                                    FX_DWORD& width, FX_DWORD& height, FX_DWORD& pitch, FX_LPBYTE& dest_buf, IFX_Pause* pPause);
    FXCODEC_STATUS		ContinueDecode(void* pJbig2Context, IFX_Pause* pPause);
    void				DestroyJbig2Context(void* pJbig2Context);
    CPDF_Jbig2Interface	m_Module;
private:
};
class CFX_DIBAttributeExif : public IFX_DIBAttributeExif
{
public:
    CFX_DIBAttributeExif();
    ~CFX_DIBAttributeExif();
    virtual FX_BOOL		GetInfo(FX_WORD tag, FX_LPVOID val);

    FX_BOOL ParseExif(CFX_MapPtrTemplate<FX_DWORD, FX_LPBYTE>* pHead, FX_LPBYTE data, FX_DWORD len, CFX_MapPtrTemplate<FX_DWORD, FX_LPBYTE>* pVal);

    typedef FX_WORD (*_Read2Bytes)(FX_LPBYTE data);
    typedef FX_DWORD (*_Read4Bytes)(FX_LPBYTE data);
    FX_LPBYTE ParseExifIFH(FX_LPBYTE data, FX_DWORD len, _Read2Bytes* pReadWord, _Read4Bytes* pReadDword);
    FX_BOOL ParseExifIFD(CFX_MapPtrTemplate<FX_DWORD, FX_LPBYTE>* pMap, FX_LPBYTE data, FX_DWORD len);

    FX_LPBYTE			m_pExifData;

    FX_DWORD			m_dwExifDataLen;

    void				clear();
    _Read2Bytes m_readWord;
    _Read4Bytes m_readDword;
    CFX_MapPtrTemplate<FX_DWORD, FX_LPBYTE> m_TagHead;
    CFX_MapPtrTemplate<FX_DWORD, FX_LPBYTE> m_TagVal;
};
