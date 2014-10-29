// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_TXTEDTBLOCK_H
#define _FDE_TXTEDTBLOCK_H
#ifdef FDE_USEFORMATBLOCK
class CFDE_TxtEdtEngine;
class CFDE_TxtEdtBlock;
class CFDE_TxtEdtFieldFormatParser;
class CFDE_TxtEdtField;
class CFDE_TxtEdtField_Integer;
class CFDE_TxtEdtField_Float;
class CFDE_TxtEdtField_Password;
class CFDE_TxtEdtField_String;
class CFDE_TxtEdtField_Fixed;
#define FDE_FORMAT_EDIT_FIELD_HADERSIZE 3
#define FDE_FORMAT_EDIT_FIELD_TAILSIZE	1
enum FDE_FORMAT_FIELD_INSERT_RET {
    FDE_FORMAT_FIELD_INSERT_RET_S_NORMAL = 0,
    FDE_FORMAT_FIELD_INSERT_RET_S_FULL		,
    FDE_FORMAT_FIELD_INSERT_RET_F_FULL		,
    FDE_FORMAT_FIELD_INSERT_RET_F_INVALIDATE,
};
enum FDE_FORMAT_FIELD_DELETE_RET {
    FDE_FORMAT_FIELD_DELETE_RET_S			= 0	,
    FDE_FORMAT_FIELD_DELETE_RET_F_INVALIDATE	,
    FDE_FORMAT_FIELD_DELETE_RET_F_BOUNDARY		,
};
enum FDE_FORMAT_FIELD_VALIDATE_RET {
    FDE_FORMAT_FIELD_VALIDATE_S			= 0	,
    FDE_FORMAT_FIELD_VALIDATE_F_FULL		,
    FDE_FORMAT_FIELD_VALIDATE_F_INVALIDATE	,
};
enum FDE_FORMAT_CARET_DIRECTION {
    FDE_FORMAT_CARET_FORWARD,
    FDE_FORMAT_CARET_MIDDLE,
    FDE_FORMAT_CARET_BACKWARD
};
class CFDE_TxtEdtBlock : public CFX_Object
{
public:
    CFDE_TxtEdtBlock(CFDE_TxtEdtEngine * pEngine, const CFX_WideString &wsBlock, FX_INT32 nPosition);
    ~CFDE_TxtEdtBlock();
    void		GetDisplayText(CFX_WideString &wsDisplay);
    FX_INT32	GetLength() const;
    void		GetBlockText(CFX_WideString &wsBlock);
    FX_INT32	CountField() const;
    void		GetFieldText(FX_INT32 nIndex, CFX_WideString &wsField);
    FX_INT32	GetFieldTextLength() const;

    FX_INT32	GetPos() const;
    void		GetRealText(CFX_WideString &wsText) const;
    void		Backup();
    void		Restore();
    void		SetIndex(FX_INT32 nIndex)
    {
        m_nIndex = nIndex;
    }
    FX_INT32	GetIndex() const
    {
        return m_nIndex;
    }
private:
    CFDE_TxtEdtEngine * m_pEngine;
    FX_INT32 m_nDisplayLength;
    FX_INT32 m_nIndex;

    FX_INT32	m_nPosition;
    CFX_ArrayTemplate<CFDE_TxtEdtField*> m_FieldArr;
    CFX_ArrayTemplate<CFDE_TxtEdtField*> m_EditFieldArr;
};
class CFDE_TxtEdtFieldFormatParser : public CFX_Object
{
public:
    CFDE_TxtEdtFieldFormatParser();
    ~CFDE_TxtEdtFieldFormatParser();
    FX_BOOL		Parse(const CFX_WideString &wsFormat);
    FX_INT32	CountItems() const;
    void		GetItem(FX_INT32 nIndex, CFX_WideString &wsKey, CFX_WideString &wsValue) const;
private:
    typedef struct : public CFX_Object {
        FX_INT32 nKeyStart;
        FX_INT32 nKeyCount;
        FX_INT32 nValStart;
        FX_INT32 nValCount;
    } FDE_TXTEDTFORMATITEM, * FDE_LPTXTEDTFORMATITEM;

    CFX_WideString m_wsFormat;
    CFX_ArrayTemplate<FDE_LPTXTEDTFORMATITEM> m_ItemArr;
};
class CFDE_TxtEdtField : public CFX_Object
{
public:
    static CFDE_TxtEdtField * Create(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock);
    virtual void		Release();
    virtual FX_INT32	Insert(	FX_INT32 nIndex, const CFX_WideString &wsIns,
                                FX_INT32 &nCaret, FX_BOOL &bBefore);
    virtual FX_INT32	Delete(	FX_INT32 nIndex, FX_INT32 nCount,
                                CFX_WideString &wsDel, FX_INT32 &nCaret,
                                FX_BOOL &bBefore);
    virtual FX_INT32	Replace(FX_INT32 nIndex, FX_INT32 nCount,
                                const CFX_WideString &wsIns, CFX_WideString &wsDel,
                                FX_INT32 &nCaret, FX_BOOL &bBefore);
    virtual void		GetDisplayText(CFX_WideString &wsDisplay);
    virtual FX_INT32	GetDisplayLength();
    virtual void		GetFieldText(CFX_WideString &wsField);
    virtual FX_INT32	GetFieldTextLength() const;
    virtual FX_INT32	GetRealIndex(FX_INT32 nIndex) const;


