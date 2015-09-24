// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_Context.h"

#include <list>

#include "JBig2_GrdProc.h"
#include "JBig2_GrrdProc.h"
#include "JBig2_HtrdProc.h"
#include "JBig2_PddProc.h"
#include "JBig2_SddProc.h"
#include "JBig2_TrdProc.h"

// Implement a very small least recently used (LRU) cache. It is very
// common for a JBIG2 dictionary to span multiple pages in a PDF file,
// and we do not want to decode the same dictionary over and over
// again. We key off of the memory location of the dictionary. The
// list keeps track of the freshness of entries, with freshest ones
// at the front. Even a tiny cache size like 2 makes a dramatic
// difference for typical JBIG2 documents.
static const int kSymbolDictCacheMaxSize = 2;

CJBig2_Context* CJBig2_Context::CreateContext(
    const uint8_t* pGlobalData,
    FX_DWORD dwGlobalLength,
    const uint8_t* pData,
    FX_DWORD dwLength,
    std::list<CJBig2_CachePair>* pSymbolDictCache,
    IFX_Pause* pPause) {
  return new CJBig2_Context(pGlobalData, dwGlobalLength, pData, dwLength,
                            pSymbolDictCache, pPause);
}

void CJBig2_Context::DestroyContext(CJBig2_Context* pContext) {
  delete pContext;
}

CJBig2_Context::CJBig2_Context(const uint8_t* pGlobalData,
                               FX_DWORD dwGlobalLength,
                               const uint8_t* pData,
                               FX_DWORD dwLength,
                               std::list<CJBig2_CachePair>* pSymbolDictCache,
                               IFX_Pause* pPause)
    : m_nSegmentDecoded(0),
      m_bInPage(false),
      m_bBufSpecified(false),
      m_PauseStep(10),
      m_pPause(pPause),
      m_ProcessingStatus(FXCODEC_STATUS_FRAME_READY),
      m_pArithDecoder(NULL),
      m_gbContext(NULL),
      m_dwOffset(0),
      m_pSymbolDictCache(pSymbolDictCache) {
  if (pGlobalData && (dwGlobalLength > 0)) {
    m_pGlobalContext = new CJBig2_Context(
        nullptr, 0, pGlobalData, dwGlobalLength, pSymbolDictCache, pPause);
  } else {
    m_pGlobalContext = nullptr;
  }

  m_pStream.reset(new CJBig2_BitStream(pData, dwLength));
}

CJBig2_Context::~CJBig2_Context() {
  delete m_pArithDecoder;
  m_pArithDecoder = NULL;
  FX_Free(m_gbContext);
  m_gbContext = NULL;
  delete m_pGlobalContext;
  m_pGlobalContext = NULL;
}

int32_t CJBig2_Context::decode_SquentialOrgnazation(IFX_Pause* pPause) {
  int32_t nRet;
  if (m_pStream->getByteLeft() <= 0)
    return JBIG2_END_OF_FILE;

  while (m_pStream->getByteLeft() >= JBIG2_MIN_SEGMENT_SIZE) {
    if (!m_pSegment) {
      m_pSegment.reset(new CJBig2_Segment);
      nRet = parseSegmentHeader(m_pSegment.get());
      if (nRet != JBIG2_SUCCESS) {
        m_pSegment.reset();
        return nRet;
      }
      m_dwOffset = m_pStream->getOffset();
    }
    nRet = parseSegmentData(m_pSegment.get(), pPause);
    if (m_ProcessingStatus == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
      m_ProcessingStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      m_PauseStep = 2;
      return JBIG2_SUCCESS;
    }
    if ((nRet == JBIG2_END_OF_PAGE) || (nRet == JBIG2_END_OF_FILE)) {
      m_pSegment.reset();
      return JBIG2_SUCCESS;
    }
    if (nRet != JBIG2_SUCCESS) {
      m_pSegment.reset();
      return nRet;
    }
    if (m_pSegment->m_dwData_length != 0xffffffff) {
      m_dwOffset += m_pSegment->m_dwData_length;
      m_pStream->setOffset(m_dwOffset);
    } else {
      m_pStream->offset(4);
    }
    m_SegmentList.push_back(m_pSegment.release());
    if (m_pStream->getByteLeft() > 0 && m_pPage && pPause &&
        pPause->NeedToPauseNow()) {
      m_ProcessingStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      m_PauseStep = 2;
      return JBIG2_SUCCESS;
    }
  }
  return JBIG2_SUCCESS;
}
int32_t CJBig2_Context::decode_EmbedOrgnazation(IFX_Pause* pPause) {
  return decode_SquentialOrgnazation(pPause);
}
int32_t CJBig2_Context::decode_RandomOrgnazation_FirstPage(IFX_Pause* pPause) {
  int32_t nRet;
  while (m_pStream->getByteLeft() > JBIG2_MIN_SEGMENT_SIZE) {
    nonstd::unique_ptr<CJBig2_Segment> pSegment(new CJBig2_Segment);
    nRet = parseSegmentHeader(pSegment.get());
    if (nRet != JBIG2_SUCCESS) {
      return nRet;
    } else if (pSegment->m_cFlags.s.type == 51) {
      break;
    }
    m_SegmentList.push_back(pSegment.release());
    if (pPause && m_pPause && pPause->NeedToPauseNow()) {
      m_PauseStep = 3;
      m_ProcessingStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      return JBIG2_SUCCESS;
    }
  }
  m_nSegmentDecoded = 0;
  return decode_RandomOrgnazation(pPause);
}
int32_t CJBig2_Context::decode_RandomOrgnazation(IFX_Pause* pPause) {
  for (; m_nSegmentDecoded < m_SegmentList.size(); ++m_nSegmentDecoded) {
    int32_t nRet =
        parseSegmentData(m_SegmentList.get(m_nSegmentDecoded), pPause);
    if ((nRet == JBIG2_END_OF_PAGE) || (nRet == JBIG2_END_OF_FILE))
      return JBIG2_SUCCESS;

    if (nRet != JBIG2_SUCCESS)
      return nRet;

    if (m_pPage && pPause && pPause->NeedToPauseNow()) {
      m_PauseStep = 4;
      m_ProcessingStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      return JBIG2_SUCCESS;
    }
  }
  return JBIG2_SUCCESS;
}
int32_t CJBig2_Context::getFirstPage(uint8_t* pBuf,
                                     int32_t width,
                                     int32_t height,
                                     int32_t stride,
                                     IFX_Pause* pPause) {
  int32_t nRet = 0;
  if (m_pGlobalContext) {
    nRet = m_pGlobalContext->decode_EmbedOrgnazation(pPause);
    if (nRet != JBIG2_SUCCESS) {
      m_ProcessingStatus = FXCODEC_STATUS_ERROR;
      return nRet;
    }
  }
  m_PauseStep = 0;
  m_pPage.reset(new CJBig2_Image(width, height, stride, pBuf));
  m_bBufSpecified = true;
  if (pPause && pPause->NeedToPauseNow()) {
    m_PauseStep = 1;
    m_ProcessingStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
    return nRet;
  }
  return Continue(pPause);
}
int32_t CJBig2_Context::Continue(IFX_Pause* pPause) {
  m_ProcessingStatus = FXCODEC_STATUS_DECODE_READY;
  int32_t nRet;
  if (m_PauseStep <= 1) {
    nRet = decode_EmbedOrgnazation(pPause);
  } else if (m_PauseStep == 2) {
    nRet = decode_SquentialOrgnazation(pPause);
  } else if (m_PauseStep == 3) {
    nRet = decode_RandomOrgnazation_FirstPage(pPause);
  } else if (m_PauseStep == 4) {
    nRet = decode_RandomOrgnazation(pPause);
  } else if (m_PauseStep == 5) {
    m_ProcessingStatus = FXCODEC_STATUS_DECODE_FINISH;
    return JBIG2_SUCCESS;
  }
  if (m_ProcessingStatus == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
    return nRet;
  }
  m_PauseStep = 5;
  if (!m_bBufSpecified && nRet == JBIG2_SUCCESS) {
    m_ProcessingStatus = FXCODEC_STATUS_DECODE_FINISH;
    return JBIG2_SUCCESS;
  }
  if (nRet == JBIG2_SUCCESS) {
    m_ProcessingStatus = FXCODEC_STATUS_DECODE_FINISH;
  } else {
    m_ProcessingStatus = FXCODEC_STATUS_ERROR;
  }
  return nRet;
}

