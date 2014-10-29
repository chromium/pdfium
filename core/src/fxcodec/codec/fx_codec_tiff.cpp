// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxcodec/fx_codec.h"
#include "../../../include/fxge/fx_dib.h"
#include "codec_int.h"
extern "C" {
#include "../fx_tiff/include/fx_tiffiop.h"
}
#if !defined(_FPDFAPI_MINI_)
void* IccLib_CreateTransform_sRGB(const unsigned char* pProfileData, unsigned int dwProfileSize, int nComponents, int intent, FX_DWORD dwSrcFormat = Icc_FORMAT_DEFAULT);
void IccLib_TranslateImage(void* pTransform, unsigned char* pDest, const unsigned char* pSrc, int pixels);
void IccLib_DestroyTransform(void* pTransform);
#endif
class CCodec_TiffContext : public CFX_Object
{
public:
    CCodec_TiffContext();
    ~CCodec_TiffContext();

    FX_BOOL	InitDecoder(IFX_FileRead* file_ptr);
    void	GetFrames(FX_INT32& frames);
    FX_BOOL	LoadFrameInfo(FX_INT32 frame, FX_DWORD& width, FX_DWORD& height, FX_DWORD& comps, FX_DWORD& bpc, CFX_DIBAttribute* pAttribute);
    FX_BOOL	Decode(CFX_DIBitmap* pDIBitmap);

    union {
        IFX_FileRead*	in;
        IFX_FileStream* out;
    } io;

    FX_DWORD		offset;

