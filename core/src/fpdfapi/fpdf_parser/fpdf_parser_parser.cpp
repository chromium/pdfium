// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_parser.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fpdfapi/fpdf_page.h"
#include "../../../src/fxcrt/fx_safe_types.h"
#include "../fpdf_page/pageint.h"
#include <utility>
#include <vector>

FX_BOOL IsSignatureDict(const CPDF_Dictionary* pDict)
{
    CPDF_Object* pType = pDict->GetElementValue(FX_BSTRC("Type"));
    if (!pType) {
        pType = pDict->GetElementValue(FX_BSTRC("FT"));
        if (!pType) {
            return FALSE;
        }
    }
    if (pType->GetString() == FX_BSTRC("Sig")) {
        return TRUE;
    }
    return FALSE;
}
static int _CompareFileSize(const void* p1, const void* p2)
{
    FX_FILESIZE ret = (*(FX_FILESIZE*)p1) - (*(FX_FILESIZE*)p2);
    if (ret > 0) {
        return 1;
    }
    if (ret < 0) {
        return -1;
    }
    return 0;
}

CPDF_Parser::CPDF_Parser()
{
    m_pDocument = NULL;
    m_pTrailer = NULL;
    m_pEncryptDict = NULL;
    m_pSecurityHandler = NULL;
    m_pLinearized = NULL;
    m_dwFirstPageNo = 0;
    m_dwXrefStartObjNum = 0;
    m_bOwnFileRead = TRUE;
    m_FileVersion = 0;
    m_bForceUseSecurityHandler = FALSE;
}
CPDF_Parser::~CPDF_Parser()
{
    CloseParser(FALSE);
}
FX_DWORD CPDF_Parser::GetLastObjNum()
{
    FX_DWORD dwSize = m_CrossRef.GetSize();
    return dwSize ? dwSize - 1 : 0;
}
void CPDF_Parser::SetEncryptDictionary(CPDF_Dictionary* pDict)
{
    m_pEncryptDict = pDict;
}
void CPDF_Parser::CloseParser(FX_BOOL bReParse)
{
    m_bVersionUpdated = FALSE;
    if (m_pDocument && !bReParse) {
        delete m_pDocument;
        m_pDocument = NULL;
    }
    if (m_pTrailer) {
        m_pTrailer->Release();
        m_pTrailer = NULL;
    }
    ReleaseEncryptHandler();
    SetEncryptDictionary(NULL);
    if (m_bOwnFileRead && m_Syntax.m_pFileAccess) {
        m_Syntax.m_pFileAccess->Release();
        m_Syntax.m_pFileAccess = NULL;
    }
    FX_POSITION pos = m_ObjectStreamMap.GetStartPosition();
    while (pos) {
        FX_LPVOID objnum;
        CPDF_StreamAcc* pStream;
        m_ObjectStreamMap.GetNextAssoc(pos, objnum, (void*&)pStream);
        delete pStream;
    }
    m_ObjectStreamMap.RemoveAll();
    m_SortedOffset.RemoveAll();
    m_CrossRef.RemoveAll();
    m_V5Type.RemoveAll();
    m_ObjVersion.RemoveAll();
    FX_INT32 iLen = m_Trailers.GetSize();
    for (FX_INT32 i = 0; i < iLen; ++i) {
        if (CPDF_Dictionary* trailer = m_Trailers.GetAt(i))
            trailer->Release();
    }
    m_Trailers.RemoveAll();
    if (m_pLinearized) {
        m_pLinearized->Release();
        m_pLinearized = NULL;
    }
}
static FX_INT32 GetHeaderOffset(IFX_FileRead* pFile)
{
    FX_DWORD tag = FXDWORD_FROM_LSBFIRST(0x46445025);
    FX_BYTE buf[4];
    FX_INT32 offset = 0;
    while (1) {
        if (!pFile->ReadBlock(buf, offset, 4)) {
            return -1;
        }
        if (*(FX_DWORD*)buf == tag) {
            return offset;
        }
        offset ++;
        if (offset > 1024) {
            return -1;
        }
    }
    return -1;
}
FX_DWORD CPDF_Parser::StartParse(FX_LPCSTR filename, FX_BOOL bReParse)
{
    IFX_FileRead* pFileAccess = FX_CreateFileRead(filename);
    if (!pFileAccess) {
        return PDFPARSE_ERROR_FILE;
    }
    return StartParse(pFileAccess, bReParse);
}
FX_DWORD CPDF_Parser::StartParse(FX_LPCWSTR filename, FX_BOOL bReParse)
{
    IFX_FileRead* pFileAccess = FX_CreateFileRead(filename);
    if (!pFileAccess) {
        return PDFPARSE_ERROR_FILE;
    }
    return StartParse(pFileAccess, bReParse);
}
CPDF_SecurityHandler* FPDF_CreateStandardSecurityHandler();
CPDF_SecurityHandler* FPDF_CreatePubKeyHandler(void*);
FX_DWORD CPDF_Parser::StartParse(IFX_FileRead* pFileAccess, FX_BOOL bReParse, FX_BOOL bOwnFileRead)
{
    CloseParser(bReParse);
    m_bXRefStream = FALSE;
    m_LastXRefOffset = 0;
    m_bOwnFileRead = bOwnFileRead;
    FX_INT32 offset = GetHeaderOffset(pFileAccess);
    if (offset == -1) {
        if (bOwnFileRead && pFileAccess) {
            pFileAccess->Release();
        }
        return PDFPARSE_ERROR_FORMAT;
    }
    m_Syntax.InitParser(pFileAccess, offset);
    FX_BYTE ch;
    if (!m_Syntax.GetCharAt(5, ch)) {
        return PDFPARSE_ERROR_FORMAT;
    }
    if (ch >= '0' && ch <= '9') {
        m_FileVersion = (ch - '0') * 10;
    }
    if (!m_Syntax.GetCharAt(7, ch)) {
        return PDFPARSE_ERROR_FORMAT;
    }
    if (ch >= '0' && ch <= '9') {
        m_FileVersion += ch - '0';
    }
    if (m_Syntax.m_FileLen <  m_Syntax.m_HeaderOffset + 9) {
        return PDFPARSE_ERROR_FORMAT;
    }
    m_Syntax.RestorePos(m_Syntax.m_FileLen - m_Syntax.m_HeaderOffset - 9);
    if (!bReParse) {
        m_pDocument = new CPDF_Document(this);
    }
    FX_BOOL bXRefRebuilt = FALSE;
    if (m_Syntax.SearchWord(FX_BSTRC("startxref"), TRUE, FALSE, 4096)) {
        FX_FILESIZE startxref_offset = m_Syntax.SavePos();
        FX_LPVOID pResult = FXSYS_bsearch(&startxref_offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
        if (pResult == NULL) {
            m_SortedOffset.Add(startxref_offset);
        }
        m_Syntax.GetKeyword();
        FX_BOOL bNumber;
        CFX_ByteString xrefpos_str = m_Syntax.GetNextWord(bNumber);
        if (!bNumber) {
            return PDFPARSE_ERROR_FORMAT;
        }
        m_LastXRefOffset = (FX_FILESIZE)FXSYS_atoi64(xrefpos_str);
        if (!LoadAllCrossRefV4(m_LastXRefOffset) && !LoadAllCrossRefV5(m_LastXRefOffset)) {
            if (!RebuildCrossRef()) {
                return PDFPARSE_ERROR_FORMAT;
            }
            bXRefRebuilt = TRUE;
            m_LastXRefOffset = 0;
        }
    } else {
        if (!RebuildCrossRef()) {
            return PDFPARSE_ERROR_FORMAT;
        }
        bXRefRebuilt = TRUE;
    }
    FX_DWORD dwRet = SetEncryptHandler();
    if (dwRet != PDFPARSE_ERROR_SUCCESS) {
        return dwRet;
    }
    m_pDocument->LoadDoc();
    if (m_pDocument->GetRoot() == NULL || m_pDocument->GetPageCount() == 0) {
        if (bXRefRebuilt) {
            return PDFPARSE_ERROR_FORMAT;
        }
        ReleaseEncryptHandler();
        if (!RebuildCrossRef()) {
            return PDFPARSE_ERROR_FORMAT;
        }
        dwRet = SetEncryptHandler();
        if (dwRet != PDFPARSE_ERROR_SUCCESS) {
            return dwRet;
        }
        m_pDocument->LoadDoc();
        if (m_pDocument->GetRoot() == NULL) {
            return PDFPARSE_ERROR_FORMAT;
        }
    }
    FXSYS_qsort(m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    FX_DWORD RootObjNum = GetRootObjNum();
    if (RootObjNum == 0) {
        ReleaseEncryptHandler();
        RebuildCrossRef();
        RootObjNum = GetRootObjNum();
        if (RootObjNum == 0) {
            return PDFPARSE_ERROR_FORMAT;
        }
        dwRet = SetEncryptHandler();
        if (dwRet != PDFPARSE_ERROR_SUCCESS) {
            return dwRet;
        }
    }
    if (m_pSecurityHandler && !m_pSecurityHandler->IsMetadataEncrypted()) {
        CPDF_Reference* pMetadata = (CPDF_Reference*)m_pDocument->GetRoot()->GetElement(FX_BSTRC("Metadata"));
        if (pMetadata && pMetadata->GetType() == PDFOBJ_REFERENCE) {
            m_Syntax.m_MetadataObjnum = pMetadata->GetRefObjNum();
        }
    }
    return PDFPARSE_ERROR_SUCCESS;
}
FX_DWORD CPDF_Parser::SetEncryptHandler()
{
    ReleaseEncryptHandler();
    SetEncryptDictionary(NULL);
    if (m_pTrailer == NULL) {
        return PDFPARSE_ERROR_FORMAT;
    }
    CPDF_Object* pEncryptObj = m_pTrailer->GetElement(FX_BSTRC("Encrypt"));
    if (pEncryptObj) {
        if (pEncryptObj->GetType() == PDFOBJ_DICTIONARY) {
            SetEncryptDictionary((CPDF_Dictionary*)pEncryptObj);
        } else if (pEncryptObj->GetType() == PDFOBJ_REFERENCE) {
            pEncryptObj = m_pDocument->GetIndirectObject(((CPDF_Reference*)pEncryptObj)->GetRefObjNum());
            if (pEncryptObj) {
                SetEncryptDictionary(pEncryptObj->GetDict());
            }
        }
    }
    if (m_bForceUseSecurityHandler) {
        FX_DWORD err = PDFPARSE_ERROR_HANDLER;
        if (m_pSecurityHandler == NULL) {
            return PDFPARSE_ERROR_HANDLER;
        }
        if (!m_pSecurityHandler->OnInit(this, m_pEncryptDict)) {
            return err;
        }
        CPDF_CryptoHandler* pCryptoHandler = m_pSecurityHandler->CreateCryptoHandler();
        if (!pCryptoHandler->Init(m_pEncryptDict, m_pSecurityHandler)) {
            delete pCryptoHandler;
            pCryptoHandler = NULL;
            return PDFPARSE_ERROR_HANDLER;
        }
        m_Syntax.SetEncrypt(pCryptoHandler);
    } else if (m_pEncryptDict) {
        CFX_ByteString filter = m_pEncryptDict->GetString(FX_BSTRC("Filter"));
        CPDF_SecurityHandler* pSecurityHandler = NULL;
        FX_DWORD err = PDFPARSE_ERROR_HANDLER;
        if (filter == FX_BSTRC("Standard")) {
            pSecurityHandler = FPDF_CreateStandardSecurityHandler();
            err = PDFPARSE_ERROR_PASSWORD;
        }
        if (pSecurityHandler == NULL) {
            return PDFPARSE_ERROR_HANDLER;
        }
        if (!pSecurityHandler->OnInit(this, m_pEncryptDict)) {
            delete pSecurityHandler;
            pSecurityHandler = NULL;
            return err;
        }
        m_pSecurityHandler = pSecurityHandler;
        CPDF_CryptoHandler* pCryptoHandler = pSecurityHandler->CreateCryptoHandler();
        if (!pCryptoHandler->Init(m_pEncryptDict, m_pSecurityHandler)) {
            delete pCryptoHandler;
            pCryptoHandler = NULL;
            return PDFPARSE_ERROR_HANDLER;
        }
        m_Syntax.SetEncrypt(pCryptoHandler);
    }
    return PDFPARSE_ERROR_SUCCESS;
}
void CPDF_Parser::ReleaseEncryptHandler()
{
    if (m_Syntax.m_pCryptoHandler) {
        delete m_Syntax.m_pCryptoHandler;
        m_Syntax.m_pCryptoHandler = NULL;
    }
    if (m_pSecurityHandler && !m_bForceUseSecurityHandler) {
        delete m_pSecurityHandler;
        m_pSecurityHandler = NULL;
    }
}
FX_FILESIZE CPDF_Parser::GetObjectOffset(FX_DWORD objnum)
{
    if (objnum >= (FX_DWORD)m_CrossRef.GetSize()) {
        return 0;
    }
    if (m_V5Type[objnum] == 1) {
        return m_CrossRef[objnum];
    }
    if (m_V5Type[objnum] == 2) {
        return m_CrossRef[(FX_INT32)m_CrossRef[objnum]];
    }
    return 0;
}
static FX_INT32 GetDirectInteger(CPDF_Dictionary* pDict, FX_BSTR key)
{
    CPDF_Object* pObj = pDict->GetElement(key);
    if (pObj == NULL) {
        return 0;
    }
    if (pObj->GetType() == PDFOBJ_NUMBER) {
        return ((CPDF_Number*)pObj)->GetInteger();
    }
    return 0;
}
static FX_BOOL CheckDirectType(CPDF_Dictionary* pDict, FX_BSTR key, FX_INT32 iType)
{
    CPDF_Object* pObj = pDict->GetElement(key);
    if (!pObj) {
        return TRUE;
    }
    return pObj->GetType() == iType;
}
FX_BOOL CPDF_Parser::LoadAllCrossRefV4(FX_FILESIZE xrefpos)
{
    if (!LoadCrossRefV4(xrefpos, 0, TRUE, FALSE)) {
        return FALSE;
    }
    m_pTrailer = LoadTrailerV4();
    if (m_pTrailer == NULL) {
        return FALSE;
    }
    FX_INT32 xrefsize = GetDirectInteger(m_pTrailer, FX_BSTRC("Size"));
    if (xrefsize <= 0 || xrefsize > (1 << 20)) {
        return FALSE;
    }
    m_CrossRef.SetSize(xrefsize);
    m_V5Type.SetSize(xrefsize);
    CFX_FileSizeArray CrossRefList, XRefStreamList;
    CrossRefList.Add(xrefpos);
    XRefStreamList.Add(GetDirectInteger(m_pTrailer, FX_BSTRC("XRefStm")));
    if (!CheckDirectType(m_pTrailer, FX_BSTRC("Prev"), PDFOBJ_NUMBER)) {
        return FALSE;
    }
    FX_FILESIZE newxrefpos = GetDirectInteger(m_pTrailer, FX_BSTRC("Prev"));
    if (newxrefpos == xrefpos) {
        return FALSE;
    }
    xrefpos = newxrefpos;
    while (xrefpos) {
        CrossRefList.InsertAt(0, xrefpos);
        LoadCrossRefV4(xrefpos, 0, TRUE, FALSE);
        CPDF_Dictionary* pDict = LoadTrailerV4();
        if (pDict == NULL) {
            return FALSE;
        }
        if (!CheckDirectType(pDict, FX_BSTRC("Prev"), PDFOBJ_NUMBER)) {
            pDict->Release();
            return FALSE;
        }
        newxrefpos = GetDirectInteger(pDict, FX_BSTRC("Prev"));
        if (newxrefpos == xrefpos) {
            pDict->Release();
            return FALSE;
        }
        xrefpos = newxrefpos;
        XRefStreamList.InsertAt(0, pDict->GetInteger(FX_BSTRC("XRefStm")));
        m_Trailers.Add(pDict);
    }
    for (FX_INT32 i = 0; i < CrossRefList.GetSize(); i ++)
        if (!LoadCrossRefV4(CrossRefList[i], XRefStreamList[i], FALSE, i == 0)) {
            return FALSE;
        }
    return TRUE;
}
FX_BOOL CPDF_Parser::LoadLinearizedAllCrossRefV4(FX_FILESIZE xrefpos, FX_DWORD dwObjCount)
{
    if (!LoadLinearizedCrossRefV4(xrefpos, dwObjCount)) {
        return FALSE;
    }
    m_pTrailer = LoadTrailerV4();
    if (m_pTrailer == NULL) {
        return FALSE;
    }
    FX_INT32 xrefsize = GetDirectInteger(m_pTrailer, FX_BSTRC("Size"));
    if (xrefsize == 0) {
        return FALSE;
    }
    CFX_FileSizeArray CrossRefList, XRefStreamList;
    CrossRefList.Add(xrefpos);
    XRefStreamList.Add(GetDirectInteger(m_pTrailer, FX_BSTRC("XRefStm")));
    xrefpos = GetDirectInteger(m_pTrailer, FX_BSTRC("Prev"));
    while (xrefpos) {
        CrossRefList.InsertAt(0, xrefpos);
        LoadCrossRefV4(xrefpos, 0, TRUE, FALSE);
        CPDF_Dictionary* pDict = LoadTrailerV4();
        if (pDict == NULL) {
            return FALSE;
        }
        xrefpos = GetDirectInteger(pDict, FX_BSTRC("Prev"));
        XRefStreamList.InsertAt(0, pDict->GetInteger(FX_BSTRC("XRefStm")));
        m_Trailers.Add(pDict);
    }
    for (FX_INT32 i = 1; i < CrossRefList.GetSize(); i ++)
        if (!LoadCrossRefV4(CrossRefList[i], XRefStreamList[i], FALSE, i == 0)) {
            return FALSE;
        }
    return TRUE;
}
FX_BOOL CPDF_Parser::LoadLinearizedCrossRefV4(FX_FILESIZE pos, FX_DWORD dwObjCount)
{
    FX_FILESIZE dwStartPos = pos - m_Syntax.m_HeaderOffset;
    m_Syntax.RestorePos(dwStartPos);
    FX_LPVOID pResult = FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    if (pResult == NULL) {
        m_SortedOffset.Add(pos);
    }
    FX_DWORD start_objnum = 0;
    FX_DWORD count = dwObjCount;
    FX_FILESIZE SavedPos = m_Syntax.SavePos();
    FX_INT32 recordsize = 20;
    char* pBuf = FX_Alloc(char, 1024 * recordsize + 1);
    pBuf[1024 * recordsize] = '\0';
    FX_INT32 nBlocks = count / 1024 + 1;
    for (FX_INT32 block = 0; block < nBlocks; block ++) {
        FX_INT32 block_size = block == nBlocks - 1 ? count % 1024 : 1024;
        FX_DWORD dwReadSize = block_size * recordsize;
        if ((FX_FILESIZE)(dwStartPos + dwReadSize) > m_Syntax.m_FileLen) {
            FX_Free(pBuf);
            return FALSE;
        }
        if (!m_Syntax.ReadBlock((FX_LPBYTE)pBuf, dwReadSize)) {
            FX_Free(pBuf);
            return FALSE;
        }
        for (FX_INT32 i = 0; i < block_size; i ++) {
            FX_DWORD objnum = start_objnum + block * 1024 + i;
            char* pEntry = pBuf + i * recordsize;
            if (pEntry[17] == 'f') {
                m_CrossRef.SetAtGrow(objnum, 0);
                m_V5Type.SetAtGrow(objnum, 0);
            } else {
                FX_INT32 offset = FXSYS_atoi(pEntry);
                if (offset == 0) {
                    for (FX_INT32 c = 0; c < 10; c ++) {
                        if (pEntry[c] < '0' || pEntry[c] > '9') {
                            FX_Free(pBuf);
                            return FALSE;
                        }
                    }
                }
                m_CrossRef.SetAtGrow(objnum, offset);
                FX_INT32 version = FXSYS_atoi(pEntry + 11);
                if (version >= 1) {
                    m_bVersionUpdated = TRUE;
                }
                m_ObjVersion.SetAtGrow(objnum, version);
                if (m_CrossRef[objnum] < m_Syntax.m_FileLen) {
                    FX_LPVOID pResult = FXSYS_bsearch(&m_CrossRef[objnum], m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
                    if (pResult == NULL) {
                        m_SortedOffset.Add(m_CrossRef[objnum]);
                    }
                }
                m_V5Type.SetAtGrow(objnum, 1);
            }
        }
    }
    FX_Free(pBuf);
    m_Syntax.RestorePos(SavedPos + count * recordsize);
    return TRUE;
}
FX_BOOL CPDF_Parser::LoadCrossRefV4(FX_FILESIZE pos, FX_FILESIZE streampos, FX_BOOL bSkip, FX_BOOL bFirst)
{
    m_Syntax.RestorePos(pos);
    if (m_Syntax.GetKeyword() != FX_BSTRC("xref")) {
        return FALSE;
    }
    FX_LPVOID pResult = FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    if (pResult == NULL) {
        m_SortedOffset.Add(pos);
    }
    if (streampos) {
        FX_LPVOID pResult = FXSYS_bsearch(&streampos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
        if (pResult == NULL) {
            m_SortedOffset.Add(streampos);
        }
    }
    while (1) {
        FX_FILESIZE SavedPos = m_Syntax.SavePos();
        FX_BOOL bIsNumber;
        CFX_ByteString word = m_Syntax.GetNextWord(bIsNumber);
        if (word.IsEmpty()) {
            return FALSE;
        }
        if (!bIsNumber) {
            m_Syntax.RestorePos(SavedPos);
            break;
        }
        FX_DWORD start_objnum = FXSYS_atoi(word);
        if (start_objnum >= (1 << 20)) {
            return FALSE;
        }
        FX_DWORD count = m_Syntax.GetDirectNum();
        m_Syntax.ToNextWord();
        SavedPos = m_Syntax.SavePos();
        FX_BOOL bFirstItem = FALSE;
        FX_INT32 recordsize = 20;
        if (bFirst) {
            bFirstItem = TRUE;
        }
        m_dwXrefStartObjNum = start_objnum;
        if (!bSkip) {
            char* pBuf = FX_Alloc(char, 1024 * recordsize + 1);
            pBuf[1024 * recordsize] = '\0';
            FX_INT32 nBlocks = count / 1024 + 1;
            FX_BOOL bFirstBlock = TRUE;
            for (FX_INT32 block = 0; block < nBlocks; block ++) {
                FX_INT32 block_size = block == nBlocks - 1 ? count % 1024 : 1024;
                m_Syntax.ReadBlock((FX_LPBYTE)pBuf, block_size * recordsize);
                for (FX_INT32 i = 0; i < block_size; i ++) {
                    FX_DWORD objnum = start_objnum + block * 1024 + i;
                    char* pEntry = pBuf + i * recordsize;
                    if (pEntry[17] == 'f') {
                        if (bFirstItem) {
                            objnum = 0;
                            bFirstItem = FALSE;
                        }
                        if (bFirstBlock) {
                            FX_FILESIZE offset = (FX_FILESIZE)FXSYS_atoi64(pEntry);
                            FX_INT32 version = FXSYS_atoi(pEntry + 11);
                            if (offset == 0 && version == 65535 && start_objnum != 0) {
                                start_objnum--;
                                objnum = 0;
                            }
                        }
                        m_CrossRef.SetAtGrow(objnum, 0);
                        m_V5Type.SetAtGrow(objnum, 0);
                    } else {
                        FX_FILESIZE offset = (FX_FILESIZE)FXSYS_atoi64(pEntry);
                        if (offset == 0) {
                            for (FX_INT32 c = 0; c < 10; c ++) {
                                if (pEntry[c] < '0' || pEntry[c] > '9') {
                                    FX_Free(pBuf);
                                    return FALSE;
                                }
                            }
                        }
                        m_CrossRef.SetAtGrow(objnum, offset);
                        FX_INT32 version = FXSYS_atoi(pEntry + 11);
                        if (version >= 1) {
                            m_bVersionUpdated = TRUE;
                        }
                        m_ObjVersion.SetAtGrow(objnum, version);
                        if (m_CrossRef[objnum] < m_Syntax.m_FileLen) {
                            FX_LPVOID pResult = FXSYS_bsearch(&m_CrossRef[objnum], m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
                            if (pResult == NULL) {
                                m_SortedOffset.Add(m_CrossRef[objnum]);
                            }
                        }
                        m_V5Type.SetAtGrow(objnum, 1);
                    }
                    if (bFirstBlock) {
                        bFirstBlock = FALSE;
                    }
                }
            }
            FX_Free(pBuf);
        }
        m_Syntax.RestorePos(SavedPos + count * recordsize);
    }
    if (streampos)
        if (!LoadCrossRefV5(streampos, streampos, FALSE)) {
            return FALSE;
        }
    return TRUE;
}
FX_BOOL CPDF_Parser::LoadAllCrossRefV5(FX_FILESIZE xrefpos)
{
    if (!LoadCrossRefV5(xrefpos, xrefpos, TRUE)) {
        return FALSE;
    }
    while (xrefpos)
        if (!LoadCrossRefV5(xrefpos, xrefpos, FALSE)) {
            return FALSE;
        }
    m_ObjectStreamMap.InitHashTable(101, FALSE);
    m_bXRefStream = TRUE;
    return TRUE;
}
FX_BOOL CPDF_Parser::RebuildCrossRef()
{
    m_CrossRef.RemoveAll();
    m_V5Type.RemoveAll();
    m_SortedOffset.RemoveAll();
    m_ObjVersion.RemoveAll();
    if (m_pTrailer) {
        m_pTrailer->Release();
        m_pTrailer = NULL;
    }
    FX_INT32 status = 0;
    FX_INT32 inside_index = 0;
    FX_DWORD objnum = 0, gennum = 0;
    FX_INT32 depth = 0;
    FX_LPBYTE buffer = FX_Alloc(FX_BYTE, 4096);
    FX_FILESIZE pos = m_Syntax.m_HeaderOffset;
    FX_FILESIZE start_pos = 0, start_pos1 = 0;
    FX_FILESIZE last_obj = -1, last_xref = -1, last_trailer = -1;
    while (pos < m_Syntax.m_FileLen) {
        FX_BOOL bOverFlow = FALSE;
        FX_DWORD size = (FX_DWORD)(m_Syntax.m_FileLen - pos);
        if (size > 4096) {
            size = 4096;
        }
        if (!m_Syntax.m_pFileAccess->ReadBlock(buffer, pos, size)) {
            break;
        }
        for (FX_DWORD i = 0; i < size; i ++) {
            FX_BYTE byte = buffer[i];
            switch (status) {
                case 0:
                    if (PDF_CharType[byte] == 'W') {
                        status = 1;
                    }
                    if (byte <= '9' && byte >= '0') {
                        --i;
                        status = 1;
                    }
                    if (byte == '%') {
                        inside_index = 0;
                        status = 9;
                    }
                    if (byte == '(') {
                        status = 10;
                        depth = 1;
                    }
                    if (byte == '<') {
                        inside_index = 1;
                        status = 11;
                    }
                    if (byte == '\\') {
                        status = 13;
                    }
                    if (byte == 't') {
                        status = 7;
                        inside_index = 1;
                    }
                    break;
                case 1:
                    if (PDF_CharType[byte] == 'W') {
                        break;
                    } else if (byte <= '9' && byte >= '0') {
                        start_pos = pos + i;
                        status = 2;
                        objnum = byte - '0';
                    } else if (byte == 't') {
                        status = 7;
                        inside_index = 1;
                    } else if (byte == 'x') {
                        status = 8;
                        inside_index = 1;
                    } else {
                        --i;
                        status = 0;
                    }
                    break;
                case 2:
                    if (byte <= '9' && byte >= '0') {
                        objnum = objnum * 10 + byte - '0';
                        break;
                    } else if (PDF_CharType[byte] == 'W') {
                        status = 3;
                    } else {
                        --i;
                        status = 14;
                        inside_index = 0;
                    }
                    break;
                case 3:
                    if (byte <= '9' && byte >= '0') {
                        start_pos1 = pos + i;
                        status = 4;
                        gennum = byte - '0';
                    } else if (PDF_CharType[byte] == 'W') {
                        break;
                    } else if (byte == 't') {
                        status = 7;
                        inside_index = 1;
                    } else {
                        --i;
                        status = 0;
                    }
                    break;
                case 4:
                    if (byte <= '9' && byte >= '0') {
                        gennum = gennum * 10 + byte - '0';
                        break;
                    } else if (PDF_CharType[byte] == 'W') {
                        status = 5;
                    } else {
                        --i;
                        status = 0;
                    }
                    break;
                case 5:
                    if (byte == 'o') {
                        status = 6;
                        inside_index = 1;
                    } else if (PDF_CharType[byte] == 'W') {
                        break;
                    } else if (byte <= '9' && byte >= '0') {
                        objnum = gennum;
                        gennum = byte - '0';
                        start_pos = start_pos1;
                        start_pos1 = pos + i;
                        status = 4;
                    } else if (byte == 't') {
                        status = 7;
                        inside_index = 1;
                    } else {
                        --i;
                        status = 0;
                    }
                    break;
                case 6:
                    switch (inside_index) {
                        case 1:
                            if (byte != 'b') {
                                --i;
                                status = 0;
                            } else {
                                inside_index ++;
                            }
                            break;
                        case 2:
                            if (byte != 'j') {
                                --i;
                                status = 0;
                            } else {
                                inside_index ++;
                            }
                            break;
                        case 3:
                            if (PDF_CharType[byte] == 'W' || PDF_CharType[byte] == 'D') {
                                if (objnum > 0x1000000) {
                                    status = 0;
                                    break;
                                }
                                FX_FILESIZE obj_pos = start_pos - m_Syntax.m_HeaderOffset;
                                last_obj = start_pos;
                                FX_LPVOID pResult = FXSYS_bsearch(&obj_pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
                                if (pResult == NULL) {
                                    m_SortedOffset.Add(obj_pos);
                                }
                                FX_FILESIZE obj_end = 0;
                                CPDF_Object *pObject = ParseIndirectObjectAtByStrict(m_pDocument, obj_pos, objnum, NULL, &obj_end);
                                if (pObject) {
                                    int iType =	pObject->GetType();
                                    if (iType == PDFOBJ_STREAM) {
                                        CPDF_Stream* pStream = (CPDF_Stream*)pObject;
                                        CPDF_Dictionary* pDict = pStream->GetDict();
                                        if (pDict) {
                                            if (pDict->KeyExist(FX_BSTRC("Type"))) {
                                                CFX_ByteString bsValue = pDict->GetString(FX_BSTRC("Type"));
                                                if (bsValue == FX_BSTRC("XRef") && pDict->KeyExist(FX_BSTRC("Size"))) {
                                                    CPDF_Object* pRoot = pDict->GetElement(FX_BSTRC("Root"));
                                                    if (pRoot && pRoot->GetDict() && pRoot->GetDict()->GetElement(FX_BSTRC("Pages"))) {
                                                        if (m_pTrailer) {
                                                            m_pTrailer->Release();
                                                        }
                                                        m_pTrailer = (CPDF_Dictionary*)pDict->Clone();
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                FX_FILESIZE offset = 0;
                                m_Syntax.RestorePos(obj_pos);
                                offset = m_Syntax.FindTag(FX_BSTRC("obj"), 0);
                                if (offset == -1) {
                                    offset = 0;
                                } else {
                                    offset += 3;
                                }
                                FX_FILESIZE nLen = obj_end - obj_pos - offset;
                                if ((FX_DWORD)nLen > size - i) {
                                    pos = obj_end + m_Syntax.m_HeaderOffset;
                                    bOverFlow = TRUE;
                                } else {
                                    i += (FX_DWORD)nLen;
                                }
                                if (m_CrossRef.GetSize() > (FX_INT32)objnum && m_CrossRef[objnum]) {
                                    if (pObject) {
                                        FX_DWORD oldgen = m_ObjVersion.GetAt(objnum);
                                        m_CrossRef[objnum] = obj_pos;
                                        m_ObjVersion.SetAt(objnum, (FX_SHORT)gennum);
                                        if (oldgen != gennum) {
                                            m_bVersionUpdated = TRUE;
                                        }
                                    }
                                } else {
                                    m_CrossRef.SetAtGrow(objnum, obj_pos);
                                    m_V5Type.SetAtGrow(objnum, 1);
                                    m_ObjVersion.SetAtGrow(objnum, (FX_SHORT)gennum);
                                }
                                if (pObject) {
                                    pObject->Release();
                                }
                            }
                            --i;
                            status = 0;
                            break;
                    }
                    break;
                case 7:
                    if (inside_index == 7) {
                        if (PDF_CharType[byte] == 'W' || PDF_CharType[byte] == 'D') {
                            last_trailer = pos + i - 7;
                            m_Syntax.RestorePos(pos + i - m_Syntax.m_HeaderOffset);
                            CPDF_Object* pObj = m_Syntax.GetObject(m_pDocument, 0, 0, 0);
                            if (pObj) {
                                if (pObj->GetType() != PDFOBJ_DICTIONARY && pObj->GetType() != PDFOBJ_STREAM) {
                                    pObj->Release();
                                } else {
                                    CPDF_Dictionary* pTrailer = NULL;
                                    if (pObj->GetType() == PDFOBJ_STREAM) {
                                        pTrailer = ((CPDF_Stream*)pObj)->GetDict();
                                    } else {
                                        pTrailer = (CPDF_Dictionary*)pObj;
                                    }
                                    if (pTrailer) {
                                        if (m_pTrailer) {
                                            CPDF_Object* pRoot = pTrailer->GetElement(FX_BSTRC("Root"));
                                            if (pRoot == NULL || (pRoot->GetType() == PDFOBJ_REFERENCE &&
                                                                  (FX_DWORD)m_CrossRef.GetSize() > ((CPDF_Reference*)pRoot)->GetRefObjNum() &&
                                                                  m_CrossRef.GetAt(((CPDF_Reference*)pRoot)->GetRefObjNum()) != 0)) {
                                                FX_POSITION pos = pTrailer->GetStartPos();
                                                while (pos) {
                                                    CFX_ByteString key;
                                                    CPDF_Object* pObj = pTrailer->GetNextElement(pos, key);
                                                    m_pTrailer->SetAt(key, pObj->Clone(), m_pDocument);
                                                }
                                                pObj->Release();
                                            } else {
                                                pObj->Release();
                                            }
                                        } else {
                                            if (pObj->GetType() == PDFOBJ_STREAM) {
                                                m_pTrailer = (CPDF_Dictionary*)pTrailer->Clone();
                                                pObj->Release();
                                            } else {
                                                m_pTrailer = pTrailer;
                                            }
                                            FX_FILESIZE dwSavePos = m_Syntax.SavePos();
                                            CFX_ByteString strWord = m_Syntax.GetKeyword();
                                            if (!strWord.Compare(FX_BSTRC("startxref"))) {
                                                FX_BOOL bNumber = FALSE;
                                                CFX_ByteString bsOffset = m_Syntax.GetNextWord(bNumber);
                                                if (bNumber) {
                                                    m_LastXRefOffset = FXSYS_atoi(bsOffset);
                                                }
                                            }
                                            m_Syntax.RestorePos(dwSavePos);
                                        }
                                    } else {
                                        pObj->Release();
                                    }
                                }
                            }
                        }
                        --i;
                        status = 0;
                    } else if (byte == "trailer"[inside_index]) {
                        inside_index ++;
                    } else {
                        --i;
                        status = 0;
                    }
                    break;
                case 8:
                    if (inside_index == 4) {
                        last_xref = pos + i - 4;
                        status = 1;
                    } else if (byte == "xref"[inside_index]) {
                        inside_index ++;
                    } else {
                        --i;
                        status = 0;
                    }
                    break;
                case 9:
                    if (byte == '\r' || byte == '\n') {
                        status = 0;
                    }
                    break;
                case 10:
                    if (byte == ')') {
                        if (depth > 0) {
                            depth--;
                        }
                    } else if (byte == '(') {
                        depth++;
                    }
                    if (!depth) {
                        status = 0;
                    }
                    break;
                case 11:
                    if (byte == '<' && inside_index == 1) {
                        status = 12;
                    } else if (byte == '>') {
                        status = 0;
                    }
                    inside_index = 0;
                    break;
                case 12:
                    --i;
                    status = 0;
                    break;
                case 13:
                    if (PDF_CharType[byte] == 'D' || PDF_CharType[byte] == 'W') {
                        --i;
                        status = 0;
                    }
                    break;
                case 14:
                    if (PDF_CharType[byte] == 'W') {
                        status = 0;
                    } else if (byte == '%' || byte == '(' || byte == '<' || byte == '\\') {
                        status = 0;
                        --i;
                    } else if (inside_index == 6) {
                        status = 0;
                        --i;
                    } else if (byte == "endobj"[inside_index]) {
                        inside_index++;
                    }
                    break;
            }
            if (bOverFlow) {
                size = 0;
                break;
            }
        }
        pos += size;
    }
    if (last_xref != -1 && last_xref > last_obj) {
        last_trailer = last_xref;
    } else if (last_trailer == -1 || last_xref < last_obj) {
        last_trailer = m_Syntax.m_FileLen;
    }
    FX_FILESIZE offset = last_trailer - m_Syntax.m_HeaderOffset;
    FX_LPVOID pResult = FXSYS_bsearch(&offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    if (pResult == NULL) {
        m_SortedOffset.Add(offset);
    }
    FX_Free(buffer);
    return TRUE;
}
static FX_DWORD _GetVarInt(FX_LPCBYTE p, FX_INT32 n)
{
    FX_DWORD result = 0;
    for (FX_INT32 i = 0; i < n; i ++) {
        result = result * 256 + p[i];
    }
    return result;
}
FX_BOOL CPDF_Parser::LoadCrossRefV5(FX_FILESIZE pos, FX_FILESIZE& prev, FX_BOOL bMainXRef)
{
    CPDF_Stream* pStream = (CPDF_Stream*)ParseIndirectObjectAt(m_pDocument, pos, 0, NULL);
    if (!pStream) {
        return FALSE;
    }
    if (m_pDocument) {
        CPDF_Dictionary * pDict = m_pDocument->GetRoot();
        if (!pDict || pDict->GetObjNum() != pStream->m_ObjNum) {
            m_pDocument->InsertIndirectObject(pStream->m_ObjNum, pStream);
        } else {
            if (pStream->GetType() == PDFOBJ_STREAM) {
                pStream->Release();
            }
            return FALSE;
        }
    }
    if (pStream->GetType() != PDFOBJ_STREAM) {
        return FALSE;
    }
    prev = pStream->GetDict()->GetInteger(FX_BSTRC("Prev"));
    FX_INT32 size = pStream->GetDict()->GetInteger(FX_BSTRC("Size"));
    if (size < 0) {
        pStream->Release();
        return FALSE;
    }
    if (bMainXRef) {
        m_pTrailer = (CPDF_Dictionary*)pStream->GetDict()->Clone();
        m_CrossRef.SetSize(size);
        if (m_V5Type.SetSize(size)) {
            FXSYS_memset32(m_V5Type.GetData(), 0, size);
        }
    } else {
        m_Trailers.Add((CPDF_Dictionary*)pStream->GetDict()->Clone());
    }
    std::vector<std::pair<FX_INT32, FX_INT32> > arrIndex;
    CPDF_Array* pArray = pStream->GetDict()->GetArray(FX_BSTRC("Index"));
    if (pArray) {
        FX_DWORD nPairSize = pArray->GetCount() / 2;
        for (FX_DWORD i = 0; i < nPairSize; i++) {
            CPDF_Object* pStartNumObj = pArray->GetElement(i * 2);
            CPDF_Object* pCountObj = pArray->GetElement(i * 2 + 1);
            if (pStartNumObj && pStartNumObj->GetType() == PDFOBJ_NUMBER
                && pCountObj && pCountObj->GetType() == PDFOBJ_NUMBER) {
                int nStartNum = pStartNumObj->GetInteger();
                int nCount = pCountObj->GetInteger();
                if (nStartNum >= 0 && nCount > 0) {
                    arrIndex.push_back(std::make_pair(nStartNum, nCount));
                }
            }
        }
    }
    if (arrIndex.size() == 0) {
        arrIndex.push_back(std::make_pair(0, size));
    }
    pArray = pStream->GetDict()->GetArray(FX_BSTRC("W"));
    if (pArray == NULL) {
        pStream->Release();
        return FALSE;
    }
    CFX_DWordArray WidthArray;
    FX_SAFE_DWORD dwAccWidth = 0;
    for (FX_DWORD i = 0; i < pArray->GetCount(); i ++) {
        WidthArray.Add(pArray->GetInteger(i));
        dwAccWidth += WidthArray[i];
    }
    if (!dwAccWidth.IsValid() || WidthArray.GetSize() < 3) {
        pStream->Release();
        return FALSE;
    }
    FX_DWORD totalWidth = dwAccWidth.ValueOrDie();
    CPDF_StreamAcc acc;
    acc.LoadAllData(pStream);
    FX_LPCBYTE pData = acc.GetData();
    FX_DWORD dwTotalSize = acc.GetSize();
    FX_DWORD segindex = 0;
    for (FX_DWORD i = 0; i < arrIndex.size(); i ++) {
        FX_INT32 startnum = arrIndex[i].first;
        if (startnum < 0) {
            continue;
        }
        m_dwXrefStartObjNum = pdfium::base::checked_cast<FX_DWORD, FX_INT32> (startnum);
        FX_DWORD count = pdfium::base::checked_cast<FX_DWORD, FX_INT32> (arrIndex[i].second);
        FX_SAFE_DWORD dwCaculatedSize = segindex;
        dwCaculatedSize += count;
        dwCaculatedSize *= totalWidth;
        if (!dwCaculatedSize.IsValid() || dwCaculatedSize.ValueOrDie() > dwTotalSize) { 
            continue;
        }
        FX_LPCBYTE segstart = pData + segindex * totalWidth;
        FX_SAFE_DWORD dwMaxObjNum = startnum;
        dwMaxObjNum += count;
        FX_DWORD dwV5Size = pdfium::base::checked_cast<FX_DWORD, FX_INT32> (m_V5Type.GetSize());
        if (!dwMaxObjNum.IsValid() || dwMaxObjNum.ValueOrDie() > dwV5Size) {
            continue;
        }
        for (FX_DWORD j = 0; j < count; j ++) {
            FX_INT32 type = 1;
            FX_LPCBYTE entrystart = segstart + j * totalWidth;
            if (WidthArray[0]) {
                type = _GetVarInt(entrystart, WidthArray[0]);
            }
            if (m_V5Type[startnum + j] == 255) {
                FX_FILESIZE offset = _GetVarInt(entrystart + WidthArray[0], WidthArray[1]);
                m_CrossRef[startnum + j] = offset;
                FX_LPVOID pResult = FXSYS_bsearch(&offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
                if (pResult == NULL) {
                    m_SortedOffset.Add(offset);
                }
                continue;
            }
            if (m_V5Type[startnum + j]) {
                continue;
            }
            m_V5Type[startnum + j] = type;
            if (type == 0) {
                m_CrossRef[startnum + j] = 0;
            } else {
                FX_FILESIZE offset = _GetVarInt(entrystart + WidthArray[0], WidthArray[1]);
                m_CrossRef[startnum + j] = offset;
                if (type == 1) {
                    FX_LPVOID pResult = FXSYS_bsearch(&offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
                    if (pResult == NULL) {
                        m_SortedOffset.Add(offset);
                    }
                } else {
                    if (offset < 0 || offset >= m_V5Type.GetSize()) {
                        pStream->Release();
                        return FALSE;
                    }
                    m_V5Type[offset] = 255;
                }
            }
        }
        segindex += count;
    }
    pStream->Release();
    return TRUE;
}
CPDF_Array* CPDF_Parser::GetIDArray()
{
    CPDF_Object* pID = m_pTrailer ? m_pTrailer->GetElement(FX_BSTRC("ID")) : NULL;
    if (pID == NULL) {
        return NULL;
    }
    if (pID->GetType() == PDFOBJ_REFERENCE) {
        pID = ParseIndirectObject(NULL, ((CPDF_Reference*)pID)->GetRefObjNum());
        m_pTrailer->SetAt(FX_BSTRC("ID"), pID);
    }
    if (pID == NULL || pID->GetType() != PDFOBJ_ARRAY) {
        return NULL;
    }
    return (CPDF_Array*)pID;
}
FX_DWORD CPDF_Parser::GetRootObjNum()
{
    CPDF_Object* pRef = m_pTrailer ? m_pTrailer->GetElement(FX_BSTRC("Root")) : NULL;
    if (pRef == NULL || pRef->GetType() != PDFOBJ_REFERENCE) {
        return 0;
    }
    return ((CPDF_Reference*) pRef)->GetRefObjNum();
}
FX_DWORD CPDF_Parser::GetInfoObjNum()
{
    CPDF_Object* pRef = m_pTrailer ? m_pTrailer->GetElement(FX_BSTRC("Info")) : NULL;
    if (pRef == NULL || pRef->GetType() != PDFOBJ_REFERENCE) {
        return 0;
    }
    return ((CPDF_Reference*) pRef)->GetRefObjNum();
}
FX_BOOL CPDF_Parser::IsFormStream(FX_DWORD objnum, FX_BOOL& bForm)
{
    bForm = FALSE;
    if (objnum >= (FX_DWORD)m_CrossRef.GetSize()) {
        return TRUE;
    }
    if (m_V5Type[objnum] == 0) {
        return TRUE;
    }
    if (m_V5Type[objnum] == 2) {
        return TRUE;
    }
    FX_FILESIZE pos = m_CrossRef[objnum];
    FX_LPVOID pResult = FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    if (pResult == NULL) {
        return TRUE;
    }
    if ((FX_FILESIZE*)pResult - (FX_FILESIZE*)m_SortedOffset.GetData() == m_SortedOffset.GetSize() - 1) {
        return FALSE;
    }
    FX_FILESIZE size = ((FX_FILESIZE*)pResult)[1] - pos;
    FX_FILESIZE SavedPos = m_Syntax.SavePos();
    m_Syntax.RestorePos(pos);
    bForm = m_Syntax.SearchMultiWord(FX_BSTRC("/Form\0stream"), TRUE, size) == 0;
    m_Syntax.RestorePos(SavedPos);
    return TRUE;
}
CPDF_Object* CPDF_Parser::ParseIndirectObject(CPDF_IndirectObjects* pObjList, FX_DWORD objnum, PARSE_CONTEXT* pContext)
{
    if (objnum >= (FX_DWORD)m_CrossRef.GetSize()) {
        return NULL;
    }
    if (m_V5Type[objnum] == 1 || m_V5Type[objnum] == 255) {
        FX_FILESIZE pos = m_CrossRef[objnum];
        if (pos <= 0) {
            return NULL;
        }
        return ParseIndirectObjectAt(pObjList, pos, objnum, pContext);
    }
    if (m_V5Type[objnum] == 2) {
        CPDF_StreamAcc* pObjStream = GetObjectStream((FX_DWORD)m_CrossRef[objnum]);
        if (pObjStream == NULL) {
            return NULL;
        }
        FX_INT32 n = pObjStream->GetDict()->GetInteger(FX_BSTRC("N"));
        FX_INT32 offset = pObjStream->GetDict()->GetInteger(FX_BSTRC("First"));
        CPDF_SyntaxParser syntax;
        CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream((FX_LPBYTE)pObjStream->GetData(), (size_t)pObjStream->GetSize(), FALSE));
        syntax.InitParser(file.Get(), 0);
        CPDF_Object* pRet = NULL;
        while (n) {
            FX_DWORD thisnum = syntax.GetDirectNum();
            FX_DWORD thisoff = syntax.GetDirectNum();
            if (thisnum == objnum) {
                syntax.RestorePos(offset + thisoff);
                pRet = syntax.GetObject(pObjList, 0, 0, pContext);
                break;
            }
            n --;
        }
        return pRet;
    }
    return NULL;
}
CPDF_StreamAcc* CPDF_Parser::GetObjectStream(FX_DWORD objnum)
{
    CPDF_StreamAcc* pStreamAcc = NULL;
    if (m_ObjectStreamMap.Lookup((void*)(FX_UINTPTR)objnum, (void*&)pStreamAcc)) {
        return pStreamAcc;
    }
    const CPDF_Stream* pStream = m_pDocument ? (CPDF_Stream*)m_pDocument->GetIndirectObject(objnum) : NULL;
    if (pStream == NULL || pStream->GetType() != PDFOBJ_STREAM) {
        return NULL;
    }
    pStreamAcc = new CPDF_StreamAcc;
    pStreamAcc->LoadAllData(pStream);
    m_ObjectStreamMap.SetAt((void*)(FX_UINTPTR)objnum, pStreamAcc);
    return pStreamAcc;
}
FX_FILESIZE CPDF_Parser::GetObjectSize(FX_DWORD objnum)
{
    if (objnum >= (FX_DWORD)m_CrossRef.GetSize()) {
        return 0;
    }
    if (m_V5Type[objnum] == 2) {
        objnum = (FX_DWORD)m_CrossRef[objnum];
    }
    if (m_V5Type[objnum] == 1 || m_V5Type[objnum] == 255) {
        FX_FILESIZE offset = m_CrossRef[objnum];
        if (offset == 0) {
            return 0;
        }
        FX_LPVOID pResult = FXSYS_bsearch(&offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
        if (pResult == NULL) {
            return 0;
        }
        if ((FX_FILESIZE*)pResult - (FX_FILESIZE*)m_SortedOffset.GetData() == m_SortedOffset.GetSize() - 1) {
            return 0;
        }
        return ((FX_FILESIZE*)pResult)[1] - offset;
    }
    return 0;
}
void CPDF_Parser::GetIndirectBinary(FX_DWORD objnum, FX_LPBYTE& pBuffer, FX_DWORD& size)
{
    pBuffer = NULL;
    size = 0;
    if (objnum >= (FX_DWORD)m_CrossRef.GetSize()) {
        return;
    }
    if (m_V5Type[objnum] == 2) {
        CPDF_StreamAcc* pObjStream = GetObjectStream((FX_DWORD)m_CrossRef[objnum]);
        if (pObjStream == NULL) {
            return;
        }
        FX_INT32 n = pObjStream->GetDict()->GetInteger(FX_BSTRC("N"));
        FX_INT32 offset = pObjStream->GetDict()->GetInteger(FX_BSTRC("First"));
        CPDF_SyntaxParser syntax;
        FX_LPCBYTE pData = pObjStream->GetData();
        FX_DWORD totalsize = pObjStream->GetSize();
        CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream((FX_LPBYTE)pData, (size_t)totalsize, FALSE));
        syntax.InitParser(file.Get(), 0);
        while (n) {
            FX_DWORD thisnum = syntax.GetDirectNum();
            FX_DWORD thisoff = syntax.GetDirectNum();
            if (thisnum == objnum) {
                if (n == 1) {
                    size = totalsize - (thisoff + offset);
                } else {
                    syntax.GetDirectNum();  // Skip nextnum.
                    FX_DWORD nextoff = syntax.GetDirectNum();
                    size = nextoff - thisoff;
                }
                pBuffer = FX_Alloc(FX_BYTE, size);
                FXSYS_memcpy32(pBuffer, pData + thisoff + offset, size);
                return;
            }
            n --;
        }
        return;
    }
    if (m_V5Type[objnum] == 1) {
        FX_FILESIZE pos = m_CrossRef[objnum];
        if (pos == 0) {
            return;
        }
        FX_FILESIZE SavedPos = m_Syntax.SavePos();
        m_Syntax.RestorePos(pos);
        FX_BOOL bIsNumber;
        CFX_ByteString word = m_Syntax.GetNextWord(bIsNumber);
        if (!bIsNumber) {
            m_Syntax.RestorePos(SavedPos);
            return;
        }
        FX_DWORD parser_objnum = FXSYS_atoi(word);
        if (parser_objnum && parser_objnum != objnum) {
            m_Syntax.RestorePos(SavedPos);
            return;
        }
        word = m_Syntax.GetNextWord(bIsNumber);
        if (!bIsNumber) {
            m_Syntax.RestorePos(SavedPos);
            return;
        }
        if (m_Syntax.GetKeyword() != FX_BSTRC("obj")) {
            m_Syntax.RestorePos(SavedPos);
            return;
        }
        FX_LPVOID pResult = FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
        if (pResult == NULL) {
            m_Syntax.RestorePos(SavedPos);
            return;
        }
        FX_FILESIZE nextoff = ((FX_FILESIZE*)pResult)[1];
        FX_BOOL bNextOffValid = FALSE;
        if (nextoff != pos) {
            m_Syntax.RestorePos(nextoff);
            word = m_Syntax.GetNextWord(bIsNumber);
            if (word == FX_BSTRC("xref")) {
                bNextOffValid = TRUE;
            } else if (bIsNumber) {
                word = m_Syntax.GetNextWord(bIsNumber);
                if (bIsNumber && m_Syntax.GetKeyword() == FX_BSTRC("obj")) {
                    bNextOffValid = TRUE;
                }
            }
        }
        if (!bNextOffValid) {
            m_Syntax.RestorePos(pos);
            while (1) {
                if (m_Syntax.GetKeyword() == FX_BSTRC("endobj")) {
                    break;
                }
                if (m_Syntax.SavePos() == m_Syntax.m_FileLen) {
                    break;
                }
            }
            nextoff = m_Syntax.SavePos();
        }
        size = (FX_DWORD)(nextoff - pos);
        pBuffer = FX_Alloc(FX_BYTE, size);
        m_Syntax.RestorePos(pos);
        m_Syntax.ReadBlock(pBuffer, size);
        m_Syntax.RestorePos(SavedPos);
    }
}
CPDF_Object* CPDF_Parser::ParseIndirectObjectAt(CPDF_IndirectObjects* pObjList, FX_FILESIZE pos, FX_DWORD objnum,
        PARSE_CONTEXT* pContext)
{
    FX_FILESIZE SavedPos = m_Syntax.SavePos();
    m_Syntax.RestorePos(pos);
    FX_BOOL bIsNumber;
    CFX_ByteString word = m_Syntax.GetNextWord(bIsNumber);
    if (!bIsNumber) {
        m_Syntax.RestorePos(SavedPos);
        return NULL;
    }
    FX_FILESIZE objOffset = m_Syntax.SavePos();
    objOffset -= word.GetLength();
    FX_DWORD parser_objnum = FXSYS_atoi(word);
    if (objnum && parser_objnum != objnum) {
        m_Syntax.RestorePos(SavedPos);
        return NULL;
    }
    word = m_Syntax.GetNextWord(bIsNumber);
    if (!bIsNumber) {
        m_Syntax.RestorePos(SavedPos);
        return NULL;
    }
    FX_DWORD parser_gennum = FXSYS_atoi(word);
    if (m_Syntax.GetKeyword() != FX_BSTRC("obj")) {
        m_Syntax.RestorePos(SavedPos);
        return NULL;
    }
    CPDF_Object* pObj = m_Syntax.GetObject(pObjList, objnum, parser_gennum, pContext);
    m_Syntax.SavePos();
    CFX_ByteString bsWord = m_Syntax.GetKeyword();
    if (bsWord == FX_BSTRC("endobj")) {
        m_Syntax.SavePos();
    }
    m_Syntax.RestorePos(SavedPos);
    if (pObj) {
        if (!objnum) {
            pObj->m_ObjNum = parser_objnum;
        }
        pObj->m_GenNum = parser_gennum;
    }
    return pObj;
}
CPDF_Object* CPDF_Parser::ParseIndirectObjectAtByStrict(CPDF_IndirectObjects* pObjList, FX_FILESIZE pos, FX_DWORD objnum,
        struct PARSE_CONTEXT* pContext, FX_FILESIZE *pResultPos)
{
    FX_FILESIZE SavedPos = m_Syntax.SavePos();
    m_Syntax.RestorePos(pos);
    FX_BOOL bIsNumber;
    CFX_ByteString word = m_Syntax.GetNextWord(bIsNumber);
    if (!bIsNumber) {
        m_Syntax.RestorePos(SavedPos);
        return NULL;
    }
    FX_DWORD parser_objnum = FXSYS_atoi(word);
    if (objnum && parser_objnum != objnum) {
        m_Syntax.RestorePos(SavedPos);
        return NULL;
    }
    word = m_Syntax.GetNextWord(bIsNumber);
    if (!bIsNumber) {
        m_Syntax.RestorePos(SavedPos);
        return NULL;
    }
    FX_DWORD gennum = FXSYS_atoi(word);
    if (m_Syntax.GetKeyword() != FX_BSTRC("obj")) {
        m_Syntax.RestorePos(SavedPos);
        return NULL;
    }
    CPDF_Object* pObj = m_Syntax.GetObjectByStrict(pObjList, objnum, gennum, pContext);
    if (pResultPos) {
        *pResultPos = m_Syntax.m_Pos;
    }
    m_Syntax.RestorePos(SavedPos);
    return pObj;
}
CPDF_Dictionary* CPDF_Parser::LoadTrailerV4()
{
    if (m_Syntax.GetKeyword() != FX_BSTRC("trailer")) {
        return NULL;
    }
    CPDF_Object* pObj = m_Syntax.GetObject(m_pDocument, 0, 0, 0);
    if (pObj == NULL || pObj->GetType() != PDFOBJ_DICTIONARY) {
        if (pObj) {
            pObj->Release();
        }
        return NULL;
    }
    return (CPDF_Dictionary*)pObj;
}
FX_DWORD CPDF_Parser::GetPermissions(FX_BOOL bCheckRevision)
{
    if (m_pSecurityHandler == NULL) {
        return (FX_DWORD) - 1;
    }
    FX_DWORD dwPermission = m_pSecurityHandler->GetPermissions();
    if (m_pEncryptDict && m_pEncryptDict->GetString(FX_BSTRC("Filter")) == FX_BSTRC("Standard")) {
        dwPermission &= 0xFFFFFFFC;
        dwPermission |= 0xFFFFF0C0;
        if(bCheckRevision && m_pEncryptDict->GetInteger(FX_BSTRC("R")) == 2) {
            dwPermission &= 0xFFFFF0FF;
        }
    }
    return dwPermission;
}
FX_BOOL CPDF_Parser::IsOwner()
{
    return m_pSecurityHandler == NULL ? TRUE : m_pSecurityHandler->IsOwner();
}
void CPDF_Parser::SetSecurityHandler(CPDF_SecurityHandler* pSecurityHandler, FX_BOOL bForced)
{
    ASSERT(m_pSecurityHandler == NULL);
    if (m_pSecurityHandler && !m_bForceUseSecurityHandler) {
        delete m_pSecurityHandler;
        m_pSecurityHandler = NULL;
    }
    m_bForceUseSecurityHandler = bForced;
    m_pSecurityHandler = pSecurityHandler;
    if (m_bForceUseSecurityHandler) {
        return;
    }
    m_Syntax.m_pCryptoHandler = pSecurityHandler->CreateCryptoHandler();
    m_Syntax.m_pCryptoHandler->Init(NULL, pSecurityHandler);
}
FX_BOOL CPDF_Parser::IsLinearizedFile(IFX_FileRead* pFileAccess, FX_DWORD offset)
{
    m_Syntax.InitParser(pFileAccess, offset);
    m_Syntax.RestorePos(m_Syntax.m_HeaderOffset + 9);
    FX_FILESIZE SavedPos = m_Syntax.SavePos();
    FX_BOOL bIsNumber;
    CFX_ByteString word = m_Syntax.GetNextWord(bIsNumber);
    if (!bIsNumber) {
        return FALSE;
    }
    FX_DWORD objnum = FXSYS_atoi(word);
    word = m_Syntax.GetNextWord(bIsNumber);
    if (!bIsNumber) {
        return FALSE;
    }
    FX_DWORD gennum = FXSYS_atoi(word);
    if (m_Syntax.GetKeyword() != FX_BSTRC("obj")) {
        m_Syntax.RestorePos(SavedPos);
        return FALSE;
    }
    m_pLinearized = m_Syntax.GetObject(NULL, objnum, gennum, 0);
    if (!m_pLinearized) {
        return FALSE;
    }
    if (m_pLinearized->GetDict() && m_pLinearized->GetDict()->GetElement(FX_BSTRC("Linearized"))) {
        m_Syntax.GetNextWord(bIsNumber);
        CPDF_Object *pLen = m_pLinearized->GetDict()->GetElement(FX_BSTRC("L"));
        if (!pLen) {
            m_pLinearized->Release();
            m_pLinearized = NULL;
            return FALSE;
        }
        if (pLen->GetInteger() != (int)pFileAccess->GetSize()) {
            return FALSE;
        }
        CPDF_Object *pNo = m_pLinearized->GetDict()->GetElement(FX_BSTRC("P"));
        if (pNo && pNo->GetType() == PDFOBJ_NUMBER) {
            m_dwFirstPageNo = pNo->GetInteger();
        }
        CPDF_Object *pTable = m_pLinearized->GetDict()->GetElement(FX_BSTRC("T"));
        if (pTable && pTable->GetType() == PDFOBJ_NUMBER) {
            m_LastXRefOffset = pTable->GetInteger();
        }
        return TRUE;
    }
    m_pLinearized->Release();
    m_pLinearized = NULL;
    return FALSE;
}
FX_DWORD CPDF_Parser::StartAsynParse(IFX_FileRead* pFileAccess, FX_BOOL bReParse, FX_BOOL bOwnFileRead)
{
    CloseParser(bReParse);
    m_bXRefStream = FALSE;
    m_LastXRefOffset = 0;
    m_bOwnFileRead = bOwnFileRead;
    FX_INT32 offset = GetHeaderOffset(pFileAccess);
    if (offset == -1) {
        return PDFPARSE_ERROR_FORMAT;
    }
    if (!IsLinearizedFile(pFileAccess, offset)) {
        m_Syntax.m_pFileAccess = NULL;
        return StartParse(pFileAccess, bReParse, bOwnFileRead);
    }
    if (!bReParse) {
        m_pDocument = new CPDF_Document(this);
    }
    FX_FILESIZE dwFirstXRefOffset = m_Syntax.SavePos();
    FX_BOOL bXRefRebuilt = FALSE;
    FX_BOOL bLoadV4 = FALSE;
    if (!(bLoadV4 = LoadCrossRefV4(dwFirstXRefOffset, 0, FALSE, FALSE)) && !LoadCrossRefV5(dwFirstXRefOffset, dwFirstXRefOffset, TRUE)) {
        if (!RebuildCrossRef()) {
            return PDFPARSE_ERROR_FORMAT;
        }
        bXRefRebuilt = TRUE;
        m_LastXRefOffset = 0;
    }
    if (bLoadV4) {
        m_pTrailer = LoadTrailerV4();
        if (m_pTrailer == NULL) {
            return FALSE;
        }
        FX_INT32 xrefsize = GetDirectInteger(m_pTrailer, FX_BSTRC("Size"));
        if (xrefsize > 0) {
            m_CrossRef.SetSize(xrefsize);
            m_V5Type.SetSize(xrefsize);
        }
    }
    FX_DWORD dwRet = SetEncryptHandler();
    if (dwRet != PDFPARSE_ERROR_SUCCESS) {
        return dwRet;
    }
    m_pDocument->LoadAsynDoc(m_pLinearized->GetDict());
    if (m_pDocument->GetRoot() == NULL || m_pDocument->GetPageCount() == 0) {
        if (bXRefRebuilt) {
            return PDFPARSE_ERROR_FORMAT;
        }
        ReleaseEncryptHandler();
        if (!RebuildCrossRef()) {
            return PDFPARSE_ERROR_FORMAT;
        }
        dwRet = SetEncryptHandler();
        if (dwRet != PDFPARSE_ERROR_SUCCESS) {
            return dwRet;
        }
        m_pDocument->LoadAsynDoc(m_pLinearized->GetDict());
        if (m_pDocument->GetRoot() == NULL) {
            return PDFPARSE_ERROR_FORMAT;
        }
    }
    FXSYS_qsort(m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    FX_DWORD RootObjNum = GetRootObjNum();
    if (RootObjNum == 0) {
        ReleaseEncryptHandler();
        RebuildCrossRef();
        RootObjNum = GetRootObjNum();
        if (RootObjNum == 0) {
            return PDFPARSE_ERROR_FORMAT;
        }
        dwRet = SetEncryptHandler();
        if (dwRet != PDFPARSE_ERROR_SUCCESS) {
            return dwRet;
        }
    }
    if (m_pSecurityHandler && m_pSecurityHandler->IsMetadataEncrypted()) {
        CPDF_Object* pMetadata = m_pDocument->GetRoot()->GetElement(FX_BSTRC("Metadata"));
        if (pMetadata && pMetadata->GetType() == PDFOBJ_REFERENCE) {
            m_Syntax.m_MetadataObjnum = ((CPDF_Reference*) pMetadata)->GetRefObjNum();
        }
    }
    return PDFPARSE_ERROR_SUCCESS;
}
FX_BOOL CPDF_Parser::LoadLinearizedAllCrossRefV5(FX_FILESIZE xrefpos)
{
    if (!LoadCrossRefV5(xrefpos, xrefpos, FALSE)) {
        return FALSE;
    }
    while (xrefpos)
        if (!LoadCrossRefV5(xrefpos, xrefpos, FALSE)) {
            return FALSE;
        }
    m_ObjectStreamMap.InitHashTable(101, FALSE);
    m_bXRefStream = TRUE;
    return TRUE;
}
FX_DWORD CPDF_Parser::LoadLinearizedMainXRefTable()
{
    FX_DWORD dwSaveMetadataObjnum = m_Syntax.m_MetadataObjnum;
    m_Syntax.m_MetadataObjnum = 0;
    if (m_pTrailer) {
        m_pTrailer->Release();
        m_pTrailer = NULL;
    }
    m_Syntax.RestorePos(m_LastXRefOffset - m_Syntax.m_HeaderOffset);
    FX_BYTE ch = 0;
    FX_DWORD dwCount = 0;
    m_Syntax.GetNextChar(ch);
    FX_INT32 type = PDF_CharType[ch];
    while (type == 'W') {
        ++dwCount;
        if (m_Syntax.m_FileLen >= (FX_FILESIZE)(m_Syntax.SavePos() + m_Syntax.m_HeaderOffset)) {
            break;
        }
        m_Syntax.GetNextChar(ch);
        type = PDF_CharType[ch];
    }
    m_LastXRefOffset += dwCount;
    FX_POSITION pos = m_ObjectStreamMap.GetStartPosition();
    while (pos) {
        FX_LPVOID objnum;
        CPDF_StreamAcc* pStream;
        m_ObjectStreamMap.GetNextAssoc(pos, objnum, (void*&)pStream);
        delete pStream;
    }
    m_ObjectStreamMap.RemoveAll();
    if (!LoadLinearizedAllCrossRefV4(m_LastXRefOffset, m_dwXrefStartObjNum) && !LoadLinearizedAllCrossRefV5(m_LastXRefOffset)) {
        m_LastXRefOffset = 0;
        m_Syntax.m_MetadataObjnum = dwSaveMetadataObjnum;
        return PDFPARSE_ERROR_FORMAT;
    }
    FXSYS_qsort(m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    m_Syntax.m_MetadataObjnum = dwSaveMetadataObjnum;
    return PDFPARSE_ERROR_SUCCESS;
}

// static
int CPDF_SyntaxParser::s_CurrentRecursionDepth = 0;

CPDF_SyntaxParser::CPDF_SyntaxParser()
{
    m_pFileAccess = NULL;
    m_pCryptoHandler = NULL;
    m_pFileBuf = NULL;
    m_BufSize = CPDF_ModuleMgr::Get()->m_FileBufSize;
    m_pFileBuf = NULL;
    m_MetadataObjnum = 0;
    m_dwWordPos = 0;
    m_bFileStream = FALSE;
}
CPDF_SyntaxParser::~CPDF_SyntaxParser()
{
    if (m_pFileBuf) {
        FX_Free(m_pFileBuf);
    }
}
FX_BOOL CPDF_SyntaxParser::GetCharAt(FX_FILESIZE pos, FX_BYTE& ch)
{
    FX_FILESIZE save_pos = m_Pos;
    m_Pos = pos;
    FX_BOOL ret = GetNextChar(ch);
    m_Pos = save_pos;
    return ret;
}
FX_BOOL CPDF_SyntaxParser::GetNextChar(FX_BYTE& ch)
{
    FX_FILESIZE pos = m_Pos + m_HeaderOffset;
    if (pos >= m_FileLen) {
        return FALSE;
    }
    if (m_BufOffset >= pos || (FX_FILESIZE)(m_BufOffset + m_BufSize) <= pos) {
        FX_FILESIZE read_pos = pos;
        FX_DWORD read_size = m_BufSize;
        if ((FX_FILESIZE)read_size > m_FileLen) {
            read_size = (FX_DWORD)m_FileLen;
        }
        if ((FX_FILESIZE)(read_pos + read_size) > m_FileLen) {
            if (m_FileLen < (FX_FILESIZE)read_size) {
                read_pos = 0;
                read_size = (FX_DWORD)m_FileLen;
            } else {
                read_pos = m_FileLen - read_size;
            }
        }
        if (!m_pFileAccess->ReadBlock(m_pFileBuf, read_pos, read_size)) {
            return FALSE;
        }
        m_BufOffset = read_pos;
    }
    ch = m_pFileBuf[pos - m_BufOffset];
    m_Pos ++;
    return TRUE;
}
FX_BOOL CPDF_SyntaxParser::GetCharAtBackward(FX_FILESIZE pos, FX_BYTE& ch)
{
    pos += m_HeaderOffset;
    if (pos >= m_FileLen) {
        return FALSE;
    }
    if (m_BufOffset >= pos || (FX_FILESIZE)(m_BufOffset + m_BufSize) <= pos) {
        FX_FILESIZE read_pos;
        if (pos < (FX_FILESIZE)m_BufSize) {
            read_pos = 0;
        } else {
            read_pos = pos - m_BufSize + 1;
        }
        FX_DWORD read_size = m_BufSize;
        if ((FX_FILESIZE)(read_pos + read_size) > m_FileLen) {
            if (m_FileLen < (FX_FILESIZE)read_size) {
                read_pos = 0;
                read_size = (FX_DWORD)m_FileLen;
            } else {
                read_pos = m_FileLen - read_size;
            }
        }
        if (!m_pFileAccess->ReadBlock(m_pFileBuf, read_pos, read_size)) {
            return FALSE;
        }
        m_BufOffset = read_pos;
    }
    ch = m_pFileBuf[pos - m_BufOffset];
    return TRUE;
}
FX_BOOL CPDF_SyntaxParser::ReadBlock(FX_LPBYTE pBuf, FX_DWORD size)
{
    if (!m_pFileAccess->ReadBlock(pBuf, m_Pos + m_HeaderOffset, size)) {
        return FALSE;
    }
    m_Pos += size;
    return TRUE;
}
#define MAX_WORD_BUFFER 256
void CPDF_SyntaxParser::GetNextWord()
{
    m_WordSize = 0;
    m_bIsNumber = TRUE;
    FX_BYTE ch;
    if (!GetNextChar(ch)) {
        return;
    }
    FX_BYTE type = PDF_CharType[ch];
    while (1) {
        while (type == 'W') {
            if (!GetNextChar(ch)) {
                return;
            }
            type = PDF_CharType[ch];
        }
        if (ch != '%') {
            break;
        }
        while (1) {
            if (!GetNextChar(ch)) {
                return;
            }
            if (ch == '\r' || ch == '\n') {
                break;
            }
        }
        type = PDF_CharType[ch];
    }
    if (type == 'D') {
        m_bIsNumber = FALSE;
        m_WordBuffer[m_WordSize++] = ch;
        if (ch == '/') {
            while (1) {
                if (!GetNextChar(ch)) {
                    return;
                }
                type = PDF_CharType[ch];
                if (type != 'R' && type != 'N') {
                    m_Pos --;
                    return;
                }
                if (m_WordSize < MAX_WORD_BUFFER) {
                    m_WordBuffer[m_WordSize++] = ch;
                }
            }
        } else if (ch == '<') {
            if (!GetNextChar(ch)) {
                return;
            }
            if (ch == '<') {
                m_WordBuffer[m_WordSize++] = ch;
            } else {
                m_Pos --;
            }
        } else if (ch == '>') {
            if (!GetNextChar(ch)) {
                return;
            }
            if (ch == '>') {
                m_WordBuffer[m_WordSize++] = ch;
            } else {
                m_Pos --;
            }
        }
        return;
    }
    while (1) {
        if (m_WordSize < MAX_WORD_BUFFER) {
            m_WordBuffer[m_WordSize++] = ch;
        }
        if (type != 'N') {
            m_bIsNumber = FALSE;
        }
        if (!GetNextChar(ch)) {
            return;
        }
        type = PDF_CharType[ch];
        if (type == 'D' || type == 'W') {
            m_Pos --;
            break;
        }
    }
}
CFX_ByteString CPDF_SyntaxParser::ReadString()
{
    FX_BYTE ch;
    if (!GetNextChar(ch)) {
        return CFX_ByteString();
    }
    CFX_ByteTextBuf buf;
    FX_INT32 parlevel = 0;
    FX_INT32 status = 0, iEscCode = 0;
    while (1) {
        switch (status) {
            case 0:
                if (ch == ')') {
                    if (parlevel == 0) {
                        return buf.GetByteString();
                    }
                    parlevel --;
                    buf.AppendChar(')');
                } else if (ch == '(') {
                    parlevel ++;
                    buf.AppendChar('(');
                } else if (ch == '\\') {
                    status = 1;
                } else {
                    buf.AppendChar(ch);
                }
                break;
            case 1:
                if (ch >= '0' && ch <= '7') {
                    iEscCode = ch - '0';
                    status = 2;
                    break;
                }
                if (ch == 'n') {
                    buf.AppendChar('\n');
                } else if (ch == 'r') {
                    buf.AppendChar('\r');
                } else if (ch == 't') {
                    buf.AppendChar('\t');
                } else if (ch == 'b') {
                    buf.AppendChar('\b');
                } else if (ch == 'f') {
                    buf.AppendChar('\f');
                } else if (ch == '\r') {
                    status = 4;
                    break;
                } else if (ch == '\n') {
                } else {
                    buf.AppendChar(ch);
                }
                status = 0;
                break;
            case 2:
                if (ch >= '0' && ch <= '7') {
                    iEscCode = iEscCode * 8 + ch - '0';
                    status = 3;
                } else {
                    buf.AppendChar(iEscCode);
                    status = 0;
                    continue;
                }
                break;
            case 3:
                if (ch >= '0' && ch <= '7') {
                    iEscCode = iEscCode * 8 + ch - '0';
                    buf.AppendChar(iEscCode);
                    status = 0;
                } else {
                    buf.AppendChar(iEscCode);
                    status = 0;
                    continue;
                }
                break;
            case 4:
                status = 0;
                if (ch != '\n') {
                    continue;
                }
                break;
        }
        if (!GetNextChar(ch)) {
            break;
        }
    }
    GetNextChar(ch);
    return buf.GetByteString();
}
CFX_ByteString CPDF_SyntaxParser::ReadHexString()
{
    FX_BYTE ch;
    if (!GetNextChar(ch)) {
        return CFX_ByteString();
    }
    CFX_BinaryBuf buf;
    FX_BOOL bFirst = TRUE;
    FX_BYTE code = 0;
    while (1) {
        if (ch == '>') {
            break;
        }
        if (ch >= '0' && ch <= '9') {
            if (bFirst) {
                code = (ch - '0') * 16;
            } else {
                code += ch - '0';
                buf.AppendByte((FX_BYTE)code);
            }
            bFirst = !bFirst;
        } else if (ch >= 'A' && ch <= 'F') {
            if (bFirst) {
                code = (ch - 'A' + 10) * 16;
            } else {
                code += ch - 'A' + 10;
                buf.AppendByte((FX_BYTE)code);
            }
            bFirst = !bFirst;
        } else if (ch >= 'a' && ch <= 'f') {
            if (bFirst) {
                code = (ch - 'a' + 10) * 16;
            } else {
                code += ch - 'a' + 10;
                buf.AppendByte((FX_BYTE)code);
            }
            bFirst = !bFirst;
        }
        if (!GetNextChar(ch)) {
            break;
        }
    }
    if (!bFirst) {
        buf.AppendByte((FX_BYTE)code);
    }
    return buf.GetByteString();
}
void CPDF_SyntaxParser::ToNextLine()
{
    FX_BYTE ch;
    while (1) {
        if (!GetNextChar(ch)) {
            return;
        }
        if (ch == '\n') {
            return;
        }
        if (ch == '\r') {
            GetNextChar(ch);
            if (ch == '\n') {
                return;
            } else {
                m_Pos --;
                return;
            }
        }
    }
}
void CPDF_SyntaxParser::ToNextWord()
{
    FX_BYTE ch;
    if (!GetNextChar(ch)) {
        return;
    }
    FX_BYTE type = PDF_CharType[ch];
    while (1) {
        while (type == 'W') {
            m_dwWordPos = m_Pos;
            if (!GetNextChar(ch)) {
                return;
            }
            type = PDF_CharType[ch];
        }
        if (ch != '%') {
            break;
        }
        while (1) {
            if (!GetNextChar(ch)) {
                return;
            }
            if (ch == '\r' || ch == '\n') {
                break;
            }
        }
        type = PDF_CharType[ch];
    }
    m_Pos --;
}
CFX_ByteString CPDF_SyntaxParser::GetNextWord(FX_BOOL& bIsNumber)
{
    GetNextWord();
    bIsNumber = m_bIsNumber;
    return CFX_ByteString((FX_LPCSTR)m_WordBuffer, m_WordSize);
}
CFX_ByteString CPDF_SyntaxParser::GetKeyword()
{
    GetNextWord();
    return CFX_ByteString((FX_LPCSTR)m_WordBuffer, m_WordSize);
}
CPDF_Object* CPDF_SyntaxParser::GetObject(CPDF_IndirectObjects* pObjList, FX_DWORD objnum, FX_DWORD gennum, PARSE_CONTEXT* pContext, FX_BOOL bDecrypt)
{
    CFX_AutoRestorer<int> restorer(&s_CurrentRecursionDepth);
    if (++s_CurrentRecursionDepth > kParserMaxRecursionDepth) {
        return NULL;
    }
    FX_FILESIZE SavedPos = m_Pos;
    FX_BOOL bTypeOnly = pContext && (pContext->m_Flags & PDFPARSE_TYPEONLY);
    FX_BOOL bIsNumber;
    CFX_ByteString word = GetNextWord(bIsNumber);
    CPDF_Object* pRet = NULL;
    if (word.GetLength() == 0) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_INVALID;
        }
        return NULL;
    }
    if (bIsNumber) {
        FX_FILESIZE SavedPos = m_Pos;
        CFX_ByteString nextword = GetNextWord(bIsNumber);
        if (bIsNumber) {
            CFX_ByteString nextword2 = GetNextWord(bIsNumber);
            if (nextword2 == FX_BSTRC("R")) {
                FX_DWORD objnum = FXSYS_atoi(word);
                if (bTypeOnly) {
                    return (CPDF_Object*)PDFOBJ_REFERENCE;
                }
                pRet = CPDF_Reference::Create(pObjList, objnum);
                return pRet;
            } else {
                m_Pos = SavedPos;
                if (bTypeOnly) {
                    return (CPDF_Object*)PDFOBJ_NUMBER;
                }
                pRet = CPDF_Number::Create(word);
                return pRet;
            }
        } else {
            m_Pos = SavedPos;
            if (bTypeOnly) {
                return (CPDF_Object*)PDFOBJ_NUMBER;
            }
            pRet = CPDF_Number::Create(word);
            return pRet;
        }
    }
    if (word == FX_BSTRC("true") || word == FX_BSTRC("false")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_BOOLEAN;
        }
        pRet = CPDF_Boolean::Create(word == FX_BSTRC("true"));
        return pRet;
    }
    if (word == FX_BSTRC("null")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_NULL;
        }
        pRet = CPDF_Null::Create();
        return pRet;
    }
    if (word == FX_BSTRC("(")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_STRING;
        }
        CFX_ByteString str = ReadString();
        if (m_pCryptoHandler && bDecrypt) {
            m_pCryptoHandler->Decrypt(objnum, gennum, str);
        }
        pRet = CPDF_String::Create(str, FALSE);
        return pRet;
    }
    if (word == FX_BSTRC("<")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_STRING;
        }
        CFX_ByteString str = ReadHexString();
        if (m_pCryptoHandler && bDecrypt) {
            m_pCryptoHandler->Decrypt(objnum, gennum, str);
        }
        pRet = CPDF_String::Create(str, TRUE);
        return pRet;
    }
    if (word == FX_BSTRC("[")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_ARRAY;
        }
        CPDF_Array* pArray = CPDF_Array::Create();
        while (1) {
            CPDF_Object* pObj = GetObject(pObjList, objnum, gennum);
            if (pObj == NULL) {
                return pArray;
            }
            pArray->Add(pObj);
        }
    }
    if (word[0] == '/') {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_NAME;
        }
        pRet = CPDF_Name::Create(PDF_NameDecode(CFX_ByteStringC(m_WordBuffer + 1, m_WordSize - 1)));
        return pRet;
    }
    if (word == FX_BSTRC("<<")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_DICTIONARY;
        }
        if (pContext) {
            pContext->m_DictStart = SavedPos;
        }
        CPDF_Dictionary* pDict = CPDF_Dictionary::Create();
        FX_INT32 nKeys = 0;
        FX_FILESIZE dwSignValuePos = 0;
        while (1) {
            FX_BOOL bIsNumber;
            CFX_ByteString key = GetNextWord(bIsNumber);
            if (key.IsEmpty()) {
                if (pDict)
                    pDict->Release();
                return NULL;
            }
            FX_FILESIZE SavedPos = m_Pos - key.GetLength();
            if (key == FX_BSTRC(">>")) {
                break;
            }
            if (key == FX_BSTRC("endobj")) {
                m_Pos = SavedPos;
                break;
            }
            if (key[0] != '/') {
                continue;
            }
            nKeys ++;
            key = PDF_NameDecode(key);
            if (key == FX_BSTRC("/Contents")) {
                dwSignValuePos = m_Pos;
            }
            CPDF_Object* pObj = GetObject(pObjList, objnum, gennum);
            if (pObj == NULL) {
                continue;
            }
            if (key.GetLength() >= 1) {
                if (nKeys < 32) {
                    pDict->SetAt(CFX_ByteStringC(key.c_str() + 1, key.GetLength() - 1), pObj);
                } else {
                    pDict->AddValue(CFX_ByteStringC(key.c_str() + 1, key.GetLength() - 1), pObj);
                }
            }
        }
        if (IsSignatureDict(pDict)) {
            FX_FILESIZE dwSavePos = m_Pos;
            m_Pos = dwSignValuePos;
            CPDF_Object* pObj = GetObject(pObjList, objnum, gennum, NULL, FALSE);
            pDict->SetAt(FX_BSTRC("Contents"), pObj);
            m_Pos = dwSavePos;
        }
        if (pContext) {
            pContext->m_DictEnd = m_Pos;
            if (pContext->m_Flags & PDFPARSE_NOSTREAM) {
                return pDict;
            }
        }
        FX_FILESIZE SavedPos = m_Pos;
        FX_BOOL bIsNumber;
        CFX_ByteString nextword = GetNextWord(bIsNumber);
        if (nextword == FX_BSTRC("stream")) {
            CPDF_Stream* pStream = ReadStream(pDict, pContext, objnum, gennum);
            if (pStream) {
                return pStream;
            }
            if (pDict)
                pDict->Release();
            return NULL;
        } else {
            m_Pos = SavedPos;
            return pDict;
        }
    }
    if (word == FX_BSTRC(">>")) {
        m_Pos = SavedPos;
        return NULL;
    }
    if (bTypeOnly) {
        return (CPDF_Object*)PDFOBJ_INVALID;
    }
    return NULL;
}
CPDF_Object* CPDF_SyntaxParser::GetObjectByStrict(CPDF_IndirectObjects* pObjList, FX_DWORD objnum, FX_DWORD gennum, struct PARSE_CONTEXT* pContext)
{
    CFX_AutoRestorer<int> restorer(&s_CurrentRecursionDepth);
    if (++s_CurrentRecursionDepth > kParserMaxRecursionDepth) {
        return NULL;
    }
    FX_FILESIZE SavedPos = m_Pos;
    FX_BOOL bTypeOnly = pContext && (pContext->m_Flags & PDFPARSE_TYPEONLY);
    FX_BOOL bIsNumber;
    CFX_ByteString word = GetNextWord(bIsNumber);
    if (word.GetLength() == 0) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_INVALID;
        }
        return NULL;
    }
    if (bIsNumber) {
        FX_FILESIZE SavedPos = m_Pos;
        CFX_ByteString nextword = GetNextWord(bIsNumber);
        if (bIsNumber) {
            CFX_ByteString nextword2 = GetNextWord(bIsNumber);
            if (nextword2 == FX_BSTRC("R")) {
                FX_DWORD objnum = FXSYS_atoi(word);
                if (bTypeOnly) {
                    return (CPDF_Object*)PDFOBJ_REFERENCE;
                }
                return CPDF_Reference::Create(pObjList, objnum);
            } else {
                m_Pos = SavedPos;
                if (bTypeOnly) {
                    return (CPDF_Object*)PDFOBJ_NUMBER;
                }
                return CPDF_Number::Create(word);
            }
        } else {
            m_Pos = SavedPos;
            if (bTypeOnly) {
                return (CPDF_Object*)PDFOBJ_NUMBER;
            }
            return CPDF_Number::Create(word);
        }
    }
    if (word == FX_BSTRC("true") || word == FX_BSTRC("false")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_BOOLEAN;
        }
        return CPDF_Boolean::Create(word == FX_BSTRC("true"));
    }
    if (word == FX_BSTRC("null")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_NULL;
        }
        return CPDF_Null::Create();
    }
    if (word == FX_BSTRC("(")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_STRING;
        }
        CFX_ByteString str = ReadString();
        if (m_pCryptoHandler) {
            m_pCryptoHandler->Decrypt(objnum, gennum, str);
        }
        return CPDF_String::Create(str, FALSE);
    }
    if (word == FX_BSTRC("<")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_STRING;
        }
        CFX_ByteString str = ReadHexString();
        if (m_pCryptoHandler) {
            m_pCryptoHandler->Decrypt(objnum, gennum, str);
        }
        return CPDF_String::Create(str, TRUE);
    }
    if (word == FX_BSTRC("[")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_ARRAY;
        }
        CPDF_Array* pArray = CPDF_Array::Create();
        while (1) {
            CPDF_Object* pObj = GetObject(pObjList, objnum, gennum);
            if (pObj == NULL) {
                if (m_WordBuffer[0] == ']') {
                    return pArray;
                }
                if (pArray) {
                    pArray->Release();
                }
                return NULL;
            }
            pArray->Add(pObj);
        }
    }
    if (word[0] == '/') {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_NAME;
        }
        return CPDF_Name::Create(PDF_NameDecode(CFX_ByteStringC(m_WordBuffer + 1, m_WordSize - 1)));
    }
    if (word == FX_BSTRC("<<")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_DICTIONARY;
        }
        if (pContext) {
            pContext->m_DictStart = SavedPos;
        }
        CPDF_Dictionary* pDict = CPDF_Dictionary::Create();
        while (1) {
            FX_BOOL bIsNumber;
            FX_FILESIZE SavedPos = m_Pos;
            CFX_ByteString key = GetNextWord(bIsNumber);
            if (key.IsEmpty()) {
                if (pDict) {
                    pDict->Release();
                }
                return NULL;
            }
            if (key == FX_BSTRC(">>")) {
                break;
            }
            if (key == FX_BSTRC("endobj")) {
                m_Pos = SavedPos;
                break;
            }
            if (key[0] != '/') {
                continue;
            }
            key = PDF_NameDecode(key);
            CPDF_Object* pObj = GetObject(pObjList, objnum, gennum);
            if (pObj == NULL) {
                if (pDict) {
                    pDict->Release();
                }
                FX_BYTE ch;
                while (1) {
                    if (!GetNextChar(ch)) {
                        break;
                    }
                    if (ch == 0x0A || ch == 0x0D) {
                        break;
                    }
                }
                return NULL;
            }
            if (key.GetLength() > 1) {
                pDict->AddValue(CFX_ByteStringC(key.c_str() + 1, key.GetLength() - 1), pObj);
            } 
        }
        if (pContext) {
            pContext->m_DictEnd = m_Pos;
            if (pContext->m_Flags & PDFPARSE_NOSTREAM) {
                return pDict;
            }
        }
        FX_FILESIZE SavedPos = m_Pos;
        FX_BOOL bIsNumber;
        CFX_ByteString nextword = GetNextWord(bIsNumber);
        if (nextword == FX_BSTRC("stream")) {
            CPDF_Stream* pStream = ReadStream(pDict, pContext, objnum, gennum);
            if (pStream) {
                return pStream;
            }
            if (pDict) {
                pDict->Release();
            }
            return NULL;
        } else {
            m_Pos = SavedPos;
            return pDict;
        }
    }
    if (word == FX_BSTRC(">>")) {
        m_Pos = SavedPos;
        return NULL;
    }
    if (bTypeOnly) {
        return (CPDF_Object*)PDFOBJ_INVALID;
    }
    return NULL;
}
CPDF_Stream* CPDF_SyntaxParser::ReadStream(CPDF_Dictionary* pDict, PARSE_CONTEXT* pContext,
        FX_DWORD objnum, FX_DWORD gennum)
{
    CPDF_Object* pLenObj = pDict->GetElement(FX_BSTRC("Length"));
    FX_FILESIZE len = 0;
    if (pLenObj && ((pLenObj->GetType() != PDFOBJ_REFERENCE) ||
                    ((((CPDF_Reference*)pLenObj)->GetObjList() != NULL) &&
                     ((CPDF_Reference*)pLenObj)->GetRefObjNum() != objnum))) {
        len = pLenObj->GetInteger();
    }

    ToNextLine();
    FX_FILESIZE StreamStartPos = m_Pos;
    if (pContext) {
        pContext->m_DataStart = m_Pos;
    }

    CPDF_CryptoHandler* pCryptoHandler = objnum == (FX_DWORD)m_MetadataObjnum ? NULL : m_pCryptoHandler;
    if (pCryptoHandler == NULL) {
        pdfium::base::CheckedNumeric<FX_FILESIZE> pos = m_Pos;
        pos += len;
        if (pos.IsValid() && pos.ValueOrDie() < m_FileLen) {
            m_Pos = pos.ValueOrDie();
        }
        GetNextWord();
        if (m_WordSize < 9 || FXSYS_memcmp32(m_WordBuffer, "endstream", 9)) {
            m_Pos = StreamStartPos;
            FX_FILESIZE offset = FindTag(FX_BSTRC("endstream"), 0);
            if (offset >= 0) {
                FX_FILESIZE curPos = m_Pos;
                m_Pos = StreamStartPos;
                FX_FILESIZE endobjOffset = FindTag(FX_BSTRC("endobj"), 0);
                if (endobjOffset < offset && endobjOffset >= 0) {
                    offset = endobjOffset;
                } else {
                    m_Pos = curPos;
                }
                FX_BYTE byte1, byte2;
                GetCharAt(StreamStartPos + offset - 1, byte1);
                GetCharAt(StreamStartPos + offset - 2, byte2);
                if (byte1 == 0x0a && byte2 == 0x0d) {
                    len -= 2;
                } else if (byte1 == 0x0a || byte1 == 0x0d) {
                    len --;
                }
                len = (FX_DWORD)offset;
                pDict->SetAtInteger(FX_BSTRC("Length"), len);
            } else {
                m_Pos = StreamStartPos;
                if (FindTag(FX_BSTRC("endobj"), 0) < 0) {
                    return NULL;
                }
            }
        }
        m_Pos = StreamStartPos;
    }
    CPDF_Stream* pStream;
    FX_LPBYTE pData = FX_Alloc(FX_BYTE, len);
    ReadBlock(pData, len);
    if (pCryptoHandler) {
        CFX_BinaryBuf dest_buf;
        dest_buf.EstimateSize(pCryptoHandler->DecryptGetSize(len));
        FX_LPVOID context = pCryptoHandler->DecryptStart(objnum, gennum);
        pCryptoHandler->DecryptStream(context, pData, len, dest_buf);
        pCryptoHandler->DecryptFinish(context, dest_buf);
        FX_Free(pData);
        pData = dest_buf.GetBuffer();
        len = dest_buf.GetSize();
        dest_buf.DetachBuffer();
    }
    pStream = new CPDF_Stream(pData, len, pDict);
    if (pContext) {
        pContext->m_DataEnd = pContext->m_DataStart + len;
    }
    StreamStartPos = m_Pos;
    GetNextWord();
    if (m_WordSize == 6 && 0 == FXSYS_memcmp32(m_WordBuffer, "endobj", 6)) {
        m_Pos = StreamStartPos;
    }
    return pStream;
}
void CPDF_SyntaxParser::InitParser(IFX_FileRead* pFileAccess, FX_DWORD HeaderOffset)
{
    if (m_pFileBuf) {
        FX_Free(m_pFileBuf);
        m_pFileBuf = NULL;
    }
    m_pFileBuf = FX_Alloc(FX_BYTE, m_BufSize);
    m_HeaderOffset = HeaderOffset;
    m_FileLen = pFileAccess->GetSize();
    m_Pos = 0;
    m_pFileAccess = pFileAccess;
    m_BufOffset = 0;
    pFileAccess->ReadBlock(m_pFileBuf, 0, (size_t)((FX_FILESIZE)m_BufSize > m_FileLen ? m_FileLen : m_BufSize));
}
FX_INT32 CPDF_SyntaxParser::GetDirectNum()
{
    GetNextWord();
    if (!m_bIsNumber) {
        return 0;
    }
    m_WordBuffer[m_WordSize] = 0;
    return FXSYS_atoi((FX_LPCSTR)m_WordBuffer);
}
FX_BOOL CPDF_SyntaxParser::IsWholeWord(FX_FILESIZE startpos, FX_FILESIZE limit, FX_LPCBYTE tag, FX_DWORD taglen)
{
    FX_BYTE type = PDF_CharType[tag[0]];
    FX_BOOL bCheckLeft = type != 'D' && type != 'W';
    type = PDF_CharType[tag[taglen - 1]];
    FX_BOOL bCheckRight = type != 'D' && type != 'W';
    FX_BYTE ch;
    if (bCheckRight && startpos + (FX_INT32)taglen <= limit && GetCharAt(startpos + (FX_INT32)taglen, ch)) {
        FX_BYTE type = PDF_CharType[ch];
        if (type == 'N' || type == 'R') {
            return FALSE;
        }
    }
    if (bCheckLeft && startpos > 0 && GetCharAt(startpos - 1, ch)) {
        FX_BYTE type = PDF_CharType[ch];
        if (type == 'N' || type == 'R') {
            return FALSE;
        }
    }
    return TRUE;
}
FX_BOOL CPDF_SyntaxParser::SearchWord(FX_BSTR tag, FX_BOOL bWholeWord, FX_BOOL bForward, FX_FILESIZE limit)
{
    FX_INT32 taglen = tag.GetLength();
    if (taglen == 0) {
        return FALSE;
    }
    FX_FILESIZE pos = m_Pos;
    FX_INT32 offset = 0;
    if (!bForward) {
        offset = taglen - 1;
    }
    FX_LPCBYTE tag_data = tag.GetPtr();
    FX_BYTE byte;
    while (1) {
        if (bForward) {
            if (limit) {
                if (pos >= m_Pos + limit) {
                    return FALSE;
                }
            }
            if (!GetCharAt(pos, byte)) {
                return FALSE;
            }
        } else {
            if (limit) {
                if (pos <= m_Pos - limit) {
                    return FALSE;
                }
            }
            if (!GetCharAtBackward(pos, byte)) {
                return FALSE;
            }
        }
        if (byte == tag_data[offset]) {
            if (bForward) {
                offset ++;
                if (offset < taglen) {
                    pos ++;
                    continue;
                }
            } else {
                offset --;
                if (offset >= 0) {
                    pos --;
                    continue;
                }
            }
            FX_FILESIZE startpos = bForward ? pos - taglen + 1 : pos;
            if (!bWholeWord || IsWholeWord(startpos, limit, tag.GetPtr(), taglen)) {
                m_Pos = startpos;
                return TRUE;
            }
        }
        if (bForward) {
            offset = byte == tag_data[0] ? 1 : 0;
            pos ++;
        } else {
            offset = byte == tag_data[taglen - 1] ? taglen - 2 : taglen - 1;
            pos --;
        }
        if (pos < 0) {
            return FALSE;
        }
    }
    return FALSE;
}
struct _SearchTagRecord {
    FX_LPCBYTE	m_pTag;
    FX_DWORD	m_Len;
    FX_DWORD	m_Offset;
};
FX_INT32 CPDF_SyntaxParser::SearchMultiWord(FX_BSTR tags, FX_BOOL bWholeWord, FX_FILESIZE limit)
{
    FX_INT32 ntags = 1, i;
    for (i = 0; i < tags.GetLength(); i ++)
        if (tags[i] == 0) {
            ntags ++;
        }
    _SearchTagRecord* pPatterns = FX_Alloc(_SearchTagRecord, ntags);
    FX_DWORD start = 0, itag = 0, max_len = 0;
    for (i = 0; i <= tags.GetLength(); i ++) {
        if (tags[i] == 0) {
            FX_DWORD len = i - start;
            if (len > max_len) {
                max_len = len;
            }
            pPatterns[itag].m_pTag = tags.GetPtr() + start;
            pPatterns[itag].m_Len = len;
            pPatterns[itag].m_Offset = 0;
            start = i + 1;
            itag ++;
        }
    }
    FX_FILESIZE pos = m_Pos;
    FX_BYTE byte;
    GetCharAt(pos++, byte);
    FX_INT32 found = -1;
    while (1) {
        for (i = 0; i < ntags; i ++) {
            if (pPatterns[i].m_pTag[pPatterns[i].m_Offset] == byte) {
                pPatterns[i].m_Offset ++;
                if (pPatterns[i].m_Offset == pPatterns[i].m_Len) {
                    if (!bWholeWord || IsWholeWord(pos - pPatterns[i].m_Len, limit, pPatterns[i].m_pTag, pPatterns[i].m_Len)) {
                        found = i;
                        goto end;
                    } else {
                        if (pPatterns[i].m_pTag[0] == byte) {
                            pPatterns[i].m_Offset = 1;
                        } else {
                            pPatterns[i].m_Offset = 0;
                        }
                    }
                }
            } else {
                if (pPatterns[i].m_pTag[0] == byte) {
                    pPatterns[i].m_Offset = 1;
                } else {
                    pPatterns[i].m_Offset = 0;
                }
            }
        }
        if (limit && pos >= m_Pos + limit) {
            goto end;
        }
        if (!GetCharAt(pos, byte)) {
            goto end;
        }
        pos ++;
    }
end:
    FX_Free(pPatterns);
    return found;
}
FX_FILESIZE CPDF_SyntaxParser::FindTag(FX_BSTR tag, FX_FILESIZE limit)
{
    FX_INT32 taglen = tag.GetLength();
    FX_INT32 match = 0;
    limit += m_Pos;
    FX_FILESIZE startpos = m_Pos;
    while (1) {
        FX_BYTE ch;
        if (!GetNextChar(ch)) {
            return -1;
        }
        if (ch == tag[match]) {
            match ++;
            if (match == taglen) {
                return m_Pos - startpos - taglen;
            }
        } else {
            match = ch == tag[0] ? 1 : 0;
        }
        if (limit && m_Pos == limit) {
            return -1;
        }
    }
    return -1;
}
void CPDF_SyntaxParser::GetBinary(FX_BYTE* buffer, FX_DWORD size)
{
    FX_DWORD offset = 0;
    FX_BYTE ch;
    while (1) {
        if (!GetNextChar(ch)) {
            return;
        }
        buffer[offset++] = ch;
        if (offset == size) {
            break;
        }
    }
}

