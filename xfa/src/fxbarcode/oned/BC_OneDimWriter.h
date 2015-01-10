// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDIMWRITER_H_
#define _BC_ONEDIMWRITER_H_
class CBC_Writer;
class CBC_CommonBitMatrix;
class CBC_OneDimWriter;
class CBC_OneDimWriter : public CBC_Writer
{
public:
    CBC_OneDimWriter();
    virtual ~CBC_OneDimWriter();
    FX_BYTE			*Encode(const CFX_ByteString &contents, BCFORMAT format,
                            FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e);
    FX_BYTE			*Encode(const CFX_ByteString &contents, BCFORMAT format,
                            FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e);
    virtual FX_BYTE *Encode(const CFX_ByteString &contents, FX_INT32 &outLength, FX_INT32 &e)
    {
        return NULL;
    };
    virtual void     RenderResult(FX_WSTR contents, FX_BYTE* code, FX_INT32 codeLength,  FX_BOOL isDevice, FX_INT32 &e);
    virtual void     RenderBitmapResult(CFX_DIBitmap *&pOutBitmap, FX_WSTR contents, FX_INT32 &e);
    virtual void     RenderDeviceResult(CFX_RenderDevice* device, const CFX_Matrix* matrix, FX_WSTR contents, FX_INT32 &e);
    virtual FX_BOOL			CheckContentValidity(FX_WSTR contents)
    {
        return TRUE;
    };
    virtual CFX_WideString	FilterContents(FX_WSTR contents)
    {
        return CFX_WideString();
    }
    virtual CFX_WideString	RenderTextContents(FX_WSTR contents)
    {
        return CFX_WideString();
    }
    virtual void			SetPrintChecksum(FX_BOOL checksum);
    virtual void			SetDataLength(FX_INT32 length);
    virtual void			SetCalcChecksum(FX_INT32 state);
    virtual void			SetFontSize(FX_FLOAT size);
    virtual void			SetFontStyle(FX_INT32 style);
    virtual void			SetFontColor(FX_ARGB color);
    virtual FX_BOOL			SetFont(CFX_Font * cFont);
protected:
    FX_BOOL			m_bPrintChecksum;
    FX_INT32		m_iDataLenth;
    FX_BOOL			m_bCalcChecksum;
    CFX_Font*		m_pFont;
    FX_FLOAT		m_fFontSize;
    FX_INT32		m_iFontStyle;
    FX_DWORD		m_fontColor;
    BC_TEXT_LOC		m_locTextLoc;
    FX_INT32		m_iContentLen;
    FX_BOOL         m_bLeftPadding;
    FX_BOOL         m_bRightPadding;
    CBC_CommonBitMatrix*   m_output;
    FX_INT32               m_barWidth;
    FX_INT32               m_multiple;
    FX_FLOAT               m_outputHScale;
    void			CalcTextInfo(const CFX_ByteString &text, FXTEXT_CHARPOS *charPos, CFX_Font *cFont, FX_FLOAT geWidth, FX_INT32 fontSize, FX_FLOAT &charsLen);
    virtual void	ShowChars(FX_WSTR contents, CFX_DIBitmap *pOutBitmap, CFX_RenderDevice *device, const CFX_Matrix* matrix, FX_INT32 barWidth, FX_INT32 multiple, FX_INT32 &e);
    virtual void	ShowBitmapChars(CFX_DIBitmap *pOutBitmap, const CFX_ByteString str, FX_FLOAT geWidth, FXTEXT_CHARPOS* pCharPos, FX_FLOAT locX, FX_FLOAT locY, FX_INT32 barWidth);
    virtual void    ShowDeviceChars(CFX_RenderDevice *device, const CFX_Matrix* matrix, const CFX_ByteString str, FX_FLOAT geWidth, FXTEXT_CHARPOS* pCharPos, FX_FLOAT locX, FX_FLOAT locY,  FX_INT32 barWidth);
    FX_INT32		AppendPattern(FX_BYTE* target, FX_INT32 pos, const FX_INT32* pattern, FX_INT32 patternLength, FX_INT32 startColor, FX_INT32 &e);
    FX_WCHAR		Upper(FX_WCHAR ch);
};
#endif
