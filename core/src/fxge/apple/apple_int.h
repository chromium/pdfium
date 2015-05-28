// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXGE_APPLE_APPLE_INT_H_
#define CORE_SRC_FXGE_APPLE_APPLE_INT_H_

#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
#if _FX_OS_ == _FX_MACOSX_
#include <Carbon/Carbon.h>
#endif
typedef enum eFXIOSFONTCHARSET {
    eFXFontCharsetDEFAULT       =   0,
    eFXFontCharsetANSI          =   1,
    eFXFontCharsetSYMBOL        =   1 << 1,
    eFXFontCharsetSHIFTJIS      =   1 << 2,
    eFXFontCharsetHANGEUL       =   1 << 3,
    eFXFontCharsetGB2312        =   1 << 4,
    eFXFontCharsetCHINESEBIG5   =   1 << 5,
    eFXFontCharsetTHAI          =   1 << 6,
    eFXFontCharsetEASTEUROPE    =   1 << 7,
    eFXFontCharsetRUSSIAN       =   1 << 8,
    eFXFontCharsetGREEK         =   1 << 9,
    eFXFontCharsetTURKISH       =   1 << 10,
    eFXFontCharsetHEBREW        =   1 << 11,
    eFXFontCharsetARABIC        =   1 << 12,
    eFXFontCharsetBALTIC        =   1 << 13,
} FX_IOSCHARSET;
FX_IOSCHARSET FX_GetiOSCharset(int charset);
typedef enum eFXIOSFONTFLAG {
    eFXFontFlagBold         =  1,
    eFXFontFlagItalic       =  1 << 1,
    eFXFontFlagFixedPitch   =  1 << 2,
    eFXFontFlagSerif        =  1 << 3,
    eFXFontFlagScript       =  1 << 4,
} FX_IOSFONTFLAG;
typedef struct _IOS_FONTDATA {
    FX_DWORD        nHashCode;
    const char*       psName;
    FX_DWORD        charsets;
    FX_DWORD        styles;
} IOS_FONTDATA;
class CQuartz2D
{
public:
    void*	createGraphics(CFX_DIBitmap* bitmap);
    void	destroyGraphics(void* graphics);

    void*	CreateFont(FX_LPCBYTE pFontData, FX_DWORD dwFontSize);
    void	DestroyFont(void* pFont);
    void	setGraphicsTextMatrix(void* graphics, CFX_AffineMatrix* matrix);
    FX_BOOL	drawGraphicsString(void*                graphics,
                               void*                font,
                               FX_FLOAT             fontSize,
                               FX_WORD*             glyphIndices,
                               CGPoint*          glyphPositions,
                               FX_INT32             chars,
                               FX_ARGB              argb,
                               CFX_AffineMatrix*    matrix = NULL);
    void saveGraphicsState(void* graphics);
    void restoreGraphicsState(void* graphics);
};
class CApplePlatform 
{
public:
    CApplePlatform()
    {
        m_pFontMapper = NULL;
    }
    ~CApplePlatform()
    {
        if (m_pFontMapper) {
            delete m_pFontMapper;
        }
    }
    CQuartz2D	_quartz2d;
    IFX_FontMapper* m_pFontMapper;
};
class CFX_QuartzDeviceDriver : public IFX_RenderDeviceDriver
{
public:
    CFX_QuartzDeviceDriver(CGContextRef context, FX_INT32 deviceClass);
    virtual ~CFX_QuartzDeviceDriver();

    virtual int		GetDeviceCaps(int caps_id);
    virtual CFX_Matrix	GetCTM() const;
    virtual CFX_DIBitmap*   GetBackDrop()
    {
        return NULL;
    }
    virtual void*	GetPlatformSurface()
    {
        return NULL;
    }
    virtual FX_BOOL IsPSPrintDriver()
    {
        return FALSE;
    }
    virtual FX_BOOL	StartRendering()
    {
        return TRUE;
    }
    virtual void	EndRendering() {}
    virtual void	SaveState();
    virtual void	RestoreState(FX_BOOL bKeepSaved);
    virtual FX_BOOL	SetClip_PathFill(const CFX_PathData* pPathData,
                                     const CFX_AffineMatrix* pObject2Device,
                                     int fill_mode
                                    );
    virtual FX_BOOL	SetClip_PathStroke(const CFX_PathData* pPathData,
                                       const CFX_AffineMatrix* pObject2Device,
                                       const CFX_GraphStateData* pGraphState
                                      );
    virtual FX_BOOL	DrawPath(const CFX_PathData* pPathData,
                             const CFX_AffineMatrix* pObject2Device,
                             const CFX_GraphStateData* pGraphState,
                             FX_DWORD fill_color,
                             FX_DWORD stroke_color,
                             int fill_mode,
                             int alpha_flag = 0,
                             void* pIccTransform = NULL,
                             int blend_type = FXDIB_BLEND_NORMAL
                            );
    virtual FX_BOOL	SetPixel(int x, int y, FX_DWORD color,
                             int alpha_flag = 0, void* pIccTransform = NULL)
    {
        return FALSE;
    }
    virtual FX_BOOL FillRect(const FX_RECT* pRect, FX_DWORD fill_color,
                             int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL);
    virtual FX_BOOL	DrawCosmeticLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2, FX_DWORD color,
                                     int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL);
    virtual FX_BOOL GetClipBox(FX_RECT* pRect);
    virtual FX_BOOL	GetDIBits(CFX_DIBitmap* pBitmap, int left, int top, void* pIccTransform = NULL, FX_BOOL bDEdge = FALSE);
    virtual FX_BOOL	SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, const FX_RECT* pSrcRect,
                              int dest_left, int dest_top, int blend_type,
                              int alpha_flag = 0, void* pIccTransform = NULL);
    virtual FX_BOOL	StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                  int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
                                  int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL);
    virtual FX_BOOL	StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                const CFX_AffineMatrix* pMatrix, FX_DWORD flags, FX_LPVOID& handle,
                                int alpha_flag = 0, void* pIccTransform = NULL,
                                int blend_type = FXDIB_BLEND_NORMAL)
    {
        return FALSE;
    }
    virtual FX_BOOL	ContinueDIBits(FX_LPVOID handle, IFX_Pause* pPause)
    {
        return FALSE;
    }
    virtual void	CancelDIBits(FX_LPVOID handle) {}
    virtual FX_BOOL DrawDeviceText(int nChars, const FXTEXT_CHARPOS* pCharPos, CFX_Font* pFont,
                                   CFX_FontCache* pCache, const CFX_AffineMatrix* pObject2Device, FX_FLOAT font_size, FX_DWORD color,
                                   int alpha_flag = 0, void* pIccTransform = NULL);
    virtual void    ClearDriver();
