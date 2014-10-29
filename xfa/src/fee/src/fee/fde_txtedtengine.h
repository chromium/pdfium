// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_TXTEDTENGINE_H
#define _FDE_TXTEDTENGINE_H
class CFDE_TxtEdtBuf;
class CFDE_TxtEdtPage;
class IFX_TxtBreak;
class CFDE_TxtEdtParag;
class CFDE_TxtEdtKMPMatch;
class IFDE_TxtEdtFormator;
class CFDE_TxtEdtField;
class CFDE_TxtEdtBlock;
class IFX_CharIter;
class CFDE_TxtEdtEngine;
class CFDE_TxtEdtDoRecord_Insert;
class CFDE_TxtEdtDoRecord_DeleteRange;
#ifdef FDE_USEFORMATBLOCK
class CFDE_TxtEdtDoRecord_FormatInsert;
class CFDE_TxtEdtDoRecord_FormatDelete;
class CFDE_TxtEdtDoRecord_FormatReplace;
class CFDE_TxtEdtDoRecord_FieldInsert;
class CFDE_TxtEdtDoRecord_FieldDelete;
class CFDE_TxtEdtDoRecord_FieldReplace;
#endif
class IFDE_TxtEdtDoRecord
{
public:
    static IFDE_TxtEdtDoRecord * Create(FX_BSTR bsDoRecord);
    virtual void	Release() = 0;
    virtual FX_BOOL Redo() = 0;
    virtual FX_BOOL Undo() = 0;
    virtual void	Serialize(CFX_ByteString &bsDoRecord) const = 0;
};
class CFDE_TxtEdtEngine : public IFDE_TxtEdtEngine, public CFX_Object
{
    friend class CFDE_TxtEdtDoRecord_Insert;
    friend class CFDE_TxtEdtDoRecord_DeleteRange;
    friend class CFDE_TxtEdtPage;
#ifdef FDE_USEFORMATBLOCK
    friend class CFDE_TxtEdtDoRecord_FormatInsert;
    friend class CFDE_TxtEdtDoRecord_FormatDelete;
    friend class CFDE_TxtEdtDoRecord_FormatReplace;
    friend class CFDE_TxtEdtBlock;
#endif
    struct _FDE_TXTEDTSELRANGE : public CFX_Object {
        FX_INT32 nStart;
        FX_INT32 nCount;
    };
    typedef _FDE_TXTEDTSELRANGE		FDE_TXTEDTSELRANGE;
    typedef _FDE_TXTEDTSELRANGE *	FDE_LPTXTEDTSELRANGE;
    struct _FDE_TXTEDTPARAGPOS : public CFX_Object {
        FX_INT32	nParagIndex;
        FX_INT32	nCharIndex;
    };
    typedef _FDE_TXTEDTPARAGPOS		FDE_TXTEDTPARAGPOS;
    typedef _FDE_TXTEDTPARAGPOS *	FDE_LPTXTEDTPARAGPOS;
public:
    CFDE_TxtEdtEngine();
    virtual void						Release();

    virtual void						SetEditParams(const FDE_TXTEDTPARAMS &params);
    virtual const FDE_TXTEDTPARAMS*		GetEditParams() const;

    virtual FX_INT32					CountPages() const;
    virtual IFDE_TxtEdtPage*			GetPage(FX_INT32 nIndex);

    virtual FX_BOOL		SetBufChunkSize(FX_INT32 nChunkSize);
    virtual void		SetTextByStream(IFX_Stream *pStream);
    virtual void		SetText(const CFX_WideString &wsText);
    virtual FX_INT32	GetTextLength() const;
    virtual void		GetText(CFX_WideString &wsText, FX_INT32 nStart, FX_INT32 nCount = -1);
    virtual void		ClearText();

    virtual FX_INT32	GetCaretRect(CFX_RectF &rtCaret) const;
    virtual FX_INT32	GetCaretPos() const;
    virtual FX_INT32	SetCaretPos(FX_INT32 nIndex, FX_BOOL bBefore);
    virtual FX_INT32	MoveCaretPos(FDE_TXTEDTMOVECARET eMoveCaret, FX_BOOL bShift = FALSE, FX_BOOL bCtrl = FALSE);
    virtual void	Lock();
    virtual void	Unlock();
    virtual FX_BOOL	IsLocked() const;

