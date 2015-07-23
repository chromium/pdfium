// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFAPI_FPDF_PARSER_FILTERS_INT_H_
#define CORE_SRC_FPDFAPI_FPDF_PARSER_FILTERS_INT_H_

class CPDF_DecryptFilter : public CFX_DataFilter
{
public:
    CPDF_DecryptFilter(CPDF_CryptoHandler* pCryptoHandler, FX_DWORD objnum, FX_DWORD gennum);
    virtual ~CPDF_DecryptFilter();
    virtual	void	v_FilterIn(const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf);
    virtual void	v_FilterFinish(CFX_BinaryBuf& dest_buf);
    CPDF_CryptoHandler*	m_pCryptoHandler;
    void*		m_pContext;
    FX_DWORD		m_ObjNum, m_GenNum;
};
class CPDF_FlateFilter : public CFX_DataFilter
{
public:
    CPDF_FlateFilter();
    virtual ~CPDF_FlateFilter();
    virtual	void	v_FilterIn(const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf);
    virtual void	v_FilterFinish(CFX_BinaryBuf& dest_buf) {}
    void*			m_pContext;
    uint8_t			m_DestBuffer[FPDF_FILTER_BUFFER_SIZE];
};
class CPDF_LzwFilter : public CFX_DataFilter
{
public:
    CPDF_LzwFilter(FX_BOOL bEarlyChange);
    virtual ~CPDF_LzwFilter() {}
    virtual	void	v_FilterIn(const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf);
    virtual void	v_FilterFinish(CFX_BinaryBuf& dest_buf) {}
    FX_BOOL			m_bEarlyChange;
    FX_DWORD		m_CodeArray[5021];
    FX_DWORD		m_nCodes;
    FX_DWORD		m_CodeLen;
    FX_DWORD		m_OldCode;
    uint8_t			m_LastChar;
    FX_DWORD		m_nLeftBits, m_LeftBits;
    uint8_t			m_DecodeStack[4000];
    FX_DWORD		m_StackLen;
    void			AddCode(FX_DWORD prefix_code, uint8_t append_char);
    void			DecodeString(FX_DWORD code);
};
class CPDF_PredictorFilter : public CFX_DataFilter
{
public:
    CPDF_PredictorFilter(int predictor, int colors, int bpc, int cols);
    virtual ~CPDF_PredictorFilter();
    virtual	void	v_FilterIn(const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf);
    virtual void	v_FilterFinish(CFX_BinaryBuf& dest_buf) {}
    FX_BOOL			m_bTiff;
    FX_DWORD		m_Pitch, m_Bpp;
    uint8_t*		m_pRefLine;
    uint8_t*		m_pCurLine;
    FX_DWORD		m_iLine, m_LineInSize;
};
class CPDF_AsciiHexFilter : public CFX_DataFilter
{
public:
    CPDF_AsciiHexFilter();
    virtual ~CPDF_AsciiHexFilter() {}
    virtual	void	v_FilterIn(const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf);
    virtual void	v_FilterFinish(CFX_BinaryBuf& dest_buf) {}
    int				m_State;
    int				m_FirstDigit;
};
class CPDF_Ascii85Filter : public CFX_DataFilter
{
public:
    CPDF_Ascii85Filter();
    virtual ~CPDF_Ascii85Filter() {}
    virtual	void	v_FilterIn(const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf);
    virtual void	v_FilterFinish(CFX_BinaryBuf& dest_buf) {}
    int				m_State;
    int				m_CharCount;
    FX_DWORD		m_CurDWord;
};
class CPDF_RunLenFilter : public CFX_DataFilter
{
public:
    CPDF_RunLenFilter();
    virtual ~CPDF_RunLenFilter() {}
    virtual	void	v_FilterIn(const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf);
    virtual void	v_FilterFinish(CFX_BinaryBuf& dest_buf) {}
    int				m_State;
    FX_DWORD		m_Count;
};
class CPDF_JpegFilter : public CFX_DataFilter
{
public:
    CPDF_JpegFilter();
    virtual ~CPDF_JpegFilter();
    virtual	void	v_FilterIn(const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf);
    virtual void	v_FilterFinish(CFX_BinaryBuf& dest_buf) {}
    void*			m_pContext;
    CFX_BinaryBuf	m_InputBuf;
    uint8_t*		m_pScanline;
    int				m_Pitch, m_Height, m_Width, m_nComps, m_iLine;
    FX_BOOL			m_bGotHeader;
};
class CPDF_FaxFilter : public CFX_DataFilter
{
public:
    CPDF_FaxFilter();
    virtual ~CPDF_FaxFilter();
    FX_BOOL			Initialize(int Encoding, int bEndOfLine, int bByteAlign, int bBlack, int nRows, int nColumns);
    virtual	void	v_FilterIn(const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf);
    virtual void	v_FilterFinish(CFX_BinaryBuf& dest_buf);
    int				m_Encoding, m_bEndOfLine, m_bByteAlign, m_bBlack;
    int				m_nRows, m_nColumns, m_Pitch, m_iRow;
    uint8_t*		m_pScanlineBuf;
	uint8_t*		m_pRefBuf;
    CFX_BinaryBuf	m_InputBuf;
    int				m_InputBitPos;
    void			ProcessData(const uint8_t* src_buf, FX_DWORD src_size, int& bitpos, FX_BOOL bFinish,
                                CFX_BinaryBuf& dest_buf);
    FX_BOOL			ReadLine(const uint8_t* src_buf, int bitsize, int& bitpos);
};

#endif  // CORE_SRC_FPDFAPI_FPDF_PARSER_FILTERS_INT_H_