CJBig2_Segment* CJBig2_Context::findSegmentByNumber(FX_DWORD dwNumber) {
  CJBig2_Segment* pSeg;
  if (m_pGlobalContext) {
    pSeg = m_pGlobalContext->findSegmentByNumber(dwNumber);
    if (pSeg) {
      return pSeg;
    }
  }
  for (size_t i = 0; i < m_SegmentList.size(); ++i) {
    pSeg = m_SegmentList.get(i);
    if (pSeg->m_dwNumber == dwNumber) {
      return pSeg;
    }
  }
  return nullptr;
}
CJBig2_Segment* CJBig2_Context::findReferredSegmentByTypeAndIndex(
    CJBig2_Segment* pSegment,
    uint8_t cType,
    int32_t nIndex) {
  CJBig2_Segment* pSeg;
  int32_t i, count;
  count = 0;
  for (i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
    pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i]);
    if (pSeg && pSeg->m_cFlags.s.type == cType) {
      if (count == nIndex) {
        return pSeg;
      } else {
        count++;
      }
    }
  }
  return NULL;
}
int32_t CJBig2_Context::parseSegmentHeader(CJBig2_Segment* pSegment) {
  if ((m_pStream->readInteger(&pSegment->m_dwNumber) != 0) ||
      (m_pStream->read1Byte(&pSegment->m_cFlags.c) != 0)) {
    return JBIG2_ERROR_TOO_SHORT;
  }

  FX_DWORD dwTemp;
  uint8_t cTemp = m_pStream->getCurByte();
  if ((cTemp >> 5) == 7) {
    if (m_pStream->readInteger(
            (FX_DWORD*)&pSegment->m_nReferred_to_segment_count) != 0) {
      return JBIG2_ERROR_TOO_SHORT;
    }
    pSegment->m_nReferred_to_segment_count &= 0x1fffffff;
    if (pSegment->m_nReferred_to_segment_count >
        JBIG2_MAX_REFERRED_SEGMENT_COUNT) {
      return JBIG2_ERROR_LIMIT;
    }
    dwTemp = 5 + 4 + (pSegment->m_nReferred_to_segment_count + 1) / 8;
  } else {
    if (m_pStream->read1Byte(&cTemp) != 0)
      return JBIG2_ERROR_TOO_SHORT;

    pSegment->m_nReferred_to_segment_count = cTemp >> 5;
    dwTemp = 5 + 1;
  }
  uint8_t cSSize =
      pSegment->m_dwNumber > 65536 ? 4 : pSegment->m_dwNumber > 256 ? 2 : 1;
  uint8_t cPSize = pSegment->m_cFlags.s.page_association_size ? 4 : 1;
  if (pSegment->m_nReferred_to_segment_count) {
    pSegment->m_pReferred_to_segment_numbers =
        FX_Alloc(FX_DWORD, pSegment->m_nReferred_to_segment_count);
    for (int32_t i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
      switch (cSSize) {
        case 1:
          if (m_pStream->read1Byte(&cTemp) != 0)
            return JBIG2_ERROR_TOO_SHORT;

          pSegment->m_pReferred_to_segment_numbers[i] = cTemp;
          break;
        case 2:
          FX_WORD wTemp;
          if (m_pStream->readShortInteger(&wTemp) != 0)
            return JBIG2_ERROR_TOO_SHORT;

          pSegment->m_pReferred_to_segment_numbers[i] = wTemp;
          break;
        case 4:
          if (m_pStream->readInteger(&dwTemp) != 0)
            return JBIG2_ERROR_TOO_SHORT;

          pSegment->m_pReferred_to_segment_numbers[i] = dwTemp;
          break;
      }
      if (pSegment->m_pReferred_to_segment_numbers[i] >= pSegment->m_dwNumber)
        return JBIG2_ERROR_TOO_SHORT;
    }
  }
  if (cPSize == 1) {
    if (m_pStream->read1Byte(&cTemp) != 0)
      return JBIG2_ERROR_TOO_SHORT;
    pSegment->m_dwPage_association = cTemp;
  } else {
    if (m_pStream->readInteger(&pSegment->m_dwPage_association) != 0) {
      return JBIG2_ERROR_TOO_SHORT;
    }
  }
  if (m_pStream->readInteger(&pSegment->m_dwData_length) != 0)
    return JBIG2_ERROR_TOO_SHORT;

  pSegment->m_pData = m_pStream->getPointer();
  pSegment->m_State = JBIG2_SEGMENT_DATA_UNPARSED;
  return JBIG2_SUCCESS;
}

int32_t CJBig2_Context::parseSegmentData(CJBig2_Segment* pSegment,
                                         IFX_Pause* pPause) {
  int32_t ret = ProcessingParseSegmentData(pSegment, pPause);
  while (m_ProcessingStatus == FXCODEC_STATUS_DECODE_TOBECONTINUE &&
         m_pStream->getByteLeft() > 0) {
    ret = ProcessingParseSegmentData(pSegment, pPause);
  }
  return ret;
}