    virtual FX_INT32	NormalizeCaretPos(	FX_INT32 nIndex,
                                            FDE_FORMAT_CARET_DIRECTION eDirection = FDE_FORMAT_CARET_MIDDLE) const;

    virtual FX_BOOL		GetEditableRange(FX_INT32 &nBgn, FX_INT32 &nEnd) const;
    virtual void		Backup();
    virtual void		Restore();
    virtual FX_BOOL		IsFix() const
    {
        return FALSE;
    }
    void				SetIndex(FX_INT32 nIndex)
    {
        m_nIndex = nIndex;
    }
    FX_INT32			GetIndex() const
    {
        return m_nIndex;
    }
    FX_INT32			GetBlockIndex() const
    {
        return m_pBlock->GetIndex();
    }
protected:
    CFDE_TxtEdtField(FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock);
    virtual				~CFDE_TxtEdtField() {}
    virtual FX_INT32	Validate(const CFX_WideString & wsText) const;
    virtual void		GetNormalizedFieldText(CFX_WideString &wsField) const;
    FX_INT32		m_nLength;
    CFX_WideString	m_wsField;
    CFX_WideString	m_wsBackup;
    FX_WCHAR		m_wcFill;
    FX_BOOL				m_bReserveSpace;
    FX_BOOL				m_bLeftAlignment;
    FX_INT32			m_nIndex;
    CFDE_TxtEdtBlock *	m_pBlock;
};
class CFDE_TxtEdtField_Integer : public CFDE_TxtEdtField
{
public:
    CFDE_TxtEdtField_Integer(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock);
protected:
    virtual ~CFDE_TxtEdtField_Integer() {}
    virtual FX_INT32	Validate(const CFX_WideString &wsText) const;
private:
    FX_BOOL	m_bSign;
};
class CFDE_TxtEdtField_Float : public CFDE_TxtEdtField
{
public:
    CFDE_TxtEdtField_Float(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock);
protected:
    virtual ~CFDE_TxtEdtField_Float() {}
    virtual FX_INT32	Validate(const CFX_WideString & wsText) const;
private:
    FX_BOOL		m_bSigned;
    FX_INT32	m_nIntPartlength;
    FX_INT32	m_nDecPartLength;
};
class CFDE_TxtEdtField_Password : public CFDE_TxtEdtField
{
public:
    CFDE_TxtEdtField_Password(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock);

protected:
    virtual ~CFDE_TxtEdtField_Password() {}
    virtual void		GetNormalizedFieldText(CFX_WideString &wsField) const;
private:
    FX_WCHAR	m_wcAlias;
};
class CFDE_TxtEdtField_String : public CFDE_TxtEdtField
{
public:
    CFDE_TxtEdtField_String(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock);
protected:
    virtual ~CFDE_TxtEdtField_String() {}
};
class CFDE_TxtEdtField_Fixed : public CFDE_TxtEdtField
{
public:
    CFDE_TxtEdtField_Fixed(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock);
    virtual FX_INT32	Insert(	FX_INT32 nIndex, const CFX_WideString &wsIns,
                                FX_INT32 &nCaret, FX_BOOL &bBefore)
    {
        return FALSE;
    }
    virtual FX_INT32	Delete(	FX_INT32 nIndex, FX_INT32 nCount, CFX_WideString &wsDel,
                                FX_INT32 &nCaret, FX_BOOL &bBefore)
    {
        return FALSE;
    }
    virtual FX_INT32	Replace(FX_INT32 nIndex, FX_INT32 nCount, const CFX_WideString &wsIns,
                                CFX_WideString &wsDel, FX_INT32 &nCaret, FX_BOOL &bBefore)
    {
        return FALSE;
    }
    virtual void		GetDisplayText(CFX_WideString &wsDisplay);
    virtual FX_INT32	NormalizeCaretPos(	FX_INT32 nIndex,
                                            FDE_FORMAT_CARET_DIRECTION eDirection ) const;
    virtual FX_BOOL		GetEditableRange(FX_INT32 &nBgn, FX_INT32 &nEnd) const
    {
        return FALSE;
    }
    virtual void		Backup() {}
    virtual void		Restore() {}

    virtual FX_BOOL		IsFix() const
    {
        return TRUE;
    }
protected:
    virtual ~CFDE_TxtEdtField_Fixed() {}
};
#endif
#endif
