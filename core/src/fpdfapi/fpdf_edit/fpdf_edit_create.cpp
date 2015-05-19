// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_serial.h"
#include "editint.h"
#define PDF_OBJECTSTREAM_MAXLENGTH	(256 * 1024)
#define PDF_XREFSTREAM_MAXSIZE		10000
extern void FlateEncode(const FX_BYTE* src_buf, FX_DWORD src_data, FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
extern void FlateEncode(FX_LPCBYTE src_buf, FX_DWORD src_size, int predictor, int Colors, int BitsPerComponent, int Columns,
                        FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
extern FX_BOOL IsSignatureDict(const CPDF_Dictionary* pDict);
FX_INT32 PDF_CreatorAppendObject(const CPDF_Object* pObj, CFX_FileBufferArchive *pFile, FX_FILESIZE& offset)
{
    FX_INT32 len = 0;
    if (pObj == NULL) {
        if (pFile->AppendString(FX_BSTRC(" null")) < 0) {
            return -1;
        }
        offset += 5;
        return 1;
    }
    switch (pObj->GetType()) {
        case PDFOBJ_NULL:
            if (pFile->AppendString(FX_BSTRC(" null")) < 0) {
                return -1;
            }
            offset += 5;
            break;
        case PDFOBJ_BOOLEAN:
        case PDFOBJ_NUMBER:
            if (pFile->AppendString(FX_BSTRC(" ")) < 0) {
                return -1;
            }
            if ((len = pFile->AppendString(pObj->GetString())) < 0) {
                return -1;
            }
            offset += len + 1;
            break;
        case PDFOBJ_STRING: {
                CFX_ByteString str = pObj->GetString();
                FX_BOOL bHex = ((CPDF_String*)pObj)->IsHex();
                if ((len = pFile->AppendString(PDF_EncodeString(str, bHex))) < 0) {
                    return -1;
                }
                offset += len;
                break;
            }
        case PDFOBJ_NAME: {
                if (pFile->AppendString(FX_BSTRC("/")) < 0) {
                    return -1;
                }
                CFX_ByteString str = pObj->GetString();
                if ((len = pFile->AppendString(PDF_NameEncode(str))) < 0) {
                    return -1;
                }
                offset += len + 1;
                break;
            }
        case PDFOBJ_REFERENCE: {
                if (pFile->AppendString(FX_BSTRC(" ")) < 0) {
                    return -1;
                }
                CPDF_Reference* p = (CPDF_Reference*)pObj;
                if ((len = pFile->AppendDWord(p->GetRefObjNum())) < 0) {
                    return -1;
                }
                if (pFile->AppendString(FX_BSTRC(" 0 R ")) < 0) {
                    return -1;
                }
                offset += len + 6;
                break;
            }
        case PDFOBJ_ARRAY: {
                if (pFile->AppendString(FX_BSTRC("[")) < 0) {
                    return -1;
                }
                offset += 1;
                CPDF_Array* p = (CPDF_Array*)pObj;
                for (FX_DWORD i = 0; i < p->GetCount(); i ++) {
                    CPDF_Object* pElement = p->GetElement(i);
                    if (pElement->GetObjNum()) {
                        if (pFile->AppendString(FX_BSTRC(" ")) < 0) {
                            return -1;
                        }
                        if ((len = pFile->AppendDWord(pElement->GetObjNum())) < 0) {
                            return -1;
                        }
                        if (pFile->AppendString(FX_BSTRC(" 0 R")) < 0) {
                            return -1;
                        }
                        offset += len + 5;
                    } else {
                        if (PDF_CreatorAppendObject(pElement, pFile, offset) < 0) {
                            return -1;
                        }
                    }
                }
                if (pFile->AppendString(FX_BSTRC("]")) < 0) {
                    return -1;
                }
                offset += 1;
                break;
            }
        case PDFOBJ_DICTIONARY: {
                if (pFile->AppendString(FX_BSTRC("<<")) < 0) {
                    return -1;
                }
                offset += 2;
                CPDF_Dictionary* p = (CPDF_Dictionary*)pObj;
                FX_POSITION pos = p->GetStartPos();
                while (pos) {
                    CFX_ByteString key;
                    CPDF_Object* pValue = p->GetNextElement(pos, key);
                    if (pFile->AppendString(FX_BSTRC("/")) < 0) {
                        return -1;
                    }
                    if ((len = pFile->AppendString(PDF_NameEncode(key))) < 0) {
                        return -1;
                    }
                    offset += len + 1;
                    if (pValue->GetObjNum()) {
                        if (pFile->AppendString(FX_BSTRC(" ")) < 0) {
                            return -1;
                        }
                        if ((len = pFile->AppendDWord(pValue->GetObjNum())) < 0) {
                            return -1;
                        }
                        if (pFile->AppendString(FX_BSTRC(" 0 R")) < 0) {
                            return -1;
                        }
                        offset += len + 5;
                    } else {
                        if (PDF_CreatorAppendObject(pValue, pFile, offset) < 0) {
                            return -1;
                        }
                    }
                }
                if (pFile->AppendString(FX_BSTRC(">>")) < 0) {
                    return -1;
                }
                offset += 2;
                break;
            }
        case PDFOBJ_STREAM: {
                CPDF_Stream* p = (CPDF_Stream*)pObj;
                if (PDF_CreatorAppendObject(p->GetDict(), pFile, offset) < 0) {
                    return -1;
                }
                if (pFile->AppendString(FX_BSTRC("stream\r\n")) < 0) {
                    return -1;
                }
                offset += 8;
                CPDF_StreamAcc acc;
                acc.LoadAllData(p, TRUE);
                if (pFile->AppendBlock(acc.GetData(), acc.GetSize()) < 0) {
                    return -1;
                }
                offset += acc.GetSize();
                if ((len = pFile->AppendString(FX_BSTRC("\r\nendstream"))) < 0) {
                    return -1;
                }
                offset += len;
                break;
            }
        default:
            ASSERT(FALSE);
            break;
    }
    return 1;
}
FX_INT32 PDF_CreatorWriteTrailer(CPDF_Document* pDocument, CFX_FileBufferArchive* pFile, CPDF_Array* pIDArray, FX_BOOL bCompress)
{
    FX_FILESIZE offset = 0;
    FX_INT32 len = 0;
    FXSYS_assert(pDocument && pFile);
    CPDF_Parser *pParser = (CPDF_Parser*)pDocument->GetParser();
    if (pParser) {
        CPDF_Dictionary* p = pParser->GetTrailer();
        FX_POSITION pos = p->GetStartPos();
        while (pos) {
            CFX_ByteString key;
            CPDF_Object* pValue = p->GetNextElement(pos, key);
            if (key == FX_BSTRC("Encrypt") || key == FX_BSTRC("Size") || key == FX_BSTRC("Filter") ||
                    key == FX_BSTRC("Index") || key == FX_BSTRC("Length") || key == FX_BSTRC("Prev") ||
                    key == FX_BSTRC("W") || key == FX_BSTRC("XRefStm") || key == FX_BSTRC("Type") || key == FX_BSTRC("ID")) {
                continue;
            }
            if (bCompress && key == FX_BSTRC("DecodeParms")) {
                continue;
            }
            if (pFile->AppendString((FX_BSTRC("/"))) < 0) {
                return -1;
            }
            if ((len = pFile->AppendString(PDF_NameEncode(key))) < 0) {
                return -1;
            }
            offset += len + 1;
            if (pValue->GetObjNum()) {
                if (pFile->AppendString(FX_BSTRC(" ")) < 0) {
                    return -1;
                }
                if ((len = pFile->AppendDWord(pValue->GetObjNum())) < 0) {
                    return -1;
                }
                if (pFile->AppendString(FX_BSTRC(" 0 R ")) < 0) {
                    return -1;
                }
                offset += len + 6;
            } else {
                if (PDF_CreatorAppendObject(pValue, pFile, offset) < 0) {
                    return -1;
                }
            }
        }
        if (pIDArray) {
            if (pFile->AppendString((FX_BSTRC("/ID"))) < 0) {
                return -1;
            }
            offset += 3;
            if (PDF_CreatorAppendObject(pIDArray, pFile, offset) < 0) {
                return -1;
            }
        }
        return offset;
    }
    if (pFile->AppendString(FX_BSTRC("\r\n/Root ")) < 0) {
        return -1;
    }
    if ((len = pFile->AppendDWord(pDocument->GetRoot()->GetObjNum())) < 0) {
        return -1;
    }
    if (pFile->AppendString(FX_BSTRC(" 0 R\r\n")) < 0) {
        return -1;
    }
    offset += len + 14;
    if (pDocument->GetInfo()) {
        if (pFile->AppendString(FX_BSTRC("/Info ")) < 0) {
            return -1;
        }
        if ((len = pFile->AppendDWord(pDocument->GetInfo()->GetObjNum())) < 0) {
            return -1;
        }
        if (pFile->AppendString(FX_BSTRC(" 0 R\r\n")) < 0) {
            return -1;
        }
        offset += len + 12;
    }
    if (pIDArray) {
        if (pFile->AppendString((FX_BSTRC("/ID"))) < 0) {
            return -1;
        }
        offset += 3;
        if (PDF_CreatorAppendObject(pIDArray, pFile, offset) < 0) {
            return -1;
        }
    }
    return offset;
}
FX_INT32 PDF_CreatorWriteEncrypt(const CPDF_Dictionary* pEncryptDict, FX_DWORD dwObjNum, CFX_FileBufferArchive *pFile)
{
    if (!pEncryptDict) {
        return 0;
    }
    FXSYS_assert(pFile);
    FX_FILESIZE offset = 0;
    FX_INT32 len = 0;
    if (pFile->AppendString(FX_BSTRC("/Encrypt")) < 0) {
        return -1;
    }
    offset += 8;
    if (pFile->AppendString(FX_BSTRC(" ")) < 0) {
        return -1;
    }
    if ((len = pFile->AppendDWord(dwObjNum)) < 0) {
        return -1;
    }
    if (pFile->AppendString(FX_BSTRC(" 0 R ")) < 0) {
        return -1;
    }
    offset += len + 6;
    return offset;
}
FX_BOOL PDF_GenerateFileID(FX_DWORD dwSeed1, FX_DWORD dwSeed2, FX_LPDWORD pBuffer)
{
    if (!pBuffer) {
        return FALSE;
    }
    FX_LPVOID pContext = FX_Random_MT_Start(dwSeed1);
    FX_INT32 i = 0;
    for (i = 0; i < 2; i++) {
        *pBuffer++ = FX_Random_MT_Generate(pContext);
    }
    FX_Random_MT_Close(pContext);
    pContext = FX_Random_MT_Start(dwSeed2);
    for (i = 0; i < 2; i++) {
        *pBuffer++ = FX_Random_MT_Generate(pContext);
    }
    FX_Random_MT_Close(pContext);
    return TRUE;
}
class CPDF_FlateEncoder
{
public:
    CPDF_FlateEncoder();
    ~CPDF_FlateEncoder();
    FX_BOOL		Initialize(CPDF_Stream* pStream, FX_BOOL bFlateEncode);
    FX_BOOL		Initialize(FX_LPCBYTE pBuffer, FX_DWORD size, FX_BOOL bFlateEncode, FX_BOOL bXRefStream = FALSE);
    void		CloneDict();
    FX_LPBYTE			m_pData;
    FX_DWORD			m_dwSize;
    CPDF_Dictionary*	m_pDict;
    FX_BOOL				m_bCloned;
    FX_BOOL				m_bNewData;
    CPDF_StreamAcc		m_Acc;
};
CPDF_FlateEncoder::CPDF_FlateEncoder()
{
    m_pData = NULL;
    m_dwSize = 0;
    m_pDict = NULL;
    m_bCloned = FALSE;
    m_bNewData = FALSE;
}
void CPDF_FlateEncoder::CloneDict()
{
    if (!m_bCloned) {
        m_pDict = (CPDF_Dictionary*)m_pDict->Clone();
        m_bCloned = TRUE;
    }
}
FX_BOOL CPDF_FlateEncoder::Initialize(CPDF_Stream* pStream, FX_BOOL bFlateEncode)
{
    m_Acc.LoadAllData(pStream, TRUE);
    if ((pStream && pStream->GetDict() && pStream->GetDict()->KeyExist("Filter")) || !bFlateEncode) {
        if (pStream->GetDict()->KeyExist("Filter") && !bFlateEncode) {
            CPDF_StreamAcc destAcc;
            destAcc.LoadAllData(pStream);
            m_dwSize = destAcc.GetSize();
            m_pData = (FX_LPBYTE)destAcc.DetachData();
            m_pDict = (CPDF_Dictionary*)pStream->GetDict()->Clone();
            m_pDict->RemoveAt(FX_BSTRC("Filter"));
            m_bNewData = TRUE;
            m_bCloned = TRUE;
        } else {
            m_pData = (FX_LPBYTE)m_Acc.GetData();
            m_dwSize = m_Acc.GetSize();
            m_pDict = pStream->GetDict();
        }
        return TRUE;
    }
    m_pData = NULL;
    m_dwSize = 0;
    m_bNewData = TRUE;
    m_bCloned = TRUE;
    ::FlateEncode(m_Acc.GetData(), m_Acc.GetSize(), m_pData, m_dwSize);
    m_pDict = (CPDF_Dictionary*)pStream->GetDict()->Clone();
    m_pDict->SetAtInteger("Length", m_dwSize);
    m_pDict->SetAtName("Filter", "FlateDecode");
    m_pDict->RemoveAt("DecodeParms");
    return TRUE;
}
FX_BOOL CPDF_FlateEncoder::Initialize(FX_LPCBYTE pBuffer, FX_DWORD size, FX_BOOL bFlateEncode, FX_BOOL bXRefStream)
{
    if (!bFlateEncode) {
        m_pData = (FX_LPBYTE)pBuffer;
        m_dwSize = size;
        return TRUE;
    }
    m_bNewData = TRUE;
    if (bXRefStream) {
        ::FlateEncode(pBuffer, size, 12, 1, 8, 7, m_pData, m_dwSize);
    } else {
        ::FlateEncode(pBuffer, size, m_pData, m_dwSize);
    }
    return TRUE;
}
CPDF_FlateEncoder::~CPDF_FlateEncoder()
{
    if (m_bCloned && m_pDict) {
        m_pDict->Release();
    }
    if (m_bNewData && m_pData) {
        FX_Free(m_pData);
    }
}
class CPDF_Encryptor
{
public:
    CPDF_Encryptor();
    ~CPDF_Encryptor();
    FX_BOOL		Initialize(CPDF_CryptoHandler* pHandler, int objnum, FX_LPBYTE src_data, FX_DWORD src_size);
    FX_LPBYTE			m_pData;
    FX_DWORD			m_dwSize;
    FX_BOOL				m_bNewBuf;
};
CPDF_Encryptor::CPDF_Encryptor()
{
    m_pData = NULL;
    m_dwSize = 0;
    m_bNewBuf = FALSE;
}
FX_BOOL CPDF_Encryptor::Initialize(CPDF_CryptoHandler* pHandler, int objnum, FX_LPBYTE src_data, FX_DWORD src_size)
{
    if (src_size == 0) {
        return TRUE;
    }
    if (pHandler == NULL) {
        m_pData = (FX_LPBYTE)src_data;
        m_dwSize = src_size;
        m_bNewBuf = FALSE;
        return TRUE;
    }
    m_dwSize = pHandler->EncryptGetSize(objnum, 0, src_data, src_size);
    m_pData = FX_Alloc(FX_BYTE, m_dwSize);
    pHandler->EncryptContent(objnum, 0, src_data, src_size, m_pData, m_dwSize);
    m_bNewBuf = TRUE;
    return TRUE;
}
CPDF_Encryptor::~CPDF_Encryptor()
{
    if (m_bNewBuf) {
        FX_Free(m_pData);
    }
}
CPDF_ObjectStream::CPDF_ObjectStream()
    : m_dwObjNum(0)
    , m_index(0)
{
}
FX_BOOL CPDF_ObjectStream::Start()
{
    m_ObjNumArray.RemoveAll();
    m_OffsetArray.RemoveAll();
    m_Buffer.Clear();
    m_dwObjNum = 0;
    m_index = 0;
    return TRUE;
}
FX_INT32 CPDF_ObjectStream::CompressIndirectObject(FX_DWORD dwObjNum, const CPDF_Object *pObj)
{
    m_ObjNumArray.Add(dwObjNum);
    m_OffsetArray.Add(m_Buffer.GetLength());
    m_Buffer << pObj;
    return 1;
}
FX_INT32 CPDF_ObjectStream::CompressIndirectObject(FX_DWORD dwObjNum, FX_LPCBYTE pBuffer, FX_DWORD dwSize)
{
    m_ObjNumArray.Add(dwObjNum);
    m_OffsetArray.Add(m_Buffer.GetLength());
    m_Buffer.AppendBlock(pBuffer, dwSize);
    return 1;
}
FX_FILESIZE CPDF_ObjectStream::End(CPDF_Creator* pCreator)
{
    FXSYS_assert(pCreator);
    if (m_ObjNumArray.GetSize() == 0) {
        return 0;
    }
    CFX_FileBufferArchive *pFile = &pCreator->m_File;
    CPDF_CryptoHandler *pHandler = pCreator->m_pCryptoHandler;
    FX_FILESIZE ObjOffset = pCreator->m_Offset;
    if (!m_dwObjNum) {
        m_dwObjNum = ++pCreator->m_dwLastObjNum;
    }
    CFX_ByteTextBuf tempBuffer;
    FX_INT32 iCount = m_ObjNumArray.GetSize();
    for (FX_INT32 i = 0; i < iCount; i++) {
        tempBuffer << m_ObjNumArray.ElementAt(i) << FX_BSTRC(" ") << m_OffsetArray.ElementAt(i) << FX_BSTRC(" ");
    }
    FX_FILESIZE &offset = pCreator->m_Offset;
    FX_INT32 len = pFile->AppendDWord(m_dwObjNum);
    if (len < 0) {
        return -1;
    }
    offset += len;
    if ((len = pFile->AppendString(FX_BSTRC(" 0 obj\r\n<</Type /ObjStm /N "))) < 0) {
        return -1;
    }
    offset += len;
    if ((len = pFile->AppendDWord((FX_DWORD)iCount)) < 0) {
        return -1;
    }
    offset += len;
    if (pFile->AppendString(FX_BSTRC("/First ")) < 0) {
        return -1;
    }
    if ((len = pFile->AppendDWord((FX_DWORD)tempBuffer.GetLength())) < 0) {
        return -1;
    }
    if (pFile->AppendString(FX_BSTRC("/Length ")) < 0) {
        return -1;
    }
    offset += len + 15;
    if (!pCreator->m_bCompress && !pHandler) {
        if ((len = pFile->AppendDWord((FX_DWORD)(tempBuffer.GetLength() + m_Buffer.GetLength()))) < 0) {
            return -1;
        }
        offset += len;
        if ((len = pFile->AppendString(FX_BSTRC(">>stream\r\n"))) < 0) {
            return -1;
        }
        if (pFile->AppendBlock(tempBuffer.GetBuffer(), tempBuffer.GetLength()) < 0) {
            return -1;
        }
        if (pFile->AppendBlock(m_Buffer.GetBuffer(), m_Buffer.GetLength()) < 0) {
            return -1;
        }
        offset += len + tempBuffer.GetLength() + m_Buffer.GetLength();
    } else {
        tempBuffer << m_Buffer;
        CPDF_FlateEncoder encoder;
        encoder.Initialize(tempBuffer.GetBuffer(), tempBuffer.GetLength(), pCreator->m_bCompress);
        CPDF_Encryptor encryptor;
        encryptor.Initialize(pHandler, m_dwObjNum, encoder.m_pData, encoder.m_dwSize);
        if ((len = pFile->AppendDWord(encryptor.m_dwSize)) < 0) {
            return -1;
        }
        offset += len;
        if (pCreator->m_bCompress) {
            if (pFile->AppendString(FX_BSTRC("/Filter /FlateDecode")) < 0) {
                return -1;
            }
            offset += 20;
        }
        if ((len = pFile->AppendString(FX_BSTRC(">>stream\r\n"))) < 0) {
            return -1;
        }
        if (pFile->AppendBlock(encryptor.m_pData, encryptor.m_dwSize) < 0) {
            return -1;
        }
        offset += len + encryptor.m_dwSize;
    }
    if ((len = pFile->AppendString(FX_BSTRC("\r\nendstream\r\nendobj\r\n"))) < 0) {
        return -1;
    }
    offset += len;
    return ObjOffset;
}
CPDF_XRefStream::CPDF_XRefStream()
    : m_PrevOffset(0)
    , m_dwTempObjNum(0)
    , m_iSeg(0)
{
}
FX_BOOL CPDF_XRefStream::Start()
{
    m_IndexArray.RemoveAll();
    m_Buffer.Clear();
    m_iSeg = 0;
    return TRUE;
}
FX_INT32 CPDF_XRefStream::CompressIndirectObject(FX_DWORD dwObjNum, const CPDF_Object *pObj, CPDF_Creator *pCreator)
{
    if (!pCreator) {
        return 0;
    }
    m_ObjStream.CompressIndirectObject(dwObjNum, pObj);
    if (m_ObjStream.m_ObjNumArray.GetSize() < pCreator->m_ObjectStreamSize &&
            m_ObjStream.m_Buffer.GetLength() < PDF_OBJECTSTREAM_MAXLENGTH) {
        return 1;
    }
    return EndObjectStream(pCreator);
}
FX_INT32 CPDF_XRefStream::CompressIndirectObject(FX_DWORD dwObjNum, FX_LPCBYTE pBuffer, FX_DWORD dwSize, CPDF_Creator *pCreator)
{
    if (!pCreator) {
        return 0;
    }
    m_ObjStream.CompressIndirectObject(dwObjNum, pBuffer, dwSize);
    if (m_ObjStream.m_ObjNumArray.GetSize() < pCreator->m_ObjectStreamSize &&
            m_ObjStream.m_Buffer.GetLength() < PDF_OBJECTSTREAM_MAXLENGTH) {
        return 1;
    }
    return EndObjectStream(pCreator);
}
static void _AppendIndex0(CFX_ByteTextBuf& buffer, FX_BOOL bFirstObject =  TRUE)
{
    buffer.AppendByte(0);
    buffer.AppendByte(0);
    buffer.AppendByte(0);
    buffer.AppendByte(0);
    buffer.AppendByte(0);
    if (bFirstObject) {
        buffer.AppendByte(0xFF);
        buffer.AppendByte(0xFF);
    } else {
        buffer.AppendByte(0);
        buffer.AppendByte(0);
    }
}
static void _AppendIndex1(CFX_ByteTextBuf& buffer, FX_FILESIZE offset)
{
    buffer.AppendByte(1);
    buffer.AppendByte(FX_GETBYTEOFFSET24(offset));
    buffer.AppendByte(FX_GETBYTEOFFSET16(offset));
    buffer.AppendByte(FX_GETBYTEOFFSET8(offset));
    buffer.AppendByte(FX_GETBYTEOFFSET0(offset));
    buffer.AppendByte(0);
    buffer.AppendByte(0);
}
static void _AppendIndex2(CFX_ByteTextBuf& buffer, FX_DWORD objnum, FX_INT32 index)
{
    buffer.AppendByte(2);
    buffer.AppendByte(FX_GETBYTEOFFSET24(objnum));
    buffer.AppendByte(FX_GETBYTEOFFSET16(objnum));
    buffer.AppendByte(FX_GETBYTEOFFSET8(objnum));
    buffer.AppendByte(FX_GETBYTEOFFSET0(objnum));
    buffer.AppendByte(FX_GETBYTEOFFSET8(index));
    buffer.AppendByte(FX_GETBYTEOFFSET0(index));
}
FX_INT32 CPDF_XRefStream::EndObjectStream(CPDF_Creator *pCreator, FX_BOOL bEOF)
{
    FX_FILESIZE objOffset = 0;
    if (bEOF) {
        objOffset = m_ObjStream.End(pCreator);
        if (objOffset < 0) {
            return -1;
        }
    }
    FX_DWORD &dwObjStmNum = m_ObjStream.m_dwObjNum;
    if (!dwObjStmNum) {
        dwObjStmNum = ++pCreator->m_dwLastObjNum;
    }
    FX_INT32 iSize = m_ObjStream.m_ObjNumArray.GetSize();
    FX_INT32 iSeg = m_IndexArray.GetSize() / 2;
    if (!(pCreator->m_dwFlags & FPDFCREATE_INCREMENTAL)) {
        if (m_dwTempObjNum == 0) {
            _AppendIndex0(m_Buffer);
            m_dwTempObjNum++;
        }
        FX_DWORD end_num = m_IndexArray.GetAt((iSeg - 1) * 2) + m_IndexArray.GetAt((iSeg - 1) * 2 + 1);
        int index = 0;
        for (; m_dwTempObjNum < end_num; m_dwTempObjNum++) {
            FX_FILESIZE* offset = pCreator->m_ObjectOffset.GetPtrAt(m_dwTempObjNum);
            if (offset) {
                if (index >= iSize || m_dwTempObjNum != m_ObjStream.m_ObjNumArray[index]) {
                    _AppendIndex1(m_Buffer, *offset);
                } else {
                    _AppendIndex2(m_Buffer, dwObjStmNum, index++);
                }
            } else {
                _AppendIndex0(m_Buffer, FALSE);
            }
        }
        if (iSize > 0 && bEOF) {
            pCreator->m_ObjectOffset.Add(dwObjStmNum, 1);
            pCreator->m_ObjectSize.Add(dwObjStmNum, 1);
            pCreator->m_ObjectOffset[dwObjStmNum] = objOffset;
        }
        m_iSeg = iSeg;
        if (bEOF) {
            m_ObjStream.Start();
        }
        return 1;
    }
    FX_INT32 &j = m_ObjStream.m_index;
    for (int i = m_iSeg; i < iSeg; i++) {
        FX_DWORD start = m_IndexArray.ElementAt(i * 2);
        FX_DWORD end = m_IndexArray.ElementAt(i * 2 + 1) + start;
        for (FX_DWORD m = start; m < end; m++) {
            if (j >= iSize || m != m_ObjStream.m_ObjNumArray.ElementAt(j)) {
                _AppendIndex1(m_Buffer, pCreator->m_ObjectOffset[m]);
            } else {
                _AppendIndex2(m_Buffer, dwObjStmNum, j++);
            }
        }
    }
    if (iSize > 0 && bEOF) {
        _AppendIndex1(m_Buffer, objOffset);
        m_IndexArray.Add(dwObjStmNum);
        m_IndexArray.Add(1);
        iSeg += 1;
    }
    m_iSeg = iSeg;
    if (bEOF) {
        m_ObjStream.Start();
    }
    return 1;
}
FX_BOOL CPDF_XRefStream::GenerateXRefStream(CPDF_Creator* pCreator, FX_BOOL bEOF)
{
    FX_FILESIZE offset_tmp = pCreator->m_Offset;
    FX_DWORD objnum = ++pCreator->m_dwLastObjNum;
    CFX_FileBufferArchive *pFile = &pCreator->m_File;
    FX_BOOL bIncremental = (pCreator->m_dwFlags & FPDFCREATE_INCREMENTAL) != 0;
    if (bIncremental) {
        AddObjectNumberToIndexArray(objnum);
    } else {
        for (; m_dwTempObjNum < pCreator->m_dwLastObjNum; m_dwTempObjNum++) {
            FX_FILESIZE* offset = pCreator->m_ObjectOffset.GetPtrAt(m_dwTempObjNum);
            if (offset) {
                _AppendIndex1(m_Buffer, *offset);
            } else {
                _AppendIndex0(m_Buffer, FALSE);
            }
        }
    }
    _AppendIndex1(m_Buffer, offset_tmp);
    FX_FILESIZE &offset = pCreator->m_Offset;
    FX_INT32 len = pFile->AppendDWord(objnum);
    if (len < 0) {
        return FALSE;
    }
    offset += len;
    if ((len = pFile->AppendString(FX_BSTRC(" 0 obj\r\n<</Type /XRef/W[1 4 2]/Index["))) < 0) {
        return FALSE;
    }
    offset += len;
    if (!bIncremental) {
        if ((len = pFile->AppendDWord(0)) < 0) {
            return FALSE;
        }
        if ((len = pFile->AppendString(FX_BSTRC(" "))) < 0) {
            return FALSE;
        }
        offset += len + 1;
        if ((len = pFile->AppendDWord(objnum + 1)) < 0) {
            return FALSE;
        }
        offset += len;
    } else {
        FX_INT32 iSeg = m_IndexArray.GetSize() / 2;
        for (FX_INT32 i = 0; i < iSeg; i++) {
            if ((len = pFile->AppendDWord(m_IndexArray.ElementAt(i * 2))) < 0) {
                return FALSE;
            }
            if (pFile->AppendString(FX_BSTRC(" ")) < 0) {
                return FALSE;
            }
            offset += len + 1;
            if ((len = pFile->AppendDWord(m_IndexArray.ElementAt(i * 2 + 1))) < 0) {
                return FALSE;
            }
            if (pFile->AppendString(FX_BSTRC(" ")) < 0) {
                return FALSE;
            }
            offset += len + 1;
        }
    }
    if (pFile->AppendString(FX_BSTRC("]/Size ")) < 0) {
        return FALSE;
    }
    if ((len = pFile->AppendDWord(objnum + 1)) < 0) {
        return FALSE;
    }
    offset += len + 7;
    if (m_PrevOffset > 0) {
        if (pFile->AppendString(FX_BSTRC("/Prev ")) < 0) {
            return -1;
        }
        FX_CHAR offset_buf[20];
        FXSYS_memset32(offset_buf, 0, sizeof(offset_buf));
        FXSYS_i64toa(m_PrevOffset, offset_buf, 10);
        FX_INT32 len = (FX_INT32)FXSYS_strlen(offset_buf);
        if (pFile->AppendBlock(offset_buf, len) < 0) {
            return -1;
        }
        offset += len + 6;
    }
    FX_BOOL bPredictor = TRUE;
    CPDF_FlateEncoder encoder;
    encoder.Initialize(m_Buffer.GetBuffer(), m_Buffer.GetLength(), pCreator->m_bCompress, bPredictor);
    if (pCreator->m_bCompress) {
        if (pFile->AppendString(FX_BSTRC("/Filter /FlateDecode")) < 0) {
            return FALSE;
        }
        offset += 20;
        if (bPredictor) {
            if ((len = pFile->AppendString(FX_BSTRC("/DecodeParms<</Columns 7/Predictor 12>>"))) < 0) {
                return FALSE;
            }
            offset += len;
        }
    }
    if (pFile->AppendString(FX_BSTRC("/Length ")) < 0) {
        return FALSE;
    }
    if ((len = pFile->AppendDWord(encoder.m_dwSize)) < 0) {
        return FALSE;
    }
    offset += len + 8;
    if (bEOF) {
        if ((len = PDF_CreatorWriteTrailer(pCreator->m_pDocument, pFile, pCreator->m_pIDArray, pCreator->m_bCompress)) < 0) {
            return -1;
        }
        offset += len;
        if (pCreator->m_pEncryptDict) {
            FX_DWORD dwEncryptObjNum = pCreator->m_pEncryptDict->GetObjNum();
            if (dwEncryptObjNum == 0) {
                dwEncryptObjNum = pCreator->m_dwEnryptObjNum;
            }
            if ((len = PDF_CreatorWriteEncrypt(pCreator->m_pEncryptDict, dwEncryptObjNum, pFile)) < 0) {
                return -1;
            }
            offset += len;
        }
    }
    if ((len = pFile->AppendString(FX_BSTRC(">>stream\r\n"))) < 0) {
        return FALSE;
    }
    offset += len;
    if (pFile->AppendBlock(encoder.m_pData, encoder.m_dwSize) < 0) {
        return FALSE;
    }
    if ((len = pFile->AppendString(FX_BSTRC("\r\nendstream\r\nendobj\r\n"))) < 0) {
        return FALSE;
    }
    offset += encoder.m_dwSize + len;
    m_PrevOffset = offset_tmp;
    return TRUE;
}
FX_BOOL CPDF_XRefStream::End(CPDF_Creator *pCreator, FX_BOOL bEOF )
{
    if (EndObjectStream(pCreator, bEOF) < 0) {
        return FALSE;
    }
    return GenerateXRefStream(pCreator, bEOF);
}
FX_BOOL CPDF_XRefStream::EndXRefStream(CPDF_Creator* pCreator)
{
    if (!(pCreator->m_dwFlags & FPDFCREATE_INCREMENTAL)) {
        _AppendIndex0(m_Buffer);
        for (FX_DWORD i = 1; i < pCreator->m_dwLastObjNum + 1; i++) {
            FX_FILESIZE* offset = pCreator->m_ObjectOffset.GetPtrAt(i);
            if (offset) {
                _AppendIndex1(m_Buffer, *offset);
            } else {
                _AppendIndex0(m_Buffer, FALSE);
            }
        }
    } else {
        FX_INT32 iSeg = m_IndexArray.GetSize() / 2;
        for (int i = 0; i < iSeg; i++) {
            FX_DWORD start = m_IndexArray.ElementAt(i * 2);
            FX_DWORD end = m_IndexArray.ElementAt(i * 2 + 1) + start;
            for (FX_DWORD j = start; j < end; j++) {
                _AppendIndex1(m_Buffer, pCreator->m_ObjectOffset[j]);
            }
        }
    }
    return GenerateXRefStream(pCreator, FALSE);
}
FX_BOOL CPDF_XRefStream::AddObjectNumberToIndexArray(FX_DWORD objnum)
{
    FX_INT32 iSize = m_IndexArray.GetSize();
    if (iSize == 0) {
        m_IndexArray.Add(objnum);
        m_IndexArray.Add(1);
    } else {
        FXSYS_assert(iSize > 1);
        FX_DWORD startobjnum = m_IndexArray.ElementAt(iSize - 2);
        FX_INT32 iCount = m_IndexArray.ElementAt(iSize - 1);
        if (objnum == startobjnum + iCount) {
            m_IndexArray[iSize - 1] = iCount + 1;
        } else {
            m_IndexArray.Add(objnum);
            m_IndexArray.Add(1);
        }
    }
    return TRUE;
}
CPDF_Creator::CPDF_Creator(CPDF_Document* pDoc)
{
    m_pDocument = pDoc;
    m_pParser = (CPDF_Parser*)pDoc->m_pParser;
    m_bCompress = TRUE;
    if (m_pParser) {
        m_pEncryptDict = m_pParser->GetEncryptDict();
        m_pCryptoHandler = m_pParser->GetCryptoHandler();
    } else {
        m_pEncryptDict = NULL;
        m_pCryptoHandler = NULL;
    }
    m_bSecurityChanged = FALSE;
    m_bStandardSecurity = FALSE;
    m_pMetadata = NULL;
    m_bEncryptCloned = FALSE;
    m_bEncryptMetadata = FALSE;
    m_Offset = 0;
    m_iStage = -1;
    m_dwFlags = 0;
    m_Pos = NULL;
    m_XrefStart = 0;
    m_pXRefStream = NULL;
    m_ObjectStreamSize = 200;
    m_dwLastObjNum = m_pDocument->GetLastObjNum();
    m_pIDArray = NULL;
    m_FileVersion = 0;
    m_dwEnryptObjNum = 0;
    m_bNewCrypto = FALSE;
}
CPDF_Creator::~CPDF_Creator()
{
    ResetStandardSecurity();
    if (m_bEncryptCloned && m_pEncryptDict) {
        m_pEncryptDict->Release();
        m_pEncryptDict = NULL;
    }
    Clear();
}
static FX_BOOL _IsXRefNeedEnd(CPDF_XRefStream* pXRef, FX_DWORD flag)
{
    if (!(flag & FPDFCREATE_INCREMENTAL)) {
        return FALSE;
    }
    FX_INT32 iSize = pXRef->m_IndexArray.GetSize() / 2;
    FX_INT32 iCount = 0;
    for (FX_INT32 i = 0; i < iSize; i++) {
        iCount += pXRef->m_IndexArray.ElementAt(i * 2 + 1);
    }
    return (iCount >= PDF_XREFSTREAM_MAXSIZE);
}
FX_INT32 CPDF_Creator::WriteIndirectObjectToStream(const CPDF_Object* pObj)
{
    if (!m_pXRefStream) {
        return 1;
    }
    FX_DWORD objnum = pObj->GetObjNum();
    if (m_pParser && m_pParser->m_ObjVersion.GetSize() > (FX_INT32)objnum && m_pParser->m_ObjVersion[objnum] > 0) {
        return 1;
    }
    if (pObj->GetType() == PDFOBJ_NUMBER) {
        return 1;
    }
    CPDF_Dictionary *pDict = pObj->GetDict();
    if (pObj->GetType() == PDFOBJ_STREAM) {
        if (pDict && pDict->GetString(FX_BSTRC("Type")) == FX_BSTRC("XRef")) {
            return 0;
        }
        return 1;
    }
    if (pDict) {
        if (pDict == m_pDocument->m_pRootDict || pDict == m_pEncryptDict) {
            return 1;
        }
        if (IsSignatureDict(pDict)) {
            return 1;
        }
        if (pDict->GetString(FX_BSTRC("Type")) == FX_BSTRC("Page")) {
            return 1;
        }
    }
    m_pXRefStream->AddObjectNumberToIndexArray(objnum);
    if (m_pXRefStream->CompressIndirectObject(objnum, pObj, this) < 0) {
        return -1;
    }
    if (!_IsXRefNeedEnd(m_pXRefStream, m_dwFlags)) {
        return 0;
    }
    if (!m_pXRefStream->End(this)) {
        return -1;
    }
    if (!m_pXRefStream->Start()) {
        return -1;
    }
    return 0;
}
FX_INT32 CPDF_Creator::WriteIndirectObjectToStream(FX_DWORD objnum, FX_LPCBYTE pBuffer, FX_DWORD dwSize)
{
    if (!m_pXRefStream) {
        return 1;
    }
    m_pXRefStream->AddObjectNumberToIndexArray(objnum);
    FX_INT32 iRet = m_pXRefStream->CompressIndirectObject(objnum, pBuffer, dwSize, this);
    if (iRet < 1) {
        return iRet;
    }
    if (!_IsXRefNeedEnd(m_pXRefStream, m_dwFlags)) {
        return 0;
    }
    if (!m_pXRefStream->End(this)) {
        return -1;
    }
    if (!m_pXRefStream->Start()) {
        return -1;
    }
    return 0;
}
FX_INT32 CPDF_Creator::AppendObjectNumberToXRef(FX_DWORD objnum)
{
    if (!m_pXRefStream) {
        return 1;
    }
    m_pXRefStream->AddObjectNumberToIndexArray(objnum);
    if (!_IsXRefNeedEnd(m_pXRefStream, m_dwFlags)) {
        return 0;
    }
    if (!m_pXRefStream->End(this)) {
        return -1;
    }
    if (!m_pXRefStream->Start()) {
        return -1;
    }
    return 0;
}
FX_INT32 CPDF_Creator::WriteStream(const CPDF_Object* pStream, FX_DWORD objnum, CPDF_CryptoHandler* pCrypto)
{
    CPDF_FlateEncoder encoder;
    encoder.Initialize((CPDF_Stream*)pStream, pStream == m_pMetadata ? FALSE : m_bCompress);
    CPDF_Encryptor encryptor;
    if(!encryptor.Initialize(pCrypto, objnum, encoder.m_pData, encoder.m_dwSize)) {
        return -1;
    }
    if ((FX_DWORD)encoder.m_pDict->GetInteger(FX_BSTRC("Length")) != encryptor.m_dwSize) {
        encoder.CloneDict();
        encoder.m_pDict->SetAtInteger(FX_BSTRC("Length"), encryptor.m_dwSize);
    }
    if (WriteDirectObj(objnum, encoder.m_pDict) < 0) {
        return -1;
    }
    int len = m_File.AppendString(FX_BSTRC("stream\r\n"));
    if (len < 0) {
        return -1;
    }
    m_Offset += len;
    if (m_File.AppendBlock(encryptor.m_pData, encryptor.m_dwSize) < 0) {
        return -1;
    }
    m_Offset += encryptor.m_dwSize;
    if ((len = m_File.AppendString(FX_BSTRC("\r\nendstream"))) < 0) {
        return -1;
    }
    m_Offset += len;
    return 1;
}
FX_INT32 CPDF_Creator::WriteIndirectObj(FX_DWORD objnum, const CPDF_Object* pObj)
{
    FX_INT32 len = m_File.AppendDWord(objnum);
    if (len < 0) {
        return -1;
    }
    m_Offset += len;
    if ((len = m_File.AppendString(FX_BSTRC(" 0 obj\r\n"))) < 0) {
        return -1;
    }
    m_Offset += len;
    if (pObj->GetType() == PDFOBJ_STREAM) {
        CPDF_CryptoHandler *pHandler = NULL;
        pHandler = (pObj == m_pMetadata && !m_bEncryptMetadata) ? NULL : m_pCryptoHandler;
        if (WriteStream(pObj, objnum, pHandler) < 0) {
            return -1;
        }
    } else {
        if (WriteDirectObj(objnum, pObj) < 0) {
            return -1;
        }
    }
    if ((len = m_File.AppendString(FX_BSTRC("\r\nendobj\r\n"))) < 0) {
        return -1;
    }
    m_Offset += len;
    if (AppendObjectNumberToXRef(objnum) < 0) {
        return -1;
    }
    return 0;
}
FX_INT32 CPDF_Creator::WriteIndirectObj(const CPDF_Object* pObj)
{
    FX_INT32 iRet = WriteIndirectObjectToStream(pObj);
    if (iRet < 1) {
        return iRet;
    }
    return WriteIndirectObj(pObj->GetObjNum(), pObj);
}
FX_INT32 CPDF_Creator::WriteDirectObj(FX_DWORD objnum, const CPDF_Object* pObj, FX_BOOL bEncrypt)
{
    FX_INT32 len = 0;
    if (pObj == NULL) {
        if (m_File.AppendString(FX_BSTRC(" null")) < 0) {
            return -1;
        }
        m_Offset += 5;
        return 1;
    }
    switch (pObj->GetType()) {
        case PDFOBJ_NULL:
            if (m_File.AppendString(FX_BSTRC(" null")) < 0) {
                return -1;
            }
            m_Offset += 5;
            break;
        case PDFOBJ_BOOLEAN:
        case PDFOBJ_NUMBER:
            if (m_File.AppendString(FX_BSTRC(" ")) < 0) {
                return -1;
            }
            if ((len = m_File.AppendString(pObj->GetString())) < 0) {
                return -1;
            }
            m_Offset += len + 1;
            break;
        case PDFOBJ_STRING: {
                CFX_ByteString str = pObj->GetString();
                FX_BOOL bHex = ((CPDF_String*)pObj)->IsHex();
                if (m_pCryptoHandler == NULL || !bEncrypt) {
                    CFX_ByteString content = PDF_EncodeString(str, bHex);
                    if ((len = m_File.AppendString(content)) < 0) {
                        return -1;
                    }
                    m_Offset += len;
                    break;
                }
                CPDF_Encryptor encryptor;
                encryptor.Initialize(m_pCryptoHandler, objnum, (FX_LPBYTE)str.c_str(), str.GetLength());
                CFX_ByteString content = PDF_EncodeString(CFX_ByteString((FX_LPCSTR)encryptor.m_pData, encryptor.m_dwSize), bHex);
                if ((len = m_File.AppendString(content)) < 0) {
                    return -1;
                }
                m_Offset += len;
                break;
            }
        case PDFOBJ_STREAM: {
                CPDF_FlateEncoder encoder;
                encoder.Initialize((CPDF_Stream*)pObj, m_bCompress);
                CPDF_Encryptor encryptor;
                CPDF_CryptoHandler* pHandler = m_pCryptoHandler;
                encryptor.Initialize(pHandler, objnum, encoder.m_pData, encoder.m_dwSize);
                if ((FX_DWORD)encoder.m_pDict->GetInteger(FX_BSTRC("Length")) != encryptor.m_dwSize) {
                    encoder.CloneDict();
                    encoder.m_pDict->SetAtInteger(FX_BSTRC("Length"), encryptor.m_dwSize);
                }
                if (WriteDirectObj(objnum, encoder.m_pDict) < 0) {
                    return -1;
                }
                if ((len = m_File.AppendString(FX_BSTRC("stream\r\n"))) < 0) {
                    return -1;
                }
                m_Offset += len;
                if (m_File.AppendBlock(encryptor.m_pData, encryptor.m_dwSize) < 0) {
                    return -1;
                }
                m_Offset += encryptor.m_dwSize;
                if ((len = m_File.AppendString(FX_BSTRC("\r\nendstream"))) < 0) {
                    return -1;
                }
                m_Offset += len;
                break;
            }
        case PDFOBJ_NAME: {
                if (m_File.AppendString(FX_BSTRC("/")) < 0) {
                    return -1;
                }
                CFX_ByteString str = pObj->GetString();
                if ((len = m_File.AppendString(PDF_NameEncode(str))) < 0) {
                    return -1;
                }
                m_Offset += len + 1;
                break;
            }
        case PDFOBJ_REFERENCE: {
                if (m_File.AppendString(FX_BSTRC(" ")) < 0) {
                    return -1;
                }
                CPDF_Reference* p = (CPDF_Reference*)pObj;
                if ((len = m_File.AppendDWord(p->GetRefObjNum())) < 0) {
                    return -1;
                }
                if (m_File.AppendString(FX_BSTRC(" 0 R")) < 0) {
                    return -1;
                }
                m_Offset += len + 5;
                break;
            }
        case PDFOBJ_ARRAY: {
                if (m_File.AppendString(FX_BSTRC("[")) < 0) {
                    return -1;
                }
                m_Offset += 1;
                CPDF_Array* p = (CPDF_Array*)pObj;
                for (FX_DWORD i = 0; i < p->GetCount(); i ++) {
                    CPDF_Object* pElement = p->GetElement(i);
                    if (pElement->GetObjNum()) {
                        if (m_File.AppendString(FX_BSTRC(" ")) < 0) {
                            return -1;
                        }
                        if ((len = m_File.AppendDWord(pElement->GetObjNum())) < 0) {
                            return -1;
                        }
                        if (m_File.AppendString(FX_BSTRC(" 0 R")) < 0) {
                            return -1;
                        }
                        m_Offset += len + 5;
                    } else {
                        if (WriteDirectObj(objnum, pElement) < 0) {
                            return -1;
                        }
                    }
                }
                if (m_File.AppendString(FX_BSTRC("]")) < 0) {
                    return -1;
                }
                m_Offset += 1;
                break;
            }
        case PDFOBJ_DICTIONARY: {
                if (m_pCryptoHandler == NULL || pObj == m_pEncryptDict) {
                    return PDF_CreatorAppendObject(pObj, &m_File, m_Offset);
                }
                if (m_File.AppendString(FX_BSTRC("<<")) < 0) {
                    return -1;
                }
                m_Offset += 2;
                CPDF_Dictionary* p = (CPDF_Dictionary*)pObj;
                FX_BOOL bSignDict = IsSignatureDict(p);
                FX_POSITION pos = p->GetStartPos();
                while (pos) {
                    FX_BOOL bSignValue = FALSE;
                    CFX_ByteString key;
                    CPDF_Object* pValue = p->GetNextElement(pos, key);
                    if (m_File.AppendString(FX_BSTRC("/")) < 0) {
                        return -1;
                    }
                    if ((len = m_File.AppendString(PDF_NameEncode(key))) < 0) {
                        return -1;
                    }
                    m_Offset += len + 1;
                    if (bSignDict && key == FX_BSTRC("Contents")) {
                        bSignValue = TRUE;
                    }
                    if (pValue->GetObjNum()) {
                        if (m_File.AppendString(FX_BSTRC(" ")) < 0) {
                            return -1;
                        }
                        if ((len = m_File.AppendDWord(pValue->GetObjNum())) < 0) {
                            return -1;
                        }
                        if (m_File.AppendString(FX_BSTRC(" 0 R ")) < 0) {
                            return -1;
                        }
                        m_Offset += len + 6;
                    } else {
                        if (WriteDirectObj(objnum, pValue, !bSignValue) < 0) {
                            return -1;
                        }
                    }
                }
                if (m_File.AppendString(FX_BSTRC(">>")) < 0) {
                    return -1;
                }
                m_Offset += 2;
                break;
            }
    }
    return 1;
}
FX_INT32 CPDF_Creator::WriteOldIndirectObject(FX_DWORD objnum)
{
    if(m_pParser->m_V5Type[objnum] == 0 || m_pParser->m_V5Type[objnum] == 255) {
        return 0;
    }
    m_ObjectOffset[objnum] = m_Offset;
    FX_LPVOID valuetemp = NULL;
    FX_BOOL bExistInMap = m_pDocument->m_IndirectObjs.Lookup((FX_LPVOID)(FX_UINTPTR)objnum, valuetemp);
    FX_BOOL bObjStm = (m_pParser->m_V5Type[objnum] == 2) && m_pEncryptDict && !m_pXRefStream;
    if(m_pParser->m_bVersionUpdated || m_bSecurityChanged || bExistInMap || bObjStm) {
        CPDF_Object* pObj = m_pDocument->GetIndirectObject(objnum);
        if (pObj == NULL) {
            m_ObjectOffset[objnum] = 0;
            m_ObjectSize[objnum] = 0;
            return 0;
        }
        if (WriteIndirectObj(pObj)) {
            return -1;
        }
        if (!bExistInMap) {
            m_pDocument->ReleaseIndirectObject(objnum);
        }
    } else {
        FX_BYTE* pBuffer;
        FX_DWORD size;
        m_pParser->GetIndirectBinary(objnum, pBuffer, size);
        if (pBuffer == NULL) {
            return 0;
        }
        if (m_pParser->m_V5Type[objnum] == 2) {
            if (m_pXRefStream) {
                if (WriteIndirectObjectToStream(objnum, pBuffer, size) < 0) {
                    FX_Free(pBuffer);
                    return -1;
                }
            } else {
                FX_INT32 len = m_File.AppendDWord(objnum);
                if (len < 0) {
                    return -1;
                }
                if (m_File.AppendString(FX_BSTRC(" 0 obj ")) < 0) {
                    return -1;
                }
                m_Offset += len + 7;
                if (m_File.AppendBlock(pBuffer, size) < 0) {
                    return -1;
                }
                m_Offset += size;
                if (m_File.AppendString(FX_BSTRC("\r\nendobj\r\n")) < 0) {
                    return -1;
                }
                m_Offset += 10;
            }
        } else {
            if (m_File.AppendBlock(pBuffer, size) < 0) {
                return -1;
            }
            m_Offset += size;
            if(AppendObjectNumberToXRef(objnum) < 0) {
                return -1;
            }
        }
        FX_Free(pBuffer);
    }
    return 1;
}
FX_INT32 CPDF_Creator::WriteOldObjs(IFX_Pause *pPause)
{
    FX_DWORD nOldSize = m_pParser->m_CrossRef.GetSize();
    FX_DWORD objnum = (FX_DWORD)(FX_UINTPTR)m_Pos;
    for(; objnum < nOldSize; objnum ++) {
        FX_INT32 iRet = WriteOldIndirectObject(objnum);
        if (!iRet) {
            continue;
        }
        if (iRet < 0) {
            return iRet;
        }
        m_ObjectSize[objnum] = (FX_DWORD)(m_Offset - m_ObjectOffset[objnum]);
        if (pPause && pPause->NeedToPauseNow()) {
            m_Pos = (FX_LPVOID)(FX_UINTPTR)(objnum + 1);
            return 1;
        }
    }
    return 0;
}
FX_INT32 CPDF_Creator::WriteNewObjs(FX_BOOL bIncremental, IFX_Pause *pPause)
{
    FX_INT32 iCount = m_NewObjNumArray.GetSize();
    FX_INT32 index = (FX_INT32)(FX_UINTPTR)m_Pos;
    while (index < iCount) {
        FX_DWORD objnum = m_NewObjNumArray.ElementAt(index);
        CPDF_Object *pObj = NULL;
        m_pDocument->m_IndirectObjs.Lookup((FX_LPVOID)(FX_UINTPTR)objnum, (FX_LPVOID&)pObj);
        if (NULL == pObj) {
            ++index;
            continue;
        }
        m_ObjectOffset[objnum] = m_Offset;
        if (WriteIndirectObj(pObj)) {
            return -1;
        }
        m_ObjectSize[objnum] = (FX_DWORD)(m_Offset - m_ObjectOffset[objnum]);
        index++;
        if (pPause && pPause->NeedToPauseNow()) {
            m_Pos = (FX_POSITION)(FX_UINTPTR)index;
            return 1;
        }
    }
    return 0;
}
void CPDF_Creator::InitOldObjNumOffsets()
{
    if (!m_pParser) {
        return;
    }
    FX_DWORD j = 0;
    FX_DWORD dwStart = 0;
    FX_DWORD dwEnd = m_pParser->GetLastObjNum();
    while (dwStart <= dwEnd) {
        while (dwStart <= dwEnd && (m_pParser->m_V5Type[dwStart] == 0 || m_pParser->m_V5Type[dwStart] == 255)) {
            dwStart++;
        }
        if (dwStart > dwEnd) {
            break;
        }
        j = dwStart;
        while (j <= dwEnd && m_pParser->m_V5Type[j] != 0 && m_pParser->m_V5Type[j] != 255) {
            j++;
        }
        m_ObjectOffset.Add(dwStart, j - dwStart);
        m_ObjectSize.Add(dwStart, j - dwStart);
        dwStart = j;
    }
}
void CPDF_Creator::InitNewObjNumOffsets()
{
    FX_BOOL bIncremental = (m_dwFlags & FPDFCREATE_INCREMENTAL) != 0;
    FX_BOOL bNoOriginal = (m_dwFlags & FPDFCREATE_NO_ORIGINAL) != 0;
    FX_DWORD nOldSize = m_pParser ? m_pParser->m_CrossRef.GetSize() : 0;
    FX_POSITION pos = m_pDocument->m_IndirectObjs.GetStartPosition();
    while (pos) {
        size_t key = 0;
        CPDF_Object* pObj;
        m_pDocument->m_IndirectObjs.GetNextAssoc(pos, (FX_LPVOID&)key, (FX_LPVOID&)pObj);
        FX_DWORD objnum = (FX_DWORD)key;
        if (pObj->GetObjNum() == -1) {
            continue;
        }
        if (bIncremental) {
            if (!pObj->IsModified()) {
                continue;
            }
        } else {
            if (objnum < nOldSize && m_pParser->m_V5Type[objnum] != 0) {
                continue;
            }
        }
        AppendNewObjNum(objnum);
    }
    FX_INT32 iCount = m_NewObjNumArray.GetSize();
    if (iCount == 0) {
        return;
    }
    FX_INT32 i = 0;
    FX_DWORD dwStartObjNum = 0;
    FX_BOOL bCrossRefValid = m_pParser && m_pParser->GetLastXRefOffset() > 0;
    while (i < iCount) {
        dwStartObjNum = m_NewObjNumArray.ElementAt(i);
        if ((bIncremental && (bNoOriginal || bCrossRefValid)) || !m_ObjectOffset.GetPtrAt(dwStartObjNum)) {
            break;
        }
        i++;
    }
    if (i >= iCount) {
        return;
    }
    FX_DWORD dwLastObjNum = dwStartObjNum;
    i++;
    FX_BOOL bNewStart = FALSE;
    for (; i < iCount; i++) {
        FX_DWORD dwCurObjNum = m_NewObjNumArray.ElementAt(i);
        FX_BOOL bExist = (dwCurObjNum < nOldSize && m_ObjectOffset.GetPtrAt(dwCurObjNum) != NULL);
        if (bExist || dwCurObjNum - dwLastObjNum > 1) {
            if (!bNewStart) {
                m_ObjectOffset.Add(dwStartObjNum, dwLastObjNum - dwStartObjNum + 1);
                m_ObjectSize.Add(dwStartObjNum, dwLastObjNum - dwStartObjNum + 1);
            }
            dwStartObjNum = dwCurObjNum;
        }
        if (bNewStart) {
            dwStartObjNum = dwCurObjNum;
        }
        bNewStart = bExist;
        dwLastObjNum = dwCurObjNum;
    }
    m_ObjectOffset.Add(dwStartObjNum, dwLastObjNum - dwStartObjNum + 1);
    m_ObjectSize.Add(dwStartObjNum, dwLastObjNum - dwStartObjNum + 1);
}
void CPDF_Creator::AppendNewObjNum(FX_DWORD objbum)
{
    FX_INT32 iStart = 0, iFind = 0;
    FX_INT32 iEnd = m_NewObjNumArray.GetUpperBound();
    while (iStart <= iEnd) {
        FX_INT32 iMid = (iStart + iEnd) / 2;
        FX_DWORD dwMid = m_NewObjNumArray.ElementAt(iMid);
        if (objbum < dwMid) {
            iEnd = iMid - 1;
        } else {
            if (iMid == iEnd) {
                iFind = iMid + 1;
                break;
            }
            FX_DWORD dwNext = m_NewObjNumArray.ElementAt(iMid + 1);
            if (objbum < dwNext) {
                iFind = iMid + 1;
                break;
            }
            iStart = iMid + 1;
        }
    }
    m_NewObjNumArray.InsertAt(iFind, objbum);
}
FX_INT32 CPDF_Creator::WriteDoc_Stage1(IFX_Pause *pPause)
{
    FXSYS_assert(m_iStage > -1 || m_iStage < 20);
    if (m_iStage == 0) {
        if (m_pParser == NULL) {
            m_dwFlags &= ~FPDFCREATE_INCREMENTAL;
        }
        if (m_bSecurityChanged && (m_dwFlags & FPDFCREATE_NO_ORIGINAL) == 0) {
            m_dwFlags &= ~FPDFCREATE_INCREMENTAL;
        }
        CPDF_Dictionary* pDict = m_pDocument->GetRoot();
        m_pMetadata = pDict ? pDict->GetElementValue(FX_BSTRC("Metadata")) : NULL;
        if (m_dwFlags & FPDFCREATE_OBJECTSTREAM) {
            m_pXRefStream = new CPDF_XRefStream;
            m_pXRefStream->Start();
            if ((m_dwFlags & FPDFCREATE_INCREMENTAL) != 0 && m_pParser) {
                FX_FILESIZE prev = m_pParser->GetLastXRefOffset();
                m_pXRefStream->m_PrevOffset = prev;
            }
        }
        m_iStage = 10;
    }
    if (m_iStage == 10) {
        if ((m_dwFlags & FPDFCREATE_INCREMENTAL) == 0) {
            if (m_File.AppendString(FX_BSTRC("%PDF-1.")) < 0) {
                return -1;
            }
            m_Offset += 7;
            FX_INT32 version = 7;
            if (m_FileVersion) {
                version = m_FileVersion;
            } else if (m_pParser) {
                version = m_pParser->GetFileVersion();
            }
            FX_INT32 len = m_File.AppendDWord(version % 10);
            if (len < 0) {
                return -1;
            }
            m_Offset += len;
            if ((len = m_File.AppendString(FX_BSTRC("\r\n%\xA1\xB3\xC5\xD7\r\n"))) < 0) {
                return -1;
            }
            m_Offset += len;
            InitOldObjNumOffsets();
            m_iStage = 20;
        } else {
            IFX_FileRead* pSrcFile = m_pParser->GetFileAccess();
            m_Offset = pSrcFile->GetSize();
            m_Pos = (FX_LPVOID)(FX_UINTPTR)m_Offset;
            m_iStage = 15;
        }
    }
    if (m_iStage == 15) {
        if ((m_dwFlags & FPDFCREATE_NO_ORIGINAL) == 0 && m_Pos) {
            IFX_FileRead* pSrcFile = m_pParser->GetFileAccess();
            FX_BYTE buffer[4096];
            FX_DWORD src_size = (FX_DWORD)(FX_UINTPTR)m_Pos;
            while (src_size) {
                FX_DWORD block_size = src_size > 4096 ? 4096 : src_size;
                if (!pSrcFile->ReadBlock(buffer, m_Offset - src_size, block_size)) {
                    return -1;
                }
                if (m_File.AppendBlock(buffer, block_size) < 0) {
                    return -1;
                }
                src_size -= block_size;
                if (pPause && pPause->NeedToPauseNow()) {
                    m_Pos = (FX_LPVOID)(FX_UINTPTR)src_size;
                    return 1;
                }
            }
        }
        if ((m_dwFlags & FPDFCREATE_NO_ORIGINAL) == 0 && m_pParser->GetLastXRefOffset() == 0) {
            InitOldObjNumOffsets();
            FX_DWORD dwEnd = m_pParser->GetLastObjNum();
            FX_BOOL bObjStm = (m_dwFlags & FPDFCREATE_OBJECTSTREAM) != 0;
            for (FX_DWORD objnum = 0; objnum <= dwEnd; objnum++) {
                if (m_pParser->m_V5Type[objnum] == 0 || m_pParser->m_V5Type[objnum] == 255) {
                    continue;
                }
                m_ObjectOffset[objnum] = m_pParser->m_CrossRef[objnum];
                if (bObjStm) {
                    m_pXRefStream->AddObjectNumberToIndexArray(objnum);
                }
            }
            if (bObjStm) {
                m_pXRefStream->EndXRefStream(this);
                m_pXRefStream->Start();
            }
        }
        m_iStage = 20;
    }
    InitNewObjNumOffsets();
    return m_iStage;
}
FX_INT32 CPDF_Creator::WriteDoc_Stage2(IFX_Pause *pPause)
{
    FXSYS_assert(m_iStage >= 20 || m_iStage < 30);
    if (m_iStage == 20) {
        if ((m_dwFlags & FPDFCREATE_INCREMENTAL) == 0 && m_pParser) {
            m_Pos = (FX_LPVOID)(FX_UINTPTR)0;
            m_iStage = 21;
        } else {
            m_iStage = 25;
        }
    }
    if (m_iStage == 21) {
        FX_INT32 iRet = WriteOldObjs(pPause);
        if (iRet) {
            return iRet;
        }
        m_iStage = 25;
    }
    if (m_iStage == 25) {
        m_Pos = (FX_LPVOID)(FX_UINTPTR)0;
        m_iStage = 26;
    }
    if (m_iStage == 26) {
        FX_INT32 iRet = WriteNewObjs((m_dwFlags & FPDFCREATE_INCREMENTAL) != 0, pPause);
        if (iRet) {
            return iRet;
        }
        m_iStage = 27;
    }
    if (m_iStage == 27) {
        if (NULL != m_pEncryptDict && 0 == m_pEncryptDict->GetObjNum()) {
            m_dwLastObjNum += 1;
            FX_FILESIZE saveOffset = m_Offset;
            if (WriteIndirectObj(m_dwLastObjNum, m_pEncryptDict) < 0) {
                return -1;
            }
            m_ObjectOffset.Add(m_dwLastObjNum, 1);
            m_ObjectOffset[m_dwLastObjNum] = saveOffset;
            m_ObjectSize.Add(m_dwLastObjNum, 1);
            m_ObjectSize[m_dwLastObjNum] = m_Offset - saveOffset;
            m_dwEnryptObjNum = m_dwLastObjNum;
            if (m_dwFlags & FPDFCREATE_INCREMENTAL) {
                m_NewObjNumArray.Add(m_dwLastObjNum);
            }
        }
        m_iStage = 80;
    }
    return m_iStage;
}
FX_INT32 CPDF_Creator::WriteDoc_Stage3(IFX_Pause *pPause)
{
    FXSYS_assert(m_iStage >= 80 || m_iStage < 90);
    FX_DWORD dwLastObjNum = m_dwLastObjNum;
    if (m_iStage == 80) {
        m_XrefStart = m_Offset;
        if (m_dwFlags & FPDFCREATE_OBJECTSTREAM) {
            m_pXRefStream->End(this, TRUE);
            m_XrefStart = m_pXRefStream->m_PrevOffset;
            m_iStage = 90;
        } else if ((m_dwFlags & FPDFCREATE_INCREMENTAL) == 0 || !m_pParser->IsXRefStream()) {
            if ((m_dwFlags & FPDFCREATE_INCREMENTAL) == 0 || m_pParser->GetLastXRefOffset() == 0) {
                CFX_ByteString str;
                str = m_ObjectOffset.GetPtrAt(1) ? FX_BSTRC("xref\r\n") : FX_BSTRC("xref\r\n0 1\r\n0000000000 65536 f\r\n");
                if (m_File.AppendString(str) < 0) {
                    return -1;
                }
                m_Pos = (FX_LPVOID)(FX_UINTPTR)1;
                m_iStage = 81;
            } else {
                if (m_File.AppendString(FX_BSTRC("xref\r\n")) < 0) {
                    return -1;
                }
                m_Pos = (FX_LPVOID)(FX_UINTPTR)0;
                m_iStage = 82;
            }
        } else {
            m_iStage = 90;
        }
    }
    if (m_iStage == 81) {
        CFX_ByteString str;
        FX_DWORD i = (FX_DWORD)(FX_UINTPTR)m_Pos, j;
        while (i <= dwLastObjNum) {
            while (i <= dwLastObjNum && !m_ObjectOffset.GetPtrAt(i)) {
                i++;
            }
            if (i > dwLastObjNum) {
                break;
            }
            j = i;
            while (j <= dwLastObjNum && m_ObjectOffset.GetPtrAt(j)) {
                j++;
            }
            if (i == 1) {
                str.Format("0 %d\r\n0000000000 65536 f\r\n", j);
            } else {
                str.Format("%d %d\r\n", i, j - i);
            }
            if (m_File.AppendBlock(str.c_str(), str.GetLength()) < 0) {
                return -1;
            }
            while (i < j) {
                str.Format("%010d 00000 n\r\n", m_ObjectOffset[i ++]);
                if (m_File.AppendBlock(str.c_str(), str.GetLength()) < 0) {
                    return -1;
                }
            }
            if (i > dwLastObjNum) {
                break;
            }
            if (pPause && pPause->NeedToPauseNow()) {
                m_Pos = (FX_LPVOID)(FX_UINTPTR)i;
                return 1;
            }
        }
        m_iStage = 90;
    }
    if (m_iStage == 82) {
        CFX_ByteString str;
        FX_INT32 iCount = m_NewObjNumArray.GetSize();
        FX_INT32 i = (FX_INT32)(FX_UINTPTR)m_Pos;
        while (i < iCount) {
            FX_INT32 j = i;
            FX_DWORD objnum = m_NewObjNumArray.ElementAt(i);
            while (j < iCount) {
                if (++j == iCount) {
                    break;
                }
                FX_DWORD dwCurrent = m_NewObjNumArray.ElementAt(j);
                if (dwCurrent - objnum > 1) {
                    break;
                }
                objnum = dwCurrent;
            }
            objnum = m_NewObjNumArray.ElementAt(i);
            if (objnum == 1) {
                str.Format("0 %d\r\n0000000000 65536 f\r\n", j - i + 1);
            } else {
                str.Format("%d %d\r\n", objnum, j - i);
            }
            if (m_File.AppendBlock(str.c_str(), str.GetLength()) < 0) {
                return -1;
            }
            while (i < j) {
                objnum = m_NewObjNumArray.ElementAt(i++);
                str.Format("%010d 00000 n\r\n", m_ObjectOffset[objnum]);
                if (m_File.AppendBlock(str.c_str(), str.GetLength()) < 0) {
                    return -1;
                }
            }
            if (pPause && (i % 100) == 0 && pPause->NeedToPauseNow()) {
                m_Pos = (FX_LPVOID)(FX_UINTPTR)i;
                return 1;
            }
        }
        m_iStage = 90;
    }
    return m_iStage;
}
static FX_INT32 _OutPutIndex(CFX_FileBufferArchive* pFile, FX_FILESIZE offset)
{
    FXSYS_assert(pFile);
    if (sizeof(offset) > 4) {
        if (FX_GETBYTEOFFSET32(offset)) {
            if (pFile->AppendByte(FX_GETBYTEOFFSET56(offset)) < 0) {
                return -1;
            }
            if (pFile->AppendByte(FX_GETBYTEOFFSET48(offset)) < 0) {
                return -1;
            }
            if (pFile->AppendByte(FX_GETBYTEOFFSET40(offset)) < 0) {
                return -1;
            }
            if (pFile->AppendByte(FX_GETBYTEOFFSET32(offset)) < 0) {
                return -1;
            }
        }
    }
    if (pFile->AppendByte(FX_GETBYTEOFFSET24(offset)) < 0) {
        return -1;
    }
    if (pFile->AppendByte(FX_GETBYTEOFFSET16(offset)) < 0) {
        return -1;
    }
    if (pFile->AppendByte(FX_GETBYTEOFFSET8(offset)) < 0) {
        return -1;
    }
    if (pFile->AppendByte(FX_GETBYTEOFFSET0(offset)) < 0) {
        return -1;
    }
    if (pFile->AppendByte(0) < 0) {
        return -1;
    }
    return 0;
}
FX_INT32 CPDF_Creator::WriteDoc_Stage4(IFX_Pause *pPause)
{
    FXSYS_assert(m_iStage >= 90);
    if ((m_dwFlags & FPDFCREATE_OBJECTSTREAM) == 0) {
        FX_BOOL bXRefStream = (m_dwFlags & FPDFCREATE_INCREMENTAL) != 0 && m_pParser->IsXRefStream();
        if (!bXRefStream) {
            if (m_File.AppendString(FX_BSTRC("trailer\r\n<<")) < 0) {
                return -1;
            }
        } else {
            if (m_File.AppendDWord(m_pDocument->m_LastObjNum + 1) < 0) {
                return -1;
            }
            if (m_File.AppendString(FX_BSTRC(" 0 obj <<")) < 0) {
                return -1;
            }
        }
        if (m_pParser) {
            CPDF_Dictionary* p = m_pParser->m_pTrailer;
            FX_POSITION pos = p->GetStartPos();
            while (pos) {
                CFX_ByteString key;
                CPDF_Object* pValue = p->GetNextElement(pos, key);
                if (key == FX_BSTRC("Encrypt") || key == FX_BSTRC("Size") || key == FX_BSTRC("Filter") ||
                        key == FX_BSTRC("Index") || key == FX_BSTRC("Length") || key == FX_BSTRC("Prev") ||
                        key == FX_BSTRC("W") || key == FX_BSTRC("XRefStm") || key == FX_BSTRC("ID")) {
                    continue;
                }
                if (m_File.AppendString((FX_BSTRC("/"))) < 0) {
                    return -1;
                }
                if (m_File.AppendString(PDF_NameEncode(key)) < 0) {
                    return -1;
                }
                if (pValue->GetObjNum()) {
                    if (m_File.AppendString(FX_BSTRC(" ")) < 0) {
                        return -1;
                    }
                    if (m_File.AppendDWord(pValue->GetObjNum()) < 0) {
                        return -1;
                    }
                    if (m_File.AppendString(FX_BSTRC(" 0 R ")) < 0) {
                        return -1;
                    }
                } else {
                    FX_FILESIZE offset = 0;
                    if (PDF_CreatorAppendObject(pValue, &m_File, offset) < 0) {
                        return -1;
                    }
                }
            }
        } else {
            if (m_File.AppendString(FX_BSTRC("\r\n/Root ")) < 0) {
                return -1;
            }
            if (m_File.AppendDWord(m_pDocument->m_pRootDict->GetObjNum()) < 0) {
                return -1;
            }
            if (m_File.AppendString(FX_BSTRC(" 0 R\r\n")) < 0) {
                return -1;
            }
            if (m_pDocument->m_pInfoDict) {
                if (m_File.AppendString(FX_BSTRC("/Info ")) < 0) {
                    return -1;
                }
                if (m_File.AppendDWord(m_pDocument->m_pInfoDict->GetObjNum()) < 0) {
                    return -1;
                }
                if (m_File.AppendString(FX_BSTRC(" 0 R\r\n")) < 0) {
                    return -1;
                }
            }
        }
        if (m_pEncryptDict) {
            if (m_File.AppendString(FX_BSTRC("/Encrypt")) < 0) {
                return -1;
            }
            FX_DWORD dwObjNum = m_pEncryptDict->GetObjNum();
            if (dwObjNum == 0) {
                dwObjNum = m_pDocument->GetLastObjNum() + 1;
            }
            if (m_File.AppendString(FX_BSTRC(" ")) < 0) {
                return -1;
            }
            if (m_File.AppendDWord(dwObjNum) < 0) {
                return -1;
            }
            if (m_File.AppendString(FX_BSTRC(" 0 R ")) < 0) {
                return -1;
            }
        }
        if (m_File.AppendString(FX_BSTRC("/Size ")) < 0) {
            return -1;
        }
        if (m_File.AppendDWord(m_dwLastObjNum + (bXRefStream ? 2 : 1)) < 0) {
            return -1;
        }
        if ((m_dwFlags & FPDFCREATE_INCREMENTAL) != 0) {
            FX_FILESIZE prev = m_pParser->GetLastXRefOffset();
            if (prev) {
                if (m_File.AppendString(FX_BSTRC("/Prev ")) < 0) {
                    return -1;
                }
                FX_CHAR offset_buf[20];
                FXSYS_memset32(offset_buf, 0, sizeof(offset_buf));
                FXSYS_i64toa(prev, offset_buf, 10);
                if (m_File.AppendBlock(offset_buf, FXSYS_strlen(offset_buf)) < 0) {
                    return -1;
                }
            }
        }
        if (m_pIDArray) {
            if (m_File.AppendString((FX_BSTRC("/ID"))) < 0) {
                return -1;
            }
            FX_FILESIZE offset = 0;
            if (PDF_CreatorAppendObject(m_pIDArray, &m_File, offset) < 0) {
                return -1;
            }
        }
        if (!bXRefStream) {
            if (m_File.AppendString(FX_BSTRC(">>")) < 0) {
                return -1;
            }
        } else {
            if (m_File.AppendString(FX_BSTRC("/W[0 4 1]/Index[")) < 0) {
                return -1;
            }
            if ((m_dwFlags & FPDFCREATE_INCREMENTAL) != 0 && m_pParser && m_pParser->GetLastXRefOffset() == 0) {
                FX_DWORD i = 0;
                for (i = 0; i < m_dwLastObjNum; i++) {
                    if (!m_ObjectOffset.GetPtrAt(i)) {
                        continue;
                    }
                    if (m_File.AppendDWord(i) < 0) {
                        return -1;
                    }
                    if (m_File.AppendString(FX_BSTRC(" 1 ")) < 0) {
                        return -1;
                    }
                }
                if (m_File.AppendString(FX_BSTRC("]/Length ")) < 0) {
                    return -1;
                }
                if (m_File.AppendDWord(m_dwLastObjNum * 5) < 0) {
                    return -1;
                }
                if (m_File.AppendString(FX_BSTRC(">>stream\r\n")) < 0) {
                    return -1;
                }
                for (i = 0; i < m_dwLastObjNum; i++) {
                    FX_FILESIZE* offset = m_ObjectOffset.GetPtrAt(i);
                    if (!offset) {
                        continue;
                    }
                    _OutPutIndex(&m_File, *offset);
                }
            } else {
                int count = m_NewObjNumArray.GetSize();
                FX_INT32 i = 0;
                for (i = 0; i < count; i++) {
                    FX_DWORD objnum = m_NewObjNumArray.ElementAt(i);
                    if (m_File.AppendDWord(objnum) < 0) {
                        return -1;
                    }
                    if (m_File.AppendString(FX_BSTRC(" 1 ")) < 0) {
                        return -1;
                    }
                }
                if (m_File.AppendString(FX_BSTRC("]/Length ")) < 0) {
                    return -1;
                }
                if (m_File.AppendDWord(count * 5) < 0) {
                    return -1;
                }
                if (m_File.AppendString(FX_BSTRC(">>stream\r\n")) < 0) {
                    return -1;
                }
                for (i = 0; i < count; i++) {
                    FX_DWORD objnum = m_NewObjNumArray.ElementAt(i);
                    FX_FILESIZE offset = m_ObjectOffset[objnum];
                    _OutPutIndex(&m_File, offset);
                }
            }
            if (m_File.AppendString(FX_BSTRC("\r\nendstream")) < 0) {
                return -1;
            }
        }
    }
    if (m_File.AppendString(FX_BSTRC("\r\nstartxref\r\n")) < 0) {
        return -1;
    }
    FX_CHAR offset_buf[20];
    FXSYS_memset32(offset_buf, 0, sizeof(offset_buf));
    FXSYS_i64toa(m_XrefStart, offset_buf, 10);
    if (m_File.AppendBlock(offset_buf, FXSYS_strlen(offset_buf)) < 0) {
        return -1;
    }
    if (m_File.AppendString(FX_BSTRC("\r\n%%EOF\r\n")) < 0) {
        return -1;
    }
    m_File.Flush();
    return m_iStage = 100;
}
void CPDF_Creator::Clear()
{
    if (m_pXRefStream) {
        delete m_pXRefStream;
        m_pXRefStream = NULL;
    }
    m_File.Clear();
    m_NewObjNumArray.RemoveAll();
    if (m_pIDArray) {
        m_pIDArray->Release();
        m_pIDArray = NULL;
    }
}
FX_BOOL CPDF_Creator::Create(FX_LPCSTR filename, FX_DWORD flags)
{
    if (!m_File.AttachFile(filename)) {
        return FALSE;
    }
    FX_BOOL bRet = Create(flags);
    if (!bRet || !(flags & FPDFCREATE_PROGRESSIVE)) {
        Clear();
    }
    return bRet;
}
FX_BOOL CPDF_Creator::Create(FX_LPCWSTR filename, FX_DWORD flags)
{
    if (!m_File.AttachFile(filename)) {
        return FALSE;
    }
    FX_BOOL bRet = Create(flags);
    if (!bRet || !(flags & FPDFCREATE_PROGRESSIVE)) {
        Clear();
    }
    return bRet;
}
FX_BOOL CPDF_Creator::Create(IFX_StreamWrite* pFile, FX_DWORD flags)
{
    if (!pFile) {
        return FALSE;
    }
    if (!m_File.AttachFile(pFile, FALSE)) {
        return FALSE;
    }
    return Create(flags);
}
FX_BOOL CPDF_Creator::Create(FX_DWORD flags)
{
    m_dwFlags = flags;
    m_iStage = 0;
    m_Offset = 0;
    m_dwLastObjNum = m_pDocument->GetLastObjNum();
    m_ObjectOffset.Clear();
    m_ObjectSize.Clear();
    m_NewObjNumArray.RemoveAll();
    InitID();
    if (flags & FPDFCREATE_PROGRESSIVE) {
        return TRUE;
    }
    return Continue(NULL) > -1;
}
void CPDF_Creator::InitID(FX_BOOL bDefault )
{
    CPDF_Array* pOldIDArray = m_pParser ? m_pParser->GetIDArray() : NULL;
    FX_BOOL bNewId = !m_pIDArray;
    if (!m_pIDArray) {
        FX_LPDWORD pBuffer = NULL;
        m_pIDArray = CPDF_Array::Create();
        CPDF_Object* pID1 = pOldIDArray ? pOldIDArray->GetElement(0) : NULL;
        if (pID1) {
            m_pIDArray->Add(pID1->Clone());
        } else {
            pBuffer = FX_Alloc(FX_DWORD, 4);
            PDF_GenerateFileID((FX_DWORD)(FX_UINTPTR)this, m_dwLastObjNum, pBuffer);
            CFX_ByteStringC bsBuffer((FX_LPCBYTE)pBuffer, 4 * sizeof(FX_DWORD));
            m_pIDArray->Add(CPDF_String::Create(bsBuffer, TRUE), m_pDocument);
        }
        if (pBuffer) {
            FX_Free(pBuffer);
        }
    }
    if (!bDefault) {
        return;
    }
    if (pOldIDArray) {
        CPDF_Object* pID2 = pOldIDArray->GetElement(1);
        if ((m_dwFlags & FPDFCREATE_INCREMENTAL) && m_pEncryptDict && pID2) {
            m_pIDArray->Add(pID2->Clone());
            return;
        }
        FX_LPDWORD pBuffer = FX_Alloc(FX_DWORD, 4);
        PDF_GenerateFileID((FX_DWORD)(FX_UINTPTR)this, m_dwLastObjNum, pBuffer);
        CFX_ByteStringC bsBuffer((FX_LPCBYTE)pBuffer, 4 * sizeof(FX_DWORD));
        m_pIDArray->Add(CPDF_String::Create(bsBuffer, TRUE), m_pDocument);
        FX_Free(pBuffer);
        return;
    }
    m_pIDArray->Add(m_pIDArray->GetElement(0)->Clone());
    if (m_pEncryptDict && !pOldIDArray && m_pParser && bNewId) {
        if (m_pEncryptDict->GetString(FX_BSTRC("Filter")) == FX_BSTRC("Standard")) {
            CPDF_StandardSecurityHandler handler;
            CFX_ByteString user_pass = m_pParser->GetPassword();
            FX_DWORD flag = PDF_ENCRYPT_CONTENT;
            handler.OnCreate(m_pEncryptDict, m_pIDArray, (FX_LPCBYTE)user_pass, user_pass.GetLength(), flag);
            if (m_pCryptoHandler && m_bNewCrypto) {
                delete m_pCryptoHandler;
            }
            m_pCryptoHandler = new CPDF_StandardCryptoHandler;
            m_pCryptoHandler->Init(m_pEncryptDict, &handler);
            m_bNewCrypto = TRUE;
            m_bSecurityChanged = TRUE;
        }
    }
}
FX_INT32 CPDF_Creator::Continue(IFX_Pause *pPause)
{
    if (m_iStage < 0) {
        return m_iStage;
    }
    FX_INT32 iRet = 0;
    while (m_iStage < 100) {
        if (m_iStage < 20) {
            iRet = WriteDoc_Stage1(pPause);
        } else if (m_iStage < 30) {
            iRet = WriteDoc_Stage2(pPause);
        } else if (m_iStage < 90) {
            iRet = WriteDoc_Stage3(pPause);
        } else {
            iRet = WriteDoc_Stage4(pPause);
        }
        if (iRet < m_iStage) {
            break;
        }
    }
    if (iRet < 1 || m_iStage == 100) {
        m_iStage = -1;
        Clear();
        return iRet > 99 ? 0 : (iRet < 1 ? -1 : iRet);
    }
    return m_iStage;
}
FX_BOOL CPDF_Creator::SetFileVersion(FX_INT32 fileVersion )
{
    if (fileVersion < 10 || fileVersion > 17) {
        return FALSE;
    }
    m_FileVersion = fileVersion;
    return TRUE;
}
void CPDF_Creator::RemoveSecurity()
{
    ResetStandardSecurity();
    m_bSecurityChanged = TRUE;
    m_pEncryptDict = NULL;
    m_pCryptoHandler = NULL;
}
void CPDF_Creator::ResetStandardSecurity()
{
    if ((m_bStandardSecurity || m_bNewCrypto) && m_pCryptoHandler) {
        delete m_pCryptoHandler;
        m_pCryptoHandler = NULL;
    }
    m_bNewCrypto = FALSE;
    if (!m_bStandardSecurity) {
        return;
    }
    if (m_pEncryptDict) {
        m_pEncryptDict->Release();
        m_pEncryptDict = NULL;
    }
    m_bStandardSecurity = FALSE;
}
