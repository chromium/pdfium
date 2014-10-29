// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_CODEC_PROGRESS_H_
#define _FX_CODEC_PROGRESS_H_
#define FXCODEC_BLOCK_SIZE 4096
#define FXCODEC_PNG_GAMMA	2.2
#if _FX_OS_ == _FX_MACOSX_ || _FX_OS_ == _FX_IOS_
#	undef FXCODEC_PNG_GAMMA
#	define FXCODEC_PNG_GAMMA	1.7
#endif
struct PixelWeight {
    int		m_SrcStart;
    int		m_SrcEnd;
    int		m_Weights[1];
};
class CFXCODEC_WeightTable : public CFX_Object
{
public:
    CFXCODEC_WeightTable()
    {
        m_pWeightTables = NULL;
    }
    ~CFXCODEC_WeightTable()
    {
        if(m_pWeightTables != NULL) {
            FX_Free(m_pWeightTables);
        }
    }

    void			Calc(int dest_len, int dest_min, int dest_max, int src_len, int src_min, int src_max, FX_BOOL bInterpol);
    PixelWeight*	GetPixelWeight(int pixel)
    {
        return (PixelWeight*)(m_pWeightTables + (pixel - m_DestMin) * m_ItemSize);
    }

    int				m_DestMin, m_ItemSize;
    FX_LPBYTE		m_pWeightTables;
};
class CFXCODEC_HorzTable : public CFX_Object
{
public:
    CFXCODEC_HorzTable()
    {
        m_pWeightTables = NULL;
    }
    ~CFXCODEC_HorzTable()
    {
        if(m_pWeightTables != NULL) {
            FX_Free(m_pWeightTables);
        }
    }

    void			Calc(int dest_len, int src_len, FX_BOOL bInterpol);
    PixelWeight*	GetPixelWeight(int pixel)
    {
        return (PixelWeight*)(m_pWeightTables + pixel * m_ItemSize);
    }

    int				m_ItemSize;
    FX_LPBYTE		m_pWeightTables;
};
class CFXCODEC_VertTable : public CFX_Object
{
public:
    CFXCODEC_VertTable()
    {
        m_pWeightTables = NULL;
    }
    ~CFXCODEC_VertTable()
    {
        if(m_pWeightTables != NULL) {
            FX_Free(m_pWeightTables);
        }
    }
    void			Calc(int dest_len, int src_len);
    PixelWeight*	GetPixelWeight(int pixel)
    {
        return (PixelWeight*)(m_pWeightTables + pixel * m_ItemSize);
    }
    int				m_ItemSize;
    FX_LPBYTE		m_pWeightTables;
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
class CCodec_ProgressiveDecoder : public ICodec_ProgressiveDecoder
{
public:
    CCodec_ProgressiveDecoder(CCodec_ModuleMgr* pCodecMgr);
    virtual ~CCodec_ProgressiveDecoder();

public:
    virtual FXCODEC_STATUS		LoadImageInfo(IFX_FileRead* pFile, FXCODEC_IMAGE_TYPE imageType, CFX_DIBAttribute* pAttribute);

    virtual FXCODEC_IMAGE_TYPE	GetType()
    {
        return m_imagType;
    }
    virtual FX_INT32			GetWidth()
    {
        return m_SrcWidth;
    }
    virtual FX_INT32			GetHeight()
    {
        return m_SrcHeight;
    }
    virtual FX_INT32			GetNumComponents()
    {
        return m_SrcComponents;
    }
    virtual FX_INT32			GetBPC()
    {
        return m_SrcBPC;
    }
    virtual void				SetClipBox(FX_RECT* clip);
    virtual FXCODEC_STATUS		GetFrames(FX_INT32& frames, IFX_Pause* pPause);

    virtual FXCODEC_STATUS		StartDecode(CFX_DIBitmap* pDIBitmap,
                                            int start_x, int start_y, int size_x, int size_y,
                                            FX_INT32 frames, FX_BOOL bInterpol);

