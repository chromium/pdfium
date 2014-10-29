// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_common.h"
#include "xfa_checksum.h"
CXFA_SAXReaderHandler::CXFA_SAXReaderHandler(CXFA_ChecksumContext *pContext)
    : m_pContext(pContext)
{
    FXSYS_assert(m_pContext);
}
CXFA_SAXReaderHandler::~CXFA_SAXReaderHandler()
{
}
FX_LPVOID CXFA_SAXReaderHandler::OnTagEnter(FX_BSTR bsTagName, FX_SAXNODE eType, FX_DWORD dwStartPos)
{
    UpdateChecksum(TRUE);
    if (eType != FX_SAXNODE_Tag && eType != FX_SAXNODE_Instruction) {
        return NULL;
    }
    m_SAXContext.m_eNode = eType;
    CFX_ByteTextBuf &textBuf = m_SAXContext.m_TextBuf;
    textBuf << FX_BSTRC("<");
    if (eType == FX_SAXNODE_Instruction) {
        textBuf << FX_BSTRC("?");
    }
    textBuf << bsTagName;
    m_SAXContext.m_bsTagName = bsTagName;
    return &m_SAXContext;
}
void CXFA_SAXReaderHandler::OnTagAttribute(FX_LPVOID pTag, FX_BSTR bsAttri, FX_BSTR bsValue)
{
    if (pTag == NULL) {
        return;
    }
    CFX_ByteTextBuf &textBuf = ((CXFA_SAXContext*)pTag)->m_TextBuf;
    textBuf << FX_BSTRC(" ");
    textBuf << bsAttri;
    textBuf << FX_BSTRC("=\"");
    textBuf << bsValue;
    textBuf << FX_BSTRC("\"");
}
void CXFA_SAXReaderHandler::OnTagBreak(FX_LPVOID pTag)
{
    if (pTag == NULL) {
        return;
    }
    CFX_ByteTextBuf &textBuf = ((CXFA_SAXContext*)pTag)->m_TextBuf;
    textBuf << FX_BSTRC(">");
    UpdateChecksum(FALSE);
}
void CXFA_SAXReaderHandler::OnTagData(FX_LPVOID pTag, FX_SAXNODE eType, FX_BSTR bsData, FX_DWORD dwStartPos)
{
    if (pTag == NULL) {
        return;
    }
    CFX_ByteTextBuf &textBuf = ((CXFA_SAXContext*)pTag)->m_TextBuf;
    if (eType == FX_SAXNODE_CharData) {
        textBuf << FX_BSTRC("<![CDATA[");
    }
    textBuf << bsData;
    if (eType == FX_SAXNODE_CharData) {
        textBuf << FX_BSTRC("]]>");
    }
}
void CXFA_SAXReaderHandler::OnTagClose(FX_LPVOID pTag, FX_DWORD dwEndPos)
{
    if (pTag == NULL) {
        return;
    }
    CXFA_SAXContext *pSAXContext = (CXFA_SAXContext*)pTag;
    CFX_ByteTextBuf &textBuf = pSAXContext->m_TextBuf;
    if (pSAXContext->m_eNode == FX_SAXNODE_Instruction) {
        textBuf << FX_BSTRC("?>");
    } else if (pSAXContext->m_eNode == FX_SAXNODE_Tag) {
        textBuf << FX_BSTRC("></");
        textBuf << pSAXContext->m_bsTagName;
        textBuf << FX_BSTRC(">");
    }
    UpdateChecksum(FALSE);
}
void CXFA_SAXReaderHandler::OnTagEnd(FX_LPVOID pTag, FX_BSTR bsTagName, FX_DWORD dwEndPos)
{
    if (pTag == NULL) {
        return;
    }
    CFX_ByteTextBuf &textBuf = ((CXFA_SAXContext*)pTag)->m_TextBuf;
    textBuf << FX_BSTRC("</");
    textBuf << bsTagName;
    textBuf << FX_BSTRC(">");
    UpdateChecksum(FALSE);
}
void CXFA_SAXReaderHandler::OnTargetData(FX_LPVOID pTag, FX_SAXNODE eType, FX_BSTR bsData, FX_DWORD dwStartPos)
{
    if (pTag == NULL && eType != FX_SAXNODE_Comment) {
        return;
    }
    if (eType == FX_SAXNODE_Comment) {
        CFX_ByteTextBuf &textBuf = m_SAXContext.m_TextBuf;
        textBuf << FX_BSTRC("<!--");
        textBuf << bsData;
        textBuf << FX_BSTRC("-->");
        UpdateChecksum(FALSE);
    } else {
        CFX_ByteTextBuf &textBuf = ((CXFA_SAXContext*)pTag)->m_TextBuf;
        textBuf << FX_BSTRC(" ");
        textBuf << bsData;
    }
}
void CXFA_SAXReaderHandler::UpdateChecksum(FX_BOOL bCheckSpace)
{
    FX_INT32 iLength = m_SAXContext.m_TextBuf.GetLength();
    if (iLength < 1) {
        return;
    }
    FX_LPBYTE pBuffer = m_SAXContext.m_TextBuf.GetBuffer();
    FX_BOOL bUpdata = TRUE;
    if (bCheckSpace) {
        bUpdata = FALSE;
        for (FX_INT32 i = 0; i < iLength; i++) {
            bUpdata = (pBuffer[i] > 0x20);
            if (bUpdata) {
                break;
            }
        }
    }
    if (bUpdata) {
        m_pContext->Update(CFX_ByteStringC(pBuffer, iLength));
    }
    m_SAXContext.m_TextBuf.Clear();
}
IXFA_ChecksumContext* XFA_Checksum_Create()
{
    return FX_NEW CXFA_ChecksumContext;
}
CXFA_ChecksumContext::CXFA_ChecksumContext()
    : m_pSAXReader(NULL)
    , m_pByteContext(NULL)
{
}
CXFA_ChecksumContext::~CXFA_ChecksumContext()
{
    FinishChecksum();
}
FX_BOOL CXFA_ChecksumContext::StartChecksum()
{
    FinishChecksum();
    m_pByteContext = FX_Alloc(FX_BYTE, 128);
    FXSYS_assert(m_pByteContext != NULL);
    CRYPT_SHA1Start(m_pByteContext);
    m_bsChecksum.Empty();
    m_pSAXReader = FX_SAXReader_Create();
    return m_pSAXReader != NULL;
}
FX_BOOL CXFA_ChecksumContext::UpdateChecksum(IFX_FileRead *pSrcFile, FX_FILESIZE offset, size_t size)
{
    if (m_pSAXReader == NULL)	{
        return FALSE;
    }
    if (pSrcFile == NULL) {
        return FALSE;
    }
    if (size < 1) {
        size = pSrcFile->GetSize();
    }
    CXFA_SAXReaderHandler handler(this);
    m_pSAXReader->SetHandler(&handler);
    if (m_pSAXReader->StartParse(pSrcFile, (FX_DWORD)offset, (FX_DWORD)size, FX_SAXPARSEMODE_NotSkipSpace | FX_SAXPARSEMODE_NotConvert_amp | FX_SAXPARSEMODE_NotConvert_lt | FX_SAXPARSEMODE_NotConvert_gt | FX_SAXPARSEMODE_NotConvert_sharp) < 0) {
        return FALSE;
    }
    return m_pSAXReader->ContinueParse(NULL) > 99;
}
void CXFA_ChecksumContext::FinishChecksum()
{
    if (m_pSAXReader != NULL) {
        m_pSAXReader->Release();
        m_pSAXReader = NULL;
    }
    if (m_pByteContext) {
        FX_BYTE digest[20];
        FXSYS_memset(digest, 0, 20);
        CRYPT_SHA1Finish(m_pByteContext, digest);
        FX_INT32 nLen = FX_Base64EncodeA(digest, 20, NULL);
        FX_LPSTR pBuffer = m_bsChecksum.GetBuffer(nLen);
        FX_Base64EncodeA(digest, 20, pBuffer);
        m_bsChecksum.ReleaseBuffer(nLen);
        FX_Free(m_pByteContext);
        m_pByteContext = NULL;
    }
}
void CXFA_ChecksumContext::GetChecksum(CFX_ByteString &bsChecksum)
{
    bsChecksum = m_bsChecksum;
}
void CXFA_ChecksumContext::Update(FX_BSTR bsText)
{
    if (m_pByteContext != NULL) {
        CRYPT_SHA1Update(m_pByteContext, bsText.GetPtr(), bsText.GetLength());
    }
}
