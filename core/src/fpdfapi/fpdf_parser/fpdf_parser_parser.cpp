// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <utility>
#include <vector>

#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fpdfapi/fpdf_page.h"
#include "../../../include/fpdfapi/fpdf_parser.h"
#include "../../../include/fxcrt/fx_safe_types.h"
#include "../fpdf_page/pageint.h"

bool IsSignatureDict(const CPDF_Dictionary* pDict)
{
    CPDF_Object* pType = pDict->GetElementValue(FX_BSTRC("Type"));
    if (!pType) {
        pType = pDict->GetElementValue(FX_BSTRC("FT"));
        if (!pType) {
            return false;
        }
    }
    if (pType->GetString() == FX_BSTRC("Sig")) {
        return true;
    }
    return false;
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
    m_bOwnFileRead = true;
    m_FileVersion = 0;
    m_bForceUseSecurityHandler = false;
}
CPDF_Parser::~CPDF_Parser()
{
    CloseParser(false);
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
void CPDF_Parser::CloseParser(bool bReParse)
{
    m_bVersionUpdated = false;
    if (!bReParse) {
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
        void* objnum;
        CPDF_StreamAcc* pStream;
        m_ObjectStreamMap.GetNextAssoc(pos, objnum, (void*&)pStream);
        delete pStream;
    }
    m_ObjectStreamMap.RemoveAll();
    m_SortedOffset.RemoveAll();
    m_CrossRef.RemoveAll();
    m_V5Type.RemoveAll();
    m_ObjVersion.RemoveAll();
    int32_t iLen = m_Trailers.GetSize();
    for (int32_t i = 0; i < iLen; ++i) {
        if (CPDF_Dictionary* trailer = m_Trailers.GetAt(i))
            trailer->Release();
    }
    m_Trailers.RemoveAll();
    if (m_pLinearized) {
        m_pLinearized->Release();
        m_pLinearized = NULL;
    }
}
static int32_t GetHeaderOffset(IFX_FileRead* pFile)
{
    FX_DWORD tag = FXDWORD_FROM_LSBFIRST(0x46445025);
    uint8_t buf[4];
    int32_t offset = 0;
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
FX_DWORD CPDF_Parser::StartParse(const FX_CHAR* filename, bool bReParse)
{
    IFX_FileRead* pFileAccess = FX_CreateFileRead(filename);
    if (!pFileAccess) {
        return PDFPARSE_ERROR_FILE;
    }
    return StartParse(pFileAccess, bReParse);
}
FX_DWORD CPDF_Parser::StartParse(const FX_WCHAR* filename, bool bReParse)
{
    IFX_FileRead* pFileAccess = FX_CreateFileRead(filename);
    if (!pFileAccess) {
        return PDFPARSE_ERROR_FILE;
    }
    return StartParse(pFileAccess, bReParse);
}
CPDF_SecurityHandler* FPDF_CreateStandardSecurityHandler();
CPDF_SecurityHandler* FPDF_CreatePubKeyHandler(void*);
FX_DWORD CPDF_Parser::StartParse(IFX_FileRead* pFileAccess, bool bReParse, bool bOwnFileRead)
{
    CloseParser(bReParse);
    m_bXRefStream = false;
    m_LastXRefOffset = 0;
    m_bOwnFileRead = bOwnFileRead;
    int32_t offset = GetHeaderOffset(pFileAccess);
    if (offset == -1) {
        if (bOwnFileRead && pFileAccess) {
            pFileAccess->Release();
        }
        return PDFPARSE_ERROR_FORMAT;
    }
    m_Syntax.InitParser(pFileAccess, offset);
    uint8_t ch;
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
    bool bXRefRebuilt = false;
    if (m_Syntax.SearchWord(FX_BSTRC("startxref"), true, false, 4096)) {
        FX_FILESIZE startxref_offset = m_Syntax.SavePos();
        void* pResult = FXSYS_bsearch(&startxref_offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
        if (pResult == NULL) {
            m_SortedOffset.Add(startxref_offset);
        }
        m_Syntax.GetKeyword();
        bool bNumber;
        CFX_ByteString xrefpos_str = m_Syntax.GetNextWord(bNumber);
        if (!bNumber) {
            return PDFPARSE_ERROR_FORMAT;
        }
        m_LastXRefOffset = (FX_FILESIZE)FXSYS_atoi64(xrefpos_str);
        if (!LoadAllCrossRefV4(m_LastXRefOffset) && !LoadAllCrossRefV5(m_LastXRefOffset)) {
            if (!RebuildCrossRef()) {
                return PDFPARSE_ERROR_FORMAT;
            }
            bXRefRebuilt = true;
            m_LastXRefOffset = 0;
        }
    } else {
        if (!RebuildCrossRef()) {
            return PDFPARSE_ERROR_FORMAT;
        }
        bXRefRebuilt = true;
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
    delete m_Syntax.m_pCryptoHandler;
    m_Syntax.m_pCryptoHandler = NULL;
    if (!m_bForceUseSecurityHandler) {
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
        return m_CrossRef[(int32_t)m_CrossRef[objnum]];
    }
    return 0;
}
static int32_t GetDirectInteger(CPDF_Dictionary* pDict, const CFX_ByteStringC& key)
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
static bool CheckDirectType(CPDF_Dictionary* pDict, const CFX_ByteStringC& key, int32_t iType)
{
    CPDF_Object* pObj = pDict->GetElement(key);
    if (!pObj) {
        return true;
    }
    return pObj->GetType() == iType;
}
bool CPDF_Parser::LoadAllCrossRefV4(FX_FILESIZE xrefpos)
{
    if (!LoadCrossRefV4(xrefpos, 0, true, false)) {
        return false;
    }
    m_pTrailer = LoadTrailerV4();
    if (m_pTrailer == NULL) {
        return false;
    }
    int32_t xrefsize = GetDirectInteger(m_pTrailer, FX_BSTRC("Size"));
    if (xrefsize <= 0 || xrefsize > (1 << 20)) {
        return false;
    }
    m_CrossRef.SetSize(xrefsize);
    m_V5Type.SetSize(xrefsize);
    CFX_FileSizeArray CrossRefList, XRefStreamList;
    CrossRefList.Add(xrefpos);
    XRefStreamList.Add(GetDirectInteger(m_pTrailer, FX_BSTRC("XRefStm")));
    if (!CheckDirectType(m_pTrailer, FX_BSTRC("Prev"), PDFOBJ_NUMBER)) {
        return false;
    }
    FX_FILESIZE newxrefpos = GetDirectInteger(m_pTrailer, FX_BSTRC("Prev"));
    if (newxrefpos == xrefpos) {
        return false;
    }
    xrefpos = newxrefpos;
    while (xrefpos) {
        CrossRefList.InsertAt(0, xrefpos);
        LoadCrossRefV4(xrefpos, 0, true, false);
        CPDF_Dictionary* pDict = LoadTrailerV4();
        if (pDict == NULL) {
            return false;
        }
        if (!CheckDirectType(pDict, FX_BSTRC("Prev"), PDFOBJ_NUMBER)) {
            pDict->Release();
            return false;
        }
        newxrefpos = GetDirectInteger(pDict, FX_BSTRC("Prev"));
        if (newxrefpos == xrefpos) {
            pDict->Release();
            return false;
        }
        xrefpos = newxrefpos;
        XRefStreamList.InsertAt(0, pDict->GetInteger(FX_BSTRC("XRefStm")));
        m_Trailers.Add(pDict);
    }
    for (int32_t i = 0; i < CrossRefList.GetSize(); i ++)
        if (!LoadCrossRefV4(CrossRefList[i], XRefStreamList[i], false, i == 0)) {
            return false;
        }
    return true;
}
bool CPDF_Parser::LoadLinearizedAllCrossRefV4(FX_FILESIZE xrefpos, FX_DWORD dwObjCount)
{
    if (!LoadLinearizedCrossRefV4(xrefpos, dwObjCount)) {
        return false;
    }
    m_pTrailer = LoadTrailerV4();
    if (m_pTrailer == NULL) {
        return false;
    }
    int32_t xrefsize = GetDirectInteger(m_pTrailer, FX_BSTRC("Size"));
    if (xrefsize == 0) {
        return false;
    }
    CFX_FileSizeArray CrossRefList, XRefStreamList;
    CrossRefList.Add(xrefpos);
    XRefStreamList.Add(GetDirectInteger(m_pTrailer, FX_BSTRC("XRefStm")));
    xrefpos = GetDirectInteger(m_pTrailer, FX_BSTRC("Prev"));
    while (xrefpos) {
        CrossRefList.InsertAt(0, xrefpos);
        LoadCrossRefV4(xrefpos, 0, true, false);
        CPDF_Dictionary* pDict = LoadTrailerV4();
        if (pDict == NULL) {
            return false;
        }
        xrefpos = GetDirectInteger(pDict, FX_BSTRC("Prev"));
        XRefStreamList.InsertAt(0, pDict->GetInteger(FX_BSTRC("XRefStm")));
        m_Trailers.Add(pDict);
    }
    for (int32_t i = 1; i < CrossRefList.GetSize(); i ++)
        if (!LoadCrossRefV4(CrossRefList[i], XRefStreamList[i], false, i == 0)) {
            return false;
        }
    return true;
}
bool CPDF_Parser::LoadLinearizedCrossRefV4(FX_FILESIZE pos, FX_DWORD dwObjCount)
{
    FX_FILESIZE dwStartPos = pos - m_Syntax.m_HeaderOffset;
    m_Syntax.RestorePos(dwStartPos);
    void* pResult = FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    if (pResult == NULL) {
        m_SortedOffset.Add(pos);
    }
    FX_DWORD start_objnum = 0;
    FX_DWORD count = dwObjCount;
    FX_FILESIZE SavedPos = m_Syntax.SavePos();
    int32_t recordsize = 20;
    char* pBuf = FX_Alloc(char, 1024 * recordsize + 1);
    pBuf[1024 * recordsize] = '\0';
    int32_t nBlocks = count / 1024 + 1;
    for (int32_t block = 0; block < nBlocks; block ++) {
        int32_t block_size = block == nBlocks - 1 ? count % 1024 : 1024;
        FX_DWORD dwReadSize = block_size * recordsize;
        if ((FX_FILESIZE)(dwStartPos + dwReadSize) > m_Syntax.m_FileLen) {
            FX_Free(pBuf);
            return false;
        }
        if (!m_Syntax.ReadBlock((uint8_t*)pBuf, dwReadSize)) {
            FX_Free(pBuf);
            return false;
        }
        for (int32_t i = 0; i < block_size; i ++) {
            FX_DWORD objnum = start_objnum + block * 1024 + i;
            char* pEntry = pBuf + i * recordsize;
            if (pEntry[17] == 'f') {
                m_CrossRef.SetAtGrow(objnum, 0);
                m_V5Type.SetAtGrow(objnum, 0);
            } else {
                int32_t offset = FXSYS_atoi(pEntry);
                if (offset == 0) {
                    for (int32_t c = 0; c < 10; c ++) {
                        if (pEntry[c] < '0' || pEntry[c] > '9') {
                            FX_Free(pBuf);
                            return false;
                        }
                    }
                }
                m_CrossRef.SetAtGrow(objnum, offset);
                int32_t version = FXSYS_atoi(pEntry + 11);
                if (version >= 1) {
                    m_bVersionUpdated = true;
                }
                m_ObjVersion.SetAtGrow(objnum, version);
                if (m_CrossRef[objnum] < m_Syntax.m_FileLen) {
                    void* pResult = FXSYS_bsearch(&m_CrossRef[objnum], m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
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
    return true;
}
bool CPDF_Parser::LoadCrossRefV4(FX_FILESIZE pos, FX_FILESIZE streampos, bool bSkip, bool bFirst)
{
    m_Syntax.RestorePos(pos);
    if (m_Syntax.GetKeyword() != FX_BSTRC("xref")) {
        return false;
    }
    void* pResult = FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    if (pResult == NULL) {
        m_SortedOffset.Add(pos);
    }
    if (streampos) {
        void* pResult = FXSYS_bsearch(&streampos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
        if (pResult == NULL) {
            m_SortedOffset.Add(streampos);
        }
    }
    while (1) {
        FX_FILESIZE SavedPos = m_Syntax.SavePos();
        bool bIsNumber;
        CFX_ByteString word = m_Syntax.GetNextWord(bIsNumber);
        if (word.IsEmpty()) {
            return false;
        }
        if (!bIsNumber) {
            m_Syntax.RestorePos(SavedPos);
            break;
        }
        FX_DWORD start_objnum = FXSYS_atoi(word);
        if (start_objnum >= (1 << 20)) {
            return false;
        }
        FX_DWORD count = m_Syntax.GetDirectNum();
        m_Syntax.ToNextWord();
        SavedPos = m_Syntax.SavePos();
        bool bFirstItem = false;
        int32_t recordsize = 20;
        if (bFirst) {
            bFirstItem = true;
        }
        m_dwXrefStartObjNum = start_objnum;
        if (!bSkip) {
            char* pBuf = FX_Alloc(char, 1024 * recordsize + 1);
            pBuf[1024 * recordsize] = '\0';
            int32_t nBlocks = count / 1024 + 1;
            bool bFirstBlock = true;
            for (int32_t block = 0; block < nBlocks; block ++) {
                int32_t block_size = block == nBlocks - 1 ? count % 1024 : 1024;
                m_Syntax.ReadBlock((uint8_t*)pBuf, block_size * recordsize);
                for (int32_t i = 0; i < block_size; i ++) {
                    FX_DWORD objnum = start_objnum + block * 1024 + i;
                    char* pEntry = pBuf + i * recordsize;
                    if (pEntry[17] == 'f') {
                        if (bFirstItem) {
                            objnum = 0;
                            bFirstItem = false;
                        }
                        if (bFirstBlock) {
                            FX_FILESIZE offset = (FX_FILESIZE)FXSYS_atoi64(pEntry);
                            int32_t version = FXSYS_atoi(pEntry + 11);
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
                            for (int32_t c = 0; c < 10; c ++) {
                                if (pEntry[c] < '0' || pEntry[c] > '9') {
                                    FX_Free(pBuf);
                                    return false;
                                }
                            }
                        }
                        m_CrossRef.SetAtGrow(objnum, offset);
                        int32_t version = FXSYS_atoi(pEntry + 11);
                        if (version >= 1) {
                            m_bVersionUpdated = true;
                        }
                        m_ObjVersion.SetAtGrow(objnum, version);
                        if (m_CrossRef[objnum] < m_Syntax.m_FileLen) {
                            void* pResult = FXSYS_bsearch(&m_CrossRef[objnum], m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
                            if (pResult == NULL) {
                                m_SortedOffset.Add(m_CrossRef[objnum]);
                            }
                        }
                        m_V5Type.SetAtGrow(objnum, 1);
                    }
                    if (bFirstBlock) {
                        bFirstBlock = false;
                    }
                }
            }
            FX_Free(pBuf);
        }
        m_Syntax.RestorePos(SavedPos + count * recordsize);
    }
    if (streampos)
        if (!LoadCrossRefV5(streampos, streampos, false)) {
            return false;
        }
    return true;
}
bool CPDF_Parser::LoadAllCrossRefV5(FX_FILESIZE xrefpos)
{
    if (!LoadCrossRefV5(xrefpos, xrefpos, true)) {
        return false;
    }
    while (xrefpos)
        if (!LoadCrossRefV5(xrefpos, xrefpos, false)) {
            return false;
        }
    m_ObjectStreamMap.InitHashTable(101, false);
    m_bXRefStream = true;
    return true;
}
bool CPDF_Parser::RebuildCrossRef()
{
    m_CrossRef.RemoveAll();
    m_V5Type.RemoveAll();
    m_SortedOffset.RemoveAll();
    m_ObjVersion.RemoveAll();
    if (m_pTrailer) {
        m_pTrailer->Release();
        m_pTrailer = NULL;
    }
    int32_t status = 0;
    int32_t inside_index = 0;
    FX_DWORD objnum = 0, gennum = 0;
    int32_t depth = 0;
    uint8_t* buffer = FX_Alloc(uint8_t, 4096);
    FX_FILESIZE pos = m_Syntax.m_HeaderOffset;
    FX_FILESIZE start_pos = 0, start_pos1 = 0;
    FX_FILESIZE last_obj = -1, last_xref = -1, last_trailer = -1;
    while (pos < m_Syntax.m_FileLen) {
        bool bOverFlow = false;
        FX_DWORD size = (FX_DWORD)(m_Syntax.m_FileLen - pos);
        if (size > 4096) {
            size = 4096;
        }
        if (!m_Syntax.m_pFileAccess->ReadBlock(buffer, pos, size)) {
            break;
        }
        for (FX_DWORD i = 0; i < size; i ++) {
            uint8_t byte = buffer[i];
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
                                void* pResult = FXSYS_bsearch(&obj_pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
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
                                    bOverFlow = true;
                                } else {
                                    i += (FX_DWORD)nLen;
                                }
                                if (m_CrossRef.GetSize() > (int32_t)objnum && m_CrossRef[objnum]) {
                                    if (pObject) {
                                        FX_DWORD oldgen = m_ObjVersion.GetAt(objnum);
                                        m_CrossRef[objnum] = obj_pos;
                                        m_ObjVersion.SetAt(objnum, (int16_t)gennum);
                                        if (oldgen != gennum) {
                                            m_bVersionUpdated = true;
                                        }
                                    }
                                } else {
                                    m_CrossRef.SetAtGrow(objnum, obj_pos);
                                    m_V5Type.SetAtGrow(objnum, 1);
                                    m_ObjVersion.SetAtGrow(objnum, (int16_t)gennum);
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
                                                bool bNumber = false;
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
    void* pResult = FXSYS_bsearch(&offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    if (pResult == NULL) {
        m_SortedOffset.Add(offset);
    }
    FX_Free(buffer);
    return true;
}
static FX_DWORD _GetVarInt(const uint8_t* p, int32_t n)
{
    FX_DWORD result = 0;
    for (int32_t i = 0; i < n; i ++) {
        result = result * 256 + p[i];
    }
    return result;
}
bool CPDF_Parser::LoadCrossRefV5(FX_FILESIZE pos, FX_FILESIZE& prev, bool bMainXRef)
{
    CPDF_Stream* pStream = (CPDF_Stream*)ParseIndirectObjectAt(m_pDocument, pos, 0, NULL);
    if (!pStream) {
        return false;
    }
    if (m_pDocument) {
        CPDF_Dictionary * pDict = m_pDocument->GetRoot();
        if (!pDict || pDict->GetObjNum() != pStream->m_ObjNum) {
            m_pDocument->InsertIndirectObject(pStream->m_ObjNum, pStream);
        } else {
            if (pStream->GetType() == PDFOBJ_STREAM) {
                pStream->Release();
            }
            return false;
        }
    }
    if (pStream->GetType() != PDFOBJ_STREAM) {
        return false;
    }
    prev = pStream->GetDict()->GetInteger(FX_BSTRC("Prev"));
    int32_t size = pStream->GetDict()->GetInteger(FX_BSTRC("Size"));
    if (size < 0) {
        pStream->Release();
        return false;
    }
    if (bMainXRef) {
        m_pTrailer = (CPDF_Dictionary*)pStream->GetDict()->Clone();
        m_CrossRef.SetSize(size);
        if (m_V5Type.SetSize(size)) {
            FXSYS_memset(m_V5Type.GetData(), 0, size);
        }
    } else {
        m_Trailers.Add((CPDF_Dictionary*)pStream->GetDict()->Clone());
    }
    std::vector<std::pair<int32_t, int32_t> > arrIndex;
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
        return false;
    }
    CFX_DWordArray WidthArray;
    FX_SAFE_DWORD dwAccWidth = 0;
    for (FX_DWORD i = 0; i < pArray->GetCount(); i ++) {
        WidthArray.Add(pArray->GetInteger(i));
        dwAccWidth += WidthArray[i];
    }
    if (!dwAccWidth.IsValid() || WidthArray.GetSize() < 3) {
        pStream->Release();
        return false;
    }
    FX_DWORD totalWidth = dwAccWidth.ValueOrDie();
    CPDF_StreamAcc acc;
    acc.LoadAllData(pStream);
    const uint8_t* pData = acc.GetData();
    FX_DWORD dwTotalSize = acc.GetSize();
    FX_DWORD segindex = 0;
    for (FX_DWORD i = 0; i < arrIndex.size(); i ++) {
        int32_t startnum = arrIndex[i].first;
        if (startnum < 0) {
            continue;
        }
        m_dwXrefStartObjNum = pdfium::base::checked_cast<FX_DWORD, int32_t> (startnum);
        FX_DWORD count = pdfium::base::checked_cast<FX_DWORD, int32_t> (arrIndex[i].second);
        FX_SAFE_DWORD dwCaculatedSize = segindex;
        dwCaculatedSize += count;
        dwCaculatedSize *= totalWidth;
        if (!dwCaculatedSize.IsValid() || dwCaculatedSize.ValueOrDie() > dwTotalSize) {
            continue;
        }
        const uint8_t* segstart = pData + segindex * totalWidth;
        FX_SAFE_DWORD dwMaxObjNum = startnum;
        dwMaxObjNum += count;
        FX_DWORD dwV5Size = pdfium::base::checked_cast<FX_DWORD, int32_t> (m_V5Type.GetSize());
        if (!dwMaxObjNum.IsValid() || dwMaxObjNum.ValueOrDie() > dwV5Size) {
            continue;
        }
        for (FX_DWORD j = 0; j < count; j ++) {
            int32_t type = 1;
            const uint8_t* entrystart = segstart + j * totalWidth;
            if (WidthArray[0]) {
                type = _GetVarInt(entrystart, WidthArray[0]);
            }
            if (m_V5Type[startnum + j] == 255) {
                FX_FILESIZE offset = _GetVarInt(entrystart + WidthArray[0], WidthArray[1]);
                m_CrossRef[startnum + j] = offset;
                void* pResult = FXSYS_bsearch(&offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
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
                    void* pResult = FXSYS_bsearch(&offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
                    if (pResult == NULL) {
                        m_SortedOffset.Add(offset);
                    }
                } else {
                    if (offset < 0 || offset >= m_V5Type.GetSize()) {
                        pStream->Release();
                        return false;
                    }
                    m_V5Type[offset] = 255;
                }
            }
        }
        segindex += count;
    }
    pStream->Release();
    return true;
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
bool CPDF_Parser::IsFormStream(FX_DWORD objnum, bool& bForm)
{
    bForm = false;
    if (objnum >= (FX_DWORD)m_CrossRef.GetSize()) {
        return true;
    }
    if (m_V5Type[objnum] == 0) {
        return true;
    }
    if (m_V5Type[objnum] == 2) {
        return true;
    }
    FX_FILESIZE pos = m_CrossRef[objnum];
    void* pResult = FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    if (pResult == NULL) {
        return true;
    }
    if ((FX_FILESIZE*)pResult - (FX_FILESIZE*)m_SortedOffset.GetData() == m_SortedOffset.GetSize() - 1) {
        return false;
    }
    FX_FILESIZE size = ((FX_FILESIZE*)pResult)[1] - pos;
    FX_FILESIZE SavedPos = m_Syntax.SavePos();
    m_Syntax.RestorePos(pos);
    bForm = m_Syntax.SearchMultiWord(FX_BSTRC("/Form\0stream"), true, size) == 0;
    m_Syntax.RestorePos(SavedPos);
    return true;
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
        int32_t n = pObjStream->GetDict()->GetInteger(FX_BSTRC("N"));
        int32_t offset = pObjStream->GetDict()->GetInteger(FX_BSTRC("First"));
        CPDF_SyntaxParser syntax;
        CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream((uint8_t*)pObjStream->GetData(), (size_t)pObjStream->GetSize(), false));
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
    if (m_ObjectStreamMap.Lookup((void*)(uintptr_t)objnum, (void*&)pStreamAcc)) {
        return pStreamAcc;
    }
    const CPDF_Stream* pStream = m_pDocument ? (CPDF_Stream*)m_pDocument->GetIndirectObject(objnum) : NULL;
    if (pStream == NULL || pStream->GetType() != PDFOBJ_STREAM) {
        return NULL;
    }
    pStreamAcc = new CPDF_StreamAcc;
    pStreamAcc->LoadAllData(pStream);
    m_ObjectStreamMap.SetAt((void*)(uintptr_t)objnum, pStreamAcc);
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
        void* pResult = FXSYS_bsearch(&offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
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
void CPDF_Parser::GetIndirectBinary(FX_DWORD objnum, uint8_t*& pBuffer, FX_DWORD& size)
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
        int32_t n = pObjStream->GetDict()->GetInteger(FX_BSTRC("N"));
        int32_t offset = pObjStream->GetDict()->GetInteger(FX_BSTRC("First"));
        CPDF_SyntaxParser syntax;
        const uint8_t* pData = pObjStream->GetData();
        FX_DWORD totalsize = pObjStream->GetSize();
        CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream((uint8_t*)pData, (size_t)totalsize, false));
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
                pBuffer = FX_Alloc(uint8_t, size);
                FXSYS_memcpy(pBuffer, pData + thisoff + offset, size);
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
        bool bIsNumber;
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
        void* pResult = FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
        if (pResult == NULL) {
            m_Syntax.RestorePos(SavedPos);
            return;
        }
        FX_FILESIZE nextoff = ((FX_FILESIZE*)pResult)[1];
        bool bNextOffValid = false;
        if (nextoff != pos) {
            m_Syntax.RestorePos(nextoff);
            word = m_Syntax.GetNextWord(bIsNumber);
            if (word == FX_BSTRC("xref")) {
                bNextOffValid = true;
            } else if (bIsNumber) {
                word = m_Syntax.GetNextWord(bIsNumber);
                if (bIsNumber && m_Syntax.GetKeyword() == FX_BSTRC("obj")) {
                    bNextOffValid = true;
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
        pBuffer = FX_Alloc(uint8_t, size);
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
    bool bIsNumber;
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
    bool bIsNumber;
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
FX_DWORD CPDF_Parser::GetPermissions(bool bCheckRevision)
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
bool CPDF_Parser::IsOwner()
{
    return m_pSecurityHandler == NULL ? true : m_pSecurityHandler->IsOwner();
}
void CPDF_Parser::SetSecurityHandler(CPDF_SecurityHandler* pSecurityHandler, bool bForced)
{
    ASSERT(m_pSecurityHandler == NULL);
    if (!m_bForceUseSecurityHandler) {
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
bool CPDF_Parser::IsLinearizedFile(IFX_FileRead* pFileAccess, FX_DWORD offset)
{
    m_Syntax.InitParser(pFileAccess, offset);
    m_Syntax.RestorePos(m_Syntax.m_HeaderOffset + 9);
    FX_FILESIZE SavedPos = m_Syntax.SavePos();
    bool bIsNumber;
    CFX_ByteString word = m_Syntax.GetNextWord(bIsNumber);
    if (!bIsNumber) {
        return false;
    }
    FX_DWORD objnum = FXSYS_atoi(word);
    word = m_Syntax.GetNextWord(bIsNumber);
    if (!bIsNumber) {
        return false;
    }
    FX_DWORD gennum = FXSYS_atoi(word);
    if (m_Syntax.GetKeyword() != FX_BSTRC("obj")) {
        m_Syntax.RestorePos(SavedPos);
        return false;
    }
    m_pLinearized = m_Syntax.GetObject(NULL, objnum, gennum, 0);
    if (!m_pLinearized) {
        return false;
    }
    if (m_pLinearized->GetDict() && m_pLinearized->GetDict()->GetElement(FX_BSTRC("Linearized"))) {
        m_Syntax.GetNextWord(bIsNumber);
        CPDF_Object *pLen = m_pLinearized->GetDict()->GetElement(FX_BSTRC("L"));
        if (!pLen) {
            m_pLinearized->Release();
            m_pLinearized = NULL;
            return false;
        }
        if (pLen->GetInteger() != (int)pFileAccess->GetSize()) {
            return false;
        }
        CPDF_Object *pNo = m_pLinearized->GetDict()->GetElement(FX_BSTRC("P"));
        if (pNo && pNo->GetType() == PDFOBJ_NUMBER) {
            m_dwFirstPageNo = pNo->GetInteger();
        }
        CPDF_Object *pTable = m_pLinearized->GetDict()->GetElement(FX_BSTRC("T"));
        if (pTable && pTable->GetType() == PDFOBJ_NUMBER) {
            m_LastXRefOffset = pTable->GetInteger();
        }
        return true;
    }
    m_pLinearized->Release();
    m_pLinearized = NULL;
    return false;
}
FX_DWORD CPDF_Parser::StartAsynParse(IFX_FileRead* pFileAccess, bool bReParse, bool bOwnFileRead)
{
    CloseParser(bReParse);
    m_bXRefStream = false;
    m_LastXRefOffset = 0;
    m_bOwnFileRead = bOwnFileRead;
    int32_t offset = GetHeaderOffset(pFileAccess);
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
    bool bXRefRebuilt = false;
    bool bLoadV4 = false;
    if (!(bLoadV4 = LoadCrossRefV4(dwFirstXRefOffset, 0, false, false)) && !LoadCrossRefV5(dwFirstXRefOffset, dwFirstXRefOffset, true)) {
        if (!RebuildCrossRef()) {
            return PDFPARSE_ERROR_FORMAT;
        }
        bXRefRebuilt = true;
        m_LastXRefOffset = 0;
    }
    if (bLoadV4) {
        m_pTrailer = LoadTrailerV4();
        if (m_pTrailer == NULL) {
            return false;
        }
        int32_t xrefsize = GetDirectInteger(m_pTrailer, FX_BSTRC("Size"));
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
bool CPDF_Parser::LoadLinearizedAllCrossRefV5(FX_FILESIZE xrefpos)
{
    if (!LoadCrossRefV5(xrefpos, xrefpos, false)) {
        return false;
    }
    while (xrefpos)
        if (!LoadCrossRefV5(xrefpos, xrefpos, false)) {
            return false;
        }
    m_ObjectStreamMap.InitHashTable(101, false);
    m_bXRefStream = true;
    return true;
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
    uint8_t ch = 0;
    FX_DWORD dwCount = 0;
    m_Syntax.GetNextChar(ch);
    int32_t type = PDF_CharType[ch];
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
        void* objnum;
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
    m_BufSize = CPDF_ModuleMgr::kFileBufSize;
    m_pFileBuf = NULL;
    m_MetadataObjnum = 0;
    m_dwWordPos = 0;
    m_bFileStream = false;
}
CPDF_SyntaxParser::~CPDF_SyntaxParser()
{
    if (m_pFileBuf) {
        FX_Free(m_pFileBuf);
    }
}
bool CPDF_SyntaxParser::GetCharAt(FX_FILESIZE pos, uint8_t& ch)
{
    FX_FILESIZE save_pos = m_Pos;
    m_Pos = pos;
    bool ret = GetNextChar(ch);
    m_Pos = save_pos;
    return ret;
}
bool CPDF_SyntaxParser::GetNextChar(uint8_t& ch)
{
    FX_FILESIZE pos = m_Pos + m_HeaderOffset;
    if (pos >= m_FileLen) {
        return false;
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
            return false;
        }
        m_BufOffset = read_pos;
    }
    ch = m_pFileBuf[pos - m_BufOffset];
    m_Pos ++;
    return true;
}
bool CPDF_SyntaxParser::GetCharAtBackward(FX_FILESIZE pos, uint8_t& ch)
{
    pos += m_HeaderOffset;
    if (pos >= m_FileLen) {
        return false;
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
            return false;
        }
        m_BufOffset = read_pos;
    }
    ch = m_pFileBuf[pos - m_BufOffset];
    return true;
}
bool CPDF_SyntaxParser::ReadBlock(uint8_t* pBuf, FX_DWORD size)
{
    if (!m_pFileAccess->ReadBlock(pBuf, m_Pos + m_HeaderOffset, size)) {
        return false;
    }
    m_Pos += size;
    return true;
}
#define MAX_WORD_BUFFER 256
void CPDF_SyntaxParser::GetNextWord()
{
    m_WordSize = 0;
    m_bIsNumber = true;
    uint8_t ch;
    if (!GetNextChar(ch)) {
        return;
    }
    uint8_t type = PDF_CharType[ch];
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
        m_bIsNumber = false;
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
            m_bIsNumber = false;
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
    uint8_t ch;
    if (!GetNextChar(ch)) {
        return CFX_ByteString();
    }
    CFX_ByteTextBuf buf;
    int32_t parlevel = 0;
    int32_t status = 0, iEscCode = 0;
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
    uint8_t ch;
    if (!GetNextChar(ch)) {
        return CFX_ByteString();
    }
    CFX_BinaryBuf buf;
    bool bFirst = true;
    uint8_t code = 0;
    while (1) {
        if (ch == '>') {
            break;
        }
        if (ch >= '0' && ch <= '9') {
            if (bFirst) {
                code = (ch - '0') * 16;
            } else {
                code += ch - '0';
                buf.AppendByte((uint8_t)code);
            }
            bFirst = !bFirst;
        } else if (ch >= 'A' && ch <= 'F') {
            if (bFirst) {
                code = (ch - 'A' + 10) * 16;
            } else {
                code += ch - 'A' + 10;
                buf.AppendByte((uint8_t)code);
            }
            bFirst = !bFirst;
        } else if (ch >= 'a' && ch <= 'f') {
            if (bFirst) {
                code = (ch - 'a' + 10) * 16;
            } else {
                code += ch - 'a' + 10;
                buf.AppendByte((uint8_t)code);
            }
            bFirst = !bFirst;
        }
        if (!GetNextChar(ch)) {
            break;
        }
    }
    if (!bFirst) {
        buf.AppendByte((uint8_t)code);
    }
    return buf.GetByteString();
}
void CPDF_SyntaxParser::ToNextLine()
{
    uint8_t ch;
    while (GetNextChar(ch)) {
        if (ch == '\n') {
            break;
        }
        if (ch == '\r') {
            GetNextChar(ch);
            if (ch != '\n') {
                --m_Pos;
            }
            break;
        }
    }
}
void CPDF_SyntaxParser::ToNextWord()
{
    uint8_t ch;
    if (!GetNextChar(ch)) {
        return;
    }
    uint8_t type = PDF_CharType[ch];
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
CFX_ByteString CPDF_SyntaxParser::GetNextWord(bool& bIsNumber)
{
    GetNextWord();
    bIsNumber = m_bIsNumber;
    return CFX_ByteString((const FX_CHAR*)m_WordBuffer, m_WordSize);
}
CFX_ByteString CPDF_SyntaxParser::GetKeyword()
{
    GetNextWord();
    return CFX_ByteString((const FX_CHAR*)m_WordBuffer, m_WordSize);
}
CPDF_Object* CPDF_SyntaxParser::GetObject(CPDF_IndirectObjects* pObjList, FX_DWORD objnum, FX_DWORD gennum, PARSE_CONTEXT* pContext, bool bDecrypt)
{
    CFX_AutoRestorer<int> restorer(&s_CurrentRecursionDepth);
    if (++s_CurrentRecursionDepth > kParserMaxRecursionDepth) {
        return NULL;
    }
    FX_FILESIZE SavedPos = m_Pos;
    bool bTypeOnly = pContext && (pContext->m_Flags & PDFPARSE_TYPEONLY);
    bool bIsNumber;
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
                return new CPDF_Reference(pObjList, objnum);
            }
        }
        m_Pos = SavedPos;
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_NUMBER;
        }
        return CPDF_Number::Create(word);
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
        if (m_pCryptoHandler && bDecrypt) {
            m_pCryptoHandler->Decrypt(objnum, gennum, str);
        }
        return CPDF_String::Create(str, false);
    }
    if (word == FX_BSTRC("<")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_STRING;
        }
        CFX_ByteString str = ReadHexString();
        if (m_pCryptoHandler && bDecrypt) {
            m_pCryptoHandler->Decrypt(objnum, gennum, str);
        }
        return CPDF_String::Create(str, true);
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
        return CPDF_Name::Create(
            PDF_NameDecode(CFX_ByteStringC(m_WordBuffer + 1, m_WordSize - 1)));
    }
    if (word == FX_BSTRC("<<")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_DICTIONARY;
        }
        if (pContext) {
            pContext->m_DictStart = SavedPos;
        }
        CPDF_Dictionary* pDict = CPDF_Dictionary::Create();
        int32_t nKeys = 0;
        FX_FILESIZE dwSignValuePos = 0;
        while (1) {
            bool bIsNumber;
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
            CPDF_Object* pObj = GetObject(pObjList, objnum, gennum, NULL, false);
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
        bool bIsNumber;
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
    bool bTypeOnly = pContext && (pContext->m_Flags & PDFPARSE_TYPEONLY);
    bool bIsNumber;
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
                if (bTypeOnly) {
                    return (CPDF_Object*)PDFOBJ_REFERENCE;
                }
                FX_DWORD objnum = FXSYS_atoi(word);
                return new CPDF_Reference(pObjList, objnum);
            }
        }
        m_Pos = SavedPos;
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_NUMBER;
        }
        return CPDF_Number::Create(word);
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
        return CPDF_String::Create(str, false);
    }
    if (word == FX_BSTRC("<")) {
        if (bTypeOnly) {
            return (CPDF_Object*)PDFOBJ_STRING;
        }
        CFX_ByteString str = ReadHexString();
        if (m_pCryptoHandler) {
            m_pCryptoHandler->Decrypt(objnum, gennum, str);
        }
        return CPDF_String::Create(str, true);
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
            bool bIsNumber;
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
                uint8_t ch;
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
        bool bIsNumber;
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
        if (m_WordSize < 9 || FXSYS_memcmp(m_WordBuffer, "endstream", 9)) {
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
                uint8_t byte1, byte2;
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
    uint8_t* pData = FX_Alloc(uint8_t, len);
    ReadBlock(pData, len);
    if (pCryptoHandler) {
        CFX_BinaryBuf dest_buf;
        dest_buf.EstimateSize(pCryptoHandler->DecryptGetSize(len));
        void* context = pCryptoHandler->DecryptStart(objnum, gennum);
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
    if (m_WordSize == 6 && 0 == FXSYS_memcmp(m_WordBuffer, "endobj", 6)) {
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
    m_pFileBuf = FX_Alloc(uint8_t, m_BufSize);
    m_HeaderOffset = HeaderOffset;
    m_FileLen = pFileAccess->GetSize();
    m_Pos = 0;
    m_pFileAccess = pFileAccess;
    m_BufOffset = 0;
    pFileAccess->ReadBlock(m_pFileBuf, 0, (size_t)((FX_FILESIZE)m_BufSize > m_FileLen ? m_FileLen : m_BufSize));
}
int32_t CPDF_SyntaxParser::GetDirectNum()
{
    GetNextWord();
    if (!m_bIsNumber) {
        return 0;
    }
    m_WordBuffer[m_WordSize] = 0;
    return FXSYS_atoi((const FX_CHAR*)m_WordBuffer);
}
bool CPDF_SyntaxParser::IsWholeWord(FX_FILESIZE startpos, FX_FILESIZE limit, const uint8_t* tag, FX_DWORD taglen)
{
    uint8_t type = PDF_CharType[tag[0]];
    bool bCheckLeft = type != 'D' && type != 'W';
    type = PDF_CharType[tag[taglen - 1]];
    bool bCheckRight = type != 'D' && type != 'W';
    uint8_t ch;
    if (bCheckRight && startpos + (int32_t)taglen <= limit && GetCharAt(startpos + (int32_t)taglen, ch)) {
        uint8_t type = PDF_CharType[ch];
        if (type == 'N' || type == 'R') {
            return false;
        }
    }
    if (bCheckLeft && startpos > 0 && GetCharAt(startpos - 1, ch)) {
        uint8_t type = PDF_CharType[ch];
        if (type == 'N' || type == 'R') {
            return false;
        }
    }
    return true;
}
bool CPDF_SyntaxParser::SearchWord(const CFX_ByteStringC& tag, bool bWholeWord, bool bForward, FX_FILESIZE limit)
{
    int32_t taglen = tag.GetLength();
    if (taglen == 0) {
        return false;
    }
    FX_FILESIZE pos = m_Pos;
    int32_t offset = 0;
    if (!bForward) {
        offset = taglen - 1;
    }
    const uint8_t* tag_data = tag.GetPtr();
    uint8_t byte;
    while (1) {
        if (bForward) {
            if (limit) {
                if (pos >= m_Pos + limit) {
                    return false;
                }
            }
            if (!GetCharAt(pos, byte)) {
                return false;
            }
        } else {
            if (limit) {
                if (pos <= m_Pos - limit) {
                    return false;
                }
            }
            if (!GetCharAtBackward(pos, byte)) {
                return false;
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
                return true;
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
            return false;
        }
    }
    return false;
}
struct _SearchTagRecord {
    const uint8_t*	m_pTag;
    FX_DWORD	m_Len;
    FX_DWORD	m_Offset;
};
int32_t CPDF_SyntaxParser::SearchMultiWord(const CFX_ByteStringC& tags, bool bWholeWord, FX_FILESIZE limit)
{
    int32_t ntags = 1, i;
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
    uint8_t byte;
    GetCharAt(pos++, byte);
    int32_t found = -1;
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
FX_FILESIZE CPDF_SyntaxParser::FindTag(const CFX_ByteStringC& tag, FX_FILESIZE limit)
{
    int32_t taglen = tag.GetLength();
    int32_t match = 0;
    limit += m_Pos;
    FX_FILESIZE startpos = m_Pos;
    while (1) {
        uint8_t ch;
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
void CPDF_SyntaxParser::GetBinary(uint8_t* buffer, FX_DWORD size)
{
    FX_DWORD offset = 0;
    uint8_t ch;
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

class CPDF_DataAvail final : public IPDF_DataAvail
{
public:
    CPDF_DataAvail(IFX_FileAvail* pFileAvail, IFX_FileRead* pFileRead);
    ~CPDF_DataAvail();

    virtual bool                     IsDocAvail(IFX_DownloadHints* pHints)  override;

    virtual void                        SetDocument(CPDF_Document* pDoc)  override;

    virtual bool                     IsPageAvail(int iPage, IFX_DownloadHints* pHints)  override;

    virtual int32_t                     IsFormAvail(IFX_DownloadHints *pHints)  override;

    virtual int32_t                     IsLinearizedPDF()  override;

    virtual bool                     IsLinearized()  override
    {
        return m_bLinearized;
    }

    virtual void                        GetLinearizedMainXRefInfo(FX_FILESIZE *pPos, FX_DWORD *pSize)  override;

protected:
    static const int kMaxDataAvailRecursionDepth = 64;
    static int s_CurrentDataAvailRecursionDepth;

    FX_DWORD                            GetObjectSize(FX_DWORD objnum, FX_FILESIZE& offset);
    bool                             IsObjectsAvail(CFX_PtrArray& obj_array, bool bParsePage, IFX_DownloadHints* pHints, CFX_PtrArray &ret_array);
    bool                             CheckDocStatus(IFX_DownloadHints *pHints);
    bool                             CheckHeader(IFX_DownloadHints* pHints);
    bool                             CheckFirstPage(IFX_DownloadHints *pHints);
    bool                             CheckEnd(IFX_DownloadHints *pHints);
    bool                             CheckCrossRef(IFX_DownloadHints* pHints);
    bool                             CheckCrossRefItem(IFX_DownloadHints *pHints);
    bool                             CheckTrailer(IFX_DownloadHints* pHints);
    bool                             CheckRoot(IFX_DownloadHints* pHints);
    bool                             CheckInfo(IFX_DownloadHints* pHints);
    bool                             CheckPages(IFX_DownloadHints* pHints);
    bool                             CheckPage(IFX_DownloadHints* pHints);
    bool                             CheckResources(IFX_DownloadHints* pHints);
    bool                             CheckAnnots(IFX_DownloadHints* pHints);
    bool                             CheckAcroForm(IFX_DownloadHints* pHints);
    bool                             CheckAcroFormSubObject(IFX_DownloadHints* pHints);
    bool                             CheckTrailerAppend(IFX_DownloadHints* pHints);
    bool                             CheckPageStatus(IFX_DownloadHints* pHints);
    bool                             CheckAllCrossRefStream(IFX_DownloadHints *pHints);

    int32_t                            CheckCrossRefStream(IFX_DownloadHints *pHints, FX_FILESIZE &xref_offset);
    bool                             IsLinearizedFile(uint8_t* pData, FX_DWORD dwLen);
    void                                SetStartOffset(FX_FILESIZE dwOffset);
    bool                             GetNextToken(CFX_ByteString &token);
    bool                             GetNextChar(uint8_t &ch);
    CPDF_Object	*                       ParseIndirectObjectAt(FX_FILESIZE pos, FX_DWORD objnum);
    CPDF_Object	*                       GetObject(FX_DWORD objnum, IFX_DownloadHints* pHints, bool *pExistInFile);
    bool                             GetPageKids(CPDF_Parser *pParser, CPDF_Object *pPages);
    bool                             PreparePageItem();
    bool                             LoadPages(IFX_DownloadHints* pHints);
    bool                             LoadAllXref(IFX_DownloadHints* pHints);
    bool                             LoadAllFile(IFX_DownloadHints* pHints);
    bool                             CheckLinearizedData(IFX_DownloadHints* pHints);
    bool                             CheckFileResources(IFX_DownloadHints* pHints);
    bool                             CheckPageAnnots(int iPage, IFX_DownloadHints* pHints);

    bool                             CheckLinearizedFirstPage(int iPage, IFX_DownloadHints* pHints);
    bool                             HaveResourceAncestor(CPDF_Dictionary *pDict);
    bool                             CheckPage(int32_t iPage, IFX_DownloadHints* pHints);
    bool                             LoadDocPages(IFX_DownloadHints* pHints);
    bool                             LoadDocPage(int32_t iPage, IFX_DownloadHints* pHints);
    bool                             CheckPageNode(CPDF_PageNode &pageNodes, int32_t iPage, int32_t &iCount, IFX_DownloadHints* pHints);
    bool                             CheckUnkownPageNode(FX_DWORD dwPageNo, CPDF_PageNode *pPageNode, IFX_DownloadHints* pHints);
    bool                             CheckArrayPageNode(FX_DWORD dwPageNo, CPDF_PageNode *pPageNode, IFX_DownloadHints* pHints);
    bool                             CheckPageCount(IFX_DownloadHints* pHints);
    bool                             IsFirstCheck(int iPage);
    void                                ResetFirstCheck(int iPage);

    CPDF_Parser                         m_parser;

    CPDF_SyntaxParser                   m_syntaxParser;

    CPDF_Object                         *m_pRoot;

    FX_DWORD                            m_dwRootObjNum;

    FX_DWORD                            m_dwInfoObjNum;

    CPDF_Object                         *m_pLinearized;

    CPDF_Object                         *m_pTrailer;

    bool                             m_bDocAvail;

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

    uint8_t                             m_WordBuffer[257];

    FX_DWORD                            m_WordSize;

    uint8_t                             m_bufferData[512];

    CFX_FileSizeArray                   m_CrossOffset;

    CFX_DWordArray                      m_XRefStreamList;

    CFX_DWordArray                      m_PageObjList;

    FX_DWORD                            m_PagesObjNum;

    bool                             m_bLinearized;

    FX_DWORD                            m_dwFirstPageNo;

    bool                             m_bLinearedDataOK;

    bool                             m_bMainXRefLoadTried;

    bool                             m_bMainXRefLoadedOK;

    bool                             m_bPagesTreeLoad;

    bool                             m_bPagesLoad;

    CPDF_Parser *                       m_pCurrentParser;

    FX_FILESIZE                         m_dwCurrentXRefSteam;

    bool                             m_bAnnotsLoad;

    bool                             m_bHaveAcroForm;

    FX_DWORD                            m_dwAcroFormObjNum;

    bool                             m_bAcroFormLoad;

    CPDF_Object	*                       m_pAcroForm;

    CFX_PtrArray                        m_arrayAcroforms;

    CPDF_Dictionary *                   m_pPageDict;

    CPDF_Object *                       m_pPageResource;

    bool                             m_bNeedDownLoadResource;

    bool                             m_bPageLoadedOK;

    bool                             m_bLinearizedFormParamLoad;

    CFX_PtrArray                        m_PagesArray;

    FX_DWORD                            m_dwEncryptObjNum;

    FX_FILESIZE                         m_dwPrevXRefOffset;

    bool                             m_bTotalLoadPageTree;

    bool                             m_bCurPageDictLoadOK;

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
    m_bDocAvail = false;
    m_bMainXRefLoadTried = false;
    m_bDocAvail = false;
    m_bLinearized = false;
    m_bPagesLoad = false;
    m_bPagesTreeLoad = false;
    m_bMainXRefLoadedOK = false;
    m_bAnnotsLoad = false;
    m_bHaveAcroForm = false;
    m_bAcroFormLoad = false;
    m_bPageLoadedOK = false;
    m_bNeedDownLoadResource = false;
    m_bLinearizedFormParamLoad = false;
    m_pLinearized = NULL;
    m_pRoot = NULL;
    m_pTrailer = NULL;
    m_pCurrentParser = NULL;
    m_pAcroForm = NULL;
    m_pPageDict = NULL;
    m_pPageResource = NULL;
    m_pageMapCheckState = NULL;
    m_docStatus = PDF_DATAAVAIL_HEADER;
    m_parser.m_bOwnFileRead = false;
    m_bTotalLoadPageTree = false;
    m_bCurPageDictLoadOK = false;
    m_bLinearedDataOK = false;
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
    delete m_pageMapCheckState;
    delete m_pagesLoadState;
    int32_t i = 0;
    int32_t iSize = m_arrayAcroforms.GetSize();
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
        void* pResult = FXSYS_bsearch(&offset, pParser->m_SortedOffset.GetData(), pParser->m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
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
bool CPDF_DataAvail::IsObjectsAvail(CFX_PtrArray& obj_array, bool bParsePage, IFX_DownloadHints* pHints, CFX_PtrArray &ret_array)
{
    if (!obj_array.GetSize()) {
        return true;
    }
    FX_DWORD count = 0;
    CFX_PtrArray new_obj_array;
    int32_t i = 0;
    for (i = 0; i < obj_array.GetSize(); i++) {
        CPDF_Object *pObj = (CPDF_Object *)obj_array[i];
        if (!pObj) {
            continue;
        }
        int32_t type = pObj->GetType();
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
        int32_t iSize = new_obj_array.GetSize();
        for (i = 0; i < iSize; ++i) {
            CPDF_Object *pObj = (CPDF_Object *)new_obj_array[i];
            int32_t type = pObj->GetType();
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
        return false;
    }
    obj_array.RemoveAll();
    obj_array.Append(new_obj_array);
    return IsObjectsAvail(obj_array, false, pHints, ret_array);
}
bool CPDF_DataAvail::IsDocAvail(IFX_DownloadHints* pHints)
{
    if (!m_dwFileLen && m_pFileRead) {
        m_dwFileLen = (FX_DWORD)m_pFileRead->GetSize();
        if (!m_dwFileLen) {
            return true;
        }
    }
    while (!m_bDocAvail) {
        if (!CheckDocStatus(pHints)) {
            return false;
        }
    }
    return true;
}
bool CPDF_DataAvail::CheckAcroFormSubObject(IFX_DownloadHints* pHints)
{
    if (!m_objs_array.GetSize()) {
        m_objs_array.RemoveAll();
        m_objnum_array.RemoveAll();
        CFX_PtrArray obj_array;
        obj_array.Append(m_arrayAcroforms);
        bool bRet = IsObjectsAvail(obj_array, false, pHints, m_objs_array);
        if (bRet) {
            m_objs_array.RemoveAll();
        }
        return bRet;
    }
    CFX_PtrArray new_objs_array;
    bool bRet = IsObjectsAvail(m_objs_array, false, pHints, new_objs_array);
    if (bRet) {
        int32_t iSize = m_arrayAcroforms.GetSize();
        for (int32_t i = 0; i < iSize; ++i) {
            ((CPDF_Object *)m_arrayAcroforms.GetAt(i))->Release();
        }
        m_arrayAcroforms.RemoveAll();
    } else {
        m_objs_array.RemoveAll();
        m_objs_array.Append(new_objs_array);
    }
    return bRet;
}
bool CPDF_DataAvail::CheckAcroForm(IFX_DownloadHints* pHints)
{
    bool bExist = false;
    m_pAcroForm = GetObject(m_dwAcroFormObjNum, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_PAGETREE;
        return true;
    }
    if (!m_pAcroForm) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
            return true;
        }
        return false;
    }
    m_arrayAcroforms.Add(m_pAcroForm);
    m_docStatus = PDF_DATAAVAIL_PAGETREE;
    return true;
}
bool CPDF_DataAvail::CheckDocStatus(IFX_DownloadHints *pHints)
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
            }
            return LoadDocPages(pHints);
        case PDF_DATAAVAIL_PAGE:
            if (m_bTotalLoadPageTree) {
                return CheckPage(pHints);
            }
            m_docStatus = PDF_DATAAVAIL_PAGE_LATERLOAD;
            return true;
        case PDF_DATAAVAIL_ERROR:
            return LoadAllFile(pHints);
        case PDF_DATAAVAIL_PAGE_LATERLOAD:
            m_docStatus = PDF_DATAAVAIL_PAGE;
        default:
            m_bDocAvail = true;
            return true;
    }
}
bool	CPDF_DataAvail::CheckPageStatus(IFX_DownloadHints* pHints)
{
    switch (m_docStatus) {
        case PDF_DATAAVAIL_PAGETREE:
            return CheckPages(pHints);
        case PDF_DATAAVAIL_PAGE:
            return CheckPage(pHints);
        case PDF_DATAAVAIL_ERROR:
            return LoadAllFile(pHints);
        default:
            m_bPagesTreeLoad = true;
            m_bPagesLoad = true;
            return true;
    }
}
bool CPDF_DataAvail::LoadAllFile(IFX_DownloadHints* pHints)
{
    if (m_pFileAvail->IsDataAvail(0, (FX_DWORD)m_dwFileLen)) {
        m_docStatus = PDF_DATAAVAIL_DONE;
        return true;
    }
    pHints->AddSegment(0, (FX_DWORD)m_dwFileLen);
    return false;
}
bool CPDF_DataAvail::LoadAllXref(IFX_DownloadHints* pHints)
{
    m_parser.m_Syntax.InitParser(m_pFileRead, (FX_DWORD)m_dwHeaderOffset);
    m_parser.m_bOwnFileRead = false;
    if (!m_parser.LoadAllCrossRefV4(m_dwLastXRefOffset) && !m_parser.LoadAllCrossRefV5(m_dwLastXRefOffset)) {
        m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
        return false;
    }
    FXSYS_qsort(m_parser.m_SortedOffset.GetData(), m_parser.m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), _CompareFileSize);
    m_dwRootObjNum = m_parser.GetRootObjNum();
    m_dwInfoObjNum = m_parser.GetInfoObjNum();
    m_pCurrentParser = &m_parser;
    m_docStatus = PDF_DATAAVAIL_ROOT;
    return true;
}
CPDF_Object* CPDF_DataAvail::GetObject(FX_DWORD objnum, IFX_DownloadHints* pHints, bool *pExistInFile)
{
    CPDF_Object *pRet         = NULL;
    FX_DWORD    original_size = 0;
    FX_FILESIZE offset        = 0;
    CPDF_Parser *pParser      = NULL;

    if (pExistInFile) {
        *pExistInFile = true;
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
           *pExistInFile = false;

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
        *pExistInFile = false;
    }

    return pRet;
}

bool CPDF_DataAvail::CheckInfo(IFX_DownloadHints* pHints)
{
    bool bExist = false;
    CPDF_Object *pInfo = GetObject(m_dwInfoObjNum, pHints, &bExist);
    if (!bExist) {
        if (m_bHaveAcroForm) {
            m_docStatus = PDF_DATAAVAIL_ACROFORM;
        } else {
            m_docStatus = PDF_DATAAVAIL_PAGETREE;
        }
        return true;
    }
    if (!pInfo) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
            return true;
        }
        if (m_Pos == m_dwFileLen) {
            m_docStatus = PDF_DATAAVAIL_ERROR;
        }
        return false;
    }
    if (pInfo) {
        pInfo->Release();
    }
    if (m_bHaveAcroForm) {
        m_docStatus = PDF_DATAAVAIL_ACROFORM;
    } else {
        m_docStatus = PDF_DATAAVAIL_PAGETREE;
    }
    return true;
}
bool CPDF_DataAvail::CheckRoot(IFX_DownloadHints* pHints)
{
    bool bExist = false;
    m_pRoot = GetObject(m_dwRootObjNum, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
        return true;
    }
    if (!m_pRoot) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
            return true;
        }
        return false;
    }
    CPDF_Dictionary* pDict = m_pRoot->GetDict();
    if (!pDict) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    CPDF_Reference* pRef = (CPDF_Reference*)pDict->GetElement(FX_BSTRC("Pages"));
    if (pRef == NULL || pRef->GetType() != PDFOBJ_REFERENCE) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    m_PagesObjNum = pRef->GetRefObjNum();
    CPDF_Reference* pAcroFormRef = (CPDF_Reference*)m_pRoot->GetDict()->GetElement(FX_BSTRC("AcroForm"));
    if (pAcroFormRef && pAcroFormRef->GetType() == PDFOBJ_REFERENCE) {
        m_bHaveAcroForm = true;
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
    return true;
}
bool CPDF_DataAvail::PreparePageItem()
{
    CPDF_Dictionary *pRoot = m_pDocument->GetRoot();
    CPDF_Reference* pRef = pRoot ? (CPDF_Reference*)pRoot->GetElement(FX_BSTRC("Pages")) : NULL;
    if (pRef == NULL || pRef->GetType() != PDFOBJ_REFERENCE) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    m_PagesObjNum = pRef->GetRefObjNum();
    m_pCurrentParser = (CPDF_Parser *)m_pDocument->GetParser();
    m_docStatus = PDF_DATAAVAIL_PAGETREE;
    return true;
}
bool CPDF_DataAvail::IsFirstCheck(int iPage)
{
    if (NULL == m_pageMapCheckState) {
        m_pageMapCheckState = new CFX_CMapDWordToDWord();
    }
    FX_DWORD dwValue = 0;
    if (!m_pageMapCheckState->Lookup(iPage, dwValue)) {
        m_pageMapCheckState->SetAt(iPage, 1);
        return true;
    }
    if (dwValue != 0) {
        return false;
    }
    m_pageMapCheckState->SetAt(iPage, 1);
    return true;
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
bool CPDF_DataAvail::CheckPage(IFX_DownloadHints* pHints)
{
    FX_DWORD iPageObjs = m_PageObjList.GetSize();
    CFX_DWordArray UnavailObjList;
    for (FX_DWORD i = 0; i < iPageObjs; ++i) {
        FX_DWORD dwPageObjNum = m_PageObjList.GetAt(i);
        bool bExist = false;
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
                int32_t iSize = pArray->GetCount();
                CPDF_Object *pItem = NULL;
                for (int32_t j = 0; j < iSize; ++j) {
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
        return false;
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
            return false;
        }
        pPages->Release();
    }
    m_PagesArray.RemoveAll();
    if (!m_PageObjList.GetSize()) {
        m_docStatus = PDF_DATAAVAIL_DONE;
    }
    return true;
}
bool CPDF_DataAvail::GetPageKids(CPDF_Parser *pParser, CPDF_Object *pPages)
{
    if (!pParser) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    CPDF_Dictionary* pDict = pPages->GetDict();
    CPDF_Object *pKids = pDict ? pDict->GetElement(FX_BSTRC("Kids")) : NULL;
    if (!pKids) {
        return true;
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
            return false;
    }
    return true;
}
bool CPDF_DataAvail::CheckPages(IFX_DownloadHints* pHints)
{
    bool bExist = false;
    CPDF_Object *pPages = GetObject(m_PagesObjNum, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
        return true;
    }
    if (!pPages) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
            return true;
        }
        return false;
    }
    if (!GetPageKids(m_pCurrentParser, pPages)) {
        pPages->Release();
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    pPages->Release();
    m_docStatus = PDF_DATAAVAIL_PAGE;
    return true;
}
bool CPDF_DataAvail::CheckHeader(IFX_DownloadHints* pHints)
{
    FX_DWORD req_size = 1024;
    if ((FX_FILESIZE)req_size > m_dwFileLen) {
        req_size = (FX_DWORD)m_dwFileLen;
    }
    if (m_pFileAvail->IsDataAvail(0, req_size)) {
        uint8_t buffer[1024];
        m_pFileRead->ReadBlock(buffer, 0, req_size);
        if (IsLinearizedFile(buffer, req_size)) {
            m_docStatus = PDF_DATAAVAIL_FIRSTPAGE;
        } else {
            if (m_docStatus == PDF_DATAAVAIL_ERROR) {
                return false;
            }
            m_docStatus = PDF_DATAAVAIL_END;
        }
        return true;
    }
    pHints->AddSegment(0, req_size);
    return false;
}
bool CPDF_DataAvail::CheckFirstPage(IFX_DownloadHints *pHints)
{
    CPDF_Dictionary* pDict = m_pLinearized->GetDict();
    CPDF_Object *pEndOffSet = pDict ? pDict->GetElement(FX_BSTRC("E")) : NULL;
    if (!pEndOffSet) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    CPDF_Object *pXRefOffset = pDict ? pDict->GetElement(FX_BSTRC("T")) : NULL;
    if (!pXRefOffset) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    CPDF_Object *pFileLen = pDict ? pDict->GetElement(FX_BSTRC("L")) : NULL;
    if (!pFileLen) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    bool bNeedDownLoad = false;
    if (pEndOffSet->GetType() == PDFOBJ_NUMBER) {
        FX_DWORD dwEnd = pEndOffSet->GetInteger();
        dwEnd += 512;
        if ((FX_FILESIZE)dwEnd > m_dwFileLen) {
            dwEnd = (FX_DWORD)m_dwFileLen;
        }
        int32_t iStartPos = (int32_t)(m_dwFileLen > 1024 ? 1024 : m_dwFileLen);
        int32_t iSize = dwEnd > 1024 ? dwEnd - 1024 : 0;
        if (!m_pFileAvail->IsDataAvail(iStartPos, iSize)) {
            pHints->AddSegment(iStartPos, iSize);
            bNeedDownLoad = true;
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
        return true;
    }
    m_docStatus = PDF_DATAAVAIL_FIRSTPAGE_PREPARE;
    return false;
}
CPDF_Object	* CPDF_DataAvail::ParseIndirectObjectAt(FX_FILESIZE pos, FX_DWORD objnum)
{
    FX_FILESIZE SavedPos = m_syntaxParser.SavePos();
    m_syntaxParser.RestorePos(pos);
    bool bIsNumber;
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
int32_t CPDF_DataAvail::IsLinearizedPDF()
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
    uint8_t buffer[1024];
    m_pFileRead->ReadBlock(buffer, 0, req_size);
    if (IsLinearizedFile(buffer, req_size)) {
        return PDF_IS_LINEARIZED;
    }
    return PDF_NOT_LINEARIZED;
}
bool CPDF_DataAvail::IsLinearizedFile(uint8_t* pData, FX_DWORD dwLen)
{
    CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream(pData, (size_t)dwLen, false));
    int32_t offset = GetHeaderOffset(file.Get());
    if (offset == -1) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    m_dwHeaderOffset = offset;
    m_syntaxParser.InitParser(file.Get(), offset);
    m_syntaxParser.RestorePos(m_syntaxParser.m_HeaderOffset + 9);
    bool bNumber = false;
    CFX_ByteString wordObjNum = m_syntaxParser.GetNextWord(bNumber);
    if (!bNumber) {
        return false;
    }
    FX_DWORD objnum = FXSYS_atoi(wordObjNum);
    if (m_pLinearized) {
        m_pLinearized->Release();
        m_pLinearized = NULL;
    }
    m_pLinearized = ParseIndirectObjectAt(m_syntaxParser.m_HeaderOffset + 9, objnum);
    if (!m_pLinearized) {
        return false;
    }
    if (m_pLinearized->GetDict() && m_pLinearized->GetDict()->GetElement(FX_BSTRC("Linearized"))) {
        CPDF_Object *pLen = m_pLinearized->GetDict()->GetElement(FX_BSTRC("L"));
        if (!pLen) {
            return false;
        }
        if ((FX_FILESIZE)pLen->GetInteger() != m_pFileRead->GetSize()) {
            return false;
        }
        m_bLinearized = true;
        CPDF_Object *pNo = m_pLinearized->GetDict()->GetElement(FX_BSTRC("P"));
        if (pNo && pNo->GetType() == PDFOBJ_NUMBER) {
            m_dwFirstPageNo = pNo->GetInteger();
        }
        return true;
    }
    return false;
}
bool CPDF_DataAvail::CheckEnd(IFX_DownloadHints* pHints)
{
    FX_DWORD req_pos = (FX_DWORD)(m_dwFileLen > 1024 ? m_dwFileLen - 1024 : 0);
    FX_DWORD dwSize = (FX_DWORD)(m_dwFileLen - req_pos);
    if (m_pFileAvail->IsDataAvail(req_pos, dwSize)) {
        uint8_t buffer[1024];
        m_pFileRead->ReadBlock(buffer, req_pos, dwSize);
        CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream(buffer, (size_t)dwSize, false));
        m_syntaxParser.InitParser(file.Get(), 0);
        m_syntaxParser.RestorePos(dwSize - 1);
        if (m_syntaxParser.SearchWord(FX_BSTRC("startxref"), true, false, dwSize)) {
            bool bNumber;
            m_syntaxParser.GetNextWord(bNumber);
            CFX_ByteString xrefpos_str = m_syntaxParser.GetNextWord(bNumber);
            if (!bNumber) {
                m_docStatus = PDF_DATAAVAIL_ERROR;
                return false;
            }
            m_dwXRefOffset = (FX_FILESIZE)FXSYS_atoi64(xrefpos_str);
            if (!m_dwXRefOffset || m_dwXRefOffset > m_dwFileLen) {
                m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
                return true;
            }
            m_dwLastXRefOffset = m_dwXRefOffset;
            SetStartOffset(m_dwXRefOffset);
            m_docStatus = PDF_DATAAVAIL_CROSSREF;
            return true;
        }
        m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
        return true;
    }
    pHints->AddSegment(req_pos, dwSize);
    return false;
}
int32_t CPDF_DataAvail::CheckCrossRefStream(IFX_DownloadHints* pHints, FX_FILESIZE &xref_offset)
{
    xref_offset = 0;
    FX_DWORD req_size = (FX_DWORD)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
    if (m_pFileAvail->IsDataAvail(m_Pos, req_size)) {
        int32_t iSize = (int32_t)(m_Pos + req_size - m_dwCurrentXRefSteam);
        CFX_BinaryBuf buf(iSize);
        uint8_t* pBuf = buf.GetBuffer();
        m_pFileRead->ReadBlock(pBuf, m_dwCurrentXRefSteam, iSize);
        CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream(pBuf, (size_t)iSize, false));
        m_parser.m_Syntax.InitParser(file.Get(), 0);
        bool bNumber = false;
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
bool CPDF_DataAvail::GetNextToken(CFX_ByteString &token)
{
    m_WordSize = 0;
    uint8_t ch;
    if (!GetNextChar(ch)) {
        return false;
    }
    uint8_t type = PDF_CharType[ch];
    while (1) {
        while (type == 'W') {
            if (!GetNextChar(ch)) {
                return false;
            }
            type = PDF_CharType[ch];
        }
        if (ch != '%') {
            break;
        }
        while (1) {
            if (!GetNextChar(ch)) {
                return false;
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
                    return false;
                }
                type = PDF_CharType[ch];
                if (type != 'R' && type != 'N') {
                    m_Pos --;
                    CFX_ByteString ret(m_WordBuffer, m_WordSize);
                    token = ret;
                    return true;
                }
                if (m_WordSize < MAX_WORD_BUFFER) {
                    m_WordBuffer[m_WordSize++] = ch;
                }
            }
        } else if (ch == '<') {
            if (!GetNextChar(ch)) {
                return false;
            }
            if (ch == '<') {
                m_WordBuffer[m_WordSize++] = ch;
            } else {
                m_Pos --;
            }
        } else if (ch == '>') {
            if (!GetNextChar(ch)) {
                return false;
            }
            if (ch == '>') {
                m_WordBuffer[m_WordSize++] = ch;
            } else {
                m_Pos --;
            }
        }
        CFX_ByteString ret(m_WordBuffer, m_WordSize);
        token = ret;
        return true;
    }
    while (1) {
        if (m_WordSize < MAX_WORD_BUFFER) {
            m_WordBuffer[m_WordSize++] = ch;
        }
        if (!GetNextChar(ch)) {
            return false;
        }
        type = PDF_CharType[ch];
        if (type == 'D' || type == 'W') {
            m_Pos --;
            break;
        }
    }
    CFX_ByteString ret(m_WordBuffer, m_WordSize);
    token = ret;
    return true;
}
bool CPDF_DataAvail::GetNextChar(uint8_t &ch)
{
    FX_FILESIZE pos = m_Pos;
    if (pos >= m_dwFileLen) {
        return false;
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
            return false;
        }
        m_bufferOffset = read_pos;
        m_bufferSize = read_size;
    }
    ch = m_bufferData[pos - m_bufferOffset];
    m_Pos ++;
    return true;
}
bool CPDF_DataAvail::CheckCrossRefItem(IFX_DownloadHints *pHints)
{
    int32_t iSize = 0;
    CFX_ByteString token;
    while (1) {
        if (!GetNextToken(token)) {
            iSize = (int32_t)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
            pHints->AddSegment(m_Pos, iSize);
            return false;
        }
        if (token == "trailer") {
            m_dwTrailerOffset = m_Pos;
            m_docStatus = PDF_DATAAVAIL_TRAILER;
            return true;
        }
    }
}
bool CPDF_DataAvail::CheckAllCrossRefStream(IFX_DownloadHints *pHints)
{
    FX_FILESIZE xref_offset = 0;
    int32_t nRet = CheckCrossRefStream(pHints, xref_offset);
    if (nRet == 1) {
        if (!xref_offset) {
            m_docStatus = PDF_DATAAVAIL_LOADALLCRSOSSREF;
        } else {
            m_dwCurrentXRefSteam = xref_offset;
            m_Pos = xref_offset;
        }
        return true;
    }
    if (nRet == -1) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
    }
    return false;
}
bool CPDF_DataAvail::CheckCrossRef(IFX_DownloadHints* pHints)
{
    int32_t iSize = 0;
    CFX_ByteString token;
    if (!GetNextToken(token)) {
        iSize = (int32_t)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
        pHints->AddSegment(m_Pos, iSize);
        return false;
    }
    if (token == "xref") {
        m_CrossOffset.InsertAt(0, m_dwXRefOffset);
        while (1) {
            if (!GetNextToken(token)) {
                iSize = (int32_t)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
                pHints->AddSegment(m_Pos, iSize);
                m_docStatus = PDF_DATAAVAIL_CROSSREF_ITEM;
                return false;
            }
            if (token == "trailer") {
                m_dwTrailerOffset = m_Pos;
                m_docStatus = PDF_DATAAVAIL_TRAILER;
                return true;
            }
        }
    } else {
        m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
        return true;
    }
    return false;
}
bool CPDF_DataAvail::CheckTrailerAppend(IFX_DownloadHints* pHints)
{
    if (m_Pos < m_dwFileLen) {
        FX_FILESIZE dwAppendPos = m_Pos + m_syntaxParser.SavePos();
        int32_t iSize = (int32_t)(dwAppendPos + 512 > m_dwFileLen ? m_dwFileLen - dwAppendPos : 512);
        if (!m_pFileAvail->IsDataAvail(dwAppendPos, iSize)) {
            pHints->AddSegment(dwAppendPos, iSize);
            return false;
        }
    }
    if (m_dwPrevXRefOffset) {
        SetStartOffset(m_dwPrevXRefOffset);
        m_docStatus = PDF_DATAAVAIL_CROSSREF;
    } else {
        m_docStatus = PDF_DATAAVAIL_LOADALLCRSOSSREF;
    }
    return true;
}
bool CPDF_DataAvail::CheckTrailer(IFX_DownloadHints* pHints)
{
    int32_t iTrailerSize = (int32_t)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
    if (m_pFileAvail->IsDataAvail(m_Pos, iTrailerSize)) {
        int32_t iSize = (int32_t)(m_Pos + iTrailerSize - m_dwTrailerOffset);
        CFX_BinaryBuf buf(iSize);
        uint8_t* pBuf = buf.GetBuffer();
        if (!pBuf) {
            m_docStatus = PDF_DATAAVAIL_ERROR;
            return false;
        }
        if (!m_pFileRead->ReadBlock(pBuf, m_dwTrailerOffset, iSize)) {
            return false;
        }
        CFX_SmartPointer<IFX_FileStream> file(FX_CreateMemoryStream(pBuf, (size_t)iSize, false));
        m_syntaxParser.InitParser(file.Get(), 0);
        CPDF_Object *pTrailer = m_syntaxParser.GetObject(NULL, 0, 0, 0);
        if (!pTrailer) {
            m_Pos += m_syntaxParser.SavePos();
            pHints->AddSegment(m_Pos, iTrailerSize);
            return false;
        }
        if (pTrailer->GetType() != PDFOBJ_DICTIONARY) {
            return false;
        }
        CPDF_Dictionary *pTrailerDict = pTrailer->GetDict();
        if (pTrailerDict) {
            CPDF_Object *pEncrypt = pTrailerDict->GetElement("Encrypt");
            if (pEncrypt && pEncrypt->GetType() == PDFOBJ_REFERENCE) {
                m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
                pTrailer->Release();
                return true;
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
            return true;
        }
        m_dwPrevXRefOffset = 0;
        m_docStatus = PDF_DATAAVAIL_TRAILER_APPEND;
        pTrailer->Release();
        return true;
    }
    pHints->AddSegment(m_Pos, iTrailerSize);
    return false;
}
bool CPDF_DataAvail::CheckPage(int32_t iPage, IFX_DownloadHints* pHints)
{
    while (true) {
        switch (m_docStatus) {
            case PDF_DATAAVAIL_PAGETREE:
                if (!LoadDocPages(pHints)) {
                    return false;
                }
                break;
            case PDF_DATAAVAIL_PAGE:
                if (!LoadDocPage(iPage, pHints)) {
                    return false;
                }
                break;
            case PDF_DATAAVAIL_ERROR:
                return LoadAllFile(pHints);
            default:
                m_bPagesTreeLoad = true;
                m_bPagesLoad = true;
                m_bCurPageDictLoadOK = true;
                m_docStatus = PDF_DATAAVAIL_PAGE;
                return true;
        }
    }
}
bool	CPDF_DataAvail::CheckArrayPageNode(FX_DWORD dwPageNo, CPDF_PageNode *pPageNode, IFX_DownloadHints* pHints)
{
    bool bExist = false;
    CPDF_Object *pPages = GetObject(dwPageNo, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    if (!pPages) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_ERROR;
            return false;
        }
        return false;
    }
    if (pPages->GetType() != PDFOBJ_ARRAY) {
        pPages->Release();
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
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
    return true;
}
bool CPDF_DataAvail::CheckUnkownPageNode(FX_DWORD dwPageNo, CPDF_PageNode *pPageNode, IFX_DownloadHints* pHints)
{
    bool bExist = false;
    CPDF_Object *pPage = GetObject(dwPageNo, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    if (!pPage) {
        if (m_docStatus == PDF_DATAAVAIL_ERROR) {
            m_docStatus = PDF_DATAAVAIL_ERROR;
            return false;
        }
        return false;
    }
    if (pPage->GetType() == PDFOBJ_ARRAY) {
        pPageNode->m_dwPageNo = dwPageNo;
        pPageNode->m_type = PDF_PAGENODE_ARRAY;
        pPage->Release();
        return true;
    }
    if (pPage->GetType() != PDFOBJ_DICTIONARY) {
        pPage->Release();
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    pPageNode->m_dwPageNo = dwPageNo;
    CPDF_Dictionary* pDict = pPage->GetDict();
    CFX_ByteString type = pDict ? pDict->GetString(FX_BSTRC("Type")) : CFX_ByteString();
    if (type == FX_BSTRC("Pages")) {
        pPageNode->m_type = PDF_PAGENODE_PAGES;
        CPDF_Object *pKids = pDict->GetElement(FX_BSTRC("Kids"));
        if (!pKids) {
            m_docStatus = PDF_DATAAVAIL_PAGE;
            return true;
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
        return false;
    }
    pPage->Release();
    return true;
}
bool CPDF_DataAvail::CheckPageNode(CPDF_PageNode &pageNodes, int32_t iPage, int32_t &iCount, IFX_DownloadHints* pHints)
{
    int32_t iSize = pageNodes.m_childNode.GetSize();
    if (iSize <= 0 || iPage >= iSize) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    for (int32_t i = 0; i < iSize; ++i) {
        CPDF_PageNode *pNode = (CPDF_PageNode*)pageNodes.m_childNode.GetAt(i);
        if (!pNode) {
            continue;
        }
        switch (pNode->m_type) {
            case PDF_PAGENODE_UNKOWN:
                if (!CheckUnkownPageNode(pNode->m_dwPageNo, pNode, pHints)) {
                    return false;
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
                    return false;
                }
                break;
            case PDF_PAGENODE_ARRAY:
                if (!CheckArrayPageNode(pNode->m_dwPageNo, pNode, pHints)) {
                    return false;
                }
                --i;
                break;
        }
        if (iPage == iCount) {
            m_docStatus = PDF_DATAAVAIL_DONE;
            return true;
        }
    }
    return true;
}
bool CPDF_DataAvail::LoadDocPage(int32_t iPage, IFX_DownloadHints* pHints)
{
    if (m_pDocument->GetPageCount() <= iPage || m_pDocument->m_PageList.GetAt(iPage)) {
        m_docStatus = PDF_DATAAVAIL_DONE;
        return true;
    }
    if (m_pageNodes.m_type == PDF_PAGENODE_PAGE) {
        if (iPage == 0) {
            m_docStatus = PDF_DATAAVAIL_DONE;
            return true;
        }
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return true;
    }
    int32_t iCount = -1;
    return CheckPageNode(m_pageNodes, iPage, iCount, pHints);
}
bool CPDF_DataAvail::CheckPageCount(IFX_DownloadHints* pHints)
{
    bool bExist = false;
    CPDF_Object *pPages = GetObject(m_PagesObjNum, pHints, &bExist);
    if (!bExist) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    if (!pPages) {
        return false;
    }
    CPDF_Dictionary* pPagesDict = pPages->GetDict();
    if (!pPagesDict) {
        pPages->Release();
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return false;
    }
    if (!pPagesDict->KeyExist(FX_BSTRC("Kids"))) {
        pPages->Release();
        return true;
    }
    int count = pPagesDict->GetInteger(FX_BSTRC("Count"));
    if (count > 0) {
        pPages->Release();
        return true;
    }
    pPages->Release();
    return false;
}
bool CPDF_DataAvail::LoadDocPages(IFX_DownloadHints* pHints)
{
    if (!CheckUnkownPageNode(m_PagesObjNum, &m_pageNodes, pHints)) {
        return false;
    }
    if (CheckPageCount(pHints)) {
        m_docStatus = PDF_DATAAVAIL_PAGE;
        return true;
    }
    m_bTotalLoadPageTree = true;
    return false;
}
bool CPDF_DataAvail::LoadPages(IFX_DownloadHints* pHints)
{
    while (!m_bPagesTreeLoad) {
        if (!CheckPageStatus(pHints)) {
            return false;
        }
    }
    if (m_bPagesLoad) {
        return true;
    }
    m_pDocument->LoadPages();
    return false;
}
bool CPDF_DataAvail::CheckLinearizedData(IFX_DownloadHints* pHints)
{
    if (m_bLinearedDataOK) {
        return true;
    }

    if (!m_bMainXRefLoadTried) {
        FX_SAFE_DWORD data_size = m_dwFileLen;
        data_size -= m_dwLastXRefOffset;
        if (!data_size.IsValid()) {
            return false;
        }
        if (!m_pFileAvail->IsDataAvail(m_dwLastXRefOffset, data_size.ValueOrDie())) {
            pHints->AddSegment(m_dwLastXRefOffset, data_size.ValueOrDie());
            return false;
        }
        FX_DWORD dwRet = ((CPDF_Parser *)m_pDocument->GetParser())->LoadLinearizedMainXRefTable();
        m_bMainXRefLoadTried = true;
        if (dwRet != PDFPARSE_ERROR_SUCCESS) {
            return false;
        }
        if (!PreparePageItem()) {
            return false;
        }
        m_bMainXRefLoadedOK = true;
        m_bLinearedDataOK   = true;
    }

    return m_bLinearedDataOK;
}
bool CPDF_DataAvail::CheckPageAnnots(int32_t iPage, IFX_DownloadHints* pHints)
{
    if (!m_objs_array.GetSize()) {
        m_objs_array.RemoveAll();
        m_objnum_array.RemoveAll();
        CPDF_Dictionary *pPageDict = m_pDocument->GetPage(iPage);
        if (!pPageDict) {
            return true;
        }
        CPDF_Object *pAnnots = pPageDict->GetElement(FX_BSTRC("Annots"));
        if (!pAnnots) {
            return true;
        }
        CFX_PtrArray obj_array;
        obj_array.Add(pAnnots);
        bool bRet = IsObjectsAvail(obj_array, false, pHints, m_objs_array);
        if (bRet) {
            m_objs_array.RemoveAll();
        }
        return bRet;
    }
    CFX_PtrArray new_objs_array;
    bool bRet = IsObjectsAvail(m_objs_array, false, pHints, new_objs_array);
    m_objs_array.RemoveAll();
    if (!bRet) {
        m_objs_array.Append(new_objs_array);
    }
    return bRet;
}
bool CPDF_DataAvail::CheckLinearizedFirstPage(int32_t iPage, IFX_DownloadHints* pHints)
{
    if (!m_bAnnotsLoad) {
        if (!CheckPageAnnots(iPage, pHints)) {
            return false;
        }
        m_bAnnotsLoad = true;
    }
    if (m_bAnnotsLoad) {
        if (!CheckLinearizedData(pHints))
            return false;
    }
    m_bPageLoadedOK = false;
    return true;
}
bool CPDF_DataAvail::HaveResourceAncestor(CPDF_Dictionary *pDict)
{
    CFX_AutoRestorer<int> restorer(&s_CurrentDataAvailRecursionDepth);
    if (++s_CurrentDataAvailRecursionDepth > kMaxDataAvailRecursionDepth) {
        return false;
    }
    CPDF_Object *pParent = pDict->GetElement("Parent");
    if (!pParent) {
        return false;
    }
    CPDF_Dictionary *pParentDict = pParent->GetDict();
    if (!pParentDict) {
        return false;
    }
    CPDF_Object *pRet = pParentDict->GetElement("Resources");
    if (pRet) {
        m_pPageResource = pRet;
        return true;
    }
    return HaveResourceAncestor(pParentDict);
}
bool CPDF_DataAvail::IsPageAvail(int32_t iPage, IFX_DownloadHints* pHints)
{
    if (!m_pDocument) {
        return false;
    }
    if (IsFirstCheck(iPage)) {
        m_bCurPageDictLoadOK = false;
        m_bPageLoadedOK = false;
        m_bAnnotsLoad = false;
        m_bNeedDownLoadResource = false;
        m_objs_array.RemoveAll();
        m_objnum_array.RemoveAll();
    }
    if (m_pagesLoadState == NULL) {
        m_pagesLoadState = new CFX_CMapDWordToDWord();
    }
    FX_DWORD dwPageLoad = 0;
    if (m_pagesLoadState->Lookup(iPage, dwPageLoad) && dwPageLoad != 0) {
        return true;
    }
    if (m_bLinearized) {
        if ((FX_DWORD)iPage == m_dwFirstPageNo) {
            m_pagesLoadState->SetAt(iPage, true);
            return true;
        }
        if (!CheckLinearizedData(pHints)) {
            return false;
        }
        if (m_bMainXRefLoadedOK) {
            if (m_bTotalLoadPageTree) {
                if (!LoadPages(pHints)) {
                    return false;
                }
            } else {
                if (!m_bCurPageDictLoadOK && !CheckPage(iPage, pHints)) {
                    return false;
                }
            }
        } else {
            if (!LoadAllFile(pHints)) {
                return false;
            }
            ((CPDF_Parser *)m_pDocument->GetParser())->RebuildCrossRef();
            ResetFirstCheck(iPage);
            return true;
        }
    } else {
        if (!m_bTotalLoadPageTree) {
            if (!m_bCurPageDictLoadOK && !CheckPage(iPage, pHints)) {
                return false;
            }
        }
    }
    if (m_bHaveAcroForm && !m_bAcroFormLoad) {
        if (!CheckAcroFormSubObject(pHints)) {
            return false;
        }
        m_bAcroFormLoad = true;
    }
    if (!m_bPageLoadedOK) {
        if (!m_objs_array.GetSize()) {
            m_objs_array.RemoveAll();
            m_objnum_array.RemoveAll();
            m_pPageDict = m_pDocument->GetPage(iPage);
            if (!m_pPageDict) {
                ResetFirstCheck(iPage);
                return true;
            }
            CFX_PtrArray obj_array;
            obj_array.Add(m_pPageDict);
            bool bRet = IsObjectsAvail(obj_array, true, pHints, m_objs_array);
            if (bRet) {
                m_objs_array.RemoveAll();
                m_bPageLoadedOK = true;
            } else {
                return bRet;
            }
        } else {
            CFX_PtrArray new_objs_array;
            bool bRet = IsObjectsAvail(m_objs_array, false, pHints, new_objs_array);
            m_objs_array.RemoveAll();
            if (bRet) {
                m_bPageLoadedOK = true;
            } else {
                m_objs_array.Append(new_objs_array);
                return bRet;
            }
        }
    }
    if (m_bPageLoadedOK) {
        if (!m_bAnnotsLoad) {
            if (!CheckPageAnnots(iPage, pHints)) {
                return false;
            }
            m_bAnnotsLoad = true;
        }
    }
    if (m_pPageDict && !m_bNeedDownLoadResource) {
        m_pPageResource = m_pPageDict->GetElement("Resources");
        if (!m_pPageResource) {
            m_bNeedDownLoadResource = HaveResourceAncestor(m_pPageDict);
        } else {
            m_bNeedDownLoadResource = true;
        }
    }
    if (m_bNeedDownLoadResource) {
        bool bRet = CheckResources(pHints);
        if (!bRet) {
            return false;
        }
        m_bNeedDownLoadResource = false;
    }
    m_bPageLoadedOK = false;
    m_bAnnotsLoad = false;
    m_bCurPageDictLoadOK = false;
    ResetFirstCheck(iPage);
    m_pagesLoadState->SetAt(iPage, true);
    return true;
}
bool CPDF_DataAvail::CheckResources(IFX_DownloadHints* pHints)
{
    if (!m_objs_array.GetSize()) {
        m_objs_array.RemoveAll();
        CFX_PtrArray obj_array;
        obj_array.Add(m_pPageResource);
        bool bRet = IsObjectsAvail(obj_array, true, pHints, m_objs_array);
        if (bRet) {
            m_objs_array.RemoveAll();
        }
        return bRet;
    }
    CFX_PtrArray new_objs_array;
    bool bRet = IsObjectsAvail(m_objs_array, false, pHints, new_objs_array);
    m_objs_array.RemoveAll();
    if (!bRet) {
        m_objs_array.Append(new_objs_array);
    }
    return bRet;
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
int32_t CPDF_DataAvail::IsFormAvail(IFX_DownloadHints *pHints)
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
        m_bLinearizedFormParamLoad = true;
    }
    CFX_PtrArray new_objs_array;
    bool bRet = IsObjectsAvail(m_objs_array, false, pHints, new_objs_array);
    m_objs_array.RemoveAll();
    if (!bRet) {
        m_objs_array.Append(new_objs_array);
        return PDFFORM_NOTAVAIL;
    }
    return PDFFORM_AVAIL;
}
void CPDF_SortObjNumArray::AddObjNum(FX_DWORD dwObjNum)
{
    int32_t iNext = 0;
    if (BinarySearch(dwObjNum, iNext)) {
        return;
    }
    m_number_array.InsertAt(iNext, dwObjNum);
}
bool CPDF_SortObjNumArray::Find(FX_DWORD dwObjNum)
{
    int32_t iNext = 0;
    return BinarySearch(dwObjNum, iNext);
}
bool CPDF_SortObjNumArray::BinarySearch(FX_DWORD value, int32_t &iNext)
{
    int32_t iLow = 0;
    int32_t iHigh = m_number_array.GetSize() - 1;
    while (iLow <= iHigh) {
        int32_t iMid = (iLow + iHigh) / 2;
        if (m_number_array.GetAt(iMid) == value) {
            iNext = iMid;
            return true;
        }
        if (m_number_array.GetAt(iMid) > value) {
            iHigh = iMid - 1;
        } else if (m_number_array.GetAt(iMid) < value) {
            iLow = iMid + 1;
        }
    }
    iNext = iLow;
    return false;
}
CPDF_PageNode::~CPDF_PageNode()
{
    int32_t iSize = m_childNode.GetSize();
    for (int32_t i = 0; i < iSize; ++i) {
        CPDF_PageNode* pNode = (CPDF_PageNode*)m_childNode[i];
        delete pNode;
    }
    m_childNode.RemoveAll();
}