    virtual FXCODEC_STATUS		ContinueDecode(IFX_Pause* pPause);

protected:
    FX_BOOL						DetectImageType(FXCODEC_IMAGE_TYPE imageType, CFX_DIBAttribute* pAttribute = NULL);
    void						GetDownScale(int& down_scale);
    void						GetTransMethod(FXDIB_Format des_format, FXCodec_Format src_format);
    void						ReSampleScanline(CFX_DIBitmap* pDeviceBitmap, FX_INT32 des_line, FX_LPBYTE src_scan, FXCodec_Format src_format);
    void						Resample(CFX_DIBitmap* pDeviceBitmap, FX_INT32 src_line, FX_LPBYTE src_scan, FXCodec_Format src_format);
    void						ResampleVert(CFX_DIBitmap* pDeviceBitmap, double scale_y, int des_row);
    FX_BOOL						JpegReadMoreData(ICodec_JpegModule* pJpegModule, FXCODEC_STATUS& err_status);
    static FX_BOOL				PngReadHeaderFunc(void* pModule, int width, int height, int bpc, int pass, int* color_type, double* gamma);
    static FX_BOOL				PngAskScanlineBufFunc(void* pModule, int line, FX_LPBYTE& src_buf);
    static void					PngFillScanlineBufCompletedFunc(void* pModule, int pass, int line);
    void						PngOneOneMapResampleHorz(CFX_DIBitmap* pDeviceBitmap, FX_INT32 des_line, FX_LPBYTE src_scan, FXCodec_Format src_format);

    FX_BOOL						GifReadMoreData(ICodec_GifModule* pGifModule, FXCODEC_STATUS& err_status);
    static void					GifRecordCurrentPositionCallback(void* pModule, FX_DWORD& cur_pos);
    static FX_LPBYTE			GifAskLocalPaletteBufCallback(void* pModule, FX_INT32 frame_num, FX_INT32 pal_size);
    static FX_BOOL				GifInputRecordPositionBufCallback(void* pModule, FX_DWORD rcd_pos, const FX_RECT& img_rc,
            FX_INT32 pal_num, void* pal_ptr,
            FX_INT32 delay_time, FX_BOOL user_input,
            FX_INT32 trans_index, FX_INT32 disposal_method, FX_BOOL interlace);
    static void					GifReadScanlineCallback(void* pModule, FX_INT32 row_num, FX_LPBYTE row_buf);
    void						GifDoubleLineResampleVert(CFX_DIBitmap* pDeviceBitmap, double scale_y, int des_row);
    FX_BOOL						BmpReadMoreData(ICodec_BmpModule* pBmpModule, FXCODEC_STATUS& err_status);
    static FX_BOOL				BmpInputImagePositionBufCallback(void* pModule, FX_DWORD rcd_pos);
    static void					BmpReadScanlineCallback(void* pModule, FX_INT32 row_num, FX_LPBYTE row_buf);
    void						ResampleVertBT(CFX_DIBitmap* pDeviceBitmap, double scale_y, int des_row);
public:
    IFX_FileRead*				m_pFile;
    CCodec_ModuleMgr*			m_pCodecMgr;
    FX_LPVOID					m_pJpegContext;
    FX_LPVOID					m_pPngContext;
    FX_LPVOID					m_pGifContext;
    FX_LPVOID					m_pBmpContext;
    FX_LPVOID					m_pTiffContext;
    FXCODEC_IMAGE_TYPE			m_imagType;
    FX_DWORD					m_offSet;
    FX_LPBYTE					m_pSrcBuf;
    FX_DWORD					m_SrcSize;
    FX_LPBYTE					m_pDecodeBuf;
    int							m_ScanlineSize;
    CFX_DIBitmap*				m_pDeviceBitmap;
    FX_BOOL						m_bInterpol;
    CFXCODEC_WeightTable		m_WeightHorz;
    CFXCODEC_VertTable			m_WeightVert;
    CFXCODEC_HorzTable			m_WeightHorzOO;
    int			m_SrcWidth;
    int			m_SrcHeight;
    int			m_SrcComponents;
    int			m_SrcBPC;
    FX_RECT		m_clipBox;
    int			m_startX;
    int			m_startY;
    int			m_sizeX;
    int			m_sizeY;
    int			m_TransMethod;
    FX_ARGB*	m_pSrcPalette;
    int			m_SrcPaletteNumber;
    int			m_SrcRow;
    FXCodec_Format m_SrcFormat;
    int			m_SrcPassNumber;
    int			m_FrameNumber;
    int			m_FrameCur;
    int				m_GifBgIndex;
    FX_LPBYTE		m_pGifPalette;
    FX_INT32		m_GifPltNumber;
    int				m_GifTransIndex;
    FX_RECT			m_GifFrameRect;
    FX_BOOL			m_BmpIsTopBottom;
    FXCODEC_STATUS m_status;
};
#endif