    virtual FX_INT32	Insert(FX_INT32 nStart, FX_LPCWSTR lpText, FX_INT32 nLength);
    virtual FX_INT32	Delete(FX_INT32 nStart, FX_BOOL bBackspace = FALSE);
    virtual FX_INT32	DeleteRange(FX_INT32 nStart, FX_INT32 nCount = -1);
    virtual FX_INT32	Replace(FX_INT32 nStart, FX_INT32 nLength, const CFX_WideString &wsReplace);

    virtual	void		SetLimit(FX_INT32 nLimit);
    virtual	void		SetAliasChar(FX_WCHAR wcAlias);
    virtual void		SetFormatBlock(FX_INT32 nIndex, const CFX_WideString &wsBlockFormat);
    virtual FX_INT32	CountEditBlocks() const;
    virtual void		GetEditBlockText(FX_INT32 nIndex, CFX_WideString &wsBlockText) const;
    virtual FX_INT32	CountEditFields(FX_INT32 nBlockIndex) const;
    virtual void		GetEditFieldText(FX_INT32 nBlockIndex, FX_INT32 nFieldIndex, CFX_WideString &wsFieldText) const;
    virtual void		StartEdit();
    virtual void		EndEdit();

    void				RemoveSelRange(FX_INT32 nStart, FX_INT32 nCount = -1);

    virtual void		AddSelRange(FX_INT32 nStart, FX_INT32 nCount = -1);
    virtual FX_INT32	CountSelRanges();
    virtual FX_INT32	GetSelRange(FX_INT32 nIndex, FX_INT32 &nStart);
    virtual void		ClearSelection();

    virtual FX_BOOL		Redo(FX_BSTR bsRedo);
    virtual FX_BOOL		Undo(FX_BSTR bsUndo);

    virtual FX_INT32	StartLayout();
    virtual FX_INT32	DoLayout(IFX_Pause *pPause);
    virtual void		EndLayout();

    virtual FX_BOOL		Optimize(IFX_Pause * pPause = NULL);
    virtual FX_INT32			CountParags() const;
    virtual IFDE_TxtEdtParag*	GetParag(FX_INT32 nParagIndex) const;
    virtual IFX_CharIter*		CreateCharIter();
    IFDE_TxtEdtBuf*	GetTextBuf() const;
    FX_INT32		GetTextBufLength() const;
    IFX_TxtBreak*	GetTextBreak() const;
    FX_INT32	GetLineCount() const;
    FX_INT32	GetPageLineCount() const;

    FX_INT32			Line2Parag(FX_INT32 nStartParag, FX_INT32 nStartLineofParag,
                                   FX_INT32 nLineIndex, FX_INT32 &nStartLine) const;
    FX_WCHAR			GetAliasChar() const
    {
        return m_wcAliasChar;
    }

protected:
    virtual ~CFDE_TxtEdtEngine();
private:
    void	Inner_Insert(FX_INT32 nStart, FX_LPCWSTR lpText, FX_INT32 nLength);
#ifdef FDE_USEFORMATBLOCK
    void	RawInsert(FX_INT32 nStart, FX_LPCWSTR lpText, FX_INT32 nLength);
#endif
    void	GetPreDeleteText(CFX_WideString &wsText, FX_INT32 nIndex, FX_INT32 nLength);
    void	GetPreInsertText(CFX_WideString &wsText, FX_INT32 nIndex, FX_LPCWSTR lpText, FX_INT32 nLength);
    void	GetPreReplaceText(CFX_WideString &wsText, FX_INT32 nIndex, FX_INT32 nOriginLength, FX_LPCWSTR lpText, FX_INT32 nLength);

    void	Inner_DeleteRange(FX_INT32 nStart, FX_INT32 nCount = -1);
    void	DeleteRange_DoRecord(FX_INT32 nStart, FX_INT32 nCount, FX_BOOL bSel = FALSE);
    void	ResetEngine();
    void	RebuildParagraphs();
    void	RemoveAllParags();
    void	RemoveAllPages();
    void	UpdateParags();
    void	UpdatePages();
    void	UpdateTxtBreak();