    TIFF*			tif_ctx;
    void*			icc_ctx;
    FX_INT32		frame_num;
    FX_INT32		frame_cur;
    FX_BOOL			isDecoder;
private:
    FX_BOOL	isSupport(CFX_DIBitmap* pDIBitmap);
    void	SetPalette(CFX_DIBitmap* pDIBitmap, FX_UINT16 bps);
    FX_BOOL	Decode1bppRGB(CFX_DIBitmap* pDIBitmap, FX_INT32 height, FX_INT32 width, FX_UINT16 bps, FX_UINT16 spp);
    FX_BOOL	Decode8bppRGB(CFX_DIBitmap* pDIBitmap, FX_INT32 height, FX_INT32 width, FX_UINT16 bps, FX_UINT16 spp);
    FX_BOOL	Decode24bppRGB(CFX_DIBitmap* pDIBitmap, FX_INT32 height, FX_INT32 width, FX_UINT16 bps, FX_UINT16 spp);
};
CCodec_TiffContext::CCodec_TiffContext()
{
    offset = 0;
    frame_num = 0;
    frame_cur = 0;
    io.in = NULL;
    tif_ctx = NULL;
    icc_ctx = NULL;
    isDecoder = TRUE;
}
CCodec_TiffContext::~CCodec_TiffContext()
{
#if !defined(_FPDFAPI_MINI_)
    if(icc_ctx) {
        IccLib_DestroyTransform(icc_ctx);
        icc_ctx = NULL;
    }
#endif
    if(tif_ctx)	{
        TIFFClose(tif_ctx);
    }
}
static tsize_t _tiff_read(thandle_t context, tdata_t buf, tsize_t length)
{
    CCodec_TiffContext* pTiffContext = (CCodec_TiffContext*)context;
    FX_BOOL ret = FALSE;
    if(pTiffContext->isDecoder) {
        ret = pTiffContext->io.in->ReadBlock(buf, pTiffContext->offset, length);
    } else {
        ret = pTiffContext->io.out->ReadBlock(buf, pTiffContext->offset, length);
    }
    if(!ret) {
        return 0;
    }
    pTiffContext->offset += (FX_DWORD)length;
    return length;
}
static tsize_t _tiff_write(thandle_t context, tdata_t buf, tsize_t length)
{
    CCodec_TiffContext* pTiffContext = (CCodec_TiffContext*)context;
    ASSERT(!pTiffContext->isDecoder);
    if(!pTiffContext->io.out->WriteBlock(buf, pTiffContext->offset, length)) {
        return 0;
    }
    pTiffContext->offset += (FX_DWORD)length;
    return length;
}
static toff_t _tiff_seek(thandle_t context, toff_t offset, int whence)
{
    CCodec_TiffContext* pTiffContext = (CCodec_TiffContext*)context;
    switch(whence) {
        case 0:
            pTiffContext->offset = (FX_DWORD)offset;
            break;
        case 1:
            pTiffContext->offset += (FX_DWORD)offset;
            break;
        case 2:
            if(pTiffContext->isDecoder) {
                if(pTiffContext->io.in->GetSize() < (FX_FILESIZE)offset) {
                    return -1;
                }
                pTiffContext->offset = (FX_DWORD)(pTiffContext->io.in->GetSize() - offset);
            } else {
                if(pTiffContext->io.out->GetSize() < (FX_FILESIZE)offset) {
                    return -1;
                }
                pTiffContext->offset = (FX_DWORD)(pTiffContext->io.out->GetSize() - offset);
            }
            break;
        default:
            return -1;
    }
    ASSERT(pTiffContext->isDecoder ?
           (pTiffContext->offset <= (FX_DWORD)pTiffContext->io.in->GetSize()) :
           TRUE);
    return pTiffContext->offset;
}
static int _tiff_close(thandle_t context)
{
    return 0;
}
static toff_t _tiff_get_size(thandle_t context)
{
    CCodec_TiffContext* pTiffContext = (CCodec_TiffContext*)context;
    return pTiffContext->isDecoder ?
           (toff_t)pTiffContext->io.in->GetSize() :
           (toff_t)pTiffContext->io.out->GetSize();
}
static int _tiff_map(thandle_t context, tdata_t*, toff_t*)
{
    return 0;
}
static void _tiff_unmap(thandle_t context, tdata_t, toff_t) {}
TIFF* _tiff_open(void* context, const char* mode)
{
    TIFF* tif = TIFFClientOpen("Tiff Image", mode,
                               (thandle_t)context,
                               _tiff_read, _tiff_write, _tiff_seek, _tiff_close,
                               _tiff_get_size, _tiff_map, _tiff_unmap);
    if(tif)	{
        tif->tif_fd = (int)(FX_INTPTR)context;
    }
    return tif;
}
void* _TIFFmalloc(tmsize_t size)
{
    return FXMEM_DefaultAlloc(size, 0);
}
void _TIFFfree(void* ptr)
{
    FXMEM_DefaultFree(ptr, 0);
}
void* _TIFFrealloc(void* ptr, tmsize_t size)
{
    return FXMEM_DefaultRealloc(ptr, size, 0);
}
void _TIFFmemset(void* ptr, int val, tmsize_t size)
{
    FXSYS_memset8(ptr, val, (size_t)size);
}
void _TIFFmemcpy(void* des, const void* src, tmsize_t size)
{
    FXSYS_memcpy32(des, src, (size_t)size);
}
int _TIFFmemcmp(const void* ptr1, const void* ptr2, tmsize_t size)
{
    return FXSYS_memcmp32(ptr1, ptr2, (size_t)size);
}
static void _tiff_warning_ext(thandle_t context, const char* module, const char* fmt, va_list ap)
{
    if(module != NULL) {
    }
}
TIFFErrorHandlerExt _TIFFwarningHandlerExt = _tiff_warning_ext;
static void _tiff_error_ext(thandle_t context, const char* module, const char* fmt, va_list ap)
{
    if(module != NULL) {
    }
}
TIFFErrorHandlerExt _TIFFerrorHandlerExt = _tiff_error_ext;
int TIFFCmyk2Rgb(thandle_t context, uint8 c, uint8 m, uint8 y, uint8 k, uint8* r, uint8* g, uint8* b)
{
    if(context == NULL) {
        return 0;
    }
    CCodec_TiffContext* p = (CCodec_TiffContext*)context;
#if !defined(_FPDFAPI_MINI_)
    if(p->icc_ctx) {
        unsigned char cmyk[4], bgr[3];
        cmyk[0] = c, cmyk[1] = m, cmyk[2] = y, cmyk[3] = k;
        IccLib_TranslateImage(p->icc_ctx, bgr, cmyk, 1);
        *r = bgr[2], *g = bgr[1], *b = bgr[0];
    } else {
#endif
        AdobeCMYK_to_sRGB1(c, m, y, k, *r, *g, *b);
#if !defined(_FPDFAPI_MINI_)
    }
#endif
    return 1;
}
FX_BOOL CCodec_TiffContext::InitDecoder(IFX_FileRead* file_ptr)
{
    io.in = file_ptr;
    tif_ctx = _tiff_open(this, "r");
    if(tif_ctx == NULL) {
        return FALSE;
    }
    return TRUE;
}
void CCodec_TiffContext::GetFrames(FX_INT32& frames)
{
    frames = frame_num = TIFFNumberOfDirectories(tif_ctx);
}
#define TIFF_EXIF_GETINFO(key, T, tag) {\
        T val = (T)0;\
        TIFFGetField(tif_ctx,tag,&val);\
        if (val) {\
            (key) = FX_Alloc(FX_BYTE,sizeof(T));\
            if ((key)) {\
                T* ptr = (T*)(key);\
                *ptr = val;\
                pExif->m_TagVal.SetAt(tag,(key));}}}\
    (key) = NULL;
