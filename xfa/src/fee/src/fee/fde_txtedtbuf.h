// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_TXTEDTBUF_H
#define _FDE_TXTEDTBUF_H
class IFX_CharIter;
class CFDE_TxtEdtBufIter;
class CFDE_TxtEdtBuf;
class CFDE_TxtEdtBufIter : public IFX_CharIter, public CFX_Object
{
public:
#ifdef FDE_USEFORMATBLOCK
    CFDE_TxtEdtBufIter(CFDE_TxtEdtBuf * pBuf, FX_BOOL bForDisplay = TRUE);
#else
    CFDE_TxtEdtBufIter(CFDE_TxtEdtBuf * pBuf, FX_WCHAR wcAlias = 0);
#endif

    virtual void		Release();
    virtual FX_BOOL		Next(FX_BOOL bPrev = FALSE);
    virtual FX_WCHAR	GetChar();
    virtual void		SetAt(FX_INT32 nIndex);
    virtual FX_INT32	GetAt() const;
    virtual FX_BOOL		IsEOF(FX_BOOL bTail = TRUE) const;
    virtual IFX_CharIter * Clone();
protected:
    ~CFDE_TxtEdtBufIter();
private:
    CFDE_TxtEdtBuf*	m_pBuf;
    FX_INT32		m_nCurChunk;
    FX_INT32		m_nCurIndex;
    FX_INT32		m_nIndex;
    FX_BOOL			m_bInField;
#ifdef FDE_USEFORMATBLOCK
    FX_BOOL			m_bForDisplay;
    FX_INT32		m_nAliasCount;
#endif
    FX_WCHAR		m_Alias;
};
class CFDE_TxtEdtBuf : public IFDE_TxtEdtBuf, public CFX_Object
{
    friend class CFDE_TxtEdtBufIter;
    struct _FDE_CHUNKHEADER : public CFX_Object {
        FX_INT32	nUsed;
        FX_WCHAR	wChars[1];
    };
    typedef _FDE_CHUNKHEADER	FDE_CHUNKHEADER;
    typedef _FDE_CHUNKHEADER*	FDE_LPCHUNKHEADER;
    struct _FDE_CHUNKPLACE : public CFX_Object {
        FX_INT32	nChunkIndex;
        FX_INT32	nCharIndex;
    };
    typedef _FDE_CHUNKPLACE		FDE_CHUNKPLACE;
    typedef _FDE_CHUNKPLACE*	FDE_LPCHUNKPLACE;

public:
    CFDE_TxtEdtBuf(FX_INT32 nDefChunkSize = FDE_DEFCHUNKLENGTH);

    virtual	void		Release();
    virtual FX_BOOL		SetChunkSize(FX_INT32 nChunkSize);
    virtual FX_INT32	GetChunkSize() const;
    virtual	FX_INT32	GetTextLength() const;
    virtual void		SetText(const CFX_WideString &wsText);
    virtual void		GetText(CFX_WideString &wsText) const;
    virtual FX_WCHAR	GetCharByIndex(FX_INT32 nIndex) const;
    virtual void		GetRange(CFX_WideString &wsText, FX_INT32 nBegine, FX_INT32 nCount = -1) const;

    virtual void		Insert(FX_INT32 nPos, FX_LPCWSTR lpText, FX_INT32 nLength = 1);
    virtual void		Delete(FX_INT32 nIndex, FX_INT32 nLength = 1);
    virtual void		Clear(FX_BOOL bRelease = TRUE);

    virtual FX_BOOL		Optimize(IFX_Pause * pPause = NULL);

protected:
    virtual ~CFDE_TxtEdtBuf();
private:
    void		ResetChunkBuffer(FX_INT32 nDefChunkCount, FX_INT32 nChunkSize);
    FX_INT32	CP2Index(const FDE_CHUNKPLACE & cp) const;
    void		Index2CP(FX_INT32 nIndex, FDE_CHUNKPLACE & cp) const;

    FX_INT32			m_nChunkSize;

    FX_INT32			m_nTotal;
    FX_BOOL				m_bChanged;
    CFX_PtrArray		m_Chunks;
    IFX_MEMAllocator *	m_pAllocator;
};
#endif
