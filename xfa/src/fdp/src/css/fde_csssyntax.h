// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_CSSSYNTAX
#define _FDE_CSSSYNTAX
class CFDE_CSSTextBuf : public CFX_Target
{
public:
    CFDE_CSSTextBuf();
    ~CFDE_CSSTextBuf();
    FX_BOOL							AttachBuffer(FX_LPCWSTR pBuffer, FX_INT32 iBufLen);
    FX_BOOL							EstimateSize(FX_INT32 iAllocSize);
    FX_INT32						LoadFromStream(IFX_Stream *pTxtStream, FX_INT32 iStreamOffset, FX_INT32 iMaxChars, FX_BOOL &bEOS);
    FX_BOOL							AppendChar(FX_WCHAR wch)
    {
        if (m_iDatLen >= m_iBufLen && !ExpandBuf(m_iBufLen * 2)) {
            return FALSE;
        }
        return (m_pBuffer[m_iDatLen++] = wch), TRUE;
    }
    void							Clear()
    {
        m_iDatPos = m_iDatLen = 0;
    }
    void							Reset();
    FX_INT32						TrimEnd()
    {
        while (m_iDatLen > 0 && m_pBuffer[m_iDatLen - 1] <= ' ') {
            --m_iDatLen;
        }
        AppendChar(0);
        return --m_iDatLen;
    }
    void							Subtract(FX_INT32 iStart, FX_INT32 iLength);
    FX_BOOL							IsEOF() const
    {
        return m_iDatPos >= m_iDatLen;
    }
    FX_WCHAR						GetAt(FX_INT32 index) const
    {
        return m_pBuffer[index];
    }
    FX_WCHAR						GetChar() const
    {
        return m_pBuffer[m_iDatPos];
    }
    FX_WCHAR						GetNextChar() const
    {
        return (m_iDatPos + 1 >= m_iDatLen) ? 0 : m_pBuffer[m_iDatPos + 1];
    }
    void							MoveNext()
    {
        m_iDatPos++;
    }
    FX_INT32						GetLength() const
    {
        return m_iDatLen;
    }
    FX_LPCWSTR						GetBuffer() const
    {
        return m_pBuffer;
    }
protected:
    FX_BOOL							ExpandBuf(FX_INT32 iDesiredSize);
    FX_BOOL							m_bExtBuf;
    FX_LPWSTR						m_pBuffer;
    FX_INT32						m_iBufLen;
    FX_INT32						m_iDatLen;
    FX_INT32						m_iDatPos;
};
#define FDE_CSSSYNTAXCHECK_AllowCharset	1
#define FDE_CSSSYNTAXCHECK_AllowImport	2
enum FDE_CSSSYNTAXMODE {
    FDE_CSSSYNTAXMODE_RuleSet,
    FDE_CSSSYNTAXMODE_Comment,
    FDE_CSSSYNTAXMODE_AtRule,
    FDE_CSSSYNTAXMODE_UnknownRule,
    FDE_CSSSYNTAXMODE_Charset,
    FDE_CSSSYNTAXMODE_Import,
    FDE_CSSSYNTAXMODE_MediaRule,
    FDE_CSSSYNTAXMODE_URI,
    FDE_CSSSYNTAXMODE_MediaType,
    FDE_CSSSYNTAXMODE_Selector,
    FDE_CSSSYNTAXMODE_PropertyName,
    FDE_CSSSYNTAXMODE_PropertyValue,
};
class CFDE_CSSSyntaxParser : public IFDE_CSSSyntaxParser, public CFX_Target
{
public:
    CFDE_CSSSyntaxParser();
    ~CFDE_CSSSyntaxParser();
    virtual void					Release()
    {
        FDE_Delete this;
    }
    virtual FX_BOOL					Init(IFX_Stream *pStream, FX_INT32 iCSSPlaneSize, FX_INT32 iTextDataSize = 32, FX_BOOL bOnlyDeclaration = FALSE);
    virtual FX_BOOL					Init(FX_LPCWSTR pBuffer, FX_INT32 iBufferSize, FX_INT32 iTextDatSize = 32, FX_BOOL bOnlyDeclaration = FALSE);
    virtual FDE_CSSSYNTAXSTATUS		DoSyntaxParse();
    virtual FX_LPCWSTR				GetCurrentString(FX_INT32 &iLength) const;
protected:
    void							Reset(FX_BOOL bOnlyDeclaration);
    void							SwitchMode(FDE_CSSSYNTAXMODE eMode);
    FX_INT32						SwitchToComment();

    FX_BOOL							RestoreMode();
    FX_BOOL							AppendChar(FX_WCHAR wch);
    FX_INT32						SaveTextData();
    FX_BOOL							IsCharsetEnabled() const
    {
        return (m_dwCheck & FDE_CSSSYNTAXCHECK_AllowCharset) != 0;
    }
    void							DisableCharset()
    {
        m_dwCheck = FDE_CSSSYNTAXCHECK_AllowImport;
    }
    FX_BOOL							IsImportEnabled() const;
    void							DisableImport()
    {
        m_dwCheck = 0;
    }
    IFX_Stream						*m_pStream;
    FX_INT32						m_iStreamPos;
    FX_INT32						m_iPlaneSize;
    CFDE_CSSTextBuf					m_TextData;
    CFDE_CSSTextBuf					m_TextPlane;
    FX_INT32						m_iTextDatLen;
    FX_DWORD						m_dwCheck;
    FDE_CSSSYNTAXMODE				m_eMode;
    FDE_CSSSYNTAXSTATUS				m_eStatus;
    CFX_StackTemplate<FDE_CSSSYNTAXMODE> m_ModeStack;
};
#endif
