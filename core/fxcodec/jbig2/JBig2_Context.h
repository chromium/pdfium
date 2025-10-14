// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JBIG2_JBIG2_CONTEXT_H_
#define CORE_FXCODEC_JBIG2_JBIG2_CONTEXT_H_

#include <stddef.h>
#include <stdint.h>

#include <list>
#include <memory>
#include <utility>
#include <vector>

#include "core/fxcodec/fx_codec_def.h"
#include "core/fxcodec/jbig2/JBig2_DocumentContext.h"
#include "core/fxcodec/jbig2/JBig2_Page.h"
#include "core/fxcodec/jbig2/JBig2_Segment.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"

class CJBig2_ArithDecoder;
class CJBig2_GRDProc;
class PauseIndicatorIface;

#define JBIG2_MIN_SEGMENT_SIZE 11

enum class JBig2_Result { kSuccess, kFailure, kEndReached };

class CJBig2_Context {
 public:
  static std::unique_ptr<CJBig2_Context> Create(
      pdfium::span<const uint8_t> pGlobalSpan,
      uint64_t global_key,
      pdfium::span<const uint8_t> pSrcSpan,
      uint64_t src_key,
      std::list<CJBig2_CachePair>* pSymbolDictCache);

  ~CJBig2_Context();

  static bool HuffmanAssignCode(pdfium::span<JBig2HuffmanCode> symcodes);

  bool GetFirstPage(pdfium::span<uint8_t> pBuf,
                    int32_t width,
                    int32_t height,
                    int32_t stride,
                    PauseIndicatorIface* pPause);

  bool Continue(PauseIndicatorIface* pPause);
  FXCODEC_STATUS GetProcessingStatus() const { return processing_status_; }

  void RejectLargeRegionsWhenFuzzing() {
    reject_large_regions_when_fuzzing_ = true;
  }

 private:
  CJBig2_Context(pdfium::span<const uint8_t> pSrcSpan,
                 uint64_t src_key,
                 std::list<CJBig2_CachePair>* pSymbolDictCache,
                 bool bIsGlobal);

  JBig2_Result DecodeSequential(PauseIndicatorIface* pPause);

  CJBig2_Segment* FindSegmentByNumber(uint32_t dwNumber);
  CJBig2_Segment* FindReferredTableSegmentByIndex(CJBig2_Segment* pSegment,
                                                  int32_t nIndex);

  JBig2_Result ParseSegmentHeader(CJBig2_Segment* pSegment);
  JBig2_Result ParseSegmentData(CJBig2_Segment* pSegment,
                                PauseIndicatorIface* pPause);
  JBig2_Result ProcessingParseSegmentData(CJBig2_Segment* pSegment,
                                          PauseIndicatorIface* pPause);
  JBig2_Result ParseSymbolDict(CJBig2_Segment* pSegment);
  JBig2_Result ParseTextRegion(CJBig2_Segment* pSegment);
  JBig2_Result ParsePatternDict(CJBig2_Segment* pSegment,
                                PauseIndicatorIface* pPause);
  JBig2_Result ParseHalftoneRegion(CJBig2_Segment* pSegment,
                                   PauseIndicatorIface* pPause);
  JBig2_Result ParseGenericRegion(CJBig2_Segment* pSegment,
                                  PauseIndicatorIface* pPause);
  JBig2_Result ParseGenericRefinementRegion(CJBig2_Segment* pSegment);
  JBig2_Result ParseTable(CJBig2_Segment* pSegment);
  JBig2_Result ParseRegionInfo(JBig2RegionInfo* pRI);

  std::vector<JBig2HuffmanCode> DecodeSymbolIDHuffmanTable(uint32_t SBNUMSYMS);
  const CJBig2_HuffmanTable* GetHuffmanTable(size_t idx);

  std::unique_ptr<CJBig2_Context> global_context_;
  std::unique_ptr<CJBig2_BitStream> stream_;
  std::vector<std::unique_ptr<CJBig2_Segment>> segment_list_;
  std::vector<std::unique_ptr<JBig2PageInfo>> page_info_list_;
  std::unique_ptr<CJBig2_Image> page_;
  std::vector<std::unique_ptr<CJBig2_HuffmanTable>> huffman_tables_;
  const bool is_global_;
  bool in_page_ = false;
  bool buf_specified_ = false;
  int32_t pause_step_ = 10;
  FXCODEC_STATUS processing_status_ = FXCODEC_STATUS::kFrameReady;
  std::vector<JBig2ArithCtx> gb_contexts_;
  std::unique_ptr<CJBig2_ArithDecoder> arith_decoder_;
  std::unique_ptr<CJBig2_GRDProc> grd_;
  std::unique_ptr<CJBig2_Segment> segment_;
  uint32_t offset_ = 0;
  JBig2RegionInfo ri_ = {};
  UnownedPtr<std::list<CJBig2_CachePair>> const symbol_dict_cache_;
  bool reject_large_regions_when_fuzzing_ = false;
};

#endif  // CORE_FXCODEC_JBIG2_JBIG2_CONTEXT_H_