    FX_BOOL	ReplaceParagEnd(FX_LPWSTR &lpText, FX_INT32 &nLength, FX_BOOL bPreIsCR = FALSE);
    void	RecoverParagEnd(CFX_WideString &wsText);
    FX_INT32	MovePage2Char(FX_INT32 nIndex);
    void		TextPos2ParagPos(FX_INT32 nIndex, FDE_TXTEDTPARAGPOS &ParagPos) const;
    FX_INT32	MoveForward(FX_BOOL &bBefore);
    FX_INT32	MoveBackward(FX_BOOL &bBefore);
    FX_BOOL		MoveUp(CFX_PointF &ptCaret);
    FX_BOOL		MoveDown(CFX_PointF &ptCaret);
    FX_BOOL		MoveLineStart();
    FX_BOOL		MoveLineEnd();
    FX_BOOL		MoveParagStart();
    FX_BOOL		MoveParagEnd();
    FX_BOOL		MoveHome();
    FX_BOOL		MoveEnd();
    FX_BOOL		IsFitArea(CFX_WideString &wsText);
    void	UpdateCaretRect(FX_INT32 nIndex, FX_BOOL bBefore = TRUE);
    void	GetCaretRect(CFX_RectF &rtCaret, FX_INT32 nPageIndex, FX_INT32 nCaret, FX_BOOL bBefore = TRUE);
    void	UpdateCaretIndex(const CFX_PointF & ptCaret);

    FX_BOOL	IsSelect();
    void	DeleteSelect();
    IFDE_TxtEdtBuf*		m_pTxtBuf;
    IFX_TxtBreak*		m_pTextBreak;
    FDE_TXTEDTPARAMS	m_Param;
    CFX_ArrayTemplate<IFDE_TxtEdtPage*>			m_PagePtrArray;
    CFX_ArrayTemplate<CFDE_TxtEdtParag*>		m_ParagPtrArray;
    CFX_ArrayTemplate<FDE_LPTXTEDTSELRANGE>		m_SelRangePtrArr;
    FX_INT32	m_nPageLineCount;
    FX_INT32	m_nLineCount;
    FX_INT32	m_nAnchorPos;
    FX_INT32	m_nLayoutPos;
    FX_FLOAT	m_fCaretPosReserve;
    FX_INT32	m_nCaret;
    FX_BOOL		m_bBefore;
    FX_INT32	m_nCaretPage;
    CFX_RectF	m_rtCaret;
    FX_DWORD	m_dwFindFlags;

    FX_BOOL		m_bLock;
    FX_INT32	m_nLimit;
    FX_WCHAR	m_wcAliasChar;
    FX_INT32	m_nFirstLineEnd;
    FX_BOOL		m_bAutoLineEnd;
    FX_WCHAR	m_wLineEnd;

