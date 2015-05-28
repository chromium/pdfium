// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_SRC_FXGE_SKIA_FX_SKIA_BLITTER_NEW_H_
#define CORE_SRC_FXGE_SKIA_FX_SKIA_BLITTER_NEW_H_

#if defined(_SKIA_SUPPORT_)
class CFX_SkiaRenderer : public SkBlitter
{
protected:
	int			m_Alpha, 
				m_Red,		// Or the complementary-color, Cyan
				m_Green,	// Magenta
				m_Blue,		// Yellow
				m_Gray;		// Black
	FX_DWORD	m_Color;	// FX_ARGB or FX_CMYK
	FX_BOOL		m_bFullCover;
	int			m_ProcessFilter;
	FX_BOOL     m_bRgbByteOrder;
	
	FX_RECT				m_ClipBox;
	CFX_DIBitmap*		m_pDevice;
	CFX_DIBitmap*		m_pOriDevice;
	const CFX_ClipRgn*	m_pClipRgn;
	const CFX_DIBitmap*	m_pClipMask;

	FX_LPBYTE m_pDestScan;
	FX_LPBYTE m_pDestExtraAlphaScan;
	FX_LPBYTE m_pOriScan;
	FX_LPBYTE m_pClipScan;

	void (CFX_SkiaRenderer::*composite_span)(FX_LPBYTE,FX_LPBYTE,int,int,int,int,FX_BYTE,int,int,int,FX_LPBYTE,FX_LPBYTE);
public:
    
    //--------------------------------------------------------------------
    virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]);
	virtual void blitH(int x, int y, int width); 
	virtual void blitV(int x, int y, int height, SkAlpha alpha);
	virtual void blitRect(int x, int y, int width, int height);
	virtual	void blitAntiRect(int x, int y, int width, int height, SkAlpha leftAlpha, SkAlpha rightAlpha);
    	
	/*------------------------------------------------------------------------------------------------------*/
	// A general alpha merge function (with clipping mask). Gray device.
	void CompositeSpan1bpp_0(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpan1bpp_1(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpan1bpp_4(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpan1bpp_5(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpan1bpp_8(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpan1bpp_9(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpan1bpp_12(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpan1bpp_13(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);

	/*--------------------------------------------------------------------------------------------------------*/
	
	// A general alpha merge function (with clipping mask). Gray device.
	void CompositeSpanGray_2(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);

	void CompositeSpanGray_3(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);

	void CompositeSpanGray_6(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);

	void CompositeSpanGray_7(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);

	void CompositeSpanGray_10(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);


	void CompositeSpanGray_11(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);

	void CompositeSpanGray_14(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);

	void CompositeSpanGray_15(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);

	/*--------------------------------------------------------------------------------------------------------*/
	void CompositeSpanARGB_2(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);

	void CompositeSpanARGB_3(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);


	void CompositeSpanARGB_6(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);


	void CompositeSpanARGB_7(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	// ...
	/*--------------------------------------------------------------------------------------------------------*/
	void CompositeSpanRGB32_2(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpanRGB32_3(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpanRGB32_6(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpanRGB32_7(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	
	/*---------------------------------------------------------------------------------------------------------*/

	void CompositeSpanRGB24_2(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpanRGB24_3(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpanRGB24_6(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpanRGB24_7(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpanRGB24_10(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpanRGB24_11(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpanRGB24_14(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
	void CompositeSpanRGB24_15(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);

	/*----------------------------------------------------------------------------------------------------------*/
	
	// A general alpha merge function (with clipping mask). Cmyka/Cmyk device.
	void CompositeSpanCMYK(FX_LPBYTE dest_scan, FX_LPBYTE ori_scan,int Bpp,
			int span_left, int span_len, int span_top, FX_BYTE cover_scan, 
			int clip_top, int clip_left, int clip_right, FX_LPBYTE clip_scan, 
			FX_LPBYTE dest_extra_alpha_scan);
   
	
	//--------------------------------------------------------------------
	FX_BOOL Init(CFX_DIBitmap* pDevice, CFX_DIBitmap* pOriDevice, const CFX_ClipRgn* pClipRgn, FX_DWORD color, FX_BOOL bFullCover, FX_BOOL bRgbByteOrder, 
		int alpha_flag = 0, void* pIccTransform = NULL); //The alpha flag must be fill_flag if exist.
};
class CFX_SkiaA8Renderer : public SkBlitter
{
public:
    //--------------------------------------------------------------------
    virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]);
	virtual void blitH(int x, int y, int width); 
	virtual void blitV(int x, int y, int height, SkAlpha alpha);
	virtual void blitRect(int x, int y, int width, int height);
	virtual	void blitAntiRect(int x, int y, int width, int height, SkAlpha leftAlpha, SkAlpha rightAlpha);
	//--------------------------------------------------------------------
	FX_BOOL Init(CFX_DIBitmap* pDevice, int Left, int Top);
	CFX_DIBitmap* m_pDevice;
	int m_Left;
	int m_Top;
	int m_dstWidth;
	int m_dstHeight;
};
#endif

#endif  // CORE_SRC_FXGE_SKIA_FX_SKIA_BLITTER_NEW_H_