class CPDF_DataAvail FX_FINAL : public IPDF_DataAvail
{
public:
    CPDF_DataAvail(IFX_FileAvail* pFileAvail, IFX_FileRead* pFileRead);
    ~CPDF_DataAvail();

    virtual FX_BOOL                     IsDocAvail(IFX_DownloadHints* pHints)  FX_OVERRIDE;

    virtual void                        SetDocument(CPDF_Document* pDoc)  FX_OVERRIDE;

    virtual FX_BOOL                     IsPageAvail(int iPage, IFX_DownloadHints* pHints)  FX_OVERRIDE;

    virtual FX_INT32                    IsFormAvail(IFX_DownloadHints *pHints)  FX_OVERRIDE;

    virtual FX_INT32                    IsLinearizedPDF()  FX_OVERRIDE;

    virtual FX_BOOL                     IsLinearized()  FX_OVERRIDE
    {
        return m_bLinearized;
    }

    virtual void                        GetLinearizedMainXRefInfo(FX_FILESIZE *pPos, FX_DWORD *pSize)  FX_OVERRIDE;

protected:
    static const int kMaxDataAvailRecursionDepth = 64;
    static int s_CurrentDataAvailRecursionDepth;

    FX_DWORD                            GetObjectSize(FX_DWORD objnum, FX_FILESIZE& offset);
    FX_BOOL                             IsObjectsAvail(CFX_PtrArray& obj_array, FX_BOOL bParsePage, IFX_DownloadHints* pHints, CFX_PtrArray &ret_array);
    FX_BOOL                             CheckDocStatus(IFX_DownloadHints *pHints);
    FX_BOOL                             CheckHeader(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckFirstPage(IFX_DownloadHints *pHints);
    FX_BOOL                             CheckEnd(IFX_DownloadHints *pHints);
    FX_BOOL                             CheckCrossRef(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckCrossRefItem(IFX_DownloadHints *pHints);
    FX_BOOL                             CheckTrailer(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckRoot(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckInfo(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckPages(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckPage(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckResources(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckAnnots(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckAcroForm(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckAcroFormSubObject(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckTrailerAppend(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckPageStatus(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckAllCrossRefStream(IFX_DownloadHints *pHints);

    FX_INT32                            CheckCrossRefStream(IFX_DownloadHints *pHints, FX_FILESIZE &xref_offset);
    FX_BOOL                             IsLinearizedFile(FX_LPBYTE pData, FX_DWORD dwLen);
    void                                SetStartOffset(FX_FILESIZE dwOffset);
    FX_BOOL                             GetNextToken(CFX_ByteString &token);
    FX_BOOL                             GetNextChar(FX_BYTE &ch);
    CPDF_Object	*                       ParseIndirectObjectAt(FX_FILESIZE pos, FX_DWORD objnum);
    CPDF_Object	*                       GetObject(FX_DWORD objnum, IFX_DownloadHints* pHints, FX_BOOL *pExistInFile);
    FX_BOOL                             GetPageKids(CPDF_Parser *pParser, CPDF_Object *pPages);
    FX_BOOL                             PreparePageItem();
    FX_BOOL                             LoadPages(IFX_DownloadHints* pHints);
    FX_BOOL                             LoadAllXref(IFX_DownloadHints* pHints);
    FX_BOOL                             LoadAllFile(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckLinearizedData(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckFileResources(IFX_DownloadHints* pHints);
    FX_BOOL                             CheckPageAnnots(int iPage, IFX_DownloadHints* pHints);

    FX_BOOL                             CheckLinearizedFirstPage(int iPage, IFX_DownloadHints* pHints);
    FX_BOOL                             HaveResourceAncestor(CPDF_Dictionary *pDict);
    FX_BOOL                             CheckPage(FX_INT32 iPage, IFX_DownloadHints* pHints);
    FX_BOOL                             LoadDocPages(IFX_DownloadHints* pHints);
    FX_BOOL                             LoadDocPage(FX_INT32 iPage, IFX_DownloadHints* pHints);
    FX_BOOL                             CheckPageNode(CPDF_PageNode &pageNodes, FX_INT32 iPage, FX_INT32 &iCount, IFX_DownloadHints* pHints);
    FX_BOOL                             CheckUnkownPageNode(FX_DWORD dwPageNo, CPDF_PageNode *pPageNode, IFX_DownloadHints* pHints);
    FX_BOOL                             CheckArrayPageNode(FX_DWORD dwPageNo, CPDF_PageNode *pPageNode, IFX_DownloadHints* pHints);
    FX_BOOL                             CheckPageCount(IFX_DownloadHints* pHints);
    FX_BOOL                             IsFirstCheck(int iPage);
    void                                ResetFirstCheck(int iPage);

    CPDF_Parser                         m_parser;

    CPDF_SyntaxParser                   m_syntaxParser;

    CPDF_Object                         *m_pRoot;

    FX_DWORD                            m_dwRootObjNum;

    FX_DWORD                            m_dwInfoObjNum;

    CPDF_Object                         *m_pLinearized;

    CPDF_Object                         *m_pTrailer;

    FX_BOOL                             m_bDocAvail;

    FX_FILESIZE                         m_dwHeaderOffset;

    FX_FILESIZE                         m_dwLastXRefOffset;

    FX_FILESIZE                         m_dwXRefOffset;

    FX_FILESIZE                         m_dwTrailerOffset;

    FX_FILESIZE                         m_dwCurrentOffset;

    PDF_DATAAVAIL_STATUS                m_docStatus;

    FX_FILESIZE	                        m_dwFileLen;

    CPDF_Document*                      m_pDocument;

    CPDF_SortObjNumArray                m_objnum_array;

    CFX_PtrArray                        m_objs_array;

    FX_FILESIZE	                        m_Pos;

    FX_FILESIZE                         m_bufferOffset;

    FX_DWORD                            m_bufferSize;

    CFX_ByteString                      m_WordBuf;

    FX_BYTE                             m_WordBuffer[257];

    FX_DWORD                            m_WordSize;

    FX_BYTE                             m_bufferData[512];

    CFX_FileSizeArray                   m_CrossOffset;

    CFX_DWordArray                      m_XRefStreamList;

    CFX_DWordArray                      m_PageObjList;

    FX_DWORD                            m_PagesObjNum;

    FX_BOOL                             m_bLinearized;

    FX_DWORD                            m_dwFirstPageNo;

    FX_BOOL                             m_bLinearedDataOK;

    FX_BOOL                             m_bMainXRefLoadTried;

    FX_BOOL                             m_bMainXRefLoadedOK;

    FX_BOOL                             m_bPagesTreeLoad;

    FX_BOOL                             m_bPagesLoad;

    CPDF_Parser *                       m_pCurrentParser;

    FX_FILESIZE                         m_dwCurrentXRefSteam;

    FX_BOOL                             m_bAnnotsLoad;

    FX_BOOL                             m_bHaveAcroForm;

    FX_DWORD                            m_dwAcroFormObjNum;

    FX_BOOL                             m_bAcroFormLoad;

    CPDF_Object	*                       m_pAcroForm;

    CFX_PtrArray                        m_arrayAcroforms;

    CPDF_Dictionary *                   m_pPageDict;

    CPDF_Object *                       m_pPageResource;

    FX_BOOL                             m_bNeedDownLoadResource;

    FX_BOOL                             m_bPageLoadedOK;

    FX_BOOL                             m_bLinearizedFormParamLoad;

    CFX_PtrArray                        m_PagesArray;

    FX_DWORD                            m_dwEncryptObjNum;

    FX_FILESIZE                         m_dwPrevXRefOffset;

    FX_BOOL                             m_bTotalLoadPageTree;

    FX_BOOL                             m_bCurPageDictLoadOK;

    CPDF_PageNode                       m_pageNodes;

    CFX_CMapDWordToDWord *              m_pageMapCheckState;

    CFX_CMapDWordToDWord *              m_pagesLoadState;
};

IPDF_DataAvail::IPDF_DataAvail(IFX_FileAvail* pFileAvail, IFX_FileRead* pFileRead) :
    m_pFileAvail(pFileAvail),
    m_pFileRead(pFileRead) {
}

// static
IPDF_DataAvail* IPDF_DataAvail::Create(IFX_FileAvail* pFileAvail, IFX_FileRead* pFileRead)
{
  return new CPDF_DataAvail(pFileAvail, pFileRead);
}

// static
int CPDF_DataAvail::s_CurrentDataAvailRecursionDepth = 0;

CPDF_DataAvail::CPDF_DataAvail(IFX_FileAvail* pFileAvail, IFX_FileRead* pFileRead)
    : IPDF_DataAvail(pFileAvail, pFileRead)
{
    m_Pos = 0;
    m_dwFileLen = 0;
    if (m_pFileRead) {
        m_dwFileLen = (FX_DWORD)m_pFileRead->GetSize();
    }
    m_dwCurrentOffset = 0;
    m_WordSize = 0;
    m_dwXRefOffset = 0;
    m_bufferOffset = 0;
    m_dwFirstPageNo = 0;
    m_bufferSize = 0;
    m_PagesObjNum = 0;
    m_dwCurrentXRefSteam = 0;
    m_dwAcroFormObjNum = 0;
    m_dwInfoObjNum = 0;
    m_pDocument = 0;
    m_dwEncryptObjNum = 0;
    m_dwPrevXRefOffset = 0;
    m_dwLastXRefOffset = 0;
    m_bDocAvail = FALSE;
    m_bMainXRefLoadTried = FALSE;
    m_bDocAvail = FALSE;
    m_bLinearized = FALSE;
    m_bPagesLoad = FALSE;
    m_bPagesTreeLoad = FALSE;
    m_bMainXRefLoadedOK = FALSE;
    m_bAnnotsLoad = FALSE;
    m_bHaveAcroForm = FALSE;
    m_bAcroFormLoad = FALSE;
    m_bPageLoadedOK = FALSE;
    m_bNeedDownLoadResource = FALSE;
    m_bLinearizedFormParamLoad = FALSE;
    m_pLinearized = NULL;
    m_pRoot = NULL;
    m_pTrailer = NULL;
    m_pCurrentParser = NULL;
    m_pAcroForm = NULL;
    m_pPageDict = NULL;
    m_pPageResource = NULL;
    m_pageMapCheckState = NULL;
    m_docStatus = PDF_DATAAVAIL_HEADER;
    m_parser.m_bOwnFileRead = FALSE;
    m_bTotalLoadPageTree = FALSE;
    m_bCurPageDictLoadOK = FALSE;
    m_bLinearedDataOK = FALSE;
    m_pagesLoadState = NULL;
}
CPDF_DataAvail::~CPDF_DataAvail()
{
    if (m_pLinearized)	{
        m_pLinearized->Release();
    }
    if (m_pRoot) {
        m_pRoot->Release();
    }
    if (m_pTrailer) {
        m_pTrailer->Release();
    }
    if (m_pageMapCheckState) {
        delete m_pageMapCheckState;
    }
    if (m_pagesLoadState) {
        delete m_pagesLoadState;
    }
    FX_INT32 i = 0;
    FX_INT32 iSize = m_arrayAcroforms.GetSize();
    for (i = 0; i < iSize; ++i) {
        ((CPDF_Object *)m_arrayAcroforms.GetAt(i))->Release();
    }
}
void CPDF_DataAvail::SetDocument(CPDF_Document* pDoc)
{
    m_pDocument = pDoc;
}
FX_DWORD CPDF_DataAvail::GetObjectSize(FX_DWORD objnum, FX_FILESIZE& offset)
{
    CPDF_Parser *pParser = (CPDF_Parser *)(m_pDocument->GetParser());
    if (pParser == NULL) {
        return 0;
    }
    if (objnum >= (FX_DWORD)pParser->m_CrossRef.GetSize()) {
        return 0;
    }
    if (pParser->m_V5Type[objnum] == 2) {
        objnum = (FX_DWORD)pParser->m_CrossRef[objnum];
    }
    if (pParser->m_V5Type[objnum] == 1 || pParser->m_V5Type[objnum] == 255) {
        offset = pParser->m_CrossRef[objnum];
        if (offset == 0) {
            return 0;
        }
        FX_LPVOID pResult = FXSYS_bsearch(&offset, pParser->m_SortedOffset.GetData(), pParser->m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
        if (pResult == NULL) {
            return 0;
        }
        if ((FX_FILESIZE*)pResult - (FX_FILESIZE*)pParser->m_SortedOffset.GetData() == pParser->m_SortedOffset.GetSize() - 1) {
            return 0;
        }
        return (FX_DWORD)(((FX_FILESIZE*)pResult)[1] - offset);
    }
    return 0;
}
FX_BOOL CPDF_DataAvail::IsObjectsAvail(CFX_PtrArray& obj_array, FX_BOOL bParsePage, IFX_DownloadHints* pHints, CFX_PtrArray &ret_array)
{
    if (!obj_array.GetSize()) {
        return TRUE;
    }
    FX_DWORD count = 0;
    CFX_PtrArray new_obj_array;
    FX_INT32 i = 0;
    for (i = 0; i < obj_array.GetSize(); i++) {
        CPDF_Object *pObj = (CPDF_Object *)obj_array[i];
        if (!pObj) {
            continue;
        }
        FX_INT32 type = pObj->GetType();
        switch (type) {
            case PDFOBJ_ARRAY: {
                    CPDF_Array *pArray = pObj->GetArray();
                    for (FX_DWORD k = 0; k < pArray->GetCount(); k++) {
                        new_obj_array.Add(pArray->GetElement(k));
                    }
                }
                break;
            case PDFOBJ_STREAM:
                pObj = pObj->GetDict();
            case PDFOBJ_DICTIONARY: {
                    CPDF_Dictionary *pDict = pObj->GetDict();
                    if (pDict && pDict->GetString("Type") == "Page" && !bParsePage) {
                        continue;
                    }
                    FX_POSITION pos = pDict->GetStartPos();
                    while (pos) {
                        CPDF_Object *value;
                        CFX_ByteString key;
                        value = pDict->GetNextElement(pos, key);
                        if (key != "Parent") {
                            new_obj_array.Add(value);
                        }
                    }
                }
                break;
            case PDFOBJ_REFERENCE: {
                    CPDF_Reference *pRef = (CPDF_Reference*)pObj;
                    FX_DWORD dwNum = pRef->GetRefObjNum();
                    FX_FILESIZE offset;
                    FX_DWORD original_size = GetObjectSize(dwNum, offset);
                    pdfium::base::CheckedNumeric<FX_DWORD> size = original_size;
                    if (size.ValueOrDefault(0) == 0 || offset < 0 || offset >= m_dwFileLen) {
                        break;
                    }
                    
                    size += offset;
                    size += 512;
                    if (!size.IsValid()) {
                        break;
                    }
                    if (size.ValueOrDie() > m_dwFileLen) {
                        size = m_dwFileLen - offset;
                    } else {
                        size = original_size + 512;
                    }
                    if (!size.IsValid()) {
                        break;
                    }
                    if (!m_pFileAvail->IsDataAvail(offset, size.ValueOrDie())) {
                        pHints->AddSegment(offset, size.ValueOrDie());
                        ret_array.Add(pObj);
                        count++;
                    } else if (!m_objnum_array.Find(dwNum)) {
                        m_objnum_array.AddObjNum(dwNum);
                        CPDF_Object *pReferred = m_pDocument->GetIndirectObject(pRef->GetRefObjNum(), NULL);
                        if (pReferred) {
                            new_obj_array.Add(pReferred);
                        }
                    }
                }
                break;
        }
    }
    if (count > 0) {
        FX_INT32 iSize = new_obj_array.GetSize();
        for (i = 0; i < iSize; ++i) {
            CPDF_Object *pObj = (CPDF_Object *)new_obj_array[i];
            FX_INT32 type = pObj->GetType();
            if (type == PDFOBJ_REFERENCE) {
                CPDF_Reference *pRef = (CPDF_Reference *)pObj;
                FX_DWORD dwNum = pRef->GetRefObjNum();
                if (!m_objnum_array.Find(dwNum)) {
                    ret_array.Add(pObj);
                }
            } else {
                ret_array.Add(pObj);
            }
        }
        return FALSE;
    }
    obj_array.RemoveAll();
    obj_array.Append(new_obj_array);
    return IsObjectsAvail(obj_array, FALSE, pHints, ret_array);
}
FX_BOOL CPDF_DataAvail::IsDocAvail(IFX_DownloadHints* pHints)
{
    if (!m_dwFileLen && m_pFileRead) {
        m_dwFileLen = (FX_DWORD)m_pFileRead->GetSize();
        if (!m_dwFileLen) {
            return TRUE;
        }
    }
    while (!m_bDocAvail) {
        if (!CheckDocStatus(pHints)) {
            return FALSE;
        }
    }
    return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckAcroFormSubObject(IFX_DownloadHints* pHints)
{
    if (!m_objs_array.GetSize()) {
        m_objs_array.RemoveAll();
        m_objnum_array.RemoveAll();
        CFX_PtrArray obj_array;
        obj_array.Append(m_arrayAcroforms);
        FX_BOOL bRet = IsObjectsAvail(obj_array, FALSE, pHints, m_objs_array);
        if (bRet) {
            m_objs_array.RemoveAll();
        }
        return bRet;
    } else {
        CFX_PtrArray new_objs_array;
        FX_BOOL bRet = IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
        if (bRet) {
            FX_INT32 iSize = m_arrayAcroforms.GetSize();
            for (FX_INT32 i = 0; i < iSize; ++i) {
                ((CPDF_Object *)m_arrayAcroforms.GetAt(i))->Release();
            }
            m_arrayAcroforms.RemoveAll();
        } else {
            m_objs_array.RemoveAll();
            m_objs_array.Append(new_objs_array);
        }
        return bRet;
    }
}
FX_BOOL CPDF_DataAvail::CheckAcroForm(IFX_DownloadHints* pHints)
{
    FX_BOOL bExist = FALSE;
    m_pAcroForm = GetObject(m_dwAcroFormObjNum, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_PAGETREE;
        return TRUE;
    }
    if (!m_pAcroForm) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
            return TRUE;
        }
        return FALSE;
    }
    m_arrayAcroforms.Add(m_pAcroForm);
    m_docStatus = PDF_DATAAVAIL_PAGETREE;
    return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckDocStatus(IFX_DownloadHints *pHints)
{
    switch (m_docStatus) {
        case PDF_DATAAVAIL_HEADER:
            return CheckHeader(pHints);
        case PDF_DATAAVAIL_FIRSTPAGE:
        case PDF_DATAAVAIL_FIRSTPAGE_PREPARE:
            return CheckFirstPage(pHints);
        case PDF_DATAAVAIL_END:
            return CheckEnd(pHints);
        case PDF_DATAAVAIL_CROSSREF:
            return CheckCrossRef(pHints);
        case PDF_DATAAVAIL_CROSSREF_ITEM:
            return CheckCrossRefItem(pHints);
        case PDF_DATAAVAIL_CROSSREF_STREAM:
            return CheckAllCrossRefStream(pHints);
        case PDF_DATAAVAIL_TRAILER:
            return CheckTrailer(pHints);
        case PDF_DATAAVAIL_TRAILER_APPEND:
            return CheckTrailerAppend(pHints);
        case PDF_DATAAVAIL_LOADALLCRSOSSREF:
            return LoadAllXref(pHints);
        case PDF_DATAAVAIL_LOADALLFILE:
            return LoadAllFile(pHints);
        case PDF_DATAAVAIL_ROOT:
            return CheckRoot(pHints);
        case PDF_DATAAVAIL_INFO:
            return CheckInfo(pHints);
        case PDF_DATAAVAIL_ACROFORM:
            return CheckAcroForm(pHints);
        case PDF_DATAAVAIL_PAGETREE:
            if (m_bTotalLoadPageTree) {
                return CheckPages(pHints);
            } else {
                return LoadDocPages(pHints);
            }
        case PDF_DATAAVAIL_PAGE:
            if (m_bTotalLoadPageTree) {
                return CheckPage(pHints);
            } else {
                m_docStatus = PDF_DATAAVAIL_PAGE_LATERLOAD;
                return TRUE;
            }
        case PDF_DATAAVAIL_ERROR:
            return LoadAllFile(pHints);
        case PDF_DATAAVAIL_PAGE_LATERLOAD:
            m_docStatus = PDF_DATAAVAIL_PAGE;
        default:
            m_bDocAvail = TRUE;
            return TRUE;
    }
}
FX_BOOL	CPDF_DataAvail::CheckPageStatus(IFX_DownloadHints* pHints)
{
    switch (m_docStatus) {
        case PDF_DATAAVAIL_PAGETREE:
            return CheckPages(pHints);
        case PDF_DATAAVAIL_PAGE:
            return CheckPage(pHints);
        case PDF_DATAAVAIL_ERROR:
            return LoadAllFile(pHints);
        default:
            m_bPagesTreeLoad = TRUE;
            m_bPagesLoad = TRUE;
            return TRUE;
    }
}
FX_BOOL CPDF_DataAvail::LoadAllFile(IFX_DownloadHints* pHints)
{
    if (m_pFileAvail->IsDataAvail(0, (FX_DWORD)m_dwFileLen)) {
        m_docStatus = PDF_DATAAVAIL_DONE;
        return TRUE;
    }
    pHints->AddSegment(0, (FX_DWORD)m_dwFileLen);
    return FALSE;
}
FX_BOOL CPDF_DataAvail::LoadAllXref(IFX_DownloadHints* pHints)
{
    m_parser.m_Syntax.InitParser(m_pFileRead, (FX_DWORD)m_dwHeaderOffset);
    m_parser.m_bOwnFileRead = FALSE;
    if (!m_parser.LoadAllCrossRefV4(m_dwLastXRefOffset) && !m_parser.LoadAllCrossRefV5(m_dwLastXRefOffset)) {
        m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
        return FALSE;
    }
    FXSYS_qsort(m_parser.m_SortedOffset.GetData(), m_parser.m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    m_dwRootObjNum = m_parser.GetRootObjNum();
    m_dwInfoObjNum = m_parser.GetInfoObjNum();
    m_pCurrentParser = &m_parser;
    m_docStatus = PDF_DATAAVAIL_ROOT;
    return TRUE;
}
CPDF_Object* CPDF_DataAvail::GetObject(FX_DWORD objnum, IFX_DownloadHints* pHints, FX_BOOL *pExistInFile)
{
    CPDF_Object *pRet         = NULL;
    FX_DWORD    original_size = 0;
    FX_FILESIZE offset        = 0;
    CPDF_Parser *pParser      = NULL;

    if (pExistInFile) { 
        *pExistInFile = TRUE;
    }

    if (m_pDocument == NULL) {
        original_size = (FX_DWORD)m_parser.GetObjectSize(objnum);
        offset        = m_parser.GetObjectOffset(objnum);
        pParser       = &m_parser; 
    } else {
        original_size = GetObjectSize(objnum, offset);
        pParser       = (CPDF_Parser *)(m_pDocument->GetParser());
    }

    pdfium::base::CheckedNumeric<FX_DWORD> size = original_size;
    if (size.ValueOrDefault(0) == 0 || offset < 0 || offset >= m_dwFileLen) {
        if (pExistInFile)
           *pExistInFile = FALSE;

        return NULL;
    }
 
    size += offset;
    size += 512;
    if (!size.IsValid()) {
        return NULL;
    }

    if (size.ValueOrDie() > m_dwFileLen) {
        size = m_dwFileLen - offset;
    } else {
        size = original_size + 512;
    }

    if (!size.IsValid()) {
        return NULL;
    }

    if (!m_pFileAvail->IsDataAvail(offset, size.ValueOrDie())) {
        pHints->AddSegment(offset, size.ValueOrDie());
        return NULL;
    }

    if (pParser) {
        pRet = pParser->ParseIndirectObject(NULL, objnum, NULL);
    }

    if (!pRet && pExistInFile) {
        *pExistInFile = FALSE;
    }
 
    return pRet;
}

FX_BOOL CPDF_DataAvail::CheckInfo(IFX_DownloadHints* pHints)
{
    FX_BOOL bExist = FALSE;
    CPDF_Object *pInfo = GetObject(m_dwInfoObjNum, pHints, &bExist);
    if (!bExist) {
        if (m_bHaveAcroForm) {
            m_docStatus = PDF_DATAAVAIL_ACROFORM;
        } else {
            m_docStatus = PDF_DATAAVAIL_PAGETREE;
        }
        return TRUE;
    }
    if (!pInfo) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
            return TRUE;
        }
        if (m_Pos == m_dwFileLen) {
            m_docStatus = PDF_DATAAVAIL_ERROR;
        }
        return FALSE;
    }
    if (pInfo) {
        pInfo->Release();
    }
    if (m_bHaveAcroForm) {
        m_docStatus = PDF_DATAAVAIL_ACROFORM;
    } else {
        m_docStatus = PDF_DATAAVAIL_PAGETREE;
    }
    return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckRoot(IFX_DownloadHints* pHints)
{
    FX_BOOL bExist = FALSE;
    m_pRoot = GetObject(m_dwRootObjNum, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
        return TRUE;
    }
    if (!m_pRoot) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
            return TRUE;
        }
        return FALSE;
    }
    CPDF_Dictionary* pDict = m_pRoot->GetDict();
    if (!pDict) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    CPDF_Reference* pRef = (CPDF_Reference*)pDict->GetElement(FX_BSTRC("Pages"));
    if (pRef == NULL || pRef->GetType() != PDFOBJ_REFERENCE) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    m_PagesObjNum = pRef->GetRefObjNum();
    CPDF_Reference* pAcroFormRef = (CPDF_Reference*)m_pRoot->GetDict()->GetElement(FX_BSTRC("AcroForm"));
    if (pAcroFormRef && pAcroFormRef->GetType() == PDFOBJ_REFERENCE) {
        m_bHaveAcroForm = TRUE;
        m_dwAcroFormObjNum = pAcroFormRef->GetRefObjNum();
    }
    if (m_dwInfoObjNum) {
        m_docStatus = PDF_DATAAVAIL_INFO;
    } else {
        if (m_bHaveAcroForm) {
            m_docStatus = PDF_DATAAVAIL_ACROFORM;
        } else {
            m_docStatus = PDF_DATAAVAIL_PAGETREE;
        }
    }
    return TRUE;
}
FX_BOOL CPDF_DataAvail::PreparePageItem()
{
    CPDF_Dictionary *pRoot = m_pDocument->GetRoot();
    CPDF_Reference* pRef = pRoot ? (CPDF_Reference*)pRoot->GetElement(FX_BSTRC("Pages")) : NULL;
    if (pRef == NULL || pRef->GetType() != PDFOBJ_REFERENCE) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    m_PagesObjNum = pRef->GetRefObjNum();
    m_pCurrentParser = (CPDF_Parser *)m_pDocument->GetParser();
    m_docStatus = PDF_DATAAVAIL_PAGETREE;
    return TRUE;
}
FX_BOOL CPDF_DataAvail::IsFirstCheck(int iPage)
{
    if (NULL == m_pageMapCheckState) {
        m_pageMapCheckState = new CFX_CMapDWordToDWord();
    }
    FX_DWORD dwValue = 0;
    if (!m_pageMapCheckState->Lookup(iPage, dwValue)) {
        m_pageMapCheckState->SetAt(iPage, 1);
        return TRUE;
    }
    if (dwValue != 0) {
        return FALSE;
    }
    m_pageMapCheckState->SetAt(iPage, 1);
    return TRUE;
}
void CPDF_DataAvail::ResetFirstCheck(int iPage)
{
    if (NULL == m_pageMapCheckState) {
        m_pageMapCheckState = new CFX_CMapDWordToDWord();
    }
    FX_DWORD dwValue = 1;
    if (!m_pageMapCheckState->Lookup(iPage, dwValue)) {
        return;
    }
    m_pageMapCheckState->SetAt(iPage, 0);
}
FX_BOOL CPDF_DataAvail::CheckPage(IFX_DownloadHints* pHints)
{
    FX_DWORD iPageObjs = m_PageObjList.GetSize();
    CFX_DWordArray UnavailObjList;
    for (FX_DWORD i = 0; i < iPageObjs; ++i) {
        FX_DWORD dwPageObjNum = m_PageObjList.GetAt(i);
        FX_BOOL bExist = FALSE;
        CPDF_Object *pObj = GetObject(dwPageObjNum, pHints, &bExist);
        if (!pObj) {
            if (bExist) {
                UnavailObjList.Add(dwPageObjNum);
            }
            continue;
        }
        if (pObj->GetType() == PDFOBJ_ARRAY) {
            CPDF_Array *pArray = pObj->GetArray();
            if (pArray) {
                FX_INT32 iSize = pArray->GetCount();
                CPDF_Object *pItem = NULL;
                for (FX_INT32 j = 0; j < iSize; ++j) {
                    pItem = pArray->GetElement(j);
                    if (pItem && pItem->GetType() == PDFOBJ_REFERENCE) {
                        UnavailObjList.Add(((CPDF_Reference *)pItem)->GetRefObjNum());
                    }
                }
            }
        }
        if (pObj->GetType() != PDFOBJ_DICTIONARY) {
            pObj->Release();
            continue;
        }
        CFX_ByteString type = pObj->GetDict()->GetString(FX_BSTRC("Type"));
        if (type == FX_BSTRC("Pages")) {
            m_PagesArray.Add(pObj);
            continue;
        }
        pObj->Release();
    }
    m_PageObjList.RemoveAll();
    if (UnavailObjList.GetSize()) {
        m_PageObjList.Append(UnavailObjList);
        return FALSE;
    }
    FX_DWORD iPages = m_PagesArray.GetSize();
    for (FX_DWORD i = 0; i < iPages; i++) {
        CPDF_Object *pPages = (CPDF_Object *)m_PagesArray.GetAt(i);
        if (!pPages) {
            continue;
        }
        if (!GetPageKids(m_pCurrentParser, pPages)) {
            pPages->Release();
            while (++i < iPages) {
                pPages = (CPDF_Object *)m_PagesArray.GetAt(i);
                pPages->Release();
            }
            m_PagesArray.RemoveAll();
            m_docStatus = PDF_DATAAVAIL_ERROR;
            return FALSE;
        }
        pPages->Release();
    }
    m_PagesArray.RemoveAll();
    if (!m_PageObjList.GetSize()) {
        m_docStatus = PDF_DATAAVAIL_DONE;
    }
    return TRUE;
}
FX_BOOL CPDF_DataAvail::GetPageKids(CPDF_Parser *pParser, CPDF_Object *pPages)
{
    if (!pParser) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    CPDF_Dictionary* pDict = pPages->GetDict();
    CPDF_Object *pKids = pDict ? pDict->GetElement(FX_BSTRC("Kids")) : NULL;
    if (!pKids) {
        return TRUE;
    }
    switch (pKids->GetType()) {
        case PDFOBJ_REFERENCE: {
                CPDF_Reference *pKid = (CPDF_Reference *)pKids;
                m_PageObjList.Add(pKid->GetRefObjNum());
            }
            break;
        case PDFOBJ_ARRAY: {
                CPDF_Array *pKidsArray = (CPDF_Array *)pKids;
                for (FX_DWORD i = 0; i < pKidsArray->GetCount(); ++i) {
                    CPDF_Object *pKid = (CPDF_Object *)pKidsArray->GetElement(i);
                    if (pKid && pKid->GetType() == PDFOBJ_REFERENCE) {
                        m_PageObjList.Add(((CPDF_Reference *)pKid)->GetRefObjNum());
                    }
                }
            }
            break;
        default:
            m_docStatus = PDF_DATAAVAIL_ERROR;
            return FALSE;
    }
    return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckPages(IFX_DownloadHints* pHints)
{
    FX_BOOL bExist = FALSE;
    CPDF_Object *pPages = GetObject(m_PagesObjNum, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
        return TRUE;
    }
    if (!pPages) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
            return TRUE;
        }
        return FALSE;
    }
    if (!GetPageKids(m_pCurrentParser, pPages)) {
        pPages->Release();
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    pPages->Release();
    m_docStatus = PDF_DATAAVAIL_PAGE;
    return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckHeader(IFX_DownloadHints* pHints)
{
    FX_DWORD req_size = 1024;
    if ((FX_FILESIZE)req_size > m_dwFileLen) {
        req_size = (FX_DWORD)m_dwFileLen;
    }
    if (m_pFileAvail->IsDataAvail(0, req_size)) {
        FX_BYTE buffer[1024];
        m_pFileRead->ReadBlock(buffer, 0, req_size);
        if (IsLinearizedFile(buffer, req_size)) {
            m_docStatus = PDF_DATAAVAIL_FIRSTPAGE;
        } else {
            if (m_docStatus == PDF_DATAAVAIL_ERROR) {
                return FALSE;
            }
            m_docStatus = PDF_DATAAVAIL_END;
        }
        return TRUE;
    }
    pHints->AddSegment(0, req_size);
    return FALSE;
}
FX_BOOL CPDF_DataAvail::CheckFirstPage(IFX_DownloadHints *pHints)
{
    CPDF_Dictionary* pDict = m_pLinearized->GetDict();
    CPDF_Object *pEndOffSet = pDict ? pDict->GetElement(FX_BSTRC("E")) : NULL;
    if (!pEndOffSet) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    CPDF_Object *pXRefOffset = pDict ? pDict->GetElement(FX_BSTRC("T")) : NULL;
    if (!pXRefOffset) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    CPDF_Object *pFileLen = pDict ? pDict->GetElement(FX_BSTRC("L")) : NULL;
    if (!pFileLen) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    FX_BOOL bNeedDownLoad = FALSE;
    if (pEndOffSet->GetType() == PDFOBJ_NUMBER) {
        FX_DWORD dwEnd = pEndOffSet->GetInteger();
        dwEnd += 512;
        if ((FX_FILESIZE)dwEnd > m_dwFileLen) {
            dwEnd = (FX_DWORD)m_dwFileLen;
        }
        FX_INT32 iStartPos = (FX_INT32)(m_dwFileLen > 1024 ? 1024 : m_dwFileLen);
        FX_INT32 iSize = dwEnd > 1024 ? dwEnd - 1024 : 0;
        if (!m_pFileAvail->IsDataAvail(iStartPos, iSize)) {
            pHints->AddSegment(iStartPos, iSize);
            bNeedDownLoad = TRUE;
        }
    }
    m_dwLastXRefOffset = 0;
    FX_FILESIZE dwFileLen = 0;
    if (pXRefOffset->GetType() == PDFOBJ_NUMBER) {
        m_dwLastXRefOffset = pXRefOffset->GetInteger();
    }
    if (pFileLen->GetType() == PDFOBJ_NUMBER) {
        dwFileLen = pFileLen->GetInteger();
    }
    if (!m_pFileAvail->IsDataAvail(m_dwLastXRefOffset, (FX_DWORD)(dwFileLen - m_dwLastXRefOffset))) {
        if (m_docStatus == PDF_DATAAVAIL_FIRSTPAGE)	{
            FX_DWORD dwSize = (FX_DWORD)(dwFileLen - m_dwLastXRefOffset);
            FX_FILESIZE offset = m_dwLastXRefOffset;
            if (dwSize < 512 && dwFileLen > 512) {
                dwSize = 512;
                offset = dwFileLen - 512;
            }
            pHints->AddSegment(offset, dwSize);
        }
    } else {
        m_docStatus = PDF_DATAAVAIL_FIRSTPAGE_PREPARE;
    }
    if (!bNeedDownLoad && m_docStatus == PDF_DATAAVAIL_FIRSTPAGE_PREPARE) {
        m_docStatus = PDF_DATAAVAIL_DONE;
        return TRUE;
    }
    m_docStatus = PDF_DATAAVAIL_FIRSTPAGE_PREPARE;
    return FALSE;
}
CPDF_Object	* CPDF_DataAvail::ParseIndirectObjectAt(FX_FILESIZE pos, FX_DWORD objnum)
{
    FX_FILESIZE SavedPos = m_syntaxParser.SavePos();
    m_syntaxParser.RestorePos(pos);
    FX_BOOL bIsNumber;
    CFX_ByteString word = m_syntaxParser.GetNextWord(bIsNumber);
    if (!bIsNumber) {
        return NULL;
    }
    FX_DWORD parser_objnum = FXSYS_atoi(word);
    if (objnum && parser_objnum != objnum) {
        return NULL;
    }
    word = m_syntaxParser.GetNextWord(bIsNumber);
    if (!bIsNumber) {
        return NULL;
    }
    FX_DWORD gennum = FXSYS_atoi(word);
    if (m_syntaxParser.GetKeyword() != FX_BSTRC("obj")) {
        m_syntaxParser.RestorePos(SavedPos);
        return NULL;
    }
    CPDF_Object* pObj = m_syntaxParser.GetObject(NULL, objnum, gennum, 0);
    m_syntaxParser.RestorePos(SavedPos);
    return pObj;
}
FX_INT32 CPDF_DataAvail::IsLinearizedPDF()
{
    FX_DWORD req_size = 1024;
    if (!m_pFileAvail->IsDataAvail(0, req_size)) {
        return PDF_UNKNOW_LINEARIZED;
    }
    if (!m_pFileRead) {
        return PDF_NOT_LINEARIZED;
    }
    FX_FILESIZE dwSize = m_pFileRead->GetSize();
    if (dwSize < (FX_FILESIZE)req_size) {
        return PDF_UNKNOW_LINEARIZED;
    }
    FX_BYTE buffer[1024];
    m_pFileRead->ReadBlock(buffer, 0, req_size);
    if (IsLinearizedFile(buffer, req_size)) {
        return PDF_IS_LINEARIZED;
    }
    return PDF_NOT_LINEARIZED;
}
FX_BOOL CPDF_DataAvail::IsLinearizedFile(FX_LPBYTE pData, FX_DWORD dwLen)
{
    CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream(pData, (size_t)dwLen, FALSE));
    FX_INT32 offset = GetHeaderOffset(file.Get());
    if (offset == -1) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    m_dwHeaderOffset = offset;
    m_syntaxParser.InitParser(file.Get(), offset);
    m_syntaxParser.RestorePos(m_syntaxParser.m_HeaderOffset + 9);
    FX_BOOL bNumber = FALSE;
    CFX_ByteString wordObjNum = m_syntaxParser.GetNextWord(bNumber);
    if (!bNumber) {
        return FALSE;
    }
    FX_DWORD objnum = FXSYS_atoi(wordObjNum);
    if (m_pLinearized) {
        m_pLinearized->Release();
        m_pLinearized = NULL;
    }
    m_pLinearized = ParseIndirectObjectAt(m_syntaxParser.m_HeaderOffset + 9, objnum);
    if (!m_pLinearized) {
        return FALSE;
    }
    if (m_pLinearized->GetDict() && m_pLinearized->GetDict()->GetElement(FX_BSTRC("Linearized"))) {
        CPDF_Object *pLen = m_pLinearized->GetDict()->GetElement(FX_BSTRC("L"));
        if (!pLen) {
            return FALSE;
        }
        if ((FX_FILESIZE)pLen->GetInteger() != m_pFileRead->GetSize()) {
            return FALSE;
        }
        m_bLinearized = TRUE;
        CPDF_Object *pNo = m_pLinearized->GetDict()->GetElement(FX_BSTRC("P"));
        if (pNo && pNo->GetType() == PDFOBJ_NUMBER) {
            m_dwFirstPageNo = pNo->GetInteger();
        }
        return TRUE;
    }
    return FALSE;
}
FX_BOOL CPDF_DataAvail::CheckEnd(IFX_DownloadHints* pHints)
{
    FX_DWORD req_pos = (FX_DWORD)(m_dwFileLen > 1024 ? m_dwFileLen - 1024 : 0);
    FX_DWORD dwSize = (FX_DWORD)(m_dwFileLen - req_pos);
    if (m_pFileAvail->IsDataAvail(req_pos, dwSize)) {
        FX_BYTE buffer[1024];
        m_pFileRead->ReadBlock(buffer, req_pos, dwSize);
        CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream(buffer, (size_t)dwSize, FALSE));
        m_syntaxParser.InitParser(file.Get(), 0);
        m_syntaxParser.RestorePos(dwSize - 1);
        if (m_syntaxParser.SearchWord(FX_BSTRC("startxref"), TRUE, FALSE, dwSize)) {
            FX_BOOL bNumber;
            m_syntaxParser.GetNextWord(bNumber);
            CFX_ByteString xrefpos_str = m_syntaxParser.GetNextWord(bNumber);
            if (!bNumber) {
                m_docStatus = PDF_DATAAVAIL_ERROR;
                return FALSE;
            }
            m_dwXRefOffset = (FX_FILESIZE)FXSYS_atoi64(xrefpos_str);
            if (!m_dwXRefOffset || m_dwXRefOffset > m_dwFileLen) {
                m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
                return TRUE;
            }
            m_dwLastXRefOffset = m_dwXRefOffset;
            SetStartOffset(m_dwXRefOffset);
            m_docStatus = PDF_DATAAVAIL_CROSSREF;
            return TRUE;
        } else {
            m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
            return TRUE;
        }
    }
    pHints->AddSegment(req_pos, dwSize);
    return FALSE;
}
FX_INT32 CPDF_DataAvail::CheckCrossRefStream(IFX_DownloadHints* pHints, FX_FILESIZE &xref_offset)
{
    xref_offset = 0;
    FX_DWORD req_size = (FX_DWORD)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
    if (m_pFileAvail->IsDataAvail(m_Pos, req_size)) {
        FX_INT32 iSize = (FX_INT32)(m_Pos + req_size - m_dwCurrentXRefSteam);
        CFX_BinaryBuf buf(iSize);
        FX_LPBYTE pBuf = buf.GetBuffer();
        m_pFileRead->ReadBlock(pBuf, m_dwCurrentXRefSteam, iSize);
        CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream(pBuf, (size_t)iSize, FALSE));
        m_parser.m_Syntax.InitParser(file.Get(), 0);
        FX_BOOL bNumber = FALSE;
        CFX_ByteString objnum = m_parser.m_Syntax.GetNextWord(bNumber);
        if (!bNumber) {
            return -1;
        }
        FX_DWORD objNum = FXSYS_atoi(objnum);
        CPDF_Object *pObj = m_parser.ParseIndirectObjectAt(NULL, 0, objNum, NULL);
        if (!pObj) {
            m_Pos += m_parser.m_Syntax.SavePos();
            return 0;
        }
        CPDF_Dictionary* pDict = pObj->GetDict();
        CPDF_Object *pName = pDict ? pDict->GetElement(FX_BSTRC("Type")) : NULL;
        if (pName && pName->GetType() == PDFOBJ_NAME) {
            if (pName->GetString() == FX_BSTRC("XRef")) {
                m_Pos += m_parser.m_Syntax.SavePos();
                xref_offset = pObj->GetDict()->GetInteger(FX_BSTRC("Prev"));
                pObj->Release();
                return 1;
            } else {
                pObj->Release();
                return -1;
            }
        }
        pObj->Release();
        return -1;
    }
    pHints->AddSegment(m_Pos, req_size);
    return 0;
}
inline void CPDF_DataAvail::SetStartOffset(FX_FILESIZE dwOffset)
{
    m_Pos = dwOffset;
}
#define MAX_WORD_BUFFER 256
FX_BOOL CPDF_DataAvail::GetNextToken(CFX_ByteString &token)
{
    m_WordSize = 0;
    FX_BYTE ch;
    if (!GetNextChar(ch)) {
        return FALSE;
    }
    FX_BYTE type = PDF_CharType[ch];
    while (1) {
        while (type == 'W') {
            if (!GetNextChar(ch)) {
                return FALSE;
            }
            type = PDF_CharType[ch];
        }
        if (ch != '%') {
            break;
        }
        while (1) {
            if (!GetNextChar(ch)) {
                return FALSE;
            }
            if (ch == '\r' || ch == '\n') {
                break;
            }
        }
        type = PDF_CharType[ch];
    }
    if (type == 'D') {
        m_WordBuffer[m_WordSize++] = ch;
        if (ch == '/') {
            while (1) {
                if (!GetNextChar(ch)) {
                    return FALSE;
                }
                type = PDF_CharType[ch];
                if (type != 'R' && type != 'N') {
                    m_Pos --;
                    CFX_ByteString ret(m_WordBuffer, m_WordSize);
                    token = ret;
                    return TRUE;
                }
                if (m_WordSize < MAX_WORD_BUFFER) {
                    m_WordBuffer[m_WordSize++] = ch;
                }
            }
        } else if (ch == '<') {
            if (!GetNextChar(ch)) {
                return FALSE;
            }
            if (ch == '<') {
                m_WordBuffer[m_WordSize++] = ch;
            } else {
                m_Pos --;
            }
        } else if (ch == '>') {
            if (!GetNextChar(ch)) {
                return FALSE;
            }
            if (ch == '>') {
                m_WordBuffer[m_WordSize++] = ch;
            } else {
                m_Pos --;
            }
        }
        CFX_ByteString ret(m_WordBuffer, m_WordSize);
        token = ret;
        return TRUE;
    }
    while (1) {
        if (m_WordSize < MAX_WORD_BUFFER) {
            m_WordBuffer[m_WordSize++] = ch;
        }
        if (!GetNextChar(ch)) {
            return FALSE;
        }
        type = PDF_CharType[ch];
        if (type == 'D' || type == 'W') {
            m_Pos --;
            break;
        }
    }
    CFX_ByteString ret(m_WordBuffer, m_WordSize);
    token = ret;
    return TRUE;
}
FX_BOOL CPDF_DataAvail::GetNextChar(FX_BYTE &ch)
{
    FX_FILESIZE pos = m_Pos;
    if (pos >= m_dwFileLen) {
        return FALSE;
    }
    if (m_bufferOffset >= pos || (FX_FILESIZE)(m_bufferOffset + m_bufferSize) <= pos) {
        FX_FILESIZE read_pos = pos;
        FX_DWORD read_size = 512;
        if ((FX_FILESIZE)read_size > m_dwFileLen) {
            read_size = (FX_DWORD)m_dwFileLen;
        }
        if ((FX_FILESIZE)(read_pos + read_size) > m_dwFileLen) {
            read_pos = m_dwFileLen - read_size;
        }
        if (!m_pFileRead->ReadBlock(m_bufferData, read_pos, read_size)) {
            return FALSE;
        }
        m_bufferOffset = read_pos;
        m_bufferSize = read_size;
    }
    ch = m_bufferData[pos - m_bufferOffset];
    m_Pos ++;
    return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckCrossRefItem(IFX_DownloadHints *pHints)
{
    FX_INT32 iSize = 0;
    CFX_ByteString token;
    while (1) {
        if (!GetNextToken(token)) {
            iSize = (FX_INT32)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
            pHints->AddSegment(m_Pos, iSize);
            return FALSE;
        }
        if (token == "trailer") {
            m_dwTrailerOffset = m_Pos;
            m_docStatus = PDF_DATAAVAIL_TRAILER;
            return TRUE;
        }
    }
}
FX_BOOL CPDF_DataAvail::CheckAllCrossRefStream(IFX_DownloadHints *pHints)
{
    FX_FILESIZE xref_offset = 0;
    FX_INT32 nRet = CheckCrossRefStream(pHints, xref_offset);
    if (nRet == 1) {
        if (!xref_offset) {
            m_docStatus = PDF_DATAAVAIL_LOADALLCRSOSSREF;
        } else {
            m_dwCurrentXRefSteam = xref_offset;
            m_Pos = xref_offset;
        }
        return TRUE;
    } else if (nRet == -1) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
    }
    return FALSE;
}
FX_BOOL CPDF_DataAvail::CheckCrossRef(IFX_DownloadHints* pHints)
{
    FX_INT32 iSize = 0;
    CFX_ByteString token;
    if (!GetNextToken(token)) {
        iSize = (FX_INT32)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
        pHints->AddSegment(m_Pos, iSize);
        return FALSE;
    }
    if (token == "xref") {
        m_CrossOffset.InsertAt(0, m_dwXRefOffset);
        while (1) {
            if (!GetNextToken(token)) {
                iSize = (FX_INT32)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
                pHints->AddSegment(m_Pos, iSize);
                m_docStatus = PDF_DATAAVAIL_CROSSREF_ITEM;
                return FALSE;
            }
            if (token == "trailer") {
                m_dwTrailerOffset = m_Pos;
                m_docStatus = PDF_DATAAVAIL_TRAILER;
                return TRUE;
            }
        }
    } else {
        m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
        return TRUE;
    }
    return FALSE;
}
FX_BOOL CPDF_DataAvail::CheckTrailerAppend(IFX_DownloadHints* pHints)
{
    if (m_Pos < m_dwFileLen) {
        FX_FILESIZE dwAppendPos = m_Pos + m_syntaxParser.SavePos();
        FX_INT32 iSize = (FX_INT32)(dwAppendPos + 512 > m_dwFileLen ? m_dwFileLen - dwAppendPos : 512);
        if (!m_pFileAvail->IsDataAvail(dwAppendPos, iSize)) {
            pHints->AddSegment(dwAppendPos, iSize);
            return FALSE;
        }
    }
    if (m_dwPrevXRefOffset) {
        SetStartOffset(m_dwPrevXRefOffset);
        m_docStatus = PDF_DATAAVAIL_CROSSREF;
    } else {
        m_docStatus = PDF_DATAAVAIL_LOADALLCRSOSSREF;
    }
    return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckTrailer(IFX_DownloadHints* pHints)
{
    FX_INT32 iTrailerSize = (FX_INT32)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
    if (m_pFileAvail->IsDataAvail(m_Pos, iTrailerSize)) {
        FX_INT32 iSize = (FX_INT32)(m_Pos + iTrailerSize - m_dwTrailerOffset);
        CFX_BinaryBuf buf(iSize);
        FX_LPBYTE pBuf = buf.GetBuffer();
        if (!pBuf) {
            m_docStatus = PDF_DATAAVAIL_ERROR;
            return FALSE;
        }
        if (!m_pFileRead->ReadBlock(pBuf, m_dwTrailerOffset, iSize)) {
            return FALSE;
        }
        CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream(pBuf, (size_t)iSize, FALSE));
        m_syntaxParser.InitParser(file.Get(), 0);
        CPDF_Object *pTrailer = m_syntaxParser.GetObject(NULL, 0, 0, 0);
        if (!pTrailer) {
            m_Pos += m_syntaxParser.SavePos();
            pHints->AddSegment(m_Pos, iTrailerSize);
            return FALSE;
        }
        if (pTrailer->GetType() != PDFOBJ_DICTIONARY) {
            return FALSE;
        }
        CPDF_Dictionary *pTrailerDict = pTrailer->GetDict();
        if (pTrailerDict) {
            CPDF_Object *pEncrypt = pTrailerDict->GetElement("Encrypt");
            if (pEncrypt && pEncrypt->GetType() == PDFOBJ_REFERENCE) {
                m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
                pTrailer->Release();
                return TRUE;
            }
        }
        FX_DWORD xrefpos = GetDirectInteger(pTrailer->GetDict(), FX_BSTRC("Prev"));
        if (xrefpos) {
            m_dwPrevXRefOffset = GetDirectInteger(pTrailer->GetDict(), FX_BSTRC("XRefStm"));
            pTrailer->Release();
            if (m_dwPrevXRefOffset) {
                m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
            } else {
                m_dwPrevXRefOffset = xrefpos;
                if (m_dwPrevXRefOffset >= m_dwFileLen) {
                    m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
                } else {
                    SetStartOffset(m_dwPrevXRefOffset);
                    m_docStatus = PDF_DATAAVAIL_TRAILER_APPEND;
                }
            }
            return TRUE;
        } else {
            m_dwPrevXRefOffset = 0;
            m_docStatus = PDF_DATAAVAIL_TRAILER_APPEND;
            pTrailer->Release();
        }
        return TRUE;
    }
    pHints->AddSegment(m_Pos, iTrailerSize);
    return FALSE;
}
FX_BOOL CPDF_DataAvail::CheckPage(FX_INT32 iPage, IFX_DownloadHints* pHints)
{
    while (TRUE) {
        switch (m_docStatus) {
            case PDF_DATAAVAIL_PAGETREE:
                if (!LoadDocPages(pHints)) {
                    return FALSE;
                }
                break;
            case PDF_DATAAVAIL_PAGE:
                if (!LoadDocPage(iPage, pHints)) {
                    return FALSE;
                }
                break;
            case PDF_DATAAVAIL_ERROR:
                return LoadAllFile(pHints);
            default:
                m_bPagesTreeLoad = TRUE;
                m_bPagesLoad = TRUE;
                m_bCurPageDictLoadOK = TRUE;
                m_docStatus = PDF_DATAAVAIL_PAGE;
                return TRUE;
        }
    }
}
FX_BOOL	CPDF_DataAvail::CheckArrayPageNode(FX_DWORD dwPageNo, CPDF_PageNode *pPageNode, IFX_DownloadHints* pHints)
{
    FX_BOOL bExist = FALSE;
    CPDF_Object *pPages = GetObject(dwPageNo, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    if (!pPages) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_ERROR;
            return FALSE;
        }
        return FALSE;
    }
    if (pPages->GetType() != PDFOBJ_ARRAY) {
        pPages->Release();
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    pPageNode->m_type = PDF_PAGENODE_PAGES;
    CPDF_Array* pArray = (CPDF_Array*)pPages;
    for (FX_DWORD i = 0; i < pArray->GetCount(); ++i) {
        CPDF_Object *pKid = (CPDF_Object *)pArray->GetElement(i);
        if (!pKid || pKid->GetType() != PDFOBJ_REFERENCE) {
            continue;
        }
        CPDF_PageNode *pNode = new CPDF_PageNode();
        pPageNode->m_childNode.Add(pNode);
        pNode->m_dwPageNo = ((CPDF_Reference*)pKid)->GetRefObjNum();
    }
    pPages->Release();
    return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckUnkownPageNode(FX_DWORD dwPageNo, CPDF_PageNode *pPageNode, IFX_DownloadHints* pHints)
{
    FX_BOOL bExist = FALSE;
    CPDF_Object *pPage = GetObject(dwPageNo, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    if (!pPage) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_ERROR;
            return FALSE;
        }
        return FALSE;
    }
    if (pPage->GetType() == PDFOBJ_ARRAY) {
        pPageNode->m_dwPageNo = dwPageNo;
        pPageNode->m_type = PDF_PAGENODE_ARRAY;
        pPage->Release();
        return TRUE;
    }
    if (pPage->GetType() != PDFOBJ_DICTIONARY) {
        pPage->Release();
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    pPageNode->m_dwPageNo = dwPageNo;
    CPDF_Dictionary* pDict = pPage->GetDict();
    CFX_ByteString type = pDict ? pDict->GetString(FX_BSTRC("Type")) : CFX_ByteString();
    if (type == FX_BSTRC("Pages")) {
        pPageNode->m_type = PDF_PAGENODE_PAGES;
        CPDF_Object *pKids = pDict->GetElement(FX_BSTRC("Kids"));
        if (!pKids) {
            m_docStatus = PDF_DATAAVAIL_PAGE;
            return TRUE;
        }
        switch (pKids->GetType()) {
            case PDFOBJ_REFERENCE: {
                    CPDF_Reference *pKid = (CPDF_Reference *)pKids;
                    CPDF_PageNode *pNode = new CPDF_PageNode();
                    pPageNode->m_childNode.Add(pNode);
                    pNode->m_dwPageNo = pKid->GetRefObjNum();
                }
                break;
            case PDFOBJ_ARRAY: {
                    CPDF_Array *pKidsArray = (CPDF_Array *)pKids;
                    for (FX_DWORD i = 0; i < pKidsArray->GetCount(); ++i) {
                        CPDF_Object *pKid = (CPDF_Object *)pKidsArray->GetElement(i);
                        if (!pKid || pKid->GetType() != PDFOBJ_REFERENCE) {
                            continue;
                        }
                        CPDF_PageNode *pNode = new CPDF_PageNode();
                        pPageNode->m_childNode.Add(pNode);
                        pNode->m_dwPageNo = ((CPDF_Reference*)pKid)->GetRefObjNum();
                    }
                }
                break;
            default:
                break;
        }
    } else if (type == FX_BSTRC("Page")) {
        pPageNode->m_type = PDF_PAGENODE_PAGE;
    } else {
        pPage->Release();
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    pPage->Release();
    return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckPageNode(CPDF_PageNode &pageNodes, FX_INT32 iPage, FX_INT32 &iCount, IFX_DownloadHints* pHints)
{
    FX_INT32 iSize = pageNodes.m_childNode.GetSize();
    if (iSize <= 0 || iPage >= iSize) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    for (FX_INT32 i = 0; i < iSize; ++i) {
        CPDF_PageNode *pNode = (CPDF_PageNode*)pageNodes.m_childNode.GetAt(i);
        if (!pNode) {
            continue;
        }
        switch (pNode->m_type) {
            case PDF_PAGENODE_UNKOWN:
                if (!CheckUnkownPageNode(pNode->m_dwPageNo, pNode, pHints)) {
                    return FALSE;
                }
                --i;
                break;
            case PDF_PAGENODE_PAGE:
                iCount++;
                if (iPage == iCount && m_pDocument) {
                    m_pDocument->m_PageList.SetAt(iPage, pNode->m_dwPageNo);
                }
                break;
            case PDF_PAGENODE_PAGES:
                if (!CheckPageNode(*pNode, iPage, iCount, pHints)) {
                    return FALSE;
                }
                break;
            case PDF_PAGENODE_ARRAY:
                if (!CheckArrayPageNode(pNode->m_dwPageNo, pNode, pHints)) {
                    return FALSE;
                }
                --i;
                break;
        }
        if (iPage == iCount) {
            m_docStatus = PDF_DATAAVAIL_DONE;
            return TRUE;
        }
    }
    return TRUE;
}
FX_BOOL CPDF_DataAvail::LoadDocPage(FX_INT32 iPage, IFX_DownloadHints* pHints)
{
    if (m_pDocument->GetPageCount() <= iPage || m_pDocument->m_PageList.GetAt(iPage)) {
        m_docStatus = PDF_DATAAVAIL_DONE;
        return TRUE;
    }
    if (m_pageNodes.m_type == PDF_PAGENODE_PAGE) {
        if (iPage == 0) {
            m_docStatus = PDF_DATAAVAIL_DONE;
            return TRUE;
        }
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return TRUE;
    }
    FX_INT32 iCount = -1;
    return CheckPageNode(m_pageNodes, iPage, iCount, pHints);
}
FX_BOOL CPDF_DataAvail::CheckPageCount(IFX_DownloadHints* pHints)
{
    FX_BOOL bExist = FALSE;
    CPDF_Object *pPages = GetObject(m_PagesObjNum, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    if (!pPages) {
        return FALSE;
    }
    CPDF_Dictionary* pPagesDict = pPages->GetDict();
    if (!pPagesDict) {
        pPages->Release();
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
    }
    if (!pPagesDict->KeyExist(FX_BSTRC("Kids"))) {
        pPages->Release();
        return TRUE;
    }
    int count = pPagesDict->GetInteger(FX_BSTRC("Count"));
    if (count > 0) {
        pPages->Release();
        return TRUE;
    }
    pPages->Release();
    return FALSE;
}
FX_BOOL CPDF_DataAvail::LoadDocPages(IFX_DownloadHints* pHints)
{
    if (!CheckUnkownPageNode(m_PagesObjNum, &m_pageNodes, pHints)) {
        return FALSE;
    }
    if (CheckPageCount(pHints)) {
        m_docStatus = PDF_DATAAVAIL_PAGE;
        return TRUE;
    } else {
        m_bTotalLoadPageTree = TRUE;
    }
    return FALSE;
}
FX_BOOL CPDF_DataAvail::LoadPages(IFX_DownloadHints* pHints)
{
    while (!m_bPagesTreeLoad) {
        if (!CheckPageStatus(pHints)) {
            return FALSE;
        }
    }
    if (m_bPagesLoad) {
        return TRUE;
    }
    m_pDocument->LoadPages();
    return FALSE;
}
FX_BOOL CPDF_DataAvail::CheckLinearizedData(IFX_DownloadHints* pHints)
{
    if (m_bLinearedDataOK) {
        return TRUE;
    }

    if (!m_bMainXRefLoadTried) {
        FX_SAFE_DWORD data_size = m_dwFileLen;
        data_size -= m_dwLastXRefOffset;
        if (!data_size.IsValid()) {
            return FALSE;
        }
        if (!m_pFileAvail->IsDataAvail(m_dwLastXRefOffset, data_size.ValueOrDie())) {
            pHints->AddSegment(m_dwLastXRefOffset, data_size.ValueOrDie());
            return FALSE;
        }
        FX_DWORD dwRet = ((CPDF_Parser *)m_pDocument->GetParser())->LoadLinearizedMainXRefTable();
        m_bMainXRefLoadTried = TRUE;
        if (dwRet != PDFPARSE_ERROR_SUCCESS) {
            return FALSE;
        }
        if (!PreparePageItem()) {
            return FALSE;
        }
        m_bMainXRefLoadedOK = TRUE;
        m_bLinearedDataOK   = TRUE;
    }

    return m_bLinearedDataOK;
}
FX_BOOL CPDF_DataAvail::CheckPageAnnots(FX_INT32 iPage, IFX_DownloadHints* pHints)
{
    if (!m_objs_array.GetSize()) {
        m_objs_array.RemoveAll();
        m_objnum_array.RemoveAll();
        CPDF_Dictionary *pPageDict = m_pDocument->GetPage(iPage);
        if (!pPageDict) {
            return TRUE;
        }
        CPDF_Object *pAnnots = pPageDict->GetElement(FX_BSTRC("Annots"));
        if (!pAnnots) {
            return TRUE;
        }
        CFX_PtrArray obj_array;
        obj_array.Add(pAnnots);
        FX_BOOL bRet = IsObjectsAvail(obj_array, FALSE, pHints, m_objs_array);
        if (bRet) {
            m_objs_array.RemoveAll();
        }
        return bRet;
    } else {
        CFX_PtrArray new_objs_array;
        FX_BOOL bRet = IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
        m_objs_array.RemoveAll();
        if (!bRet) {
            m_objs_array.Append(new_objs_array);
        }
        return bRet;
    }
}
FX_BOOL CPDF_DataAvail::CheckLinearizedFirstPage(FX_INT32 iPage, IFX_DownloadHints* pHints)
{
    if (!m_bAnnotsLoad) {
        if (!CheckPageAnnots(iPage, pHints)) {
            return FALSE;
        }
        m_bAnnotsLoad = TRUE;
    }
    if (m_bAnnotsLoad)
        if (!CheckLinearizedData(pHints)) {
            return FALSE;
        }
    m_bPageLoadedOK = FALSE;
    return TRUE;
}
FX_BOOL CPDF_DataAvail::HaveResourceAncestor(CPDF_Dictionary *pDict)
{
    CFX_AutoRestorer<int> restorer(&s_CurrentDataAvailRecursionDepth);
    if (++s_CurrentDataAvailRecursionDepth > kMaxDataAvailRecursionDepth) {
        return FALSE;
    }
    CPDF_Object *pParent = pDict->GetElement("Parent");
    if (!pParent) {
        return FALSE;
    }
    CPDF_Dictionary *pParentDict = pParent->GetDict();
    if (!pParentDict) {
        return FALSE;
    }
    CPDF_Object *pRet = pParentDict->GetElement("Resources");
    if (pRet) {
        m_pPageResource = pRet;
        return TRUE;
    }
    return HaveResourceAncestor(pParentDict);
}
FX_BOOL CPDF_DataAvail::IsPageAvail(FX_INT32 iPage, IFX_DownloadHints* pHints)
{
    if (!m_pDocument) {
        return FALSE;
    }
    if (IsFirstCheck(iPage)) {
        m_bCurPageDictLoadOK = FALSE;
        m_bPageLoadedOK = FALSE;
        m_bAnnotsLoad = FALSE;
        m_bNeedDownLoadResource = FALSE;
        m_objs_array.RemoveAll();
        m_objnum_array.RemoveAll();
    }
    if (m_pagesLoadState == NULL) {
        m_pagesLoadState = new CFX_CMapDWordToDWord();
    }
    FX_DWORD dwPageLoad = 0;
    if (m_pagesLoadState->Lookup(iPage, dwPageLoad) && dwPageLoad != 0) {
        return TRUE;
    }
    if (m_bLinearized) {
        if ((FX_DWORD)iPage == m_dwFirstPageNo) {
            m_pagesLoadState->SetAt(iPage, TRUE);
            return TRUE;
        }
        if (!CheckLinearizedData(pHints)) {
            return FALSE;
        }
        if (m_bMainXRefLoadedOK) {
            if (m_bTotalLoadPageTree) {
                if (!LoadPages(pHints)) {
                    return FALSE;
                }
            } else {
                if (!m_bCurPageDictLoadOK && !CheckPage(iPage, pHints)) {
                    return FALSE;
                }
            }
        } else {
            if (!LoadAllFile(pHints)) {
                return FALSE;
            }
            ((CPDF_Parser *)m_pDocument->GetParser())->RebuildCrossRef();
            ResetFirstCheck(iPage);
            return TRUE;
        }
    } else {
        if (!m_bTotalLoadPageTree) {
            if (!m_bCurPageDictLoadOK && !CheckPage(iPage, pHints)) {
                return FALSE;
            }
        }
    }
    if (m_bHaveAcroForm && !m_bAcroFormLoad) {
        if (!CheckAcroFormSubObject(pHints)) {
            return FALSE;
        }
        m_bAcroFormLoad = TRUE;
    }
    if (!m_bPageLoadedOK) {
        if (!m_objs_array.GetSize()) {
            m_objs_array.RemoveAll();
            m_objnum_array.RemoveAll();
            m_pPageDict = m_pDocument->GetPage(iPage);
            if (!m_pPageDict) {
                ResetFirstCheck(iPage);
                return TRUE;
            }
            CFX_PtrArray obj_array;
            obj_array.Add(m_pPageDict);
            FX_BOOL bRet = IsObjectsAvail(obj_array, TRUE, pHints, m_objs_array);
            if (bRet) {
                m_objs_array.RemoveAll();
                m_bPageLoadedOK = TRUE;
            } else {
                return bRet;
            }
        } else {
            CFX_PtrArray new_objs_array;
            FX_BOOL bRet = IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
            m_objs_array.RemoveAll();
            if (bRet) {
                m_bPageLoadedOK = TRUE;
            } else {
                m_objs_array.Append(new_objs_array);
                return bRet;
            }
        }
    }
    if (m_bPageLoadedOK) {
        if (!m_bAnnotsLoad) {
            if (!CheckPageAnnots(iPage, pHints)) {
                return FALSE;
            }
            m_bAnnotsLoad = TRUE;
        }
    }
    if (m_pPageDict && !m_bNeedDownLoadResource) {
        m_pPageResource = m_pPageDict->GetElement("Resources");
        if (!m_pPageResource) {
            m_bNeedDownLoadResource = HaveResourceAncestor(m_pPageDict);
        } else {
            m_bNeedDownLoadResource = TRUE;
        }
    }
    if (m_bNeedDownLoadResource) {
        FX_BOOL bRet = CheckResources(pHints);
        if (!bRet) {
            return FALSE;
        }
        m_bNeedDownLoadResource = FALSE;
    }
    m_bPageLoadedOK = FALSE;
    m_bAnnotsLoad = FALSE;
    m_bCurPageDictLoadOK = FALSE;
    ResetFirstCheck(iPage);
    m_pagesLoadState->SetAt(iPage, TRUE);
    return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckResources(IFX_DownloadHints* pHints)
{
    if (!m_objs_array.GetSize()) {
        m_objs_array.RemoveAll();
        CFX_PtrArray obj_array;
        obj_array.Add(m_pPageResource);
        FX_BOOL bRet = IsObjectsAvail(obj_array, TRUE, pHints, m_objs_array);
        if (bRet) {
            m_objs_array.RemoveAll();
        }
        return bRet;
    } else {
        CFX_PtrArray new_objs_array;
        FX_BOOL bRet = IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
        m_objs_array.RemoveAll();
        if (!bRet) {
            m_objs_array.Append(new_objs_array);
        }
        return bRet;
    }
}
void CPDF_DataAvail::GetLinearizedMainXRefInfo(FX_FILESIZE *pPos, FX_DWORD *pSize)
{
    if (pPos) {
        *pPos = m_dwLastXRefOffset;
    }
    if (pSize) {
        *pSize = (FX_DWORD)(m_dwFileLen - m_dwLastXRefOffset);
    }
}
FX_INT32 CPDF_DataAvail::IsFormAvail(IFX_DownloadHints *pHints)
{
    if (!m_pDocument) {
        return PDFFORM_AVAIL;
    }
    if (!m_bLinearizedFormParamLoad) {
        CPDF_Dictionary *pRoot = m_pDocument->GetRoot();
        if (!pRoot) {
            return PDFFORM_AVAIL;
        }
        CPDF_Object *pAcroForm = pRoot->GetElement(FX_BSTRC("AcroForm"));
        if (!pAcroForm) {
            return PDFFORM_NOTEXIST;
        }
        if (!CheckLinearizedData(pHints)) {
            return PDFFORM_NOTAVAIL;
        }
        if (!m_objs_array.GetSize()) {
            m_objs_array.Add(pAcroForm->GetDict());
        }
        m_bLinearizedFormParamLoad = TRUE;
    }
    CFX_PtrArray new_objs_array;
    FX_BOOL bRet = IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
    m_objs_array.RemoveAll();
    if (!bRet) {
        m_objs_array.Append(new_objs_array);
        return PDFFORM_NOTAVAIL;
    }
    return PDFFORM_AVAIL;
}
void CPDF_SortObjNumArray::AddObjNum(FX_DWORD dwObjNum)
{
    FX_INT32 iNext = 0;
    if (BinarySearch(dwObjNum, iNext)) {
        return;
    }
    m_number_array.InsertAt(iNext, dwObjNum);
}
FX_BOOL CPDF_SortObjNumArray::Find(FX_DWORD dwObjNum)
{
    FX_INT32 iNext = 0;
    return BinarySearch(dwObjNum, iNext);
}
FX_BOOL CPDF_SortObjNumArray::BinarySearch(FX_DWORD value, FX_INT32 &iNext)
{
    FX_INT32 iLen = m_number_array.GetSize();
    FX_INT32 iLow = 0;
    FX_INT32 iHigh = iLen - 1;
    FX_INT32 iMid = 0;
    while (iLow <= iHigh) {
        iMid = (iLow + iHigh) / 2;
        if (m_number_array.GetAt(iMid) == value) {
            iNext = iMid;
            return TRUE;
        } else if (m_number_array.GetAt(iMid) > value) {
            iHigh = iMid - 1;
        } else if (m_number_array.GetAt(iMid) < value) {
            iLow = iMid + 1;
        }
    }
    iNext = iLow;
    return FALSE;
}
CPDF_PageNode::~CPDF_PageNode()
{
    FX_INT32 iSize = m_childNode.GetSize();
    for (FX_INT32 i = 0; i < iSize; ++i) {
        CPDF_PageNode *pNode = (CPDF_PageNode*)m_childNode[i];
        if (pNode) {
            delete pNode;
        }
    }
    m_childNode.RemoveAll();
}