    FDE_TXTEDT_TEXTCHANGE_INFO m_ChangeInfo;
};
class CFDE_TxtEdtDoRecord_Insert : public IFDE_TxtEdtDoRecord, public CFX_Object
{
public:
    CFDE_TxtEdtDoRecord_Insert(FX_BSTR bsDoRecord);
    CFDE_TxtEdtDoRecord_Insert(	CFDE_TxtEdtEngine * pEngine,
                                FX_INT32 nCaret,
                                FX_LPCWSTR lpText,
                                FX_INT32 nLength);
    virtual void	Release();
    virtual FX_BOOL Undo();
    virtual FX_BOOL Redo();
    virtual void	Serialize(CFX_ByteString &bsDoRecord) const;
protected:
    ~CFDE_TxtEdtDoRecord_Insert();
    void			Deserialize(FX_BSTR bsDoRecord);
private:
    CFDE_TxtEdtEngine *	m_pEngine;
    FX_INT32			m_nCaret;
    CFX_WideString		m_wsInsert;
};
class CFDE_TxtEdtDoRecord_DeleteRange : public IFDE_TxtEdtDoRecord, public CFX_Object
{
public:
    CFDE_TxtEdtDoRecord_DeleteRange(FX_BSTR bsDoRecord);
    CFDE_TxtEdtDoRecord_DeleteRange(CFDE_TxtEdtEngine * pEngine,
                                    FX_INT32 nIndex,
                                    FX_INT32 nCaret,
                                    const CFX_WideString &wsRange,
                                    FX_BOOL bSel = FALSE);
    virtual	void	Release();
    virtual FX_BOOL Undo();
    virtual FX_BOOL Redo();
    virtual void	Serialize(CFX_ByteString &bsDoRecord) const;
protected:
    ~CFDE_TxtEdtDoRecord_DeleteRange();
    void			Deserialize(FX_BSTR bsDoRecord);
private:
    CFDE_TxtEdtEngine *	m_pEngine;
    FX_BOOL				m_bSel;
    FX_INT32			m_nIndex;
    FX_INT32			m_nCaret;
    CFX_WideString		m_wsRange;
};
#ifdef FDE_USEFORMATBLOCK
class CFDE_TxtEdtDoRecord_FieldInsert : public IFDE_TxtEdtDoRecord, public CFX_Object
{
public:
    CFDE_TxtEdtDoRecord_FieldInsert(FX_BSTR bsDoRecord);
    CFDE_TxtEdtDoRecord_FieldInsert(CFDE_TxtEdtEngine * pEngine,
                                    FX_INT32 nCaret,
                                    CFDE_TxtEdtField * pField,
                                    FX_INT32 nIndexInField,
                                    FX_INT32 nFieldBgn,
                                    FX_INT32 nOldFieldLength,
                                    FX_INT32 nNewFieldLength,
                                    const CFX_WideString &wsIns,
                                    FX_BOOL bSel = FALSE);
    virtual void	Release();
    virtual FX_BOOL	Undo();
    virtual FX_BOOL	Redo();
    virtual void	Serialize(CFX_ByteString &bsDoRecord) const;
protected:
    ~CFDE_TxtEdtDoRecord_FieldInsert();
    void	Deserialize(FX_BSTR bsDoRecord);

private:
    CFDE_TxtEdtEngine *	m_pEngine;
    FX_INT32			m_nCaret;
    CFDE_TxtEdtField *	m_pField;
    FX_INT32			m_nIndexInField;
    FX_INT32			m_nFieldBgn;
    FX_INT32			m_nOldFieldLength;
    FX_INT32			m_nNewFieldLength;
    CFX_WideString		m_wsIns;
    FX_BOOL				m_bSel;
};
class CFDE_TxtEdtDoRecord_FieldDelete : public IFDE_TxtEdtDoRecord, public CFX_Object
{
public:
    CFDE_TxtEdtDoRecord_FieldDelete(FX_BSTR bsDoRecord);
    CFDE_TxtEdtDoRecord_FieldDelete(CFDE_TxtEdtEngine * pEngine,
                                    FX_INT32 nCaret,
                                    CFDE_TxtEdtField * pField,
                                    FX_INT32 nIndexInField,
                                    FX_INT32 nFieldBgn,
                                    FX_INT32 nOldLength,
                                    FX_INT32 nNewLength,
                                    const CFX_WideString &wsDel,
                                    FX_BOOL bSel = FALSE);
    virtual void	Release();
    virtual FX_BOOL	Undo();
    virtual FX_BOOL	Redo();
    virtual void	Serialize(CFX_ByteString &bsDoRecord) const;
protected:
    ~CFDE_TxtEdtDoRecord_FieldDelete();
    void	Deserialize(FX_BSTR bsDoRecord);
private:
    CFDE_TxtEdtEngine * m_pEngine;
    FX_INT32			m_nCaret;
    CFDE_TxtEdtField *	m_pField;
    FX_INT32			m_nIndexInField;
    FX_INT32			m_nFieldBgn;
    FX_INT32			m_nOldFieldLength;
    FX_INT32			m_nNewFieldLength;
    CFX_WideString		m_wsDel;
    FX_BOOL				m_bSel;
};
class CFDE_TxtEdtDoRecord_FieldReplace : public IFDE_TxtEdtDoRecord, public CFX_Object
{
public:
    CFDE_TxtEdtDoRecord_FieldReplace(FX_BSTR bsDoRecord);
    CFDE_TxtEdtDoRecord_FieldReplace(	CFDE_TxtEdtEngine * pEngine,
                                        FX_INT32 nCaret,
                                        FX_INT32 nNewCaret,
                                        CFDE_TxtEdtField * pField,
                                        FX_INT32	nIndexInField,
                                        FX_INT32	nFieldBgn,
                                        FX_INT32	nFieldNewLength,
                                        const CFX_WideString &wsDel,
                                        const CFX_WideString &wsIns,
                                        FX_BOOL	bSel);
    virtual void	Release();
    virtual FX_BOOL	Undo();
    virtual FX_BOOL	Redo();
    virtual void	Serialize(CFX_ByteString &bsDoRecord) const;
protected:
    ~CFDE_TxtEdtDoRecord_FieldReplace();
    void	Deserialize(FX_BSTR bsDoRecord);
private:
    CFDE_TxtEdtEngine * m_pEngine;
    FX_INT32			m_nCaret;
    FX_INT32			m_nNewCaret;
    CFDE_TxtEdtField *	m_pField;
    FX_INT32			m_nIndexInField;
    FX_INT32			m_nFieldBgn;
    FX_INT32			m_nFieldNewLength;
    CFX_WideString		m_wsDel;
    CFX_WideString		m_wsIns;
    FX_BOOL				m_bSel;
};
#endif
#endif
