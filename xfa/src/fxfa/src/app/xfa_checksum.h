// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef	_FXFA_FORMFILLER_CHECKSUM_IMP_H
#define _FXFA_FORMFILLER_CHECKSUM_IMP_H
class CXFA_SAXReaderHandler;
class CXFA_ChecksumContext;
class CXFA_SAXContext
{
public:
    CXFA_SAXContext() : m_eNode(FX_SAXNODE_Unknown)
    {
    }
    CFX_ByteTextBuf		m_TextBuf;
    CFX_ByteString		m_bsTagName;
    FX_SAXNODE			m_eNode;
};
class CXFA_SAXReaderHandler : public IFX_SAXReaderHandler
{
public:
    CXFA_SAXReaderHandler(CXFA_ChecksumContext *pContext);
    virtual ~CXFA_SAXReaderHandler();
    virtual void*	OnTagEnter(FX_BSTR bsTagName, FX_SAXNODE eType, FX_DWORD dwStartPos);
    virtual void		OnTagAttribute(void* pTag, FX_BSTR bsAttri, FX_BSTR bsValue);
    virtual void		OnTagBreak(void* pTag);
    virtual void		OnTagData(void* pTag, FX_SAXNODE eType, FX_BSTR bsData, FX_DWORD dwStartPos);
    virtual void		OnTagClose(void* pTag, FX_DWORD dwEndPos);
    virtual void		OnTagEnd(void* pTag, FX_BSTR bsTagName, FX_DWORD dwEndPos);

    virtual void		OnTargetData(void* pTag, FX_SAXNODE eType, FX_BSTR bsData, FX_DWORD dwStartPos);
protected:
    void				UpdateChecksum(FX_BOOL bCheckSpace);
    CXFA_ChecksumContext	*m_pContext;
    CXFA_SAXContext			m_SAXContext;
};
class CXFA_ChecksumContext : public IXFA_ChecksumContext
{
public:
    CXFA_ChecksumContext();
    ~CXFA_ChecksumContext();
    virtual	void		Release()
    {
        delete this;
    }
    virtual FX_BOOL		StartChecksum();
    virtual FX_BOOL		UpdateChecksum(IFX_FileRead *pSrcFile, FX_FILESIZE offset = 0, size_t size = 0);
    virtual void		FinishChecksum();
    virtual void		GetChecksum(CFX_ByteString &bsChecksum);
    void				Update(FX_BSTR bsText);
protected:
    IFX_SAXReader		*m_pSAXReader;
    uint8_t*			m_pByteContext;
    CFX_ByteString		m_bsChecksum;
};
#endif