#define TIFF_EXIF_GETSTRINGINFO(key, tag) {\
        FX_DWORD size = 0;\
        FX_LPBYTE buf = NULL;\
        TIFFGetField(tif_ctx,tag,&size, &buf);\
        if (size && buf) {\
            (key) = FX_Alloc(FX_BYTE,size);\
            if ((key)) {\
                FXSYS_memcpy32((key),buf,size);\
                pExif->m_TagVal.SetAt(tag,(key));}}}\
    (key) = NULL;
template <class T>
static FX_BOOL Tiff_Exif_GetInfo(TIFF* tif_ctx, ttag_t tag, CFX_DIBAttributeExif* pExif)
{
    FX_LPBYTE key = NULL;
    T val = (T)0;
    TIFFGetField(tif_ctx, tag, &val);
    if (val) {
        (key) = FX_Alloc(FX_BYTE, sizeof(T));
        if ((key) == NULL) {
            return FALSE;
        }
        T* ptr = (T*)(key);
        *ptr = val;
        pExif->m_TagVal.SetAt(tag, (key));
        return TRUE;
    }
    return FALSE;
}
static void Tiff_Exif_GetStringInfo(TIFF* tif_ctx, ttag_t tag, CFX_DIBAttributeExif* pExif)
{
    FX_LPSTR buf = NULL;
    FX_LPBYTE key = NULL;
    TIFFGetField(tif_ctx, tag, &buf);
    if (buf) {
        FX_INT32 size = (FX_INT32)FXSYS_strlen(buf);
        (key) = FX_Alloc(FX_BYTE, size + 1);
        if ((key) == NULL) {
            return;
        }
        FXSYS_memcpy32((key), buf, size);
        key[size] = 0;
        pExif->m_TagVal.SetAt(tag, (key));
    }
}
FX_BOOL CCodec_TiffContext::LoadFrameInfo(FX_INT32 frame, FX_DWORD& width, FX_DWORD& height, FX_DWORD& comps, FX_DWORD& bpc, CFX_DIBAttribute* pAttribute)
{
    if (!TIFFSetDirectory(tif_ctx, (uint16)frame))	{
        return FALSE;
    }
    FX_WORD tif_cs;
    FX_DWORD tif_icc_size = 0;
    FX_LPBYTE tif_icc_buf = NULL;
    FX_WORD tif_bpc = 0;
    FX_WORD tif_cps;
    FX_DWORD tif_rps;
    width = height = comps = 0;
    TIFFGetField(tif_ctx, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif_ctx, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(tif_ctx, TIFFTAG_SAMPLESPERPIXEL, &comps);
    TIFFGetField(tif_ctx, TIFFTAG_BITSPERSAMPLE, &tif_bpc);
    TIFFGetField(tif_ctx, TIFFTAG_PHOTOMETRIC, &tif_cs);
    TIFFGetField(tif_ctx, TIFFTAG_COMPRESSION, &tif_cps);
    TIFFGetField(tif_ctx, TIFFTAG_ROWSPERSTRIP, &tif_rps);
    TIFFGetField(tif_ctx, TIFFTAG_ICCPROFILE, &tif_icc_size, &tif_icc_buf);
    if (pAttribute) {
        pAttribute->m_wDPIUnit = FXCODEC_RESUNIT_INCH;
        if (TIFFGetField(tif_ctx, TIFFTAG_RESOLUTIONUNIT, &pAttribute->m_wDPIUnit)) {
            pAttribute->m_wDPIUnit -= 1;
        }
        CFX_DIBAttributeExif* pExif = (CFX_DIBAttributeExif*)pAttribute->m_pExif;
        pExif->clear();
        Tiff_Exif_GetInfo<FX_WORD>(tif_ctx, TIFFTAG_ORIENTATION, pExif);
        if (Tiff_Exif_GetInfo<FX_FLOAT>(tif_ctx, TIFFTAG_XRESOLUTION, pExif)) {
            FX_FLOAT fDpi = 0;
            pExif->GetInfo(TIFFTAG_XRESOLUTION, &fDpi);
            pAttribute->m_nXDPI = (FX_INT32)(fDpi + 0.5f);
        }
        if (Tiff_Exif_GetInfo<FX_FLOAT>(tif_ctx, TIFFTAG_YRESOLUTION, pExif)) {
            FX_FLOAT fDpi = 0;
            pExif->GetInfo(TIFFTAG_YRESOLUTION, &fDpi);
            pAttribute->m_nYDPI = (FX_INT32)(fDpi + 0.5f);
        }
        Tiff_Exif_GetStringInfo(tif_ctx, TIFFTAG_IMAGEDESCRIPTION, pExif);
        Tiff_Exif_GetStringInfo(tif_ctx, TIFFTAG_MAKE, pExif);
        Tiff_Exif_GetStringInfo(tif_ctx, TIFFTAG_MODEL, pExif);
    }
    bpc = tif_bpc;
    if(tif_rps > height) {
        TIFFSetField(tif_ctx, TIFFTAG_ROWSPERSTRIP, tif_rps = height);
    }
    return TRUE;
}
void _TiffBGRA2RGBA(FX_LPBYTE pBuf, FX_INT32 pixel, FX_INT32 spp)
{
    register FX_BYTE tmp;
    for (FX_INT32 n = 0; n < pixel; n++) {
        tmp = pBuf[0];
        pBuf[0] = pBuf[2];
        pBuf[2] = tmp;
        pBuf += spp;
    }
}
FX_BOOL CCodec_TiffContext::isSupport(CFX_DIBitmap* pDIBitmap)
{
    if (TIFFIsTiled(tif_ctx)) {
        return FALSE;
    }
    FX_UINT16 photometric;
    if (!TIFFGetField(tif_ctx, TIFFTAG_PHOTOMETRIC, &photometric)) {
        return FALSE;
    }
    switch (pDIBitmap->GetBPP()) {
        case 1:
        case 8:
            if (photometric != PHOTOMETRIC_PALETTE) {
                return FALSE;
            }
            break;
        case 24:
            if (photometric != PHOTOMETRIC_RGB) {
                return FALSE;
            }
            break;
        default:
            return FALSE;
    }
    FX_UINT16 planarconfig;
    if (!TIFFGetFieldDefaulted(tif_ctx, TIFFTAG_PLANARCONFIG, &planarconfig)) {
        return FALSE;
    }
    if (planarconfig == PLANARCONFIG_SEPARATE) {
        return FALSE;
    }
    return TRUE;
}
void CCodec_TiffContext::SetPalette(CFX_DIBitmap* pDIBitmap, FX_UINT16 bps)
{
    FX_UINT16 *red_orig, *green_orig, *blue_orig;
    TIFFGetField(tif_ctx, TIFFTAG_COLORMAP, &red_orig, &green_orig, &blue_orig);
    for (FX_INT32 i = (1L << bps) - 1; i >= 0; i--) {
#define	CVT(x)		((FX_UINT16)((x)>>8))
        red_orig[i] = CVT(red_orig[i]);
        green_orig[i] = CVT(green_orig[i]);
        blue_orig[i] = CVT(blue_orig[i]);
#undef	CVT
    }
    FX_INT32 len = 1 << bps;
    for(FX_INT32 index = 0; index < len; index++) {
        FX_DWORD r = red_orig[index] & 0xFF;
        FX_DWORD g = green_orig[index] & 0xFF;
        FX_DWORD b = blue_orig[index] & 0xFF;
        FX_DWORD color = (FX_UINT32)b | ((FX_UINT32)g << 8) | ((FX_UINT32)r << 16) | (((uint32)0xffL) << 24);
        pDIBitmap->SetPaletteEntry(index, color);
    }
}
FX_BOOL CCodec_TiffContext::Decode1bppRGB(CFX_DIBitmap* pDIBitmap, FX_INT32 height, FX_INT32 width, FX_UINT16 bps, FX_UINT16 spp)
{
    if (pDIBitmap->GetBPP() != 1 || spp != 1 || bps != 1 || !isSupport(pDIBitmap)) {
        return FALSE;
    }
    SetPalette(pDIBitmap, bps);
    FX_INT32 size = (FX_INT32)TIFFScanlineSize(tif_ctx);
    FX_LPBYTE buf = (FX_LPBYTE)_TIFFmalloc(size);
    if (buf == NULL) {
        TIFFError(TIFFFileName(tif_ctx), "No space for scanline buffer");
        return FALSE;
    }
    FX_LPBYTE bitMapbuffer = (FX_LPBYTE)pDIBitmap->GetBuffer();
    FX_DWORD pitch = pDIBitmap->GetPitch();
    for(FX_INT32 row = 0; row < height; row++) {
        TIFFReadScanline(tif_ctx, buf, row, 0);
        for(FX_INT32 j = 0; j < size; j++) {
            bitMapbuffer[row * pitch + j] = buf[j];
        }
    }
    _TIFFfree(buf);
    return TRUE;
}
FX_BOOL CCodec_TiffContext::Decode8bppRGB(CFX_DIBitmap* pDIBitmap, FX_INT32 height, FX_INT32 width, FX_UINT16 bps, FX_UINT16 spp)
{
    if (pDIBitmap->GetBPP() != 8 || spp != 1 || (bps != 4 && bps != 8) || !isSupport(pDIBitmap)) {
        return FALSE;
    }
    SetPalette(pDIBitmap, bps);
    FX_INT32 size = (FX_INT32)TIFFScanlineSize(tif_ctx);
    FX_LPBYTE buf = (FX_LPBYTE)_TIFFmalloc(size);
    if (buf == NULL) {
        TIFFError(TIFFFileName(tif_ctx), "No space for scanline buffer");
        return FALSE;
    }
    FX_LPBYTE bitMapbuffer = (FX_LPBYTE)pDIBitmap->GetBuffer();
    FX_DWORD pitch = pDIBitmap->GetPitch();
    for(FX_INT32 row = 0; row < height; row++) {
        TIFFReadScanline(tif_ctx, buf, row, 0);
        for(FX_INT32 j = 0; j < size; j++) {
            switch(bps) {
                case 4:
                    bitMapbuffer[row * pitch + 2 * j + 0] = (buf[j] & 0xF0) >> 4;
                    bitMapbuffer[row * pitch + 2 * j + 1] = (buf[j] & 0x0F) >> 0;
                    break;
                case 8:
                    bitMapbuffer[row * pitch + j] = buf[j];
                    break;
            }
        }
    }
    _TIFFfree(buf);
    return TRUE;
}
FX_BOOL CCodec_TiffContext::Decode24bppRGB(CFX_DIBitmap* pDIBitmap, FX_INT32 height, FX_INT32 width, FX_UINT16 bps, FX_UINT16 spp)
{
    if (pDIBitmap->GetBPP() != 24 || !isSupport(pDIBitmap)) {
        return FALSE;
    }
    FX_INT32 size = (FX_INT32)TIFFScanlineSize(tif_ctx);
    FX_LPBYTE buf = (FX_LPBYTE)_TIFFmalloc(size);
    if (buf == NULL) {
        TIFFError(TIFFFileName(tif_ctx), "No space for scanline buffer");
        return FALSE;
    }
    FX_LPBYTE bitMapbuffer = (FX_LPBYTE)pDIBitmap->GetBuffer();
    FX_DWORD pitch = pDIBitmap->GetPitch();
    for(FX_INT32 row = 0; row < height; row++) {
        TIFFReadScanline(tif_ctx, buf, row, 0);
        for(FX_INT32 j = 0; j < size - 2; j += 3) {
            bitMapbuffer[row * pitch + j + 0] = buf[j + 2];
            bitMapbuffer[row * pitch + j + 1] = buf[j + 1];
            bitMapbuffer[row * pitch + j + 2] = buf[j + 0];
        }
    }
    _TIFFfree(buf);
    return TRUE;
}
FX_BOOL CCodec_TiffContext::Decode(CFX_DIBitmap* pDIBitmap)
{
    FX_DWORD img_wid = pDIBitmap->GetWidth();
    FX_DWORD img_hei = pDIBitmap->GetHeight();
    FX_DWORD width = 0;
    FX_DWORD height = 0;
    TIFFGetField(tif_ctx, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif_ctx, TIFFTAG_IMAGELENGTH, &height);
    if (img_wid != width || img_hei != height) {
        return FALSE;
    }
    if (pDIBitmap->GetBPP() == 32) {
        FX_WORD rotation = ORIENTATION_TOPLEFT;
        TIFFGetField(tif_ctx, TIFFTAG_ORIENTATION, &rotation);
        if(TIFFReadRGBAImageOriented(tif_ctx, img_wid, img_hei,
                                     (uint32*)pDIBitmap->GetBuffer(), rotation, 1)) {
            for (FX_DWORD row = 0; row < img_hei; row++) {
                FX_LPBYTE row_buf = (FX_LPBYTE)pDIBitmap->GetScanline(row);
                _TiffBGRA2RGBA(row_buf, img_wid, 4);
            }
            return TRUE;
        }
    }
    FX_UINT16 spp, bps;
    TIFFGetField(tif_ctx, TIFFTAG_SAMPLESPERPIXEL, &spp);
    TIFFGetField(tif_ctx, TIFFTAG_BITSPERSAMPLE, &bps);
    FX_DWORD bpp = bps * spp;
    if (bpp == 1) {
        return Decode1bppRGB(pDIBitmap, height, width, bps, spp);
    } else if (bpp <= 8) {
        return Decode8bppRGB(pDIBitmap, height, width, bps, spp);
    } else if (bpp <= 24) {
        return Decode24bppRGB(pDIBitmap, height, width, bps, spp);
    }
    return FALSE;
}
FX_LPVOID CCodec_TiffModule::CreateDecoder(IFX_FileRead* file_ptr)
{
    CCodec_TiffContext* pDecoder = FX_NEW CCodec_TiffContext;
    if (pDecoder == NULL) {
        return NULL;
    }
    if (!pDecoder->InitDecoder(file_ptr)) {
        delete pDecoder;
        return NULL;
    }
    return pDecoder;
}
void CCodec_TiffModule::GetFrames(FX_LPVOID ctx, FX_INT32& frames)
{
    CCodec_TiffContext* pDecoder = (CCodec_TiffContext*)ctx;
    pDecoder->GetFrames(frames);
}
FX_BOOL CCodec_TiffModule::LoadFrameInfo(FX_LPVOID ctx, FX_INT32 frame, FX_DWORD& width, FX_DWORD& height, FX_DWORD& comps, FX_DWORD& bpc, CFX_DIBAttribute* pAttribute)
{
    CCodec_TiffContext* pDecoder = (CCodec_TiffContext*)ctx;
    return pDecoder->LoadFrameInfo(frame, width, height, comps, bpc, pAttribute);
}
FX_BOOL CCodec_TiffModule::Decode(void* ctx, class CFX_DIBitmap* pDIBitmap)
{
    CCodec_TiffContext* pDecoder = (CCodec_TiffContext*)ctx;
    return pDecoder->Decode(pDIBitmap);
}
void CCodec_TiffModule::DestroyDecoder(void* ctx)
{
    CCodec_TiffContext* pDecoder = (CCodec_TiffContext*)ctx;
    delete pDecoder;
}
