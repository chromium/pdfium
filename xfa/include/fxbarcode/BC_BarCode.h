// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_CODEBASE_H_
#define _BC_CODEBASE_H_
void BC_Library_Init();
void BC_Library_Destory();
class CBC_CodeBase;
class CBC_Writer;
class CBC_Reader;
enum BC_TEXT_LOC {
    BC_TEXT_LOC_NONE		= 0,
    BC_TEXT_LOC_ABOVE,
    BC_TEXT_LOC_BELOW,
    BC_TEXT_LOC_ABOVEEMBED,
    BC_TEXT_LOC_BELOWEMBED
};
enum BC_CHAR_ENCODING {
    CHAR_ENCODING_UTF8		= 0,
    CHAR_ENCODING_UNICODE
};
enum BC_TYPE {
    BC_UNKNOWN				= -1,
    BC_CODE39				= 0,
    BC_CODABAR,
    BC_CODE128,
    BC_CODE128_B,
    BC_CODE128_C,
    BC_EAN8,
    BC_UPCA,
    BC_EAN13,
    BC_QR_CODE,
    BC_PDF417,
    BC_DATAMATRIX
};
class CBC_CodeBase : public CFX_Object
{
public:
    CBC_CodeBase();
    virtual ~CBC_CodeBase();
    virtual BC_TYPE	 GetType() = 0;
    virtual FX_BOOL  Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e) = 0;
    virtual FX_BOOL	 RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e) = 0;
    virtual FX_BOOL	 RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e) = 0;
    virtual CFX_WideString Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 hight, FX_INT32 &e) = 0;
    virtual CFX_WideString Decode(CFX_DIBitmap *pBitmap, FX_INT32 &e) = 0;

    virtual FX_BOOL  SetCharEncoding(FX_INT32 encoding);
    virtual FX_BOOL  SetModuleHeight(FX_INT32 moduleHeight);
    virtual FX_BOOL  SetModuleWidth(FX_INT32 moduleWidth);

    virtual FX_BOOL  SetHeight(FX_INT32 height);
    virtual FX_BOOL  SetWidth(FX_INT32 width);
    virtual void     SetBackgroundColor(FX_ARGB backgroundColor);
    virtual void     SetBarcodeColor(FX_ARGB foregroundColor);
protected:
    CBC_Writer *	m_pBCWriter;
    CBC_Reader *	m_pBCReader;
};
class CBC_OneCode : public CBC_CodeBase
{
public:
    CBC_OneCode();
    virtual ~CBC_OneCode();
    virtual BC_TYPE	 GetType() = 0;
    virtual FX_BOOL  Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e) = 0;
    virtual FX_BOOL	 RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e) = 0;
    virtual FX_BOOL	 RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e) = 0;
    virtual CFX_WideString Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 hight, FX_INT32 &e) = 0;
    virtual CFX_WideString Decode(CFX_DIBitmap *pBitmap, FX_INT32 &e) = 0;
    virtual FX_BOOL			CheckContentValidity(FX_WSTR contents);
    virtual CFX_WideString	FilterContents(FX_WSTR contents);
    virtual void			SetPrintChecksum(FX_BOOL checksum);
    virtual void			SetDataLength(FX_INT32 length);
    virtual void			SetCalChecksum(FX_BOOL calc);
    virtual FX_BOOL			SetFont(CFX_Font * cFont);
    virtual void			SetFontSize(FX_FLOAT size);
    virtual void			SetFontStyle(FX_INT32 style);
    virtual void			SetFontColor(FX_ARGB color);
};
class CBC_Code39 : public CBC_OneCode
{
public:
    CBC_Code39();
    CBC_Code39(FX_BOOL usingCheckDigit);
    CBC_Code39(FX_BOOL usingCheckDigit, FX_BOOL extendedMode);
    virtual ~CBC_Code39();
    FX_BOOL 		Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e);
    FX_BOOL			RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e);
    FX_BOOL			RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e);
    CFX_WideString	Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 hight, FX_INT32 &e);
    CFX_WideString	Decode(CFX_DIBitmap *pBitmap, FX_INT32 &e);
    BC_TYPE			GetType()
    {
        return BC_CODE39;
    }
    FX_BOOL			SetTextLocation(BC_TEXT_LOC location);
    FX_BOOL			SetWideNarrowRatio(FX_INT32 ratio);
private:
    CFX_WideString  m_renderContents;
};
class CBC_Codabar : public CBC_OneCode
{
public:
    CBC_Codabar();
    virtual ~CBC_Codabar();
    FX_BOOL		    Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e);
    FX_BOOL			RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e);
    FX_BOOL			RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e);
    CFX_WideString	Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 hight, FX_INT32 &e);
    CFX_WideString	Decode(CFX_DIBitmap *pBitmap, FX_INT32 &e);
    BC_TYPE			GetType()
    {
        return BC_CODABAR;
    }
    FX_BOOL			SetStartChar(FX_CHAR start);
    FX_BOOL			SetEndChar(FX_CHAR end);
    FX_BOOL			SetTextLocation(BC_TEXT_LOC location);
    FX_BOOL			SetWideNarrowRatio(FX_INT32 ratio);