protected:
    void setStrokeInfo(const CFX_GraphStateData * graphState, FX_ARGB argb, FX_FLOAT lineWidth);
    void setFillInfo(FX_ARGB argb);
    void setPathToContext(const CFX_PathData * pathData);
    FX_FLOAT getLineWidth(const CFX_GraphStateData * graphState, CGAffineTransform ctm);
    FX_BOOL CG_DrawGlypRun(int                        nChars,
                           const FXTEXT_CHARPOS*      pCharPos,
                           CFX_Font*                  pFont,
                           CFX_FontCache*             pCache,
                           const CFX_AffineMatrix*    pGlyphMatrix,
                           const CFX_AffineMatrix*    pObject2Device,
                           FX_FLOAT                   font_size,
                           FX_DWORD                   argb,
                           int                        alpha_flag,
                           void*                      pIccTransform);
    void    CG_SetImageTransform(int dest_left, int dest_top, int dest_width, int dest_height, CGRect* rect = NULL);
protected:
    CGContextRef	_context;
    CGAffineTransform _foxitDevice2User;
    CGAffineTransform _user2FoxitDevice;
    FX_INT32        m_saveCount;

    FX_INT32		_width;
    FX_INT32		_height;
    FX_INT32		_bitsPerPixel;
    FX_INT32		_deviceClass;
    FX_INT32		_renderCaps;
    FX_INT32	_horzSize;
    FX_INT32	_vertSize;
};
class CFX_FontProvider FX_FINAL : public IFX_FileRead
{
public:
    virtual void			Release() FX_OVERRIDE
    {
        delete this;
    }
    virtual FX_FILESIZE		GetSize() FX_OVERRIDE
    {
        return (FX_FILESIZE)_totalSize;
    }
    virtual FX_BOOL			ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) FX_OVERRIDE;

    virtual FX_BOOL			IsEOF() FX_OVERRIDE
    {
        return _offSet == _totalSize;
    }
    virtual FX_FILESIZE		GetPosition() FX_OVERRIDE
    {
        return (FX_FILESIZE)_offSet;
    }
    virtual size_t			ReadBlock(void* buffer, size_t size) FX_OVERRIDE;
public:
    CFX_FontProvider(CGFontRef cgFont);
    ~CFX_FontProvider();
    void            InitTableOffset();
    unsigned long   Read(unsigned long offset, unsigned char *buffer, unsigned long count);
protected:
    uint32_t CalcTableCheckSum(const uint32_t *table, uint32_t numberOfBytesInTable);
    uint32_t CalcTableDataRefCheckSum(CFDataRef dataRef);
private:
    CGFontRef   m_cgFont;
    UInt32      m_iTableSize;
    size_t      _offSet;
    typedef struct FontHeader {
        int32_t fVersion;
        uint16_t fNumTables;
        uint16_t fSearchRange;
        uint16_t fEntrySelector;
        uint16_t fRangeShift;
    } FontHeader;
    typedef struct TableEntry {
        uint32_t fTag;
        uint32_t fCheckSum;
        uint32_t fOffset;
        uint32_t fLength;
    } TableEntry;
    FontHeader _fontHeader;
    unsigned char * _tableEntries;
    size_t * _tableOffsets;
    int _tableCount;
    int _totalSize;
};
FX_UINT32 FX_GetHashCode( FX_LPCSTR pStr);
FX_DWORD  FX_IOSGetMatchFamilyNameHashcode(FX_LPCSTR pFontName);
FX_UINT32 FX_IOSGetFamilyNamesCount();
FX_LPCSTR FX_IOSGetFamilyName( FX_UINT32 uIndex);
#endif

#endif  // CORE_SRC_FXGE_APPLE_APPLE_INT_H_
