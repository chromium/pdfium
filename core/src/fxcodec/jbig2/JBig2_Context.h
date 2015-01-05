// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_CONTEXT_H_
#define _JBIG2_CONTEXT_H_
#include "JBig2_Module.h"
#include "JBig2_List.h"
#include "JBig2_Segment.h"
#include "JBig2_Page.h"
#include "JBig2_GeneralDecoder.h"
#include "../../../include/fxcodec/fx_codec_def.h"
#include "../../../include/fxcrt/fx_basic.h"
#include <utility>
typedef std::pair<FX_BYTE*, CJBig2_SymbolDict*> CJBig2_CachePair;
typedef enum {
    JBIG2_OUT_OF_PAGE = 0,
    JBIG2_IN_PAGE,
} JBig2State;
#define JBIG2_SUCCESS			 0
#define JBIG2_FAILED			-1
#define JBIG2_ERROR_TOO_SHORT	-2
#define JBIG2_ERROR_FETAL		-3
#define JBIG2_END_OF_PAGE		 2
#define JBIG2_END_OF_FILE		 3
#define JBIG2_ERROR_FILE_FORMAT -4
#define JBIG2_ERROR_STREAM_TYPE -5
#define JBIG2_ERROR_LIMIT		-6
#define JBIG2_FILE_STREAM			0
#define JBIG2_SQUENTIAL_STREAM		1
#define JBIG2_RANDOM_STREAM			2
#define JBIG2_EMBED_STREAM			3
#define JBIG2_MIN_SEGMENT_SIZE					11
class CJBig2_Context : public CJBig2_Object
{
public:

    static CJBig2_Context *CreateContext(CJBig2_Module *pModule, FX_BYTE *pGlobalData, FX_DWORD dwGlobalLength,
                                         FX_BYTE *pData, FX_DWORD dwLength, FX_INT32 nStreamType, std::list<CJBig2_CachePair>* pSymbolDictCache, IFX_Pause* pPause = NULL);

    static void DestroyContext(CJBig2_Context *pContext);

    FX_INT32 getFirstPage(FX_BYTE *pBuf, FX_INT32 width, FX_INT32 height, FX_INT32 stride, IFX_Pause* pPause);

    FX_INT32 getNextPage(FX_BYTE *pBuf, FX_INT32 width, FX_INT32 height, FX_INT32 stride, IFX_Pause* pPause);

    FX_INT32 getFirstPage(CJBig2_Image **image, IFX_Pause* pPause);

    FX_INT32 getNextPage(CJBig2_Image **image, IFX_Pause* pPause);
    FX_INT32 Continue(IFX_Pause* pPause);
    FXCODEC_STATUS GetProcessiveStatus()
    {
        return m_ProcessiveStatus;
    };
private:

    CJBig2_Context(FX_BYTE *pGlobalData, FX_DWORD dwGlobalLength,
                   FX_BYTE *pData, FX_DWORD dwLength, FX_INT32 nStreamType, std::list<CJBig2_CachePair>* pSymbolDictCache, IFX_Pause* pPause);

    ~CJBig2_Context();

    FX_INT32 decodeFile(IFX_Pause* pPause);

    FX_INT32 decode_SquentialOrgnazation(IFX_Pause* pPause);

    FX_INT32 decode_EmbedOrgnazation(IFX_Pause* pPause);

    FX_INT32 decode_RandomOrgnazation_FirstPage(IFX_Pause* pPause);

    FX_INT32 decode_RandomOrgnazation(IFX_Pause* pPause);

    CJBig2_Segment *findSegmentByNumber(FX_DWORD dwNumber);

    CJBig2_Segment *findReferredSegmentByTypeAndIndex(CJBig2_Segment *pSegment, FX_BYTE cType, FX_INT32 nIndex);

    FX_INT32 parseSegmentHeader(CJBig2_Segment *pSegment);

    FX_INT32 parseSegmentData(CJBig2_Segment *pSegment, IFX_Pause* pPause);
    FX_INT32 ProcessiveParseSegmentData(CJBig2_Segment *pSegment, IFX_Pause* pPause);

    FX_INT32 parseSymbolDict(CJBig2_Segment *pSegment, IFX_Pause* pPause);

    FX_INT32 parseTextRegion(CJBig2_Segment *pSegment);

    FX_INT32 parsePatternDict(CJBig2_Segment *pSegment, IFX_Pause* pPause);

    FX_INT32 parseHalftoneRegion(CJBig2_Segment *pSegment, IFX_Pause* pPause);

    FX_INT32 parseGenericRegion(CJBig2_Segment *pSegment, IFX_Pause* pPause);

    FX_INT32 parseGenericRefinementRegion(CJBig2_Segment *pSegment);

    FX_INT32 parseTable(CJBig2_Segment *pSegment);

    FX_INT32 parseRegionInfo(JBig2RegionInfo *pRI);



    JBig2HuffmanCode *decodeSymbolIDHuffmanTable(CJBig2_BitStream *pStream, FX_DWORD SBNUMSYMS);

    void huffman_assign_code(int* CODES, int* PREFLEN, int NTEMP);

    void huffman_assign_code(JBig2HuffmanCode *SBSYMCODES, int NTEMP);

private:

    CJBig2_Context *m_pGlobalContext;

    FX_INT32 m_nStreamType;

    CJBig2_BitStream *m_pStream;

    FX_INT32 m_nState;

    CJBig2_List<CJBig2_Segment> *m_pSegmentList;

    CJBig2_List<JBig2PageInfo> *m_pPageInfoList;

    CJBig2_Image *m_pPage;

    FX_BOOL m_bBufSpecified;

    FX_INT32 m_nSegmentDecoded;
    IFX_Pause*	m_pPause;
    FX_INT32	m_PauseStep;
    FXCODEC_STATUS m_ProcessiveStatus;
    FX_BOOL	m_bFirstPage;
    CJBig2_ArithDecoder *m_pArithDecoder;
    CJBig2_GRDProc *m_pGRD;
    JBig2ArithCtx *m_gbContext;
    CJBig2_Segment *m_pSegment;
    FX_DWORD m_dwOffset;
    JBig2RegionInfo m_ri;
    std::list<CJBig2_CachePair>* m_pSymbolDictCache;
};
#endif