private:
    CFX_WideString  m_renderContents;
};
class CBC_Code128 : public CBC_OneCode
{
private:
    BC_TYPE  m_type;
public:
    CBC_Code128(BC_TYPE type);
    virtual ~CBC_Code128();
    FX_BOOL			Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e);
    FX_BOOL			RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e);
    FX_BOOL			RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e);
    CFX_WideString	Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 hight, FX_INT32 &e);
    CFX_WideString	Decode(CFX_DIBitmap *pBitmap, FX_INT32 &e);
    BC_TYPE			GetType()
    {
        return BC_CODE128;
    }
    FX_BOOL			SetTextLocation(BC_TEXT_LOC loction);
private:
    CFX_WideString  m_renderContents;
};
class CBC_EAN8 : public CBC_OneCode
{
public:
    CBC_EAN8();
    virtual ~CBC_EAN8();
    FX_BOOL			Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e);
    FX_BOOL			RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e);
    FX_BOOL			RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e);
    CFX_WideString	Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 hight, FX_INT32 &e);
    CFX_WideString	Decode(CFX_DIBitmap *pBitmap, FX_INT32 &e);
    BC_TYPE			GetType()
    {
        return BC_EAN8;
    }
private:
    CFX_WideString	Preprocess(FX_WSTR contents);
    CFX_WideString  m_renderContents;
};
class CBC_EAN13 : public CBC_OneCode
{
public:
    CBC_EAN13();
    virtual ~CBC_EAN13();
    FX_BOOL 		Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e);
    FX_BOOL			RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e);
    FX_BOOL			RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e);
    CFX_WideString	Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 hight, FX_INT32 &e);
    CFX_WideString	Decode(CFX_DIBitmap *pBitmap, FX_INT32 &e);
    BC_TYPE			GetType()
    {
        return BC_EAN13;
    }
private:
    CFX_WideString	Preprocess(FX_WSTR contents);
    CFX_WideString  m_renderContents;
};
class CBC_UPCA : public CBC_OneCode
{
public:
    CBC_UPCA();
    virtual ~CBC_UPCA();
    FX_BOOL 		Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e);
    FX_BOOL			RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e);
    FX_BOOL			RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e);
    CFX_WideString	Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 hight, FX_INT32 &e);
    CFX_WideString	Decode(CFX_DIBitmap *pBitmap, FX_INT32 &e);
    BC_TYPE			GetType()
    {
        return BC_UPCA;
    }
private:
    CFX_WideString	Preprocess(FX_WSTR contents);
    CFX_WideString  m_renderContents;
};
class CBC_QRCode : public CBC_CodeBase
{
public:
    CBC_QRCode();
    virtual ~CBC_QRCode();
    FX_BOOL 		Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e);
    FX_BOOL			RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e);
    FX_BOOL			RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e);
    CFX_WideString	Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 hight, FX_INT32 &e);
    CFX_WideString	Decode(CFX_DIBitmap *pBitmap, FX_INT32 &e);
    BC_TYPE		GetType()
    {
        return BC_QR_CODE;
    }
    FX_BOOL		SetVersion(FX_INT32 version);
    FX_BOOL		SetErrorCorrectionLevel (FX_INT32 level);
};
class CBC_PDF417I : public CBC_CodeBase
{
public:
    CBC_PDF417I();
    virtual ~CBC_PDF417I();
    FX_BOOL			Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e);
    FX_BOOL			RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e);
    FX_BOOL			RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e);
    CFX_WideString	Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 hight, FX_INT32 &e);
    CFX_WideString	Decode(CFX_DIBitmap *pBitmap, FX_INT32 &e);
    BC_TYPE		GetType()
    {
        return BC_PDF417;
    }
    FX_BOOL		SetErrorCorrectionLevel (FX_INT32 level);
    void		SetTruncated(FX_BOOL truncated);
};
class CBC_DataMatrix : public CBC_CodeBase
{
public:
    CBC_DataMatrix();
    virtual ~CBC_DataMatrix();
    FX_BOOL 		Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e);
    FX_BOOL			RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e);
    FX_BOOL			RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e);
    CFX_WideString	Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 hight, FX_INT32 &e);
    CFX_WideString	Decode(CFX_DIBitmap *pBitmap, FX_INT32 &e);
    BC_TYPE		GetType()
    {
        return BC_DATAMATRIX;
    }
};
#endif