int32_t CJBig2_Context::ProcessingParseSegmentData(CJBig2_Segment* pSegment,
                                                   IFX_Pause* pPause) {
  switch (pSegment->m_cFlags.s.type) {
    case 0:
      return parseSymbolDict(pSegment, pPause);
    case 4:
    case 6:
    case 7:
      if (!m_bInPage)
        return JBIG2_ERROR_FATAL;
      return parseTextRegion(pSegment);
    case 16:
      return parsePatternDict(pSegment, pPause);
    case 20:
    case 22:
    case 23:
      if (!m_bInPage)
        return JBIG2_ERROR_FATAL;
      return parseHalftoneRegion(pSegment, pPause);
    case 36:
    case 38:
    case 39:
      if (!m_bInPage)
        return JBIG2_ERROR_FATAL;
      return parseGenericRegion(pSegment, pPause);
    case 40:
    case 42:
    case 43:
      if (!m_bInPage)
        return JBIG2_ERROR_FATAL;
      return parseGenericRefinementRegion(pSegment);
    case 48: {
      FX_WORD wTemp;
      nonstd::unique_ptr<JBig2PageInfo> pPageInfo(new JBig2PageInfo);
      if ((m_pStream->readInteger(&pPageInfo->m_dwWidth) != 0) ||
          (m_pStream->readInteger(&pPageInfo->m_dwHeight) != 0) ||
          (m_pStream->readInteger(&pPageInfo->m_dwResolutionX) != 0) ||
          (m_pStream->readInteger(&pPageInfo->m_dwResolutionY) != 0) ||
          (m_pStream->read1Byte(&pPageInfo->m_cFlags) != 0) ||
          (m_pStream->readShortInteger(&wTemp) != 0)) {
        return JBIG2_ERROR_TOO_SHORT;
      }
      pPageInfo->m_bIsStriped = ((wTemp >> 15) & 1) ? TRUE : FALSE;
      pPageInfo->m_wMaxStripeSize = wTemp & 0x7fff;
      bool bMaxHeight = (pPageInfo->m_dwHeight == 0xffffffff);
      if (bMaxHeight && pPageInfo->m_bIsStriped != TRUE)
        pPageInfo->m_bIsStriped = TRUE;

      if (!m_bBufSpecified) {
        FX_DWORD height =
            bMaxHeight ? pPageInfo->m_wMaxStripeSize : pPageInfo->m_dwHeight;
        m_pPage.reset(new CJBig2_Image(pPageInfo->m_dwWidth, height));
      }

      if (!m_pPage->m_pData) {
        m_ProcessingStatus = FXCODEC_STATUS_ERROR;
        return JBIG2_ERROR_TOO_SHORT;
      }

      m_pPage->fill((pPageInfo->m_cFlags & 4) ? 1 : 0);
      m_PageInfoList.push_back(pPageInfo.release());
      m_bInPage = true;
    } break;
    case 49:
      m_bInPage = false;
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
}

int32_t CJBig2_Context::parseSymbolDict(CJBig2_Segment* pSegment,
                                        IFX_Pause* pPause) {
  FX_DWORD dwTemp;
  FX_WORD wFlags;
  uint8_t cSDHUFFDH, cSDHUFFDW, cSDHUFFBMSIZE, cSDHUFFAGGINST;
  CJBig2_HuffmanTable* Table_B1 = nullptr;
  CJBig2_HuffmanTable* Table_B2 = nullptr;
  CJBig2_HuffmanTable* Table_B3 = nullptr;
  CJBig2_HuffmanTable* Table_B4 = nullptr;
  CJBig2_HuffmanTable* Table_B5 = nullptr;
  int32_t i, nIndex, nRet;
  CJBig2_Segment* pSeg = nullptr;
  CJBig2_Segment* pLRSeg = nullptr;
  FX_BOOL bUsed;
  CJBig2_Image** SDINSYMS = nullptr;
  JBig2ArithCtx* gbContext = nullptr;
  JBig2ArithCtx* grContext = nullptr;
  CJBig2_ArithDecoder* pArithDecoder;
  CJBig2_SDDProc* pSymbolDictDecoder = new CJBig2_SDDProc();
  const uint8_t* key = pSegment->m_pData;
  FX_BOOL cache_hit = false;
  if (m_pStream->readShortInteger(&wFlags) != 0) {
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
  if (pSymbolDictDecoder->SDHUFF == 0) {
    if (pSymbolDictDecoder->SDTEMPLATE == 0) {
      dwTemp = 8;
    } else {
      dwTemp = 2;
    }
    for (i = 0; i < (int32_t)dwTemp; i++) {
      if (m_pStream->read1Byte((uint8_t*)&pSymbolDictDecoder->SDAT[i]) != 0) {
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
      }
    }
  }
  if ((pSymbolDictDecoder->SDREFAGG == 1) &&
      (pSymbolDictDecoder->SDRTEMPLATE == 0)) {
    for (i = 0; i < 4; i++) {
      if (m_pStream->read1Byte((uint8_t*)&pSymbolDictDecoder->SDRAT[i]) != 0) {
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
      }
    }
  }
  if ((m_pStream->readInteger(&pSymbolDictDecoder->SDNUMEXSYMS) != 0) ||
      (m_pStream->readInteger(&pSymbolDictDecoder->SDNUMNEWSYMS) != 0)) {
    nRet = JBIG2_ERROR_TOO_SHORT;
    goto failed;
  }
  if (pSymbolDictDecoder->SDNUMEXSYMS > JBIG2_MAX_EXPORT_SYSMBOLS ||
      pSymbolDictDecoder->SDNUMNEWSYMS > JBIG2_MAX_NEW_SYSMBOLS) {
    nRet = JBIG2_ERROR_LIMIT;
    goto failed;
  }
  for (i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
    if (!findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i])) {
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
  }
  pSymbolDictDecoder->SDNUMINSYMS = 0;
  for (i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
    pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i]);
    if (pSeg->m_cFlags.s.type == 0) {
      pSymbolDictDecoder->SDNUMINSYMS += pSeg->m_Result.sd->SDNUMEXSYMS;
      pLRSeg = pSeg;
    }
  }
  if (pSymbolDictDecoder->SDNUMINSYMS == 0) {
    SDINSYMS = NULL;
  } else {
    SDINSYMS = FX_Alloc(CJBig2_Image*, pSymbolDictDecoder->SDNUMINSYMS);
    dwTemp = 0;
    for (i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
      pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i]);
      if (pSeg->m_cFlags.s.type == 0) {
        JBIG2_memcpy(SDINSYMS + dwTemp, pSeg->m_Result.sd->SDEXSYMS,
                     pSeg->m_Result.sd->SDNUMEXSYMS * sizeof(CJBig2_Image*));
        dwTemp += pSeg->m_Result.sd->SDNUMEXSYMS;
      }
    }
  }
  pSymbolDictDecoder->SDINSYMS = SDINSYMS;
  if (pSymbolDictDecoder->SDHUFF == 1) {
    if ((cSDHUFFDH == 2) || (cSDHUFFDW == 2)) {
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
    nIndex = 0;
    if (cSDHUFFDH == 0) {
      Table_B4 = new CJBig2_HuffmanTable(HuffmanTable_B4,
                                         FX_ArraySize(HuffmanTable_B4),
                                         HuffmanTable_HTOOB_B4);
      pSymbolDictDecoder->SDHUFFDH = Table_B4;
    } else if (cSDHUFFDH == 1) {
      Table_B5 = new CJBig2_HuffmanTable(HuffmanTable_B5,
                                         FX_ArraySize(HuffmanTable_B5),
                                         HuffmanTable_HTOOB_B5);
      pSymbolDictDecoder->SDHUFFDH = Table_B5;
    } else {
      pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
      if (!pSeg) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      pSymbolDictDecoder->SDHUFFDH = pSeg->m_Result.ht;
    }
    if (cSDHUFFDW == 0) {
      Table_B2 = new CJBig2_HuffmanTable(HuffmanTable_B2,
                                         FX_ArraySize(HuffmanTable_B2),
                                         HuffmanTable_HTOOB_B2);
      pSymbolDictDecoder->SDHUFFDW = Table_B2;
    } else if (cSDHUFFDW == 1) {
      Table_B3 = new CJBig2_HuffmanTable(HuffmanTable_B3,
                                         FX_ArraySize(HuffmanTable_B3),
                                         HuffmanTable_HTOOB_B3);
      pSymbolDictDecoder->SDHUFFDW = Table_B3;
    } else {
      pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
      if (!pSeg) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      pSymbolDictDecoder->SDHUFFDW = pSeg->m_Result.ht;
    }
    if (cSDHUFFBMSIZE == 0) {
      Table_B1 = new CJBig2_HuffmanTable(HuffmanTable_B1,
                                         FX_ArraySize(HuffmanTable_B1),
                                         HuffmanTable_HTOOB_B1);
      pSymbolDictDecoder->SDHUFFBMSIZE = Table_B1;
    } else {
      pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
      if (!pSeg) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      pSymbolDictDecoder->SDHUFFBMSIZE = pSeg->m_Result.ht;
    }
    if (pSymbolDictDecoder->SDREFAGG == 1) {
      if (cSDHUFFAGGINST == 0) {
        if (!Table_B1) {
          Table_B1 = new CJBig2_HuffmanTable(HuffmanTable_B1,
                                             FX_ArraySize(HuffmanTable_B1),
                                             HuffmanTable_HTOOB_B1);
        }
        pSymbolDictDecoder->SDHUFFAGGINST = Table_B1;
      } else {
        pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
        if (!pSeg) {
          nRet = JBIG2_ERROR_FATAL;
          goto failed;
        }
        pSymbolDictDecoder->SDHUFFAGGINST = pSeg->m_Result.ht;
      }
    }
  }
  if ((wFlags & 0x0100) && pLRSeg && pLRSeg->m_Result.sd->m_bContextRetained) {
    if (pSymbolDictDecoder->SDHUFF == 0) {
      dwTemp = pSymbolDictDecoder->SDTEMPLATE == 0
                   ? 65536
                   : pSymbolDictDecoder->SDTEMPLATE == 1 ? 8192 : 1024;
      gbContext = FX_Alloc(JBig2ArithCtx, dwTemp);
      JBIG2_memcpy(gbContext, pLRSeg->m_Result.sd->m_gbContext,
                   sizeof(JBig2ArithCtx) * dwTemp);
    }
    if (pSymbolDictDecoder->SDREFAGG == 1) {
      dwTemp = pSymbolDictDecoder->SDRTEMPLATE ? 1 << 10 : 1 << 13;
      grContext = FX_Alloc(JBig2ArithCtx, dwTemp);
      JBIG2_memcpy(grContext, pLRSeg->m_Result.sd->m_grContext,
                   sizeof(JBig2ArithCtx) * dwTemp);
    }
  } else {
    if (pSymbolDictDecoder->SDHUFF == 0) {
      dwTemp = pSymbolDictDecoder->SDTEMPLATE == 0
                   ? 65536
                   : pSymbolDictDecoder->SDTEMPLATE == 1 ? 8192 : 1024;
      gbContext = FX_Alloc(JBig2ArithCtx, dwTemp);
      JBIG2_memset(gbContext, 0, sizeof(JBig2ArithCtx) * dwTemp);
    }
    if (pSymbolDictDecoder->SDREFAGG == 1) {
      dwTemp = pSymbolDictDecoder->SDRTEMPLATE ? 1 << 10 : 1 << 13;
      grContext = FX_Alloc(JBig2ArithCtx, dwTemp);
      JBIG2_memset(grContext, 0, sizeof(JBig2ArithCtx) * dwTemp);
    }
  }
  pSegment->m_nResultType = JBIG2_SYMBOL_DICT_POINTER;
  for (std::list<CJBig2_CachePair>::iterator it = m_pSymbolDictCache->begin();
       it != m_pSymbolDictCache->end(); ++it) {
    if (it->first == key) {
      pSegment->m_Result.sd = it->second->DeepCopy();
      m_pSymbolDictCache->push_front(*it);
      m_pSymbolDictCache->erase(it);
      cache_hit = true;
      break;
    }
  }
  if (!cache_hit) {
    if (pSymbolDictDecoder->SDHUFF == 0) {
      pArithDecoder = new CJBig2_ArithDecoder(m_pStream.get());
      pSegment->m_Result.sd =
          pSymbolDictDecoder->decode_Arith(pArithDecoder, gbContext, grContext);
      delete pArithDecoder;
      if (pSegment->m_Result.sd == NULL) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      m_pStream->alignByte();
      m_pStream->offset(2);
    } else {
      pSegment->m_Result.sd = pSymbolDictDecoder->decode_Huffman(
          m_pStream.get(), gbContext, grContext, pPause);
      if (pSegment->m_Result.sd == NULL) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      m_pStream->alignByte();
    }
    CJBig2_SymbolDict* value = pSegment->m_Result.sd->DeepCopy();
    if (value && kSymbolDictCacheMaxSize > 0) {
      while (m_pSymbolDictCache->size() >= kSymbolDictCacheMaxSize) {
        delete m_pSymbolDictCache->back().second;
        m_pSymbolDictCache->pop_back();
      }
      m_pSymbolDictCache->push_front(CJBig2_CachePair(key, value));
    }
  }
  if (wFlags & 0x0200) {
    pSegment->m_Result.sd->m_bContextRetained = TRUE;
    if (pSymbolDictDecoder->SDHUFF == 0) {
      pSegment->m_Result.sd->m_gbContext = gbContext;
    }
    if (pSymbolDictDecoder->SDREFAGG == 1) {
      pSegment->m_Result.sd->m_grContext = grContext;
    }
    bUsed = TRUE;
  } else {
    bUsed = FALSE;
  }
  delete pSymbolDictDecoder;
  FX_Free(SDINSYMS);
  delete Table_B1;
  delete Table_B2;
  delete Table_B3;
  delete Table_B4;
  delete Table_B5;
  if (bUsed == FALSE) {
    FX_Free(gbContext);
    FX_Free(grContext);
  }
  return JBIG2_SUCCESS;
failed:
  delete pSymbolDictDecoder;
  FX_Free(SDINSYMS);
  delete Table_B1;
  delete Table_B2;
  delete Table_B3;
  delete Table_B4;
  delete Table_B5;
  FX_Free(gbContext);
  FX_Free(grContext);
  return nRet;
}

