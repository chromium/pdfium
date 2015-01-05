// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <map>
#include <list>
#include "JBig2_Context.h"

// Implement a very small least recently used (LRU) cache. It is very
// common for a JBIG2 dictionary to span multiple pages in a PDF file,
// and we do not want to decode the same dictionary over and over
// again. We key off of the memory location of the dictionary. The
// list keeps track of the freshness of entries, with freshest ones
// at the front. Even a tiny cache size like 2 makes a dramatic
// difference for typical JBIG2 documents.
const int kSymbolDictCacheMaxSize = 2;

void OutputBitmap(CJBig2_Image* pImage)
{
    if(!pImage) {
        return;
    }
}
CJBig2_Context *CJBig2_Context::CreateContext(CJBig2_Module *pModule, FX_BYTE *pGlobalData, FX_DWORD dwGlobalLength,
        FX_BYTE *pData, FX_DWORD dwLength, FX_INT32 nStreamType, std::list<CJBig2_CachePair>* pSymbolDictCache, IFX_Pause* pPause)
{
    return new(pModule)CJBig2_Context(pGlobalData, dwGlobalLength, pData, dwLength, nStreamType, pSymbolDictCache, pPause);
}
void CJBig2_Context::DestroyContext(CJBig2_Context *pContext)
{
    if(pContext) {
        delete pContext;
    }
}
CJBig2_Context::CJBig2_Context(FX_BYTE *pGlobalData, FX_DWORD dwGlobalLength,
                               FX_BYTE *pData, FX_DWORD dwLength, FX_INT32 nStreamType, std::list<CJBig2_CachePair>* pSymbolDictCache, IFX_Pause* pPause)
{
    if(pGlobalData && (dwGlobalLength > 0)) {
        JBIG2_ALLOC(m_pGlobalContext, CJBig2_Context(NULL, 0, pGlobalData, dwGlobalLength,
                    JBIG2_EMBED_STREAM, pSymbolDictCache, pPause));
    } else {
        m_pGlobalContext = NULL;
    }
    JBIG2_ALLOC(m_pStream, CJBig2_BitStream(pData, dwLength));
    m_nStreamType = nStreamType;
    m_nState = JBIG2_OUT_OF_PAGE;
    JBIG2_ALLOC(m_pSegmentList, CJBig2_List<CJBig2_Segment>);
    JBIG2_ALLOC(m_pPageInfoList, CJBig2_List<JBig2PageInfo>(1));
    m_pPage = NULL;
    m_bBufSpecified = FALSE;
    m_pPause = pPause;
    m_nSegmentDecoded = 0;
    m_PauseStep = 10;
    m_pArithDecoder = NULL;
    m_pGRD = NULL;
    m_gbContext = NULL;
    m_pSegment = NULL;
    m_dwOffset = 0;
    m_ProcessiveStatus = FXCODEC_STATUS_FRAME_READY;
    m_pSymbolDictCache = pSymbolDictCache;
}
CJBig2_Context::~CJBig2_Context()
{
    if(m_pArithDecoder) {
        delete m_pArithDecoder;
    }
    m_pArithDecoder = NULL;
    if(m_pGRD) {
        delete m_pGRD;
    }
    m_pGRD = NULL;
    if(m_gbContext) {
        m_pModule->JBig2_Free(m_gbContext);
    }
    m_gbContext = NULL;
    if(m_pGlobalContext) {
        delete m_pGlobalContext;
    }
    m_pGlobalContext = NULL;
    if(m_pPageInfoList) {
        delete m_pPageInfoList;
    }
    m_pPageInfoList = NULL;
    if(m_bBufSpecified && m_pPage) {
        delete m_pPage;
    }
    m_pPage = NULL;
    if(m_pStream) {
        delete m_pStream;
    }
    m_pStream = NULL;
    if(m_pSegmentList) {
        delete m_pSegmentList;
    }
    m_pSegmentList = NULL;
}
FX_INT32 CJBig2_Context::decodeFile(IFX_Pause* pPause)
{
    FX_BYTE cFlags;
    FX_DWORD dwTemp;
    const FX_BYTE fileID[] = {0x97, 0x4A, 0x42, 0x32, 0x0D, 0x0A, 0x1A, 0x0A};
    FX_INT32 nRet;
    if(m_pStream->getByteLeft() < 8) {
        m_pModule->JBig2_Error("file header too short.");
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
    }
    if(JBIG2_memcmp(m_pStream->getPointer(), fileID, 8) != 0) {
        m_pModule->JBig2_Error("not jbig2 file");
        nRet = JBIG2_ERROR_FILE_FORMAT;
        goto failed;
    }
    m_pStream->offset(8);
    if(m_pStream->read1Byte(&cFlags) != 0) {
        m_pModule->JBig2_Error("file header too short.");
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
    }
    if(!(cFlags & 0x02)) {
        if(m_pStream->readInteger(&dwTemp) != 0) {
            m_pModule->JBig2_Error("file header too short.");
            nRet = JBIG2_ERROR_TOO_SHORT;
            goto failed;
        }
        if(dwTemp > 0) {
            delete m_pPageInfoList;
            JBIG2_ALLOC(m_pPageInfoList, CJBig2_List<JBig2PageInfo>(dwTemp));
        }
    }
    if(cFlags & 0x01) {
        m_nStreamType = JBIG2_SQUENTIAL_STREAM;
        return decode_SquentialOrgnazation(pPause);
    } else {
        m_nStreamType = JBIG2_RANDOM_STREAM;
        return decode_RandomOrgnazation_FirstPage(pPause);
    }
failed:
    return nRet;
}
FX_INT32 CJBig2_Context::decode_SquentialOrgnazation(IFX_Pause* pPause)
{
    FX_INT32 nRet;
    if(m_pStream->getByteLeft() > 0) {
        while(m_pStream->getByteLeft() >= JBIG2_MIN_SEGMENT_SIZE) {
            if(m_pSegment == NULL) {
                JBIG2_ALLOC(m_pSegment, CJBig2_Segment());
                nRet = parseSegmentHeader(m_pSegment);
                if(nRet != JBIG2_SUCCESS) {
                    delete m_pSegment;
                    m_pSegment = NULL;
                    return nRet;
                }
                m_dwOffset = m_pStream->getOffset();
            }
            nRet = parseSegmentData(m_pSegment, pPause);
            if(m_ProcessiveStatus  == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
                m_ProcessiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
                m_PauseStep = 2;
                return JBIG2_SUCCESS;
            }
            if((nRet == JBIG2_END_OF_PAGE) || (nRet == JBIG2_END_OF_FILE)) {
                delete m_pSegment;
                m_pSegment = NULL;
                break;
            } else if(nRet != JBIG2_SUCCESS) {
                delete m_pSegment;
                m_pSegment = NULL;
                return nRet;
            }
            m_pSegmentList->addItem(m_pSegment);
            if(m_pSegment->m_dwData_length != 0xffffffff) {
                m_dwOffset = m_dwOffset + m_pSegment->m_dwData_length;
                m_pStream->setOffset(m_dwOffset);
            } else {
                m_pStream->offset(4);
            }
            OutputBitmap(m_pPage);
            m_pSegment = NULL;
            if(m_pStream->getByteLeft() > 0 && m_pPage && pPause && pPause->NeedToPauseNow()) {
                m_ProcessiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
                m_PauseStep = 2;
                return JBIG2_SUCCESS;
            }
        }
    } else {
        return JBIG2_END_OF_FILE;
    }
    return JBIG2_SUCCESS;
}
FX_INT32 CJBig2_Context::decode_EmbedOrgnazation(IFX_Pause* pPause)
{
    return decode_SquentialOrgnazation(pPause);
}
FX_INT32 CJBig2_Context::decode_RandomOrgnazation_FirstPage(IFX_Pause* pPause)
{
    CJBig2_Segment *pSegment;
    FX_INT32 nRet;
    while(m_pStream->getByteLeft() > JBIG2_MIN_SEGMENT_SIZE) {
        JBIG2_ALLOC(pSegment, CJBig2_Segment());
        nRet = parseSegmentHeader(pSegment);
        if(nRet != JBIG2_SUCCESS) {
            delete pSegment;
            return nRet;
        } else if(pSegment->m_cFlags.s.type == 51) {
            delete pSegment;
            break;
        }
        m_pSegmentList->addItem(pSegment);
        if(pPause && m_pPause && pPause->NeedToPauseNow()) {
            m_PauseStep = 3;
            m_ProcessiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return JBIG2_SUCCESS;
        }
    }
    m_nSegmentDecoded = 0;
    return decode_RandomOrgnazation(pPause);
}
FX_INT32 CJBig2_Context::decode_RandomOrgnazation(IFX_Pause* pPause)
{
    FX_INT32 nRet;
    for(; m_nSegmentDecoded < m_pSegmentList->getLength(); m_nSegmentDecoded++) {
        nRet = parseSegmentData(m_pSegmentList->getAt(m_nSegmentDecoded), pPause);
        if((nRet == JBIG2_END_OF_PAGE) || (nRet == JBIG2_END_OF_FILE)) {
            break;
        } else if(nRet != JBIG2_SUCCESS) {
            return nRet;
        }
        if(m_pPage && pPause && pPause->NeedToPauseNow()) {
            m_PauseStep = 4;
            m_ProcessiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return JBIG2_SUCCESS;
        }
    }
    return JBIG2_SUCCESS;
}
FX_INT32 CJBig2_Context::getFirstPage(FX_BYTE *pBuf, FX_INT32 width, FX_INT32 height, FX_INT32 stride, IFX_Pause* pPause)
{
    FX_INT32 nRet = 0;
    if(m_pGlobalContext) {
        nRet = m_pGlobalContext->decode_EmbedOrgnazation(pPause);
        if(nRet != JBIG2_SUCCESS) {
            m_ProcessiveStatus = FXCODEC_STATUS_ERROR;
            return nRet;
        }
    }
    m_bFirstPage = TRUE;
    m_PauseStep = 0;
    if(m_pPage) {
        delete m_pPage;
    }
    JBIG2_ALLOC(m_pPage, CJBig2_Image(width, height, stride, pBuf));
    m_bBufSpecified = TRUE;
    if(m_pPage && pPause && pPause->NeedToPauseNow()) {
        m_PauseStep = 1;
        m_ProcessiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
        return nRet;
    }
    int ret = Continue(pPause);
    return ret;
}
FX_INT32 CJBig2_Context::Continue(IFX_Pause* pPause)
{
    m_ProcessiveStatus = FXCODEC_STATUS_DECODE_READY;
    FX_INT32 nRet;
    if(m_PauseStep <= 1) {
        switch(m_nStreamType) {
            case JBIG2_FILE_STREAM:
                nRet = decodeFile(pPause);
                break;
            case JBIG2_SQUENTIAL_STREAM:
                nRet = decode_SquentialOrgnazation(pPause);
                break;
            case JBIG2_RANDOM_STREAM:
                if(m_bFirstPage) {
                    nRet = decode_RandomOrgnazation_FirstPage(pPause);
                } else {
                    nRet = decode_RandomOrgnazation(pPause);
                }
                break;
            case JBIG2_EMBED_STREAM:
                nRet = decode_EmbedOrgnazation(pPause);
                break;
            default:
                m_ProcessiveStatus = FXCODEC_STATUS_ERROR;
                return JBIG2_ERROR_STREAM_TYPE;
        }
    } else if(m_PauseStep == 2) {
        nRet = decode_SquentialOrgnazation(pPause);
    } else if(m_PauseStep == 3) {
        nRet = decode_RandomOrgnazation_FirstPage(pPause);
    } else if(m_PauseStep == 4) {
        nRet = decode_RandomOrgnazation(pPause);
    } else if(m_PauseStep == 5) {
        m_ProcessiveStatus = FXCODEC_STATUS_DECODE_FINISH;
        return JBIG2_SUCCESS;
    }
    if(m_ProcessiveStatus == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
        return nRet;
    }
    m_PauseStep = 5;
    if(!m_bBufSpecified && nRet == JBIG2_SUCCESS) {
        m_ProcessiveStatus = FXCODEC_STATUS_DECODE_FINISH;
        return JBIG2_SUCCESS;
    }
    if(nRet == JBIG2_SUCCESS) {
        m_ProcessiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    } else {
        m_ProcessiveStatus = FXCODEC_STATUS_ERROR;
    }
    return nRet;
}
FX_INT32 CJBig2_Context::getNextPage(FX_BYTE *pBuf, FX_INT32 width, FX_INT32 height, FX_INT32 stride, IFX_Pause* pPause)
{
    FX_INT32 nRet = JBIG2_ERROR_STREAM_TYPE;
    m_bFirstPage = FALSE;
    m_PauseStep = 0;
    if(m_pPage) {
        delete m_pPage;
    }
    JBIG2_ALLOC(m_pPage, CJBig2_Image(width, height, stride, pBuf));
    m_bBufSpecified = TRUE;
    if(m_pPage && pPause && pPause->NeedToPauseNow()) {
        m_PauseStep = 1;
        m_ProcessiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
        return nRet;
    }
    return Continue(pPause);
    switch(m_nStreamType) {
        case JBIG2_FILE_STREAM:
            nRet = decodeFile(pPause);
            break;
        case JBIG2_SQUENTIAL_STREAM:
            nRet = decode_SquentialOrgnazation(pPause);
            break;
        case JBIG2_RANDOM_STREAM:
            nRet = decode_RandomOrgnazation(pPause);
            break;
        case JBIG2_EMBED_STREAM:
            nRet = decode_EmbedOrgnazation(pPause);
            break;
        default:
            return JBIG2_ERROR_STREAM_TYPE;
    }
    return nRet;
}
FX_INT32 CJBig2_Context::getFirstPage(CJBig2_Image **image, IFX_Pause* pPause)
{
    FX_INT32 nRet;
    m_bFirstPage = TRUE;
    m_PauseStep = 0;
    if(m_pGlobalContext) {
        nRet = m_pGlobalContext->decode_EmbedOrgnazation(pPause);
        if(nRet != JBIG2_SUCCESS) {
            return nRet;
        }
    }
    m_bBufSpecified = FALSE;
    return Continue(pPause);
}
FX_INT32 CJBig2_Context::getNextPage(CJBig2_Image **image, IFX_Pause* pPause)
{
    FX_INT32 nRet;
    m_bBufSpecified = FALSE;
    m_bFirstPage = FALSE;
    m_PauseStep = 0;
    switch(m_nStreamType) {
        case JBIG2_FILE_STREAM:
            nRet = decodeFile(pPause);
            break;
        case JBIG2_SQUENTIAL_STREAM:
            nRet = decode_SquentialOrgnazation(pPause);
            break;
        case JBIG2_RANDOM_STREAM:
            nRet = decode_RandomOrgnazation(pPause);
            break;
        case JBIG2_EMBED_STREAM:
            nRet = decode_EmbedOrgnazation(pPause);
            break;
        default:
            return JBIG2_ERROR_STREAM_TYPE;
    }
    if(nRet == JBIG2_SUCCESS) {
        *image = m_pPage;
        m_pPage = NULL;
        return JBIG2_SUCCESS;
    }
    return nRet;
}
CJBig2_Segment *CJBig2_Context::findSegmentByNumber(FX_DWORD dwNumber)
{
    CJBig2_Segment *pSeg;
    FX_INT32 i;
    if(m_pGlobalContext) {
        pSeg = m_pGlobalContext->findSegmentByNumber(dwNumber);
        if(pSeg) {
            return pSeg;
        }
    }
    for(i = 0; i < m_pSegmentList->getLength(); i++) {
        pSeg = m_pSegmentList->getAt(i);
        if(pSeg->m_dwNumber == dwNumber) {
            return pSeg;
        }
    }
    return NULL;
}
CJBig2_Segment *CJBig2_Context::findReferredSegmentByTypeAndIndex(CJBig2_Segment *pSegment,
        FX_BYTE cType, FX_INT32 nIndex)
{
    CJBig2_Segment *pSeg;
    FX_INT32 i, count;
    count = 0;
    for(i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
        pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i]);
        if(pSeg && pSeg->m_cFlags.s.type == cType) {
            if(count == nIndex) {
                return pSeg;
            } else {
                count ++;
            }
        }
    }
    return NULL;
}
FX_INT32 CJBig2_Context::parseSegmentHeader(CJBig2_Segment *pSegment)
{
    FX_BYTE  cSSize, cPSize;
    FX_BYTE cTemp;
    FX_WORD wTemp;
    FX_DWORD dwTemp;
    if((m_pStream->readInteger(&pSegment->m_dwNumber) != 0)
            || (m_pStream->read1Byte(&pSegment->m_cFlags.c) != 0)) {
        goto failed;
    }
    cTemp = m_pStream->getCurByte();
    if((cTemp >> 5) == 7) {
        if(m_pStream->readInteger((FX_DWORD*)&pSegment->m_nReferred_to_segment_count) != 0) {
            goto failed;
        }
        pSegment->m_nReferred_to_segment_count &= 0x1fffffff;
        if (pSegment->m_nReferred_to_segment_count > JBIG2_MAX_REFERRED_SEGMENT_COUNT) {
            m_pModule->JBig2_Error("Too many referred segments.");
            return JBIG2_ERROR_LIMIT;
        }
        dwTemp = 5 + 4 + (pSegment->m_nReferred_to_segment_count + 1) / 8;
    } else {
        if(m_pStream->read1Byte(&cTemp) != 0) {
            goto failed;
        }
        pSegment->m_nReferred_to_segment_count = cTemp >> 5;
        dwTemp = 5 + 1;
    }
    cSSize = pSegment->m_dwNumber > 65536 ? 4 : pSegment->m_dwNumber > 256 ? 2 : 1;
    cPSize = pSegment->m_cFlags.s.page_association_size ? 4 : 1;
    if(pSegment->m_nReferred_to_segment_count) {
        pSegment->m_pReferred_to_segment_numbers = (FX_DWORD*)m_pModule->JBig2_Malloc2(
                    sizeof(FX_DWORD), pSegment->m_nReferred_to_segment_count);
        for(FX_INT32 i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
            switch(cSSize) {
                case 1:
                    if(m_pStream->read1Byte(&cTemp) != 0) {
                        goto failed;
                    }
                    pSegment->m_pReferred_to_segment_numbers[i] = cTemp;
                    break;
                case 2:
                    if(m_pStream->readShortInteger(&wTemp) != 0) {
                        goto failed;
                    }
                    pSegment->m_pReferred_to_segment_numbers[i] = wTemp;
                    break;
                case 4:
                    if(m_pStream->readInteger(&dwTemp) != 0) {
                        goto failed;
                    }
                    pSegment->m_pReferred_to_segment_numbers[i] = dwTemp;
                    break;
            }
            if (pSegment->m_pReferred_to_segment_numbers[i] >= pSegment->m_dwNumber) {
                m_pModule->JBig2_Error("The referred segment number is greater than this segment number.");
                goto failed;
            }
        }
    }
    if(cPSize == 1) {
        if(m_pStream->read1Byte(&cTemp) != 0) {
            goto failed;
        }
        pSegment->m_dwPage_association = cTemp;
    } else {
        if(m_pStream->readInteger(&pSegment->m_dwPage_association) != 0) {
            goto failed;
        }
    }
    if(m_pStream->readInteger(&pSegment->m_dwData_length) != 0) {
        goto failed;
    }
    pSegment->m_pData = m_pStream->getPointer();
    pSegment->m_State = JBIG2_SEGMENT_DATA_UNPARSED;
    return JBIG2_SUCCESS;
failed:
    m_pModule->JBig2_Error("header too short.");
    return JBIG2_ERROR_TOO_SHORT;
}
FX_INT32 CJBig2_Context::parseSegmentData(CJBig2_Segment *pSegment, IFX_Pause* pPause)
{
    FX_INT32 ret = ProcessiveParseSegmentData(pSegment, pPause);
    while(m_ProcessiveStatus  == FXCODEC_STATUS_DECODE_TOBECONTINUE && m_pStream->getByteLeft() > 0) {
        ret = ProcessiveParseSegmentData(pSegment, pPause);
    }
    return ret;
}
FX_INT32 CJBig2_Context::ProcessiveParseSegmentData(CJBig2_Segment *pSegment, IFX_Pause* pPause)
{
    switch(pSegment->m_cFlags.s.type) {
        case 0:
            return parseSymbolDict(pSegment, pPause);
        case 4:
        case 6:
        case 7:
            if(m_nState == JBIG2_OUT_OF_PAGE) {
                goto failed2;
            } else {
                return parseTextRegion(pSegment);
            }
        case 16:
            return parsePatternDict(pSegment, pPause);
        case 20:
        case 22:
        case 23:
            if(m_nState == JBIG2_OUT_OF_PAGE) {
                goto failed2;
            } else {
                return parseHalftoneRegion(pSegment, pPause);
            }
        case 36:
        case 38:
        case 39:
            if(m_nState == JBIG2_OUT_OF_PAGE) {
                goto failed2;
            } else {
                return parseGenericRegion(pSegment, pPause);
            }
        case 40:
        case 42:
        case 43:
            if(m_nState == JBIG2_OUT_OF_PAGE) {
                goto failed2;
            } else {
                return parseGenericRefinementRegion(pSegment);
            }
        case 48: {
                FX_WORD wTemp;
                JBig2PageInfo *pPageInfo;
                JBIG2_ALLOC(pPageInfo, JBig2PageInfo);
                if((m_pStream->readInteger(&pPageInfo->m_dwWidth) != 0)
                        || (m_pStream->readInteger(&pPageInfo->m_dwHeight) != 0)
                        || (m_pStream->readInteger(&pPageInfo->m_dwResolutionX) != 0)
                        || (m_pStream->readInteger(&pPageInfo->m_dwResolutionY) != 0)
                        || (m_pStream->read1Byte(&pPageInfo->m_cFlags) != 0)
                        || (m_pStream->readShortInteger(&wTemp) != 0)) {
                    delete pPageInfo;
                    goto failed1;
                }
                pPageInfo->m_bIsStriped = ((wTemp >> 15) & 1) ? 1 : 0;
                pPageInfo->m_wMaxStripeSize = wTemp & 0x7fff;
                if((pPageInfo->m_dwHeight == 0xffffffff) && (pPageInfo->m_bIsStriped != 1)) {
                    m_pModule->JBig2_Warn("page height = 0xffffffff buf stripe field is 0");
                    pPageInfo->m_bIsStriped = 1;
                }
                if(!m_bBufSpecified) {
                    if(m_pPage) {
                        delete m_pPage;
                    }
                    if(pPageInfo->m_dwHeight == 0xffffffff) {
                        JBIG2_ALLOC(m_pPage, CJBig2_Image(pPageInfo->m_dwWidth, pPageInfo->m_wMaxStripeSize));
                    } else {
                        JBIG2_ALLOC(m_pPage, CJBig2_Image(pPageInfo->m_dwWidth, pPageInfo->m_dwHeight));
                    }
                }
                m_pPage->fill((pPageInfo->m_cFlags & 4) ? 1 : 0);
                m_pPageInfoList->addItem(pPageInfo);
                m_nState = JBIG2_IN_PAGE;
            }
            break;
        case 49:
            m_nState = JBIG2_OUT_OF_PAGE;
            return JBIG2_END_OF_PAGE;
            break;
        case 50:
            m_pStream->offset(pSegment->m_dwData_length);
            break;
        case 51:
            return JBIG2_END_OF_FILE;
        case 52:
            m_pStream->offset(pSegment->m_dwData_length);
            break;
        case 53:
            return parseTable(pSegment);
        case 62:
            m_pStream->offset(pSegment->m_dwData_length);
            break;
        default:
            break;
    }
    return JBIG2_SUCCESS;
failed1:
    m_pModule->JBig2_Error("segment data too short.");
    return JBIG2_ERROR_TOO_SHORT;
failed2:
    m_pModule->JBig2_Error("segment syntax error.");
    return JBIG2_ERROR_FETAL;
}
FX_INT32 CJBig2_Context::parseSymbolDict(CJBig2_Segment *pSegment, IFX_Pause* pPause)
{
    FX_DWORD dwTemp;
    FX_WORD wFlags;
    FX_BYTE cSDHUFFDH, cSDHUFFDW, cSDHUFFBMSIZE, cSDHUFFAGGINST;
    CJBig2_HuffmanTable *Table_B1 = NULL, *Table_B2 = NULL, *Table_B3 = NULL, *Table_B4 = NULL, *Table_B5 = NULL;
    FX_INT32 i, nIndex, nRet;
    CJBig2_Segment *pSeg = NULL, *pLRSeg = NULL;
    FX_BOOL bUsed;
    CJBig2_Image ** SDINSYMS = NULL;
    CJBig2_SDDProc *pSymbolDictDecoder;
    JBig2ArithCtx *gbContext = NULL, *grContext = NULL;
    CJBig2_ArithDecoder *pArithDecoder;
    JBIG2_ALLOC(pSymbolDictDecoder, CJBig2_SDDProc());
    FX_BYTE *key = pSegment->m_pData;
    FX_BOOL cache_hit = false;
    if(m_pStream->readShortInteger(&wFlags) != 0) {
        m_pModule->JBig2_Error("symbol dictionary segment : data header too short.");
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
    }
    pSymbolDictDecoder->SDHUFF = wFlags & 0x0001;
    pSymbolDictDecoder->SDREFAGG = (wFlags >> 1) & 0x0001;
    pSymbolDictDecoder->SDTEMPLATE = (wFlags >> 10) & 0x0003;
    pSymbolDictDecoder->SDRTEMPLATE = (wFlags >> 12) & 0x0003;
    cSDHUFFDH = (wFlags >> 2) & 0x0003;
    cSDHUFFDW = (wFlags >> 4) & 0x0003;
    cSDHUFFBMSIZE = (wFlags >> 6) & 0x0001;
    cSDHUFFAGGINST = (wFlags >> 7) & 0x0001;
    if(pSymbolDictDecoder->SDHUFF == 0) {
        if(pSymbolDictDecoder->SDTEMPLATE == 0) {
            dwTemp = 8;
        } else {
            dwTemp = 2;
        }
        for(i = 0; i < (FX_INT32)dwTemp; i++) {
            if(m_pStream->read1Byte((FX_BYTE*)&pSymbolDictDecoder->SDAT[i]) != 0) {
                m_pModule->JBig2_Error("symbol dictionary segment : data header too short.");
                nRet = JBIG2_ERROR_TOO_SHORT;
                goto failed;
            }
        }
    }
    if((pSymbolDictDecoder->SDREFAGG == 1) && (pSymbolDictDecoder->SDRTEMPLATE == 0)) {
        for(i = 0; i < 4; i++) {
            if(m_pStream->read1Byte((FX_BYTE*)&pSymbolDictDecoder->SDRAT[i]) != 0) {
                m_pModule->JBig2_Error("symbol dictionary segment : data header too short.");
                nRet = JBIG2_ERROR_TOO_SHORT;
                goto failed;
            }
        }
    }
    if((m_pStream->readInteger(&pSymbolDictDecoder->SDNUMEXSYMS) != 0)
            || (m_pStream->readInteger(&pSymbolDictDecoder->SDNUMNEWSYMS) != 0)) {
        m_pModule->JBig2_Error("symbol dictionary segment : data header too short.");
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
    }
    if (pSymbolDictDecoder->SDNUMEXSYMS > JBIG2_MAX_EXPORT_SYSMBOLS
            || pSymbolDictDecoder->SDNUMNEWSYMS > JBIG2_MAX_NEW_SYSMBOLS) {
        m_pModule->JBig2_Error("symbol dictionary segment : too many export/new symbols.");
        nRet = JBIG2_ERROR_LIMIT;
        goto failed;
    }
    for(i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
        if(!findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i])) {
            m_pModule->JBig2_Error("symbol dictionary segment : can't find refered to segments");
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
    }
    pSymbolDictDecoder->SDNUMINSYMS = 0;
    for(i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
        pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i]);
        if(pSeg->m_cFlags.s.type == 0) {
            pSymbolDictDecoder->SDNUMINSYMS += pSeg->m_Result.sd->SDNUMEXSYMS;
            pLRSeg = pSeg;
        }
    }
    if(pSymbolDictDecoder->SDNUMINSYMS == 0) {
        SDINSYMS = NULL;
    } else {
        SDINSYMS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(
                       sizeof(CJBig2_Image*), pSymbolDictDecoder->SDNUMINSYMS);
        dwTemp = 0;
        for(i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
            pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i]);
            if(pSeg->m_cFlags.s.type == 0) {
                JBIG2_memcpy(SDINSYMS + dwTemp, pSeg->m_Result.sd->SDEXSYMS,
                             pSeg->m_Result.sd->SDNUMEXSYMS * sizeof(CJBig2_Image*));
                dwTemp += pSeg->m_Result.sd->SDNUMEXSYMS;
            }
        }
    }
    pSymbolDictDecoder->SDINSYMS = SDINSYMS;
    if(pSymbolDictDecoder->SDHUFF == 1) {
        if((cSDHUFFDH == 2) || (cSDHUFFDW == 2)) {
            m_pModule->JBig2_Error("symbol dictionary segment : SDHUFFDH=2 or SDHUFFDW=2 is not permitted.");
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
        nIndex = 0;
        if(cSDHUFFDH == 0) {
            JBIG2_ALLOC(Table_B4, CJBig2_HuffmanTable(HuffmanTable_B4,
                        sizeof(HuffmanTable_B4) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B4));
            pSymbolDictDecoder->SDHUFFDH = Table_B4;
        } else if(cSDHUFFDH == 1) {
            JBIG2_ALLOC(Table_B5, CJBig2_HuffmanTable(HuffmanTable_B5,
                        sizeof(HuffmanTable_B5) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B5));
            pSymbolDictDecoder->SDHUFFDH = Table_B5;
        } else {
            pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
            if(!pSeg) {
                m_pModule->JBig2_Error("symbol dictionary segment : SDHUFFDH can't find user supplied table.");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            pSymbolDictDecoder->SDHUFFDH = pSeg->m_Result.ht;
        }
        if(cSDHUFFDW == 0) {
            JBIG2_ALLOC(Table_B2, CJBig2_HuffmanTable(HuffmanTable_B2,
                        sizeof(HuffmanTable_B2) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B2));
            pSymbolDictDecoder->SDHUFFDW = Table_B2;
        } else if(cSDHUFFDW == 1) {
            JBIG2_ALLOC(Table_B3, CJBig2_HuffmanTable(HuffmanTable_B3,
                        sizeof(HuffmanTable_B3) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B3));
            pSymbolDictDecoder->SDHUFFDW = Table_B3;
        } else {
            pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
            if(!pSeg) {
                m_pModule->JBig2_Error("symbol dictionary segment : SDHUFFDW can't find user supplied table.");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            pSymbolDictDecoder->SDHUFFDW = pSeg->m_Result.ht;
        }
        if(cSDHUFFBMSIZE == 0) {
            JBIG2_ALLOC(Table_B1, CJBig2_HuffmanTable(HuffmanTable_B1,
                        sizeof(HuffmanTable_B1) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B1));
            pSymbolDictDecoder->SDHUFFBMSIZE = Table_B1;
        } else {
            pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
            if(!pSeg) {
                m_pModule->JBig2_Error("symbol dictionary segment : SDHUFFBMSIZE can't find user supplied table.");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            pSymbolDictDecoder->SDHUFFBMSIZE = pSeg->m_Result.ht;
        }
        if(pSymbolDictDecoder->SDREFAGG == 1) {
            if(cSDHUFFAGGINST == 0) {
                if(!Table_B1) {
                    JBIG2_ALLOC(Table_B1, CJBig2_HuffmanTable(HuffmanTable_B1,
                                sizeof(HuffmanTable_B1) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B1));
                }
                pSymbolDictDecoder->SDHUFFAGGINST = Table_B1;
            } else {
                pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
                if(!pSeg) {
                    m_pModule->JBig2_Error("symbol dictionary segment : SDHUFFAGGINST can't find user supplied table.");
                    nRet = JBIG2_ERROR_FETAL;
                    goto failed;
                }
                pSymbolDictDecoder->SDHUFFAGGINST = pSeg->m_Result.ht;
            }
        }
    }
    if((wFlags & 0x0100) && pLRSeg && pLRSeg->m_Result.sd->m_bContextRetained) {
        if (pSymbolDictDecoder->SDHUFF == 0) {
            dwTemp = pSymbolDictDecoder->SDTEMPLATE == 0 ? 65536 : pSymbolDictDecoder->SDTEMPLATE == 1 ?
                     8192 : 1024;
            gbContext = (JBig2ArithCtx*)m_pModule->JBig2_Malloc2(sizeof(JBig2ArithCtx), dwTemp);
            JBIG2_memcpy(gbContext, pLRSeg->m_Result.sd->m_gbContext, sizeof(JBig2ArithCtx)*dwTemp);
        }
        if (pSymbolDictDecoder->SDREFAGG == 1) {
            dwTemp = pSymbolDictDecoder->SDRTEMPLATE ? 1 << 10 : 1 << 13;
            grContext = (JBig2ArithCtx*)m_pModule->JBig2_Malloc2(sizeof(JBig2ArithCtx), dwTemp);
            JBIG2_memcpy(grContext, pLRSeg->m_Result.sd->m_grContext, sizeof(JBig2ArithCtx)*dwTemp);
        }
    } else {
        if (pSymbolDictDecoder->SDHUFF == 0) {
            dwTemp = pSymbolDictDecoder->SDTEMPLATE == 0 ? 65536 : pSymbolDictDecoder->SDTEMPLATE == 1 ?
                     8192 : 1024;
            gbContext = (JBig2ArithCtx*)m_pModule->JBig2_Malloc2(sizeof(JBig2ArithCtx), dwTemp);
            JBIG2_memset(gbContext, 0, sizeof(JBig2ArithCtx)*dwTemp);
        }
        if (pSymbolDictDecoder->SDREFAGG == 1) {
            dwTemp = pSymbolDictDecoder->SDRTEMPLATE ? 1 << 10 : 1 << 13;
            grContext = (JBig2ArithCtx*)m_pModule->JBig2_Malloc2(sizeof(JBig2ArithCtx), dwTemp);
            JBIG2_memset(grContext, 0, sizeof(JBig2ArithCtx)*dwTemp);
        }
    }
    pSegment->m_nResultType = JBIG2_SYMBOL_DICT_POINTER;
    for(std::list<CJBig2_CachePair>::iterator it =
            m_pSymbolDictCache->begin(); it != m_pSymbolDictCache->end(); ++it) {
        if (it->first == key) {
            pSegment->m_Result.sd = it->second->DeepCopy();
            m_pSymbolDictCache->push_front(*it);
            m_pSymbolDictCache->erase(it);
            cache_hit = true;
            break;
        }
    }
    if (!cache_hit) {
        if(pSymbolDictDecoder->SDHUFF == 0) {
            JBIG2_ALLOC(pArithDecoder, CJBig2_ArithDecoder(m_pStream));
            pSegment->m_Result.sd = pSymbolDictDecoder->decode_Arith(pArithDecoder, gbContext, grContext);
            delete pArithDecoder;
            if(pSegment->m_Result.sd == NULL) {
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            m_pStream->alignByte();
            m_pStream->offset(2);
        } else {
            pSegment->m_Result.sd = pSymbolDictDecoder->decode_Huffman(m_pStream, gbContext, grContext, pPause);
            if(pSegment->m_Result.sd == NULL) {
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            m_pStream->alignByte();
        }
        CJBig2_SymbolDict *value = pSegment->m_Result.sd->DeepCopy();
        if (value && kSymbolDictCacheMaxSize > 0) {
            while (m_pSymbolDictCache->size() >= kSymbolDictCacheMaxSize) {
                delete m_pSymbolDictCache->back().second;
                m_pSymbolDictCache->pop_back();
            }
            m_pSymbolDictCache->push_front(CJBig2_CachePair(key, value));
        }
    }
    if(wFlags & 0x0200) {
        pSegment->m_Result.sd->m_bContextRetained = TRUE;
        if(pSymbolDictDecoder->SDHUFF == 0) {
            pSegment->m_Result.sd->m_gbContext = gbContext;
        }
        if(pSymbolDictDecoder->SDREFAGG == 1) {
            pSegment->m_Result.sd->m_grContext = grContext;
        }
        bUsed = TRUE;
    } else {
        bUsed = FALSE;
    }
    delete pSymbolDictDecoder;
    if(SDINSYMS) {
        m_pModule->JBig2_Free(SDINSYMS);
    }
    if(Table_B1) {
        delete Table_B1;
    }
    if(Table_B2) {
        delete Table_B2;
    }
    if(Table_B3) {
        delete Table_B3;
    }
    if(Table_B4) {
        delete Table_B4;
    }
    if(Table_B5) {
        delete Table_B5;
    }
    if(bUsed == FALSE) {
        if(gbContext) {
            m_pModule->JBig2_Free(gbContext);
        }
        if(grContext) {
            m_pModule->JBig2_Free(grContext);
        }
    }
    return JBIG2_SUCCESS;
failed:
    delete pSymbolDictDecoder;
    if(SDINSYMS) {
        m_pModule->JBig2_Free(SDINSYMS);
    }
    if(Table_B1) {
        delete Table_B1;
    }
    if(Table_B2) {
        delete Table_B2;
    }
    if(Table_B3) {
        delete Table_B3;
    }
    if(Table_B4) {
        delete Table_B4;
    }
    if(Table_B5) {
        delete Table_B5;
    }
    if(gbContext) {
        m_pModule->JBig2_Free(gbContext);
    }
    if(grContext) {
        m_pModule->JBig2_Free(grContext);
    }
    return nRet;
}

FX_BOOL CJBig2_Context::parseTextRegion(CJBig2_Segment *pSegment)
{
    FX_DWORD dwTemp;
    FX_WORD wFlags;
    FX_INT32 i, nIndex, nRet;
    JBig2RegionInfo ri;
    CJBig2_Segment *pSeg;
    CJBig2_Image **SBSYMS = NULL;
    JBig2HuffmanCode *SBSYMCODES = NULL;
    FX_BYTE cSBHUFFFS, cSBHUFFDS, cSBHUFFDT, cSBHUFFRDW, cSBHUFFRDH, cSBHUFFRDX, cSBHUFFRDY, cSBHUFFRSIZE;
    CJBig2_HuffmanTable *Table_B1 = NULL,
                         *Table_B6 = NULL,
                          *Table_B7 = NULL,
                           *Table_B8 = NULL,
                            *Table_B9 = NULL,
                             *Table_B10 = NULL,
                              *Table_B11 = NULL,
                               *Table_B12 = NULL,
                                *Table_B13 = NULL,
                                 *Table_B14 = NULL,
                                  *Table_B15 = NULL;
    JBig2ArithCtx *grContext = NULL;
    CJBig2_ArithDecoder *pArithDecoder;
    CJBig2_TRDProc *pTRD;
    JBIG2_ALLOC(pTRD, CJBig2_TRDProc());
    if((parseRegionInfo(&ri) != JBIG2_SUCCESS)
            || (m_pStream->readShortInteger(&wFlags) != 0)) {
        m_pModule->JBig2_Error("text region segment : data header too short.");
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
    }
    pTRD->SBW = ri.width;
    pTRD->SBH = ri.height;
    pTRD->SBHUFF = wFlags & 0x0001;
    pTRD->SBREFINE = (wFlags >> 1) & 0x0001;
    dwTemp = (wFlags >> 2) & 0x0003;
    pTRD->SBSTRIPS = 1 << dwTemp;
    pTRD->REFCORNER = (JBig2Corner)((wFlags >> 4) & 0x0003);
    pTRD->TRANSPOSED = (wFlags >> 6) & 0x0001;
    pTRD->SBCOMBOP = (JBig2ComposeOp)((wFlags >> 7) & 0x0003);
    pTRD->SBDEFPIXEL = (wFlags >> 9) & 0x0001;
    pTRD->SBDSOFFSET = (wFlags >> 10) & 0x001f;
    if(pTRD->SBDSOFFSET >= 0x0010) {
        pTRD->SBDSOFFSET = pTRD->SBDSOFFSET - 0x0020;
    }
    pTRD->SBRTEMPLATE = (wFlags >> 15) & 0x0001;
    if(pTRD->SBHUFF == 1) {
        if(m_pStream->readShortInteger(&wFlags) != 0) {
            m_pModule->JBig2_Error("text region segment : data header too short.");
            nRet = JBIG2_ERROR_TOO_SHORT;
            goto failed;
        }
        cSBHUFFFS = wFlags & 0x0003;
        cSBHUFFDS = (wFlags >> 2) & 0x0003;
        cSBHUFFDT = (wFlags >> 4) & 0x0003;
        cSBHUFFRDW = (wFlags >> 6) & 0x0003;
        cSBHUFFRDH = (wFlags >> 8) & 0x0003;
        cSBHUFFRDX = (wFlags >> 10) & 0x0003;
        cSBHUFFRDY = (wFlags >> 12) & 0x0003;
        cSBHUFFRSIZE = (wFlags >> 14) & 0x0001;
    }
    if((pTRD->SBREFINE == 1) && (pTRD->SBRTEMPLATE == 0)) {
        for(i = 0; i < 4; i++) {
            if(m_pStream->read1Byte((FX_BYTE*)&pTRD->SBRAT[i]) != 0) {
                m_pModule->JBig2_Error("text region segment : data header too short.");
                nRet = JBIG2_ERROR_TOO_SHORT;
                goto failed;
            }
        }
    }
    if(m_pStream->readInteger(&pTRD->SBNUMINSTANCES) != 0) {
        m_pModule->JBig2_Error("text region segment : data header too short.");
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
    }
    for(i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
        if(!findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i])) {
            m_pModule->JBig2_Error("text region segment : can't find refered to segments");
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
    }
    pTRD->SBNUMSYMS = 0;
    for(i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
        pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i]);
        if(pSeg->m_cFlags.s.type == 0) {
            pTRD->SBNUMSYMS += pSeg->m_Result.sd->SDNUMEXSYMS;
        }
    }
    if (pTRD->SBNUMSYMS > 0) {
        SBSYMS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(
                     sizeof(CJBig2_Image*), pTRD->SBNUMSYMS);
        dwTemp = 0;
        for(i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
            pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i]);
            if(pSeg->m_cFlags.s.type == 0) {
                JBIG2_memcpy(SBSYMS + dwTemp, pSeg->m_Result.sd->SDEXSYMS,
                             pSeg->m_Result.sd->SDNUMEXSYMS * sizeof(CJBig2_Image*));
                dwTemp += pSeg->m_Result.sd->SDNUMEXSYMS;
            }
        }
        pTRD->SBSYMS = SBSYMS;
    } else {
        pTRD->SBSYMS = NULL;
    }
    if(pTRD->SBHUFF == 1) {
        SBSYMCODES = decodeSymbolIDHuffmanTable(m_pStream, pTRD->SBNUMSYMS);
        if(SBSYMCODES == NULL) {
            m_pModule->JBig2_Error("text region segment: symbol ID huffman table decode failure!");
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
        m_pStream->alignByte();
        pTRD->SBSYMCODES = SBSYMCODES;
    } else {
        dwTemp = 0;
        while((FX_DWORD)(1 << dwTemp) < pTRD->SBNUMSYMS) {
            dwTemp ++;
        }
        pTRD->SBSYMCODELEN = (FX_BYTE)dwTemp;
    }
    if(pTRD->SBHUFF == 1) {
        if((cSBHUFFFS == 2) || (cSBHUFFRDW == 2) || (cSBHUFFRDH == 2)
                || (cSBHUFFRDX == 2) || (cSBHUFFRDY == 2)) {
            m_pModule->JBig2_Error("text region segment : SBHUFFFS=2 or SBHUFFRDW=2 or "
                                   "SBHUFFRDH=2 or SBHUFFRDX=2 or SBHUFFRDY=2 is not permitted");
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
        nIndex = 0;
        if(cSBHUFFFS == 0) {
            JBIG2_ALLOC(Table_B6, CJBig2_HuffmanTable(HuffmanTable_B6,
                        sizeof(HuffmanTable_B6) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B6));
            pTRD->SBHUFFFS = Table_B6;
        } else if(cSBHUFFFS == 1) {
            JBIG2_ALLOC(Table_B7, CJBig2_HuffmanTable(HuffmanTable_B7,
                        sizeof(HuffmanTable_B7) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B7));
            pTRD->SBHUFFFS = Table_B7;
        } else {
            pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
            if(!pSeg) {
                m_pModule->JBig2_Error("text region segment : SBHUFFFS can't find user supplied table");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            pTRD->SBHUFFFS = pSeg->m_Result.ht;
        }
        if(cSBHUFFDS == 0) {
            JBIG2_ALLOC(Table_B8, CJBig2_HuffmanTable(HuffmanTable_B8,
                        sizeof(HuffmanTable_B8) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B8));
            pTRD->SBHUFFDS = Table_B8;
        } else if(cSBHUFFDS == 1) {
            JBIG2_ALLOC(Table_B9, CJBig2_HuffmanTable(HuffmanTable_B9,
                        sizeof(HuffmanTable_B9) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B9));
            pTRD->SBHUFFDS = Table_B9;
        } else if(cSBHUFFDS == 2) {
            JBIG2_ALLOC(Table_B10, CJBig2_HuffmanTable(HuffmanTable_B10,
                        sizeof(HuffmanTable_B10) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B10));
            pTRD->SBHUFFDS = Table_B10;
        } else {
            pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
            if(!pSeg) {
                m_pModule->JBig2_Error("text region segment : SBHUFFDS can't find user supplied table");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            pTRD->SBHUFFDS = pSeg->m_Result.ht;
        }
        if(cSBHUFFDT == 0) {
            JBIG2_ALLOC(Table_B11, CJBig2_HuffmanTable(HuffmanTable_B11,
                        sizeof(HuffmanTable_B11) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B11));
            pTRD->SBHUFFDT = Table_B11;
        } else if(cSBHUFFDT == 1) {
            JBIG2_ALLOC(Table_B12, CJBig2_HuffmanTable(HuffmanTable_B12,
                        sizeof(HuffmanTable_B12) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B12));
            pTRD->SBHUFFDT = Table_B12;
        } else if(cSBHUFFDT == 2) {
            JBIG2_ALLOC(Table_B13, CJBig2_HuffmanTable(HuffmanTable_B13,
                        sizeof(HuffmanTable_B13) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B13));
            pTRD->SBHUFFDT = Table_B13;
        } else {
            pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
            if(!pSeg) {
                m_pModule->JBig2_Error("text region segment : SBHUFFDT can't find user supplied table");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            pTRD->SBHUFFDT = pSeg->m_Result.ht;
        }
        if(cSBHUFFRDW == 0) {
            JBIG2_ALLOC(Table_B14, CJBig2_HuffmanTable(HuffmanTable_B14,
                        sizeof(HuffmanTable_B14) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B14));
            pTRD->SBHUFFRDW = Table_B14;
        } else if(cSBHUFFRDW == 1) {
            JBIG2_ALLOC(Table_B15, CJBig2_HuffmanTable(HuffmanTable_B15,
                        sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
            pTRD->SBHUFFRDW = Table_B15;
        } else {
            pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
            if(!pSeg) {
                m_pModule->JBig2_Error("text region segment : SBHUFFRDW can't find user supplied table");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            pTRD->SBHUFFRDW = pSeg->m_Result.ht;
        }
        if(cSBHUFFRDH == 0) {
            if(!Table_B14) {
                JBIG2_ALLOC(Table_B14, CJBig2_HuffmanTable(HuffmanTable_B14,
                            sizeof(HuffmanTable_B14) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B14));
            }
            pTRD->SBHUFFRDH = Table_B14;
        } else if(cSBHUFFRDH == 1) {
            if(!Table_B15) {
                JBIG2_ALLOC(Table_B15, CJBig2_HuffmanTable(HuffmanTable_B15,
                            sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
            }
            pTRD->SBHUFFRDH = Table_B15;
        } else {
            pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
            if(!pSeg) {
                m_pModule->JBig2_Error("text region segment : SBHUFFRDH can't find user supplied table");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            pTRD->SBHUFFRDH = pSeg->m_Result.ht;
        }
        if(cSBHUFFRDX == 0) {
            if(!Table_B14) {
                JBIG2_ALLOC(Table_B14, CJBig2_HuffmanTable(HuffmanTable_B14,
                            sizeof(HuffmanTable_B14) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B14));
            }
            pTRD->SBHUFFRDX = Table_B14;
        } else if(cSBHUFFRDX == 1) {
            if(!Table_B15) {
                JBIG2_ALLOC(Table_B15, CJBig2_HuffmanTable(HuffmanTable_B15,
                            sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
            }
            pTRD->SBHUFFRDX = Table_B15;
        } else {
            pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
            if(!pSeg) {
                m_pModule->JBig2_Error("text region segment : SBHUFFRDX can't find user supplied table");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            pTRD->SBHUFFRDX = pSeg->m_Result.ht;
        }
        if(cSBHUFFRDY == 0) {
            if(!Table_B14) {
                JBIG2_ALLOC(Table_B14, CJBig2_HuffmanTable(HuffmanTable_B14,
                            sizeof(HuffmanTable_B14) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B14));
            }
            pTRD->SBHUFFRDY = Table_B14;
        } else if(cSBHUFFRDY == 1) {
            if(!Table_B15) {
                JBIG2_ALLOC(Table_B15, CJBig2_HuffmanTable(HuffmanTable_B15,
                            sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
            }
            pTRD->SBHUFFRDY = Table_B15;
        } else {
            pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
            if(!pSeg) {
                m_pModule->JBig2_Error("text region segment : SBHUFFRDY can't find user supplied table");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            pTRD->SBHUFFRDY = pSeg->m_Result.ht;
        }
        if(cSBHUFFRSIZE == 0) {
            JBIG2_ALLOC(Table_B1, CJBig2_HuffmanTable(HuffmanTable_B1,
                        sizeof(HuffmanTable_B1) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B1));
            pTRD->SBHUFFRSIZE = Table_B1;
        } else {
            pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
            if(!pSeg) {
                m_pModule->JBig2_Error("text region segment : SBHUFFRSIZE can't find user supplied table");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            pTRD->SBHUFFRSIZE = pSeg->m_Result.ht;
        }
    }
    if(pTRD->SBREFINE == 1) {
        dwTemp = pTRD->SBRTEMPLATE ? 1 << 10 : 1 << 13;
        grContext = (JBig2ArithCtx*)m_pModule->JBig2_Malloc2(sizeof(JBig2ArithCtx), dwTemp);
        JBIG2_memset(grContext, 0, sizeof(JBig2ArithCtx)*dwTemp);
    }
    if(pTRD->SBHUFF == 0) {
        JBIG2_ALLOC(pArithDecoder, CJBig2_ArithDecoder(m_pStream));
        pSegment->m_nResultType = JBIG2_IMAGE_POINTER;
        pSegment->m_Result.im = pTRD->decode_Arith(pArithDecoder, grContext);
        delete pArithDecoder;
        if(pSegment->m_Result.im == NULL) {
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
        m_pStream->alignByte();
        m_pStream->offset(2);
    } else {
        pSegment->m_nResultType = JBIG2_IMAGE_POINTER;
        pSegment->m_Result.im = pTRD->decode_Huffman(m_pStream, grContext);
        if(pSegment->m_Result.im == NULL) {
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
        m_pStream->alignByte();
    }
    if(pSegment->m_cFlags.s.type != 4) {
        if(!m_bBufSpecified) {
            JBig2PageInfo *pPageInfo = m_pPageInfoList->getLast();
            if ((pPageInfo->m_bIsStriped == 1) && (ri.y + ri.height > m_pPage->m_nHeight)) {
                m_pPage->expand(ri.y + ri.height, (pPageInfo->m_cFlags & 4) ? 1 : 0);
            }
        }
        m_pPage->composeFrom(ri.x, ri.y, pSegment->m_Result.im, (JBig2ComposeOp)(ri.flags & 0x03));
        delete pSegment->m_Result.im;
        pSegment->m_Result.im = NULL;
    }
    delete pTRD;
    if(SBSYMS) {
        m_pModule->JBig2_Free(SBSYMS);
    }
    if(SBSYMCODES) {
        m_pModule->JBig2_Free(SBSYMCODES);
    }
    if(grContext) {
        m_pModule->JBig2_Free(grContext);
    }
    if(Table_B1) {
        delete Table_B1;
    }
    if(Table_B6) {
        delete Table_B6;
    }
    if(Table_B7) {
        delete Table_B7;
    }
    if(Table_B8) {
        delete Table_B8;
    }
    if(Table_B9) {
        delete Table_B9;
    }
    if(Table_B10) {
        delete Table_B10;
    }
    if(Table_B11) {
        delete Table_B11;
    }
    if(Table_B12) {
        delete Table_B12;
    }
    if(Table_B13) {
        delete Table_B13;
    }
    if(Table_B14) {
        delete Table_B14;
    }
    if(Table_B15) {
        delete Table_B15;
    }
    return JBIG2_SUCCESS;
failed:
    delete pTRD;
    if(SBSYMS) {
        m_pModule->JBig2_Free(SBSYMS);
    }
    if(SBSYMCODES) {
        m_pModule->JBig2_Free(SBSYMCODES);
    }
    if(grContext) {
        m_pModule->JBig2_Free(grContext);
    }
    if(Table_B1) {
        delete Table_B1;
    }
    if(Table_B6) {
        delete Table_B6;
    }
    if(Table_B7) {
        delete Table_B7;
    }
    if(Table_B8) {
        delete Table_B8;
    }
    if(Table_B9) {
        delete Table_B9;
    }
    if(Table_B10) {
        delete Table_B10;
    }
    if(Table_B11) {
        delete Table_B11;
    }
    if(Table_B12) {
        delete Table_B12;
    }
    if(Table_B13) {
        delete Table_B13;
    }
    if(Table_B14) {
        delete Table_B14;
    }
    if(Table_B15) {
        delete Table_B15;
    }
    return nRet;
}

FX_BOOL CJBig2_Context::parsePatternDict(CJBig2_Segment *pSegment, IFX_Pause* pPause)
{
    FX_DWORD dwTemp;
    FX_BYTE cFlags;
    JBig2ArithCtx *gbContext;
    CJBig2_ArithDecoder *pArithDecoder;
    CJBig2_PDDProc *pPDD;
    FX_INT32 nRet;
    JBIG2_ALLOC(pPDD, CJBig2_PDDProc());
    if((m_pStream->read1Byte(&cFlags) != 0)
            || (m_pStream->read1Byte(&pPDD->HDPW) != 0)
            || (m_pStream->read1Byte(&pPDD->HDPH) != 0)
            || (m_pStream->readInteger(&pPDD->GRAYMAX) != 0)) {
        m_pModule->JBig2_Error("pattern dictionary segment : data header too short.");
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
    }
    if (pPDD->GRAYMAX > JBIG2_MAX_PATTERN_INDEX) {
        m_pModule->JBig2_Error("pattern dictionary segment : too max gray max.");
        nRet = JBIG2_ERROR_LIMIT;
        goto failed;
    }
    pPDD->HDMMR = cFlags & 0x01;
    pPDD->HDTEMPLATE = (cFlags >> 1) & 0x03;
    pSegment->m_nResultType = JBIG2_PATTERN_DICT_POINTER;
    if(pPDD->HDMMR == 0) {
        dwTemp = pPDD->HDTEMPLATE == 0 ? 65536 : pPDD->HDTEMPLATE == 1 ? 8192 : 1024;
        gbContext = (JBig2ArithCtx*)m_pModule->JBig2_Malloc2(sizeof(JBig2ArithCtx), dwTemp);
        JBIG2_memset(gbContext, 0, sizeof(JBig2ArithCtx)*dwTemp);
        JBIG2_ALLOC(pArithDecoder, CJBig2_ArithDecoder(m_pStream));
        pSegment->m_Result.pd = pPDD->decode_Arith(pArithDecoder, gbContext, pPause);
        delete pArithDecoder;
        if(pSegment->m_Result.pd == NULL) {
            m_pModule->JBig2_Free(gbContext);
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
        m_pModule->JBig2_Free(gbContext);
        m_pStream->alignByte();
        m_pStream->offset(2);
    } else {
        pSegment->m_Result.pd = pPDD->decode_MMR(m_pStream, pPause);
        if(pSegment->m_Result.pd == NULL) {
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
        m_pStream->alignByte();
    }
    delete pPDD;
    return JBIG2_SUCCESS;
failed:
    delete pPDD;
    return nRet;
}
FX_BOOL CJBig2_Context::parseHalftoneRegion(CJBig2_Segment *pSegment, IFX_Pause* pPause)
{
    FX_DWORD dwTemp;
    FX_BYTE cFlags;
    JBig2RegionInfo ri;
    CJBig2_Segment *pSeg;
    CJBig2_PatternDict *pPatternDict;
    JBig2ArithCtx *gbContext;
    CJBig2_ArithDecoder *pArithDecoder;
    CJBig2_HTRDProc *pHRD;
    FX_INT32 nRet;
    JBIG2_ALLOC(pHRD, CJBig2_HTRDProc());
    if((parseRegionInfo(&ri) != JBIG2_SUCCESS)
            || (m_pStream->read1Byte(&cFlags) != 0)
            || (m_pStream->readInteger(&pHRD->HGW) != 0)
            || (m_pStream->readInteger(&pHRD->HGH) != 0)
            || (m_pStream->readInteger((FX_DWORD*)&pHRD->HGX) != 0)
            || (m_pStream->readInteger((FX_DWORD*)&pHRD->HGY) != 0)
            || (m_pStream->readShortInteger(&pHRD->HRX) != 0)
            || (m_pStream->readShortInteger(&pHRD->HRY) != 0)) {
        m_pModule->JBig2_Error("halftone region segment : data header too short.");
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
    }
    pHRD->HBW = ri.width;
    pHRD->HBH = ri.height;
    pHRD->HMMR = cFlags & 0x01;
    pHRD->HTEMPLATE = (cFlags >> 1) & 0x03;
    pHRD->HENABLESKIP = (cFlags >> 3) & 0x01;
    pHRD->HCOMBOP = (JBig2ComposeOp)((cFlags >> 4) & 0x07);
    pHRD->HDEFPIXEL = (cFlags >> 7) & 0x01;
    if(pSegment->m_nReferred_to_segment_count != 1) {
        m_pModule->JBig2_Error("halftone region segment : refered to segment count not equals 1");
        nRet = JBIG2_ERROR_FETAL;
        goto failed;
    }
    pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[0]);
    if( (pSeg == NULL) || (pSeg->m_cFlags.s.type != 16)) {
        m_pModule->JBig2_Error("halftone region segment : refered to segment is not pattern dict");
        nRet = JBIG2_ERROR_FETAL;
        goto failed;
    }
    pPatternDict = pSeg->m_Result.pd;
    if((pPatternDict == NULL) || (pPatternDict->NUMPATS == 0)) {
        m_pModule->JBig2_Error("halftone region segment : has no patterns input");
        nRet = JBIG2_ERROR_FETAL;
        goto failed;
    }
    pHRD->HNUMPATS = pPatternDict->NUMPATS;
    pHRD->HPATS = pPatternDict->HDPATS;
    pHRD->HPW = pPatternDict->HDPATS[0]->m_nWidth;
    pHRD->HPH = pPatternDict->HDPATS[0]->m_nHeight;
    pSegment->m_nResultType = JBIG2_IMAGE_POINTER;
    if(pHRD->HMMR == 0) {
        dwTemp = pHRD->HTEMPLATE == 0 ? 65536 : pHRD->HTEMPLATE == 1 ? 8192 : 1024;
        gbContext = (JBig2ArithCtx*)m_pModule->JBig2_Malloc2(sizeof(JBig2ArithCtx), dwTemp);
        JBIG2_memset(gbContext, 0, sizeof(JBig2ArithCtx)*dwTemp);
        JBIG2_ALLOC(pArithDecoder, CJBig2_ArithDecoder(m_pStream));
        pSegment->m_Result.im = pHRD->decode_Arith(pArithDecoder, gbContext, pPause);
        delete pArithDecoder;
        if(pSegment->m_Result.im == NULL) {
            m_pModule->JBig2_Free(gbContext);
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
        m_pModule->JBig2_Free(gbContext);
        m_pStream->alignByte();
        m_pStream->offset(2);
    } else {
        pSegment->m_Result.im = pHRD->decode_MMR(m_pStream, pPause);
        if(pSegment->m_Result.im == NULL) {
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
        m_pStream->alignByte();
    }
    if(pSegment->m_cFlags.s.type != 20) {
        if(!m_bBufSpecified) {
            JBig2PageInfo *pPageInfo = m_pPageInfoList->getLast();
            if ((pPageInfo->m_bIsStriped == 1) && (ri.y + ri.height > m_pPage->m_nHeight)) {
                m_pPage->expand(ri.y + ri.height, (pPageInfo->m_cFlags & 4) ? 1 : 0);
            }
        }
        m_pPage->composeFrom(ri.x, ri.y, pSegment->m_Result.im, (JBig2ComposeOp)(ri.flags & 0x03));
        delete pSegment->m_Result.im;
        pSegment->m_Result.im = NULL;
    }
    delete pHRD;
    return JBIG2_SUCCESS;
failed:
    delete pHRD;
    return nRet;
}

FX_BOOL CJBig2_Context::parseGenericRegion(CJBig2_Segment *pSegment, IFX_Pause* pPause)
{
    FX_DWORD dwTemp;
    FX_BYTE cFlags;
    FX_INT32 i, nRet;
    if(m_pGRD == NULL) {
        JBIG2_ALLOC(m_pGRD, CJBig2_GRDProc());
        if((parseRegionInfo(&m_ri) != JBIG2_SUCCESS)
                || (m_pStream->read1Byte(&cFlags) != 0)) {
            m_pModule->JBig2_Error("generic region segment : data header too short.");
            nRet = JBIG2_ERROR_TOO_SHORT;
            goto failed;
        }
        if (m_ri.height < 0 || m_ri.width < 0) {
            m_pModule->JBig2_Error("generic region segment : wrong data.");
            nRet = JBIG2_FAILED;
            goto failed;
        }
        m_pGRD->GBW = m_ri.width;
        m_pGRD->GBH = m_ri.height;
        m_pGRD->MMR = cFlags & 0x01;
        m_pGRD->GBTEMPLATE = (cFlags >> 1) & 0x03;
        m_pGRD->TPGDON = (cFlags >> 3) & 0x01;
        if(m_pGRD->MMR == 0) {
            if(m_pGRD->GBTEMPLATE == 0) {
                for(i = 0; i < 8; i++) {
                    if(m_pStream->read1Byte((FX_BYTE*)&m_pGRD->GBAT[i]) != 0) {
                        m_pModule->JBig2_Error("generic region segment : data header too short.");
                        nRet = JBIG2_ERROR_TOO_SHORT;
                        goto failed;
                    }
                }
            } else {
                for(i = 0; i < 2; i++) {
                    if(m_pStream->read1Byte((FX_BYTE*)&m_pGRD->GBAT[i]) != 0) {
                        m_pModule->JBig2_Error("generic region segment : data header too short.");
                        nRet = JBIG2_ERROR_TOO_SHORT;
                        goto failed;
                    }
                }
            }
        }
        m_pGRD->USESKIP = 0;
    }
    pSegment->m_nResultType = JBIG2_IMAGE_POINTER;
    if(m_pGRD->MMR == 0) {
        dwTemp = m_pGRD->GBTEMPLATE == 0 ? 65536 : m_pGRD->GBTEMPLATE == 1 ? 8192 : 1024;
        if(m_gbContext == NULL) {
            m_gbContext = (JBig2ArithCtx*)m_pModule->JBig2_Malloc(sizeof(JBig2ArithCtx) * dwTemp);
            JBIG2_memset(m_gbContext, 0, sizeof(JBig2ArithCtx)*dwTemp);
        }
        if(m_pArithDecoder == NULL) {
            JBIG2_ALLOC(m_pArithDecoder, CJBig2_ArithDecoder(m_pStream));
            m_ProcessiveStatus = m_pGRD->Start_decode_Arith(&pSegment->m_Result.im, m_pArithDecoder, m_gbContext, pPause);
        } else {
            m_ProcessiveStatus = m_pGRD->Continue_decode(pPause);
        }
        OutputBitmap(pSegment->m_Result.im);
        if(m_ProcessiveStatus == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
            if(pSegment->m_cFlags.s.type != 36) {
                if(!m_bBufSpecified) {
                    JBig2PageInfo *pPageInfo = m_pPageInfoList->getLast();
                    if ((pPageInfo->m_bIsStriped == 1) && (m_ri.y + m_ri.height > m_pPage->m_nHeight)) {
                        m_pPage->expand(m_ri.y + m_ri.height, (pPageInfo->m_cFlags & 4) ? 1 : 0);
                    }
                }
                FX_RECT Rect = m_pGRD->GetReplaceRect();
                m_pPage->composeFrom(m_ri.x + Rect.left, m_ri.y + Rect.top, pSegment->m_Result.im, (JBig2ComposeOp)(m_ri.flags & 0x03), &Rect);
            }
            return JBIG2_SUCCESS;
        } else {
            delete m_pArithDecoder;
            m_pArithDecoder = NULL;
            if(pSegment->m_Result.im == NULL) {
                m_pModule->JBig2_Free(m_gbContext);
                nRet = JBIG2_ERROR_FETAL;
                m_gbContext = NULL;
                m_ProcessiveStatus = FXCODEC_STATUS_ERROR;
                goto failed;
            }
            m_pModule->JBig2_Free(m_gbContext);
            m_gbContext = NULL;
            m_pStream->alignByte();
            m_pStream->offset(2);
        }
    } else {
        FXCODEC_STATUS status = m_pGRD->Start_decode_MMR(&pSegment->m_Result.im, m_pStream, pPause);
        while(status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
            m_pGRD->Continue_decode(pPause);
        }
        if(pSegment->m_Result.im == NULL) {
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
        m_pStream->alignByte();
    }
    if(pSegment->m_cFlags.s.type != 36) {
        if(!m_bBufSpecified) {
            JBig2PageInfo *pPageInfo = m_pPageInfoList->getLast();
            if ((pPageInfo->m_bIsStriped == 1) && (m_ri.y + m_ri.height > m_pPage->m_nHeight)) {
                m_pPage->expand(m_ri.y + m_ri.height, (pPageInfo->m_cFlags & 4) ? 1 : 0);
            }
        }
        FX_RECT Rect = m_pGRD->GetReplaceRect();
        m_pPage->composeFrom(m_ri.x + Rect.left, m_ri.y + Rect.top, pSegment->m_Result.im, (JBig2ComposeOp)(m_ri.flags & 0x03), &Rect);
        delete pSegment->m_Result.im;
        pSegment->m_Result.im = NULL;
    }
    delete m_pGRD;
    m_pGRD = NULL;
    return JBIG2_SUCCESS;
failed:
    delete m_pGRD;
    m_pGRD = NULL;
    return nRet;
}

FX_BOOL CJBig2_Context::parseGenericRefinementRegion(CJBig2_Segment *pSegment)
{
    FX_DWORD dwTemp;
    JBig2RegionInfo ri;
    CJBig2_Segment *pSeg;
    FX_INT32 i, nRet;
    FX_BYTE cFlags;
    JBig2ArithCtx *grContext;
    CJBig2_GRRDProc *pGRRD;
    CJBig2_ArithDecoder *pArithDecoder;
    JBIG2_ALLOC(pGRRD, CJBig2_GRRDProc());
    if((parseRegionInfo(&ri) != JBIG2_SUCCESS)
            || (m_pStream->read1Byte(&cFlags) != 0)) {
        m_pModule->JBig2_Error("generic refinement region segment : data header too short.");
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
    }
    pGRRD->GRW = ri.width;
    pGRRD->GRH = ri.height;
    pGRRD->GRTEMPLATE = cFlags & 0x01;
    pGRRD->TPGRON = (cFlags >> 1) & 0x01;
    if(pGRRD->GRTEMPLATE == 0) {
        for(i = 0; i < 4; i++) {
            if(m_pStream->read1Byte((FX_BYTE*)&pGRRD->GRAT[i]) != 0) {
                m_pModule->JBig2_Error("generic refinement region segment : data header too short.");
                nRet = JBIG2_ERROR_TOO_SHORT;
                goto failed;
            }
        }
    }
    pSeg = NULL;
    if(pSegment->m_nReferred_to_segment_count > 0) {
        for(i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
            pSeg = this->findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[0]);
            if(pSeg == NULL) {
                m_pModule->JBig2_Error("generic refinement region segment : can't find refered to segments");
                nRet = JBIG2_ERROR_FETAL;
                goto failed;
            }
            if((pSeg->m_cFlags.s.type == 4) || (pSeg->m_cFlags.s.type == 20)
                    || (pSeg->m_cFlags.s.type == 36) || (pSeg->m_cFlags.s.type == 40)) {
                break;
            }
        }
        if(i >= pSegment->m_nReferred_to_segment_count) {
            m_pModule->JBig2_Error("generic refinement region segment : can't find refered to intermediate region");
            nRet = JBIG2_ERROR_FETAL;
            goto failed;
        }
        pGRRD->GRREFERENCE = pSeg->m_Result.im;
    } else {
        pGRRD->GRREFERENCE = m_pPage;
    }
    pGRRD->GRREFERENCEDX = 0;
    pGRRD->GRREFERENCEDY = 0;
    dwTemp = pGRRD->GRTEMPLATE ? 1 << 10 : 1 << 13;
    grContext = (JBig2ArithCtx*)m_pModule->JBig2_Malloc2(sizeof(JBig2ArithCtx), dwTemp);
    JBIG2_memset(grContext, 0, sizeof(JBig2ArithCtx)*dwTemp);
    JBIG2_ALLOC(pArithDecoder, CJBig2_ArithDecoder(m_pStream));
    pSegment->m_nResultType = JBIG2_IMAGE_POINTER;
    pSegment->m_Result.im = pGRRD->decode(pArithDecoder, grContext);
    delete pArithDecoder;
    if(pSegment->m_Result.im == NULL) {
        m_pModule->JBig2_Free(grContext);
        nRet = JBIG2_ERROR_FETAL;
        goto failed;
    }
    m_pModule->JBig2_Free(grContext);
    m_pStream->alignByte();
    m_pStream->offset(2);
    if(pSegment->m_cFlags.s.type != 40) {
        if(!m_bBufSpecified) {
            JBig2PageInfo *pPageInfo = m_pPageInfoList->getLast();
            if ((pPageInfo->m_bIsStriped == 1) && (ri.y + ri.height > m_pPage->m_nHeight)) {
                m_pPage->expand(ri.y + ri.height, (pPageInfo->m_cFlags & 4) ? 1 : 0);
            }
        }
        m_pPage->composeFrom(ri.x, ri.y, pSegment->m_Result.im, (JBig2ComposeOp)(ri.flags & 0x03));
        delete pSegment->m_Result.im;
        pSegment->m_Result.im = NULL;
    }
    delete pGRRD;
    return JBIG2_SUCCESS;
failed:
    delete pGRRD;
    return nRet;
}
FX_BOOL CJBig2_Context::parseTable(CJBig2_Segment *pSegment)
{
    pSegment->m_nResultType = JBIG2_HUFFMAN_TABLE_POINTER;
    JBIG2_ALLOC(pSegment->m_Result.ht, CJBig2_HuffmanTable(m_pStream));
    if(!pSegment->m_Result.ht->isOK()) {
        delete pSegment->m_Result.ht;
        pSegment->m_Result.ht = NULL;
        return JBIG2_ERROR_FETAL;
    }
    m_pStream->alignByte();
    return JBIG2_SUCCESS;
}
FX_INT32 CJBig2_Context::parseRegionInfo(JBig2RegionInfo *pRI)
{
    if((m_pStream->readInteger((FX_DWORD*)&pRI->width) != 0)
            || (m_pStream->readInteger((FX_DWORD*)&pRI->height) != 0)
            || (m_pStream->readInteger((FX_DWORD*)&pRI->x) != 0)
            || (m_pStream->readInteger((FX_DWORD*)&pRI->y) != 0)
            || (m_pStream->read1Byte(&pRI->flags) != 0)) {
        return JBIG2_ERROR_TOO_SHORT;
    }
    return JBIG2_SUCCESS;
}
JBig2HuffmanCode *CJBig2_Context::decodeSymbolIDHuffmanTable(CJBig2_BitStream *pStream,
        FX_DWORD SBNUMSYMS)
{
    JBig2HuffmanCode *SBSYMCODES;
    FX_INT32 runcodes[35], runcodes_len[35], runcode;
    FX_INT32 i, j, nTemp, nVal, nBits;
    FX_INT32 run;
    SBSYMCODES = (JBig2HuffmanCode*)m_pModule->JBig2_Malloc2(sizeof(JBig2HuffmanCode), SBNUMSYMS);
    for (i = 0; i < 35; i ++) {
        if(pStream->readNBits(4, &runcodes_len[i]) != 0) {
            goto failed;
        }
    }
    huffman_assign_code(runcodes, runcodes_len, 35);
    i = 0;
    while(i < (int)SBNUMSYMS) {
        nVal = 0;
        nBits = 0;
        for(;;) {
            if(pStream->read1Bit(&nTemp) != 0) {
                goto failed;
            }
            nVal = (nVal << 1) | nTemp;
            nBits ++;
            for(j = 0; j < 35; j++) {
                if((nBits == runcodes_len[j]) && (nVal == runcodes[j])) {
                    break;
                }
            }
            if(j < 35) {
                break;
            }
        }
        runcode = j;
        if(runcode < 32) {
            SBSYMCODES[i].codelen = runcode;
            run = 0;
        } else if(runcode == 32) {
            if(pStream->readNBits(2, &nTemp) != 0) {
                goto failed;
            }
            run = nTemp + 3;
        } else if(runcode == 33) {
            if(pStream->readNBits(3, &nTemp) != 0) {
                goto failed;
            }
            run = nTemp + 3;
        } else if(runcode == 34) {
            if(pStream->readNBits(7, &nTemp) != 0) {
                goto failed;
            }
            run = nTemp + 11;
        }
        if(run > 0) {
            if (i + run > (int)SBNUMSYMS) {
                goto failed;
            }
            for(j = 0; j < run; j++) {
                if(runcode == 32 && i > 0) {
                    SBSYMCODES[i + j].codelen = SBSYMCODES[i - 1].codelen;
                } else {
                    SBSYMCODES[i + j].codelen = 0;
                }
            }
            i += run;
        } else {
            i ++;
        }
    }
    huffman_assign_code(SBSYMCODES, SBNUMSYMS);
    return SBSYMCODES;
failed:
    m_pModule->JBig2_Free(SBSYMCODES);
    return NULL;
}
void CJBig2_Context::huffman_assign_code(int* CODES, int* PREFLEN, int NTEMP)
{
    int CURLEN, LENMAX, CURCODE, CURTEMP, i;
    int *LENCOUNT;
    int *FIRSTCODE;
    LENMAX = 0;
    for(i = 0; i < NTEMP; i++) {
        if(PREFLEN[i] > LENMAX) {
            LENMAX = PREFLEN[i];
        }
    }
    LENCOUNT = (int*)m_pModule->JBig2_Malloc2(sizeof(int), (LENMAX + 1));
    JBIG2_memset(LENCOUNT, 0, sizeof(int) * (LENMAX + 1));
    FIRSTCODE = (int*)m_pModule->JBig2_Malloc2(sizeof(int), (LENMAX + 1));
    for(i = 0; i < NTEMP; i++) {
        LENCOUNT[PREFLEN[i]] ++;
    }
    CURLEN = 1;
    FIRSTCODE[0] = 0;
    LENCOUNT[0]  = 0;
    while(CURLEN <= LENMAX) {
        FIRSTCODE[CURLEN] = (FIRSTCODE[CURLEN - 1] + LENCOUNT[CURLEN - 1]) << 1;
        CURCODE = FIRSTCODE[CURLEN];
        CURTEMP = 0;
        while(CURTEMP < NTEMP) {
            if(PREFLEN[CURTEMP] == CURLEN) {
                CODES[CURTEMP] = CURCODE;
                CURCODE = CURCODE + 1;
            }
            CURTEMP = CURTEMP + 1;
        }
        CURLEN = CURLEN + 1;
    }
    m_pModule->JBig2_Free(LENCOUNT);
    m_pModule->JBig2_Free(FIRSTCODE);
}
void CJBig2_Context::huffman_assign_code(JBig2HuffmanCode *SBSYMCODES, int NTEMP)
{
    int CURLEN, LENMAX, CURCODE, CURTEMP, i;
    int *LENCOUNT;
    int *FIRSTCODE;
    LENMAX = 0;
    for(i = 0; i < NTEMP; i++) {
        if(SBSYMCODES[i].codelen > LENMAX) {
            LENMAX = SBSYMCODES[i].codelen;
        }
    }
    LENCOUNT = (int*)m_pModule->JBig2_Malloc2(sizeof(int), (LENMAX + 1));
    JBIG2_memset(LENCOUNT, 0, sizeof(int) * (LENMAX + 1));
    FIRSTCODE = (int*)m_pModule->JBig2_Malloc2(sizeof(int), (LENMAX + 1));
    for(i = 0; i < NTEMP; i++) {
        LENCOUNT[SBSYMCODES[i].codelen] ++;
    }
    CURLEN = 1;
    FIRSTCODE[0] = 0;
    LENCOUNT[0]  = 0;
    while(CURLEN <= LENMAX) {
        FIRSTCODE[CURLEN] = (FIRSTCODE[CURLEN - 1] + LENCOUNT[CURLEN - 1]) << 1;
        CURCODE = FIRSTCODE[CURLEN];
        CURTEMP = 0;
        while(CURTEMP < NTEMP) {
            if(SBSYMCODES[CURTEMP].codelen == CURLEN) {
                SBSYMCODES[CURTEMP].code = CURCODE;
                CURCODE = CURCODE + 1;
            }
            CURTEMP = CURTEMP + 1;
        }
        CURLEN = CURLEN + 1;
    }
    m_pModule->JBig2_Free(LENCOUNT);
    m_pModule->JBig2_Free(FIRSTCODE);
}
