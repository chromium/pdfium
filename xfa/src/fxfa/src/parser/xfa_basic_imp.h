// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_BASIC_IMP
#define _XFA_BASIC_IMP
typedef struct _XFA_NOTSUREATTRIBUTE {
    XFA_ELEMENT				eElement;
    XFA_ATTRIBUTE			eAttribute;
    XFA_ATTRIBUTETYPE		eType;
    FX_LPVOID				pValue;
} XFA_NOTSUREATTRIBUTE, * XFA_LPNOTSUREATTRIBUTE;
typedef XFA_NOTSUREATTRIBUTE const * XFA_LPCNOTSUREATTRIBUTE;
XFA_LPCNOTSUREATTRIBUTE XFA_GetNotsureAttribute(XFA_ELEMENT eElement, XFA_ATTRIBUTE eAttribute, XFA_ATTRIBUTETYPE eType = XFA_ATTRIBUTETYPE_NOTSURE);
class CXFA_WideTextRead : public IFX_Stream, public CFX_Object
{
public:
    CXFA_WideTextRead(const CFX_WideString &wsBuffer);
    virtual void				Release();
    virtual IFX_Stream*		Retain();

    virtual FX_DWORD			GetAccessModes() const;
    virtual FX_INT32			GetLength() const;
    virtual FX_INT32			Seek(FX_STREAMSEEK eSeek, FX_INT32 iOffset);
    virtual FX_INT32			GetPosition();
    virtual FX_BOOL				IsEOF() const;

    virtual FX_INT32			ReadData(FX_LPBYTE pBuffer, FX_INT32 iBufferSize)
    {
        return 0;
    }
    virtual FX_INT32			ReadString(FX_LPWSTR pStr, FX_INT32 iMaxLength, FX_BOOL &bEOS, FX_INT32 const *pByteSize = NULL);
    virtual FX_INT32			WriteData(FX_LPCBYTE pBuffer, FX_INT32 iBufferSize)
    {
        return 0;
    }
    virtual FX_INT32			WriteString(FX_LPCWSTR pStr, FX_INT32 iLength)
    {
        return 0;
    }
    virtual void				Flush() {}
    virtual FX_BOOL				SetLength(FX_INT32 iLength)
    {
        return FALSE;
    }

    virtual FX_INT32			GetBOM(FX_BYTE bom[4]) const
    {
        return 0;
    }
    virtual FX_WORD				GetCodePage() const;
    virtual FX_WORD				SetCodePage(FX_WORD wCodePage);

    virtual void				Lock() {}
    virtual void				Unlock() {}

    virtual IFX_Stream*		CreateSharedStream(FX_DWORD dwAccess, FX_INT32 iOffset, FX_INT32 iLength)
    {
        return NULL;
    }

    CFX_WideString				GetSrcText() const
    {
        return m_wsBuffer;
    }
protected:
    CFX_WideString				m_wsBuffer;
    FX_INT32					m_iPosition;
    FX_INT32					m_iRefCount;
};
#endif