int32_t CJBig2_Context::parseTextRegion(CJBig2_Segment* pSegment) {
  FX_DWORD dwTemp;
  FX_WORD wFlags;
  int32_t i, nIndex, nRet;
  JBig2RegionInfo ri;
  CJBig2_Segment* pSeg;
  CJBig2_Image** SBSYMS = nullptr;
  JBig2HuffmanCode* SBSYMCODES = nullptr;
  uint8_t cSBHUFFFS, cSBHUFFDS, cSBHUFFDT, cSBHUFFRDW, cSBHUFFRDH, cSBHUFFRDX,
      cSBHUFFRDY, cSBHUFFRSIZE;
  CJBig2_HuffmanTable* Table_B1 = nullptr;
  CJBig2_HuffmanTable* Table_B6 = nullptr;
  CJBig2_HuffmanTable* Table_B7 = nullptr;
  CJBig2_HuffmanTable* Table_B8 = nullptr;
  CJBig2_HuffmanTable* Table_B9 = nullptr;
  CJBig2_HuffmanTable* Table_B10 = nullptr;
  CJBig2_HuffmanTable* Table_B11 = nullptr;
  CJBig2_HuffmanTable* Table_B12 = nullptr;
  CJBig2_HuffmanTable* Table_B13 = nullptr;
  CJBig2_HuffmanTable* Table_B14 = nullptr;
  CJBig2_HuffmanTable* Table_B15 = nullptr;
  JBig2ArithCtx* grContext = nullptr;
  CJBig2_ArithDecoder* pArithDecoder;
  CJBig2_TRDProc* pTRD = new CJBig2_TRDProc();
  if ((parseRegionInfo(&ri) != JBIG2_SUCCESS) ||
      (m_pStream->readShortInteger(&wFlags) != 0)) {
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
  if (pTRD->SBDSOFFSET >= 0x0010) {
    pTRD->SBDSOFFSET = pTRD->SBDSOFFSET - 0x0020;
  }
  pTRD->SBRTEMPLATE = (wFlags >> 15) & 0x0001;
  if (pTRD->SBHUFF == 1) {
    if (m_pStream->readShortInteger(&wFlags) != 0) {
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
  if ((pTRD->SBREFINE == 1) && (pTRD->SBRTEMPLATE == 0)) {
    for (i = 0; i < 4; i++) {
      if (m_pStream->read1Byte((uint8_t*)&pTRD->SBRAT[i]) != 0) {
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
      }
    }
  }
  if (m_pStream->readInteger(&pTRD->SBNUMINSTANCES) != 0) {
    nRet = JBIG2_ERROR_TOO_SHORT;
    goto failed;
  }
  for (i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
    if (!findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i])) {
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
  }
  pTRD->SBNUMSYMS = 0;
  for (i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
    pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i]);
    if (pSeg->m_cFlags.s.type == 0) {
      pTRD->SBNUMSYMS += pSeg->m_Result.sd->SDNUMEXSYMS;
    }
  }
  if (pTRD->SBNUMSYMS > 0) {
    SBSYMS = FX_Alloc(CJBig2_Image*, pTRD->SBNUMSYMS);
    dwTemp = 0;
    for (i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
      pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[i]);
      if (pSeg->m_cFlags.s.type == 0) {
        JBIG2_memcpy(SBSYMS + dwTemp, pSeg->m_Result.sd->SDEXSYMS,
                     pSeg->m_Result.sd->SDNUMEXSYMS * sizeof(CJBig2_Image*));
        dwTemp += pSeg->m_Result.sd->SDNUMEXSYMS;
      }
    }
    pTRD->SBSYMS = SBSYMS;
  } else {
    pTRD->SBSYMS = NULL;
  }
  if (pTRD->SBHUFF == 1) {
    SBSYMCODES = decodeSymbolIDHuffmanTable(m_pStream.get(), pTRD->SBNUMSYMS);
    if (SBSYMCODES == NULL) {
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
    m_pStream->alignByte();
    pTRD->SBSYMCODES = SBSYMCODES;
  } else {
    dwTemp = 0;
    while ((FX_DWORD)(1 << dwTemp) < pTRD->SBNUMSYMS) {
      dwTemp++;
    }
    pTRD->SBSYMCODELEN = (uint8_t)dwTemp;
  }
  if (pTRD->SBHUFF == 1) {
    if ((cSBHUFFFS == 2) || (cSBHUFFRDW == 2) || (cSBHUFFRDH == 2) ||
        (cSBHUFFRDX == 2) || (cSBHUFFRDY == 2)) {
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
    nIndex = 0;
    if (cSBHUFFFS == 0) {
      Table_B6 = new CJBig2_HuffmanTable(HuffmanTable_B6,
                                         FX_ArraySize(HuffmanTable_B6),
                                         HuffmanTable_HTOOB_B6);
      pTRD->SBHUFFFS = Table_B6;
    } else if (cSBHUFFFS == 1) {
      Table_B7 = new CJBig2_HuffmanTable(HuffmanTable_B7,
                                         FX_ArraySize(HuffmanTable_B7),
                                         HuffmanTable_HTOOB_B7);
      pTRD->SBHUFFFS = Table_B7;
    } else {
      pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
      if (!pSeg) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      pTRD->SBHUFFFS = pSeg->m_Result.ht;
    }
    if (cSBHUFFDS == 0) {
      Table_B8 = new CJBig2_HuffmanTable(HuffmanTable_B8,
                                         FX_ArraySize(HuffmanTable_B8),
                                         HuffmanTable_HTOOB_B8);
      pTRD->SBHUFFDS = Table_B8;
    } else if (cSBHUFFDS == 1) {
      Table_B9 = new CJBig2_HuffmanTable(HuffmanTable_B9,
                                         FX_ArraySize(HuffmanTable_B9),
                                         HuffmanTable_HTOOB_B9);
      pTRD->SBHUFFDS = Table_B9;
    } else if (cSBHUFFDS == 2) {
      Table_B10 = new CJBig2_HuffmanTable(HuffmanTable_B10,
                                          FX_ArraySize(HuffmanTable_B10),
                                          HuffmanTable_HTOOB_B10);
      pTRD->SBHUFFDS = Table_B10;
    } else {
      pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
      if (!pSeg) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      pTRD->SBHUFFDS = pSeg->m_Result.ht;
    }
    if (cSBHUFFDT == 0) {
      Table_B11 = new CJBig2_HuffmanTable(HuffmanTable_B11,
                                          FX_ArraySize(HuffmanTable_B11),
                                          HuffmanTable_HTOOB_B11);
      pTRD->SBHUFFDT = Table_B11;
    } else if (cSBHUFFDT == 1) {
      Table_B12 = new CJBig2_HuffmanTable(HuffmanTable_B12,
                                          FX_ArraySize(HuffmanTable_B12),
                                          HuffmanTable_HTOOB_B12);
      pTRD->SBHUFFDT = Table_B12;
    } else if (cSBHUFFDT == 2) {
      Table_B13 = new CJBig2_HuffmanTable(HuffmanTable_B13,
                                          FX_ArraySize(HuffmanTable_B13),
                                          HuffmanTable_HTOOB_B13);
      pTRD->SBHUFFDT = Table_B13;
    } else {
      pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
      if (!pSeg) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      pTRD->SBHUFFDT = pSeg->m_Result.ht;
    }
    if (cSBHUFFRDW == 0) {
      Table_B14 = new CJBig2_HuffmanTable(HuffmanTable_B14,
                                          FX_ArraySize(HuffmanTable_B14),
                                          HuffmanTable_HTOOB_B14);
      pTRD->SBHUFFRDW = Table_B14;
    } else if (cSBHUFFRDW == 1) {
      Table_B15 = new CJBig2_HuffmanTable(HuffmanTable_B15,
                                          FX_ArraySize(HuffmanTable_B15),
                                          HuffmanTable_HTOOB_B15);
      pTRD->SBHUFFRDW = Table_B15;
    } else {
      pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
      if (!pSeg) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      pTRD->SBHUFFRDW = pSeg->m_Result.ht;
    }
    if (cSBHUFFRDH == 0) {
      if (!Table_B14) {
        Table_B14 = new CJBig2_HuffmanTable(HuffmanTable_B14,
                                            FX_ArraySize(HuffmanTable_B14),
                                            HuffmanTable_HTOOB_B14);
      }
      pTRD->SBHUFFRDH = Table_B14;
    } else if (cSBHUFFRDH == 1) {
      if (!Table_B15) {
        Table_B15 = new CJBig2_HuffmanTable(HuffmanTable_B15,
                                            FX_ArraySize(HuffmanTable_B15),
                                            HuffmanTable_HTOOB_B15);
      }
      pTRD->SBHUFFRDH = Table_B15;
    } else {
      pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
      if (!pSeg) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      pTRD->SBHUFFRDH = pSeg->m_Result.ht;
    }
    if (cSBHUFFRDX == 0) {
      if (!Table_B14) {
        Table_B14 = new CJBig2_HuffmanTable(HuffmanTable_B14,
                                            FX_ArraySize(HuffmanTable_B14),
                                            HuffmanTable_HTOOB_B14);
      }
      pTRD->SBHUFFRDX = Table_B14;
    } else if (cSBHUFFRDX == 1) {
      if (!Table_B15) {
        Table_B15 = new CJBig2_HuffmanTable(HuffmanTable_B15,
                                            FX_ArraySize(HuffmanTable_B15),
                                            HuffmanTable_HTOOB_B15);
      }
      pTRD->SBHUFFRDX = Table_B15;
    } else {
      pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
      if (!pSeg) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      pTRD->SBHUFFRDX = pSeg->m_Result.ht;
    }
    if (cSBHUFFRDY == 0) {
      if (!Table_B14) {
        Table_B14 = new CJBig2_HuffmanTable(HuffmanTable_B14,
                                            FX_ArraySize(HuffmanTable_B14),
                                            HuffmanTable_HTOOB_B14);
      }
      pTRD->SBHUFFRDY = Table_B14;
    } else if (cSBHUFFRDY == 1) {
      if (!Table_B15) {
        Table_B15 = new CJBig2_HuffmanTable(HuffmanTable_B15,
                                            FX_ArraySize(HuffmanTable_B15),
                                            HuffmanTable_HTOOB_B15);
      }
      pTRD->SBHUFFRDY = Table_B15;
    } else {
      pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
      if (!pSeg) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      pTRD->SBHUFFRDY = pSeg->m_Result.ht;
    }
    if (cSBHUFFRSIZE == 0) {
      Table_B1 = new CJBig2_HuffmanTable(HuffmanTable_B1,
                                         FX_ArraySize(HuffmanTable_B1),
                                         HuffmanTable_HTOOB_B1);
      pTRD->SBHUFFRSIZE = Table_B1;
    } else {
      pSeg = findReferredSegmentByTypeAndIndex(pSegment, 53, nIndex++);
      if (!pSeg) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      pTRD->SBHUFFRSIZE = pSeg->m_Result.ht;
    }
  }
  if (pTRD->SBREFINE == 1) {
    dwTemp = pTRD->SBRTEMPLATE ? 1 << 10 : 1 << 13;
    grContext = FX_Alloc(JBig2ArithCtx, dwTemp);
    JBIG2_memset(grContext, 0, sizeof(JBig2ArithCtx) * dwTemp);
  }
  if (pTRD->SBHUFF == 0) {
    pArithDecoder = new CJBig2_ArithDecoder(m_pStream.get());
    pSegment->m_nResultType = JBIG2_IMAGE_POINTER;
    pSegment->m_Result.im = pTRD->decode_Arith(pArithDecoder, grContext);
    delete pArithDecoder;
    if (pSegment->m_Result.im == NULL) {
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
    m_pStream->alignByte();
    m_pStream->offset(2);
  } else {
    pSegment->m_nResultType = JBIG2_IMAGE_POINTER;
    pSegment->m_Result.im = pTRD->decode_Huffman(m_pStream.get(), grContext);
    if (pSegment->m_Result.im == NULL) {
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
    m_pStream->alignByte();
  }
  if (pSegment->m_cFlags.s.type != 4) {
    if (!m_bBufSpecified) {
      JBig2PageInfo* pPageInfo = m_PageInfoList.back();
      if ((pPageInfo->m_bIsStriped == 1) &&
          (ri.y + ri.height > m_pPage->m_nHeight)) {
        m_pPage->expand(ri.y + ri.height, (pPageInfo->m_cFlags & 4) ? 1 : 0);
      }
    }
    m_pPage->composeFrom(ri.x, ri.y, pSegment->m_Result.im,
                         (JBig2ComposeOp)(ri.flags & 0x03));
    delete pSegment->m_Result.im;
    pSegment->m_Result.im = NULL;
  }
  delete pTRD;
  FX_Free(SBSYMS);
  FX_Free(SBSYMCODES);
  FX_Free(grContext);
  delete Table_B1;
  delete Table_B6;
  delete Table_B7;
  delete Table_B8;
  delete Table_B9;
  delete Table_B10;
  delete Table_B11;
  delete Table_B12;
  delete Table_B13;
  delete Table_B14;
  delete Table_B15;
  return JBIG2_SUCCESS;
failed:
  delete pTRD;
  FX_Free(SBSYMS);
  FX_Free(SBSYMCODES);
  FX_Free(grContext);
  delete Table_B1;
  delete Table_B6;
  delete Table_B7;
  delete Table_B8;
  delete Table_B9;
  delete Table_B10;
  delete Table_B11;
  delete Table_B12;
  delete Table_B13;
  delete Table_B14;
  delete Table_B15;
  return nRet;
}

int32_t CJBig2_Context::parsePatternDict(CJBig2_Segment* pSegment,
                                         IFX_Pause* pPause) {
  FX_DWORD dwTemp;
  uint8_t cFlags;
  JBig2ArithCtx* gbContext;
  CJBig2_ArithDecoder* pArithDecoder;
  int32_t nRet;
  CJBig2_PDDProc* pPDD = new CJBig2_PDDProc();
  if ((m_pStream->read1Byte(&cFlags) != 0) ||
      (m_pStream->read1Byte(&pPDD->HDPW) != 0) ||
      (m_pStream->read1Byte(&pPDD->HDPH) != 0) ||
      (m_pStream->readInteger(&pPDD->GRAYMAX) != 0)) {
    nRet = JBIG2_ERROR_TOO_SHORT;
    goto failed;
  }
  if (pPDD->GRAYMAX > JBIG2_MAX_PATTERN_INDEX) {
    nRet = JBIG2_ERROR_LIMIT;
    goto failed;
  }
  pPDD->HDMMR = cFlags & 0x01;
  pPDD->HDTEMPLATE = (cFlags >> 1) & 0x03;
  pSegment->m_nResultType = JBIG2_PATTERN_DICT_POINTER;
  if (pPDD->HDMMR == 0) {
    dwTemp =
        pPDD->HDTEMPLATE == 0 ? 65536 : pPDD->HDTEMPLATE == 1 ? 8192 : 1024;
    gbContext = FX_Alloc(JBig2ArithCtx, dwTemp);
    JBIG2_memset(gbContext, 0, sizeof(JBig2ArithCtx) * dwTemp);
    pArithDecoder = new CJBig2_ArithDecoder(m_pStream.get());
    pSegment->m_Result.pd =
        pPDD->decode_Arith(pArithDecoder, gbContext, pPause);
    delete pArithDecoder;
    if (pSegment->m_Result.pd == NULL) {
      FX_Free(gbContext);
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
    FX_Free(gbContext);
    m_pStream->alignByte();
    m_pStream->offset(2);
  } else {
    pSegment->m_Result.pd = pPDD->decode_MMR(m_pStream.get(), pPause);
    if (pSegment->m_Result.pd == NULL) {
      nRet = JBIG2_ERROR_FATAL;
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
int32_t CJBig2_Context::parseHalftoneRegion(CJBig2_Segment* pSegment,
                                            IFX_Pause* pPause) {
  FX_DWORD dwTemp;
  uint8_t cFlags;
  JBig2RegionInfo ri;
  CJBig2_Segment* pSeg;
  CJBig2_PatternDict* pPatternDict;
  JBig2ArithCtx* gbContext;
  CJBig2_ArithDecoder* pArithDecoder;
  int32_t nRet;
  CJBig2_HTRDProc* pHRD = new CJBig2_HTRDProc();
  if ((parseRegionInfo(&ri) != JBIG2_SUCCESS) ||
      (m_pStream->read1Byte(&cFlags) != 0) ||
      (m_pStream->readInteger(&pHRD->HGW) != 0) ||
      (m_pStream->readInteger(&pHRD->HGH) != 0) ||
      (m_pStream->readInteger((FX_DWORD*)&pHRD->HGX) != 0) ||
      (m_pStream->readInteger((FX_DWORD*)&pHRD->HGY) != 0) ||
      (m_pStream->readShortInteger(&pHRD->HRX) != 0) ||
      (m_pStream->readShortInteger(&pHRD->HRY) != 0)) {
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
  if (pSegment->m_nReferred_to_segment_count != 1) {
    nRet = JBIG2_ERROR_FATAL;
    goto failed;
  }
  pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[0]);
  if ((pSeg == NULL) || (pSeg->m_cFlags.s.type != 16)) {
    nRet = JBIG2_ERROR_FATAL;
    goto failed;
  }
  pPatternDict = pSeg->m_Result.pd;
  if ((pPatternDict == NULL) || (pPatternDict->NUMPATS == 0)) {
    nRet = JBIG2_ERROR_FATAL;
    goto failed;
  }
  pHRD->HNUMPATS = pPatternDict->NUMPATS;
  pHRD->HPATS = pPatternDict->HDPATS;
  pHRD->HPW = pPatternDict->HDPATS[0]->m_nWidth;
  pHRD->HPH = pPatternDict->HDPATS[0]->m_nHeight;
  pSegment->m_nResultType = JBIG2_IMAGE_POINTER;
  if (pHRD->HMMR == 0) {
    dwTemp = pHRD->HTEMPLATE == 0 ? 65536 : pHRD->HTEMPLATE == 1 ? 8192 : 1024;
    gbContext = FX_Alloc(JBig2ArithCtx, dwTemp);
    JBIG2_memset(gbContext, 0, sizeof(JBig2ArithCtx) * dwTemp);
    pArithDecoder = new CJBig2_ArithDecoder(m_pStream.get());
    pSegment->m_Result.im =
        pHRD->decode_Arith(pArithDecoder, gbContext, pPause);
    delete pArithDecoder;
    if (pSegment->m_Result.im == NULL) {
      FX_Free(gbContext);
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
    FX_Free(gbContext);
    m_pStream->alignByte();
    m_pStream->offset(2);
  } else {
    pSegment->m_Result.im = pHRD->decode_MMR(m_pStream.get(), pPause);
    if (pSegment->m_Result.im == NULL) {
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
    m_pStream->alignByte();
  }
  if (pSegment->m_cFlags.s.type != 20) {
    if (!m_bBufSpecified) {
      JBig2PageInfo* pPageInfo = m_PageInfoList.back();
      if ((pPageInfo->m_bIsStriped == 1) &&
          (ri.y + ri.height > m_pPage->m_nHeight)) {
        m_pPage->expand(ri.y + ri.height, (pPageInfo->m_cFlags & 4) ? 1 : 0);
      }
    }
    m_pPage->composeFrom(ri.x, ri.y, pSegment->m_Result.im,
                         (JBig2ComposeOp)(ri.flags & 0x03));
    delete pSegment->m_Result.im;
    pSegment->m_Result.im = NULL;
  }
  delete pHRD;
  return JBIG2_SUCCESS;
failed:
  delete pHRD;
  return nRet;
}

int32_t CJBig2_Context::parseGenericRegion(CJBig2_Segment* pSegment,
                                           IFX_Pause* pPause) {
  FX_DWORD dwTemp;
  uint8_t cFlags;
  int32_t i, nRet;
  if (!m_pGRD) {
    m_pGRD.reset(new CJBig2_GRDProc);
    if ((parseRegionInfo(&m_ri) != JBIG2_SUCCESS) ||
        (m_pStream->read1Byte(&cFlags) != 0)) {
      nRet = JBIG2_ERROR_TOO_SHORT;
      goto failed;
    }
    if (m_ri.height < 0 || m_ri.width < 0) {
      nRet = JBIG2_FAILED;
      goto failed;
    }
    m_pGRD->GBW = m_ri.width;
    m_pGRD->GBH = m_ri.height;
    m_pGRD->MMR = cFlags & 0x01;
    m_pGRD->GBTEMPLATE = (cFlags >> 1) & 0x03;
    m_pGRD->TPGDON = (cFlags >> 3) & 0x01;
    if (m_pGRD->MMR == 0) {
      if (m_pGRD->GBTEMPLATE == 0) {
        for (i = 0; i < 8; i++) {
          if (m_pStream->read1Byte((uint8_t*)&m_pGRD->GBAT[i]) != 0) {
            nRet = JBIG2_ERROR_TOO_SHORT;
            goto failed;
          }
        }
      } else {
        for (i = 0; i < 2; i++) {
          if (m_pStream->read1Byte((uint8_t*)&m_pGRD->GBAT[i]) != 0) {
            nRet = JBIG2_ERROR_TOO_SHORT;
            goto failed;
          }
        }
      }
    }
    m_pGRD->USESKIP = 0;
  }
  pSegment->m_nResultType = JBIG2_IMAGE_POINTER;
  if (m_pGRD->MMR == 0) {
    dwTemp =
        m_pGRD->GBTEMPLATE == 0 ? 65536 : m_pGRD->GBTEMPLATE == 1 ? 8192 : 1024;
    if (m_gbContext == NULL) {
      m_gbContext = FX_Alloc(JBig2ArithCtx, dwTemp);
      JBIG2_memset(m_gbContext, 0, sizeof(JBig2ArithCtx) * dwTemp);
    }
    if (m_pArithDecoder == NULL) {
      m_pArithDecoder = new CJBig2_ArithDecoder(m_pStream.get());
      m_ProcessingStatus = m_pGRD->Start_decode_Arith(
          &pSegment->m_Result.im, m_pArithDecoder, m_gbContext, pPause);
    } else {
      m_ProcessingStatus = m_pGRD->Continue_decode(pPause);
    }
    if (m_ProcessingStatus == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
      if (pSegment->m_cFlags.s.type != 36) {
        if (!m_bBufSpecified) {
          JBig2PageInfo* pPageInfo = m_PageInfoList.back();
          if ((pPageInfo->m_bIsStriped == 1) &&
              (m_ri.y + m_ri.height > m_pPage->m_nHeight)) {
            m_pPage->expand(m_ri.y + m_ri.height,
                            (pPageInfo->m_cFlags & 4) ? 1 : 0);
          }
        }
        FX_RECT Rect = m_pGRD->GetReplaceRect();
        m_pPage->composeFrom(m_ri.x + Rect.left, m_ri.y + Rect.top,
                             pSegment->m_Result.im,
                             (JBig2ComposeOp)(m_ri.flags & 0x03), &Rect);
      }
      return JBIG2_SUCCESS;
    } else {
      delete m_pArithDecoder;
      m_pArithDecoder = NULL;
      if (pSegment->m_Result.im == NULL) {
        FX_Free(m_gbContext);
        nRet = JBIG2_ERROR_FATAL;
        m_gbContext = NULL;
        m_ProcessingStatus = FXCODEC_STATUS_ERROR;
        goto failed;
      }
      FX_Free(m_gbContext);
      m_gbContext = NULL;
      m_pStream->alignByte();
      m_pStream->offset(2);
    }
  } else {
    FXCODEC_STATUS status = m_pGRD->Start_decode_MMR(&pSegment->m_Result.im,
                                                     m_pStream.get(), pPause);
    while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
      m_pGRD->Continue_decode(pPause);
    }
    if (pSegment->m_Result.im == NULL) {
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
    m_pStream->alignByte();
  }
  if (pSegment->m_cFlags.s.type != 36) {
    if (!m_bBufSpecified) {
      JBig2PageInfo* pPageInfo = m_PageInfoList.back();
      if ((pPageInfo->m_bIsStriped == 1) &&
          (m_ri.y + m_ri.height > m_pPage->m_nHeight)) {
        m_pPage->expand(m_ri.y + m_ri.height,
                        (pPageInfo->m_cFlags & 4) ? 1 : 0);
      }
    }
    FX_RECT Rect = m_pGRD->GetReplaceRect();
    m_pPage->composeFrom(m_ri.x + Rect.left, m_ri.y + Rect.top,
                         pSegment->m_Result.im,
                         (JBig2ComposeOp)(m_ri.flags & 0x03), &Rect);
    delete pSegment->m_Result.im;
    pSegment->m_Result.im = NULL;
  }
  m_pGRD.reset();
  return JBIG2_SUCCESS;
failed:
  m_pGRD.reset();
  return nRet;
}

int32_t CJBig2_Context::parseGenericRefinementRegion(CJBig2_Segment* pSegment) {
  FX_DWORD dwTemp;
  JBig2RegionInfo ri;
  CJBig2_Segment* pSeg;
  int32_t i, nRet;
  uint8_t cFlags;
  JBig2ArithCtx* grContext;
  CJBig2_ArithDecoder* pArithDecoder;
  CJBig2_GRRDProc* pGRRD = new CJBig2_GRRDProc();
  if ((parseRegionInfo(&ri) != JBIG2_SUCCESS) ||
      (m_pStream->read1Byte(&cFlags) != 0)) {
    nRet = JBIG2_ERROR_TOO_SHORT;
    goto failed;
  }
  pGRRD->GRW = ri.width;
  pGRRD->GRH = ri.height;
  pGRRD->GRTEMPLATE = cFlags & 0x01;
  pGRRD->TPGRON = (cFlags >> 1) & 0x01;
  if (pGRRD->GRTEMPLATE == 0) {
    for (i = 0; i < 4; i++) {
      if (m_pStream->read1Byte((uint8_t*)&pGRRD->GRAT[i]) != 0) {
        nRet = JBIG2_ERROR_TOO_SHORT;
        goto failed;
      }
    }
  }
  pSeg = NULL;
  if (pSegment->m_nReferred_to_segment_count > 0) {
    for (i = 0; i < pSegment->m_nReferred_to_segment_count; i++) {
      pSeg = findSegmentByNumber(pSegment->m_pReferred_to_segment_numbers[0]);
      if (pSeg == NULL) {
        nRet = JBIG2_ERROR_FATAL;
        goto failed;
      }
      if ((pSeg->m_cFlags.s.type == 4) || (pSeg->m_cFlags.s.type == 20) ||
          (pSeg->m_cFlags.s.type == 36) || (pSeg->m_cFlags.s.type == 40)) {
        break;
      }
    }
    if (i >= pSegment->m_nReferred_to_segment_count) {
      nRet = JBIG2_ERROR_FATAL;
      goto failed;
    }
    pGRRD->GRREFERENCE = pSeg->m_Result.im;
  } else {
    pGRRD->GRREFERENCE = m_pPage.get();
  }
  pGRRD->GRREFERENCEDX = 0;
  pGRRD->GRREFERENCEDY = 0;
  dwTemp = pGRRD->GRTEMPLATE ? 1 << 10 : 1 << 13;
  grContext = FX_Alloc(JBig2ArithCtx, dwTemp);
  JBIG2_memset(grContext, 0, sizeof(JBig2ArithCtx) * dwTemp);
  pArithDecoder = new CJBig2_ArithDecoder(m_pStream.get());
  pSegment->m_nResultType = JBIG2_IMAGE_POINTER;
  pSegment->m_Result.im = pGRRD->decode(pArithDecoder, grContext);
  delete pArithDecoder;
  if (pSegment->m_Result.im == NULL) {
    FX_Free(grContext);
    nRet = JBIG2_ERROR_FATAL;
    goto failed;
  }
  FX_Free(grContext);
  m_pStream->alignByte();
  m_pStream->offset(2);
  if (pSegment->m_cFlags.s.type != 40) {
    if (!m_bBufSpecified) {
      JBig2PageInfo* pPageInfo = m_PageInfoList.back();
      if ((pPageInfo->m_bIsStriped == 1) &&
          (ri.y + ri.height > m_pPage->m_nHeight)) {
        m_pPage->expand(ri.y + ri.height, (pPageInfo->m_cFlags & 4) ? 1 : 0);
      }
    }
    m_pPage->composeFrom(ri.x, ri.y, pSegment->m_Result.im,
                         (JBig2ComposeOp)(ri.flags & 0x03));
    delete pSegment->m_Result.im;
    pSegment->m_Result.im = NULL;
  }
  delete pGRRD;
  return JBIG2_SUCCESS;
failed:
  delete pGRRD;
  return nRet;
}
int32_t CJBig2_Context::parseTable(CJBig2_Segment* pSegment) {
  pSegment->m_nResultType = JBIG2_HUFFMAN_TABLE_POINTER;
  pSegment->m_Result.ht = new CJBig2_HuffmanTable(m_pStream.get());
  if (!pSegment->m_Result.ht->isOK()) {
    delete pSegment->m_Result.ht;
    pSegment->m_Result.ht = NULL;
    return JBIG2_ERROR_FATAL;
  }
  m_pStream->alignByte();
  return JBIG2_SUCCESS;
}
int32_t CJBig2_Context::parseRegionInfo(JBig2RegionInfo* pRI) {
  if ((m_pStream->readInteger((FX_DWORD*)&pRI->width) != 0) ||
      (m_pStream->readInteger((FX_DWORD*)&pRI->height) != 0) ||
      (m_pStream->readInteger((FX_DWORD*)&pRI->x) != 0) ||
      (m_pStream->readInteger((FX_DWORD*)&pRI->y) != 0) ||
      (m_pStream->read1Byte(&pRI->flags) != 0)) {
    return JBIG2_ERROR_TOO_SHORT;
  }
  return JBIG2_SUCCESS;
}
JBig2HuffmanCode* CJBig2_Context::decodeSymbolIDHuffmanTable(
    CJBig2_BitStream* pStream,
    FX_DWORD SBNUMSYMS) {
  JBig2HuffmanCode* SBSYMCODES;
  int32_t runcodes[35];
  int32_t runcodes_len[35];
  int32_t runcode;
  int32_t i;
  int32_t j;
  int32_t nVal;
  int32_t nBits;
  int32_t run;
  FX_DWORD nTemp;
  SBSYMCODES = FX_Alloc(JBig2HuffmanCode, SBNUMSYMS);
  for (i = 0; i < 35; i++) {
    if (pStream->readNBits(4, &runcodes_len[i]) != 0) {
      goto failed;
    }
  }
  huffman_assign_code(runcodes, runcodes_len, 35);
  i = 0;
  while (i < (int)SBNUMSYMS) {
    nVal = 0;
    nBits = 0;
    for (;;) {
      if (pStream->read1Bit(&nTemp) != 0) {
        goto failed;
      }
      nVal = (nVal << 1) | nTemp;
      nBits++;
      for (j = 0; j < 35; j++) {
        if ((nBits == runcodes_len[j]) && (nVal == runcodes[j])) {
          break;
        }
      }
      if (j < 35) {
        break;
      }
    }
    runcode = j;
    if (runcode < 32) {
      SBSYMCODES[i].codelen = runcode;
      run = 0;
    } else if (runcode == 32) {
      if (pStream->readNBits(2, &nTemp) != 0) {
        goto failed;
      }
      run = nTemp + 3;
    } else if (runcode == 33) {
      if (pStream->readNBits(3, &nTemp) != 0) {
        goto failed;
      }
      run = nTemp + 3;
    } else if (runcode == 34) {
      if (pStream->readNBits(7, &nTemp) != 0) {
        goto failed;
      }
      run = nTemp + 11;
    }
    if (run > 0) {
      if (i + run > (int)SBNUMSYMS) {
        goto failed;
      }
      for (j = 0; j < run; j++) {
        if (runcode == 32 && i > 0) {
          SBSYMCODES[i + j].codelen = SBSYMCODES[i - 1].codelen;
        } else {
          SBSYMCODES[i + j].codelen = 0;
        }
      }
      i += run;
    } else {
      i++;
    }
  }
  huffman_assign_code(SBSYMCODES, SBNUMSYMS);
  return SBSYMCODES;
failed:
  FX_Free(SBSYMCODES);
  return NULL;
}
void CJBig2_Context::huffman_assign_code(int* CODES, int* PREFLEN, int NTEMP) {
  int CURLEN, LENMAX, CURCODE, CURTEMP, i;
  int* LENCOUNT;
  int* FIRSTCODE;
  LENMAX = 0;
  for (i = 0; i < NTEMP; i++) {
    if (PREFLEN[i] > LENMAX) {
      LENMAX = PREFLEN[i];
    }
  }
  LENCOUNT = FX_Alloc(int, LENMAX + 1);
  JBIG2_memset(LENCOUNT, 0, sizeof(int) * (LENMAX + 1));
  FIRSTCODE = FX_Alloc(int, LENMAX + 1);
  for (i = 0; i < NTEMP; i++) {
    LENCOUNT[PREFLEN[i]]++;
  }
  CURLEN = 1;
  FIRSTCODE[0] = 0;
  LENCOUNT[0] = 0;
  while (CURLEN <= LENMAX) {
    FIRSTCODE[CURLEN] = (FIRSTCODE[CURLEN - 1] + LENCOUNT[CURLEN - 1]) << 1;
    CURCODE = FIRSTCODE[CURLEN];
    CURTEMP = 0;
    while (CURTEMP < NTEMP) {
      if (PREFLEN[CURTEMP] == CURLEN) {
        CODES[CURTEMP] = CURCODE;
        CURCODE = CURCODE + 1;
      }
      CURTEMP = CURTEMP + 1;
    }
    CURLEN = CURLEN + 1;
  }
  FX_Free(LENCOUNT);
  FX_Free(FIRSTCODE);
}
void CJBig2_Context::huffman_assign_code(JBig2HuffmanCode* SBSYMCODES,
                                         int NTEMP) {
  int CURLEN, LENMAX, CURCODE, CURTEMP, i;
  int* LENCOUNT;
  int* FIRSTCODE;
  LENMAX = 0;
  for (i = 0; i < NTEMP; i++) {
    if (SBSYMCODES[i].codelen > LENMAX) {
      LENMAX = SBSYMCODES[i].codelen;
    }
  }
  LENCOUNT = FX_Alloc(int, (LENMAX + 1));
  JBIG2_memset(LENCOUNT, 0, sizeof(int) * (LENMAX + 1));
  FIRSTCODE = FX_Alloc(int, (LENMAX + 1));
  for (i = 0; i < NTEMP; i++) {
    LENCOUNT[SBSYMCODES[i].codelen]++;
  }
  CURLEN = 1;
  FIRSTCODE[0] = 0;
  LENCOUNT[0] = 0;
  while (CURLEN <= LENMAX) {
    FIRSTCODE[CURLEN] = (FIRSTCODE[CURLEN - 1] + LENCOUNT[CURLEN - 1]) << 1;
    CURCODE = FIRSTCODE[CURLEN];
    CURTEMP = 0;
    while (CURTEMP < NTEMP) {
      if (SBSYMCODES[CURTEMP].codelen == CURLEN) {
        SBSYMCODES[CURTEMP].code = CURCODE;
        CURCODE = CURCODE + 1;
      }
      CURTEMP = CURTEMP + 1;
    }
    CURLEN = CURLEN + 1;
  }
  FX_Free(LENCOUNT);
  FX_Free(FIRSTCODE);
}
