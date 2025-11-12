// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jbig2/JBig2_Context.h"

#include <algorithm>
#include <array>
#include <limits>
#include <list>
#include <utility>
#include <vector>

#include "core/fxcodec/jbig2/JBig2_ArithDecoder.h"
#include "core/fxcodec/jbig2/JBig2_BitStream.h"
#include "core/fxcodec/jbig2/JBig2_GrdProc.h"
#include "core/fxcodec/jbig2/JBig2_GrrdProc.h"
#include "core/fxcodec/jbig2/JBig2_HtrdProc.h"
#include "core/fxcodec/jbig2/JBig2_PddProc.h"
#include "core/fxcodec/jbig2/JBig2_SddProc.h"
#include "core/fxcodec/jbig2/JBig2_TrdProc.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fixed_size_data_vector.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/pauseindicator_iface.h"
#include "core/fxcrt/ptr_util.h"

namespace {

size_t GetHuffContextSize(uint8_t val) {
  return val == 0 ? 65536 : val == 1 ? 8192 : 1024;
}

size_t GetRefAggContextSize(bool val) {
  return val ? 1024 : 8192;
}

JBig2ComposeOp GetRegionInfoComposeOp(const JBig2RegionInfo& ri) {
  if ((ri.flags & 0x07) == 4) {
    return JBig2ComposeOp::JBIG2_COMPOSE_REPLACE;
  }
  return static_cast<JBig2ComposeOp>(ri.flags & 0x03);
}

}  // namespace

// Implement a very small least recently used (LRU) cache. It is very
// common for a JBIG2 dictionary to span multiple pages in a PDF file,
// and we do not want to decode the same dictionary over and over
// again. We key off of the memory location of the dictionary. The
// list keeps track of the freshness of entries, with freshest ones
// at the front. Even a tiny cache size like 2 makes a dramatic
// difference for typical JBIG2 documents.
static const size_t kSymbolDictCacheMaxSize = 2;
static_assert(kSymbolDictCacheMaxSize > 0,
              "Symbol Dictionary Cache must have non-zero size");

// static
std::unique_ptr<CJBig2_Context> CJBig2_Context::Create(
    pdfium::span<const uint8_t> pGlobalSpan,
    uint64_t global_key,
    pdfium::span<const uint8_t> pSrcSpan,
    uint64_t src_key,
    std::list<CJBig2_CachePair>* pSymbolDictCache) {
  auto result = pdfium::WrapUnique(
      new CJBig2_Context(pSrcSpan, src_key, pSymbolDictCache, false));
  if (!pGlobalSpan.empty()) {
    result->global_context_ = pdfium::WrapUnique(
        new CJBig2_Context(pGlobalSpan, global_key, pSymbolDictCache, true));
  }
  return result;
}

CJBig2_Context::CJBig2_Context(pdfium::span<const uint8_t> pSrcSpan,
                               uint64_t src_key,
                               std::list<CJBig2_CachePair>* pSymbolDictCache,
                               bool bIsGlobal)
    : stream_(std::make_unique<CJBig2_BitStream>(pSrcSpan, src_key)),
      huffman_tables_(CJBig2_HuffmanTable::kNumHuffmanTables),
      is_global_(bIsGlobal),
      symbol_dict_cache_(pSymbolDictCache) {}

CJBig2_Context::~CJBig2_Context() = default;

JBig2_Result CJBig2_Context::DecodeSequential(PauseIndicatorIface* pPause) {
  if (stream_->getByteLeft() <= 0) {
    return JBig2_Result::kEndReached;
  }

  while (stream_->getByteLeft() >= JBIG2_MIN_SEGMENT_SIZE) {
    JBig2_Result nRet;
    if (!segment_) {
      segment_ = std::make_unique<CJBig2_Segment>();
      nRet = ParseSegmentHeader(segment_.get());
      if (nRet != JBig2_Result::kSuccess) {
        segment_.reset();
        return nRet;
      }
      offset_ = stream_->getOffset();
    }
    nRet = ParseSegmentData(segment_.get(), pPause);
    if (processing_status_ == FXCODEC_STATUS::kDecodeToBeContinued) {
      pause_step_ = 2;
      return JBig2_Result::kSuccess;
    }
    if (nRet == JBig2_Result::kEndReached) {
      segment_.reset();
      return JBig2_Result::kSuccess;
    }
    if (nRet != JBig2_Result::kSuccess) {
      segment_.reset();
      return nRet;
    }
    if (segment_->data_length_ != 0xffffffff) {
      FX_SAFE_UINT32 new_offset = offset_;
      new_offset += segment_->data_length_;
      if (!new_offset.IsValid()) {
        return JBig2_Result::kFailure;
      }
      offset_ = new_offset.ValueOrDie();
      stream_->setOffset(offset_);
    } else {
      stream_->addOffset(4);
    }
    segment_list_.push_back(std::move(segment_));
    if (stream_->getByteLeft() > 0 && page_ && pPause &&
        pPause->NeedToPauseNow()) {
      processing_status_ = FXCODEC_STATUS::kDecodeToBeContinued;
      pause_step_ = 2;
      return JBig2_Result::kSuccess;
    }
  }
  return JBig2_Result::kSuccess;
}

bool CJBig2_Context::GetFirstPage(pdfium::span<uint8_t> pBuf,
                                  int32_t width,
                                  int32_t height,
                                  int32_t stride,
                                  PauseIndicatorIface* pPause) {
  if (global_context_) {
    JBig2_Result nRet = global_context_->DecodeSequential(pPause);
    if (nRet != JBig2_Result::kSuccess) {
      processing_status_ = FXCODEC_STATUS::kError;
      return nRet == JBig2_Result::kSuccess;
    }
  }
  pause_step_ = 0;
  page_ = std::make_unique<CJBig2_Image>(width, height, stride, pBuf);
  buf_specified_ = true;
  if (pPause && pPause->NeedToPauseNow()) {
    pause_step_ = 1;
    processing_status_ = FXCODEC_STATUS::kDecodeToBeContinued;
    return true;
  }
  return Continue(pPause);
}

bool CJBig2_Context::Continue(PauseIndicatorIface* pPause) {
  processing_status_ = FXCODEC_STATUS::kDecodeReady;
  JBig2_Result nRet = JBig2_Result::kSuccess;
  if (pause_step_ == 5) {
    processing_status_ = FXCODEC_STATUS::kDecodeFinished;
    return true;
  }

  if (pause_step_ <= 2) {
    nRet = DecodeSequential(pPause);
  }
  if (processing_status_ == FXCODEC_STATUS::kDecodeToBeContinued) {
    return nRet == JBig2_Result::kSuccess;
  }

  pause_step_ = 5;
  if (!buf_specified_ && nRet == JBig2_Result::kSuccess) {
    processing_status_ = FXCODEC_STATUS::kDecodeFinished;
    return true;
  }
  processing_status_ = nRet == JBig2_Result::kSuccess
                           ? FXCODEC_STATUS::kDecodeFinished
                           : FXCODEC_STATUS::kError;
  return nRet == JBig2_Result::kSuccess;
}

CJBig2_Segment* CJBig2_Context::FindSegmentByNumber(uint32_t dwNumber) {
  if (global_context_) {
    CJBig2_Segment* pSeg = global_context_->FindSegmentByNumber(dwNumber);
    if (pSeg) {
      return pSeg;
    }
  }
  for (const auto& pSeg : segment_list_) {
    if (pSeg->number_ == dwNumber) {
      return pSeg.get();
    }
  }
  return nullptr;
}

CJBig2_Segment* CJBig2_Context::FindReferredTableSegmentByIndex(
    CJBig2_Segment* pSegment,
    int32_t nIndex) {
  static const uint8_t kTableType = 53;
  int32_t count = 0;
  for (int32_t i = 0; i < pSegment->referred_to_segment_count_; ++i) {
    CJBig2_Segment* pSeg =
        FindSegmentByNumber(pSegment->referred_to_segment_numbers_[i]);
    if (pSeg && pSeg->flags_.s.type == kTableType) {
      if (count == nIndex) {
        return pSeg;
      }
      ++count;
    }
  }
  return nullptr;
}

JBig2_Result CJBig2_Context::ParseSegmentHeader(CJBig2_Segment* pSegment) {
  if (stream_->readInteger(&pSegment->number_) != 0 ||
      stream_->read1Byte(&pSegment->flags_.c) != 0) {
    return JBig2_Result::kFailure;
  }

  uint8_t cTemp = stream_->getCurByte();
  if ((cTemp >> 5) == 7) {
    if (stream_->readInteger(
            (uint32_t*)&pSegment->referred_to_segment_count_) != 0) {
      return JBig2_Result::kFailure;
    }
    pSegment->referred_to_segment_count_ &= 0x1fffffff;
    if (pSegment->referred_to_segment_count_ > kJBig2MaxReferredSegmentCount) {
      return JBig2_Result::kFailure;
    }

    int number_of_bits_to_skip = 1 + pSegment->referred_to_segment_count_;
    stream_->addOffset((number_of_bits_to_skip + 7) / 8);
  } else {
    if (stream_->read1Byte(&cTemp) != 0) {
      return JBig2_Result::kFailure;
    }

    pSegment->referred_to_segment_count_ = cTemp >> 5;
  }
  uint8_t cSSize = pSegment->number_ > 65536 ? 4
                   : pSegment->number_ > 256 ? 2
                                             : 1;
  uint8_t cPSize = pSegment->flags_.s.page_association_size ? 4 : 1;
  if (pSegment->referred_to_segment_count_) {
    pSegment->referred_to_segment_numbers_.resize(
        pSegment->referred_to_segment_count_);
    for (int32_t i = 0; i < pSegment->referred_to_segment_count_; ++i) {
      switch (cSSize) {
        case 1:
          if (stream_->read1Byte(&cTemp) != 0) {
            return JBig2_Result::kFailure;
          }

          pSegment->referred_to_segment_numbers_[i] = cTemp;
          break;
        case 2:
          uint16_t wTemp;
          if (stream_->readShortInteger(&wTemp) != 0) {
            return JBig2_Result::kFailure;
          }

          pSegment->referred_to_segment_numbers_[i] = wTemp;
          break;
        case 4:
          uint32_t dwTemp;
          if (stream_->readInteger(&dwTemp) != 0) {
            return JBig2_Result::kFailure;
          }

          pSegment->referred_to_segment_numbers_[i] = dwTemp;
          break;
      }
      if (pSegment->referred_to_segment_numbers_[i] >= pSegment->number_) {
        return JBig2_Result::kFailure;
      }
    }
  }
  if (cPSize == 1) {
    if (stream_->read1Byte(&cTemp) != 0) {
      return JBig2_Result::kFailure;
    }
    pSegment->page_association_ = cTemp;
  } else if (stream_->readInteger(&pSegment->page_association_) != 0) {
    return JBig2_Result::kFailure;
  }
  if (stream_->readInteger(&pSegment->data_length_) != 0) {
    return JBig2_Result::kFailure;
  }

  pSegment->key_ = stream_->getKey();
  pSegment->data_offset_ = stream_->getOffset();
  pSegment->state_ = JBIG2_SEGMENT_DATA_UNPARSED;
  return JBig2_Result::kSuccess;
}

JBig2_Result CJBig2_Context::ParseSegmentData(CJBig2_Segment* pSegment,
                                              PauseIndicatorIface* pPause) {
  JBig2_Result ret = ProcessingParseSegmentData(pSegment, pPause);
  while (processing_status_ == FXCODEC_STATUS::kDecodeToBeContinued &&
         stream_->getByteLeft() > 0) {
    ret = ProcessingParseSegmentData(pSegment, pPause);
  }
  return ret;
}

JBig2_Result CJBig2_Context::ProcessingParseSegmentData(
    CJBig2_Segment* pSegment,
    PauseIndicatorIface* pPause) {
  switch (pSegment->flags_.s.type) {
    case 0:
      return ParseSymbolDict(pSegment);
    case 4:
    case 6:
    case 7:
      if (!in_page_) {
        return JBig2_Result::kFailure;
      }
      return ParseTextRegion(pSegment);
    case 16:
      return ParsePatternDict(pSegment, pPause);
    case 20:
    case 22:
    case 23:
      if (!in_page_) {
        return JBig2_Result::kFailure;
      }
      return ParseHalftoneRegion(pSegment, pPause);
    case 36:
    case 38:
    case 39:
      if (!in_page_) {
        return JBig2_Result::kFailure;
      }
      return ParseGenericRegion(pSegment, pPause);
    case 40:
    case 42:
    case 43:
      if (!in_page_) {
        return JBig2_Result::kFailure;
      }
      return ParseGenericRefinementRegion(pSegment);
    case 48: {
      uint8_t segment_flags;
      uint16_t striping_info;
      auto pPageInfo = std::make_unique<JBig2PageInfo>();
      if (stream_->readInteger(&pPageInfo->width_) != 0 ||
          stream_->readInteger(&pPageInfo->height_) != 0 ||
          stream_->readInteger(&pPageInfo->resolution_x_) != 0 ||
          stream_->readInteger(&pPageInfo->resolution_y_) != 0 ||
          stream_->read1Byte(&segment_flags) != 0 ||
          stream_->readShortInteger(&striping_info) != 0) {
        return JBig2_Result::kFailure;
      }

      pPageInfo->default_pixel_value_ = !!(segment_flags & 4);
      pPageInfo->is_striped_ = !!(striping_info & 0x8000);
      pPageInfo->max_stripe_size_ = striping_info & 0x7fff;
      bool bMaxHeight = (pPageInfo->height_ == 0xffffffff);
      if (bMaxHeight && !pPageInfo->is_striped_) {
        pPageInfo->is_striped_ = true;
      }

      if (!buf_specified_) {
        uint32_t height =
            bMaxHeight ? pPageInfo->max_stripe_size_ : pPageInfo->height_;
        page_ = std::make_unique<CJBig2_Image>(pPageInfo->width_, height);
      }

      if (!page_->data()) {
        processing_status_ = FXCODEC_STATUS::kError;
        return JBig2_Result::kFailure;
      }

      page_->Fill(pPageInfo->default_pixel_value_);
      page_info_list_.push_back(std::move(pPageInfo));
      in_page_ = true;
      break;
    }
    case 49:
      in_page_ = false;
      return JBig2_Result::kEndReached;
    case 50:
      stream_->addOffset(pSegment->data_length_);
      break;
    case 51:
      return JBig2_Result::kEndReached;
    case 52:
      stream_->addOffset(pSegment->data_length_);
      break;
    case 53:
      return ParseTable(pSegment);
    case 62:
      stream_->addOffset(pSegment->data_length_);
      break;
    default:
      break;
  }
  return JBig2_Result::kSuccess;
}

JBig2_Result CJBig2_Context::ParseSymbolDict(CJBig2_Segment* pSegment) {
  uint16_t wFlags;
  if (stream_->readShortInteger(&wFlags) != 0) {
    return JBig2_Result::kFailure;
  }

  auto pSymbolDictDecoder = std::make_unique<CJBig2_SDDProc>();
  pSymbolDictDecoder->SDHUFF = wFlags & 0x0001;
  pSymbolDictDecoder->SDREFAGG = (wFlags >> 1) & 0x0001;
  pSymbolDictDecoder->SDTEMPLATE = (wFlags >> 10) & 0x0003;
  pSymbolDictDecoder->SDRTEMPLATE = !!((wFlags >> 12) & 0x0003);
  if (!pSymbolDictDecoder->SDHUFF) {
    const uint32_t dwTemp = (pSymbolDictDecoder->SDTEMPLATE == 0) ? 8 : 2;
    for (uint32_t i = 0; i < dwTemp; ++i) {
      if (stream_->read1Byte((uint8_t*)&pSymbolDictDecoder->SDAT[i]) != 0) {
        return JBig2_Result::kFailure;
      }
    }
  }
  if (pSymbolDictDecoder->SDREFAGG && !pSymbolDictDecoder->SDRTEMPLATE) {
    for (int32_t i = 0; i < 4; ++i) {
      if (stream_->read1Byte((uint8_t*)&pSymbolDictDecoder->SDRAT[i]) != 0) {
        return JBig2_Result::kFailure;
      }
    }
  }
  if (stream_->readInteger(&pSymbolDictDecoder->SDNUMEXSYMS) != 0 ||
      stream_->readInteger(&pSymbolDictDecoder->SDNUMNEWSYMS) != 0) {
    return JBig2_Result::kFailure;
  }
  if (pSymbolDictDecoder->SDNUMEXSYMS > kJBig2MaxExportSymbols ||
      pSymbolDictDecoder->SDNUMNEWSYMS > kJBig2MaxNewSymbols) {
    return JBig2_Result::kFailure;
  }
  for (int32_t i = 0; i < pSegment->referred_to_segment_count_; ++i) {
    if (!FindSegmentByNumber(pSegment->referred_to_segment_numbers_[i])) {
      return JBig2_Result::kFailure;
    }
  }
  CJBig2_Segment* pLRSeg = nullptr;
  FX_SAFE_UINT32 dwNumSyms = 0;
  for (int32_t i = 0; i < pSegment->referred_to_segment_count_; ++i) {
    CJBig2_Segment* pSeg =
        FindSegmentByNumber(pSegment->referred_to_segment_numbers_[i]);
    if (pSeg->flags_.s.type == 0) {
      dwNumSyms += pSeg->symbol_dict_->NumImages();
      pLRSeg = pSeg;
    }
  }
  pSymbolDictDecoder->SDNUMINSYMS = dwNumSyms.ValueOrDie();

  std::vector<UnownedPtr<CJBig2_Image>> SDINSYMS(
      pSymbolDictDecoder->SDNUMINSYMS);
  if (!SDINSYMS.empty()) {
    dwNumSyms = 0;
    for (int32_t i = 0; i < pSegment->referred_to_segment_count_; ++i) {
      CJBig2_Segment* pSeg =
          FindSegmentByNumber(pSegment->referred_to_segment_numbers_[i]);
      if (pSeg->flags_.s.type == 0) {
        const CJBig2_SymbolDict& dict = *pSeg->symbol_dict_;
        for (uint32_t j = 0; j < dict.NumImages(); ++j) {
          uint32_t dwTemp = (dwNumSyms + j).ValueOrDie();
          SDINSYMS[dwTemp] = dict.GetImage(j);
        }
        dwNumSyms += dict.NumImages();
      }
    }
  }
  pSymbolDictDecoder->SDINSYMS = std::move(SDINSYMS);

  uint8_t cSDHUFFDH = (wFlags >> 2) & 0x0003;
  uint8_t cSDHUFFDW = (wFlags >> 4) & 0x0003;
  if (pSymbolDictDecoder->SDHUFF) {
    if (cSDHUFFDH == 2 || cSDHUFFDW == 2) {
      return JBig2_Result::kFailure;
    }

    int32_t nIndex = 0;
    if (cSDHUFFDH == 0) {
      pSymbolDictDecoder->SDHUFFDH = GetHuffmanTable(4);
    } else if (cSDHUFFDH == 1) {
      pSymbolDictDecoder->SDHUFFDH = GetHuffmanTable(5);
    } else {
      CJBig2_Segment* pSeg =
          FindReferredTableSegmentByIndex(pSegment, nIndex++);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }
      pSymbolDictDecoder->SDHUFFDH = pSeg->huffman_table_.get();
    }
    if (cSDHUFFDW == 0) {
      pSymbolDictDecoder->SDHUFFDW = GetHuffmanTable(2);
    } else if (cSDHUFFDW == 1) {
      pSymbolDictDecoder->SDHUFFDW = GetHuffmanTable(3);
    } else {
      CJBig2_Segment* pSeg =
          FindReferredTableSegmentByIndex(pSegment, nIndex++);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }
      pSymbolDictDecoder->SDHUFFDW = pSeg->huffman_table_.get();
    }
    uint8_t cSDHUFFBMSIZE = (wFlags >> 6) & 0x0001;
    if (cSDHUFFBMSIZE == 0) {
      pSymbolDictDecoder->SDHUFFBMSIZE = GetHuffmanTable(1);
    } else {
      CJBig2_Segment* pSeg =
          FindReferredTableSegmentByIndex(pSegment, nIndex++);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }
      pSymbolDictDecoder->SDHUFFBMSIZE = pSeg->huffman_table_.get();
    }
    if (pSymbolDictDecoder->SDREFAGG) {
      uint8_t cSDHUFFAGGINST = (wFlags >> 7) & 0x0001;
      if (cSDHUFFAGGINST == 0) {
        pSymbolDictDecoder->SDHUFFAGGINST = GetHuffmanTable(1);
      } else {
        CJBig2_Segment* pSeg =
            FindReferredTableSegmentByIndex(pSegment, nIndex++);
        if (!pSeg) {
          return JBig2_Result::kFailure;
        }
        pSymbolDictDecoder->SDHUFFAGGINST = pSeg->huffman_table_.get();
      }
    }
  }

  const bool bUseGbContext = !pSymbolDictDecoder->SDHUFF;
  const bool bUseGrContext = pSymbolDictDecoder->SDREFAGG;
  const size_t gbContextSize =
      GetHuffContextSize(pSymbolDictDecoder->SDTEMPLATE);
  const size_t grContextSize =
      GetRefAggContextSize(pSymbolDictDecoder->SDRTEMPLATE);
  std::vector<JBig2ArithCtx> gbContexts;
  std::vector<JBig2ArithCtx> grContexts;
  if ((wFlags & 0x0100) && pLRSeg) {
    if (bUseGbContext) {
      gbContexts = pLRSeg->symbol_dict_->GbContexts();
      if (gbContexts.size() != gbContextSize) {
        return JBig2_Result::kFailure;
      }
    }
    if (bUseGrContext) {
      grContexts = pLRSeg->symbol_dict_->GrContexts();
      if (grContexts.size() != grContextSize) {
        return JBig2_Result::kFailure;
      }
    }
  } else {
    if (bUseGbContext) {
      gbContexts.resize(gbContextSize);
    }
    if (bUseGrContext) {
      grContexts.resize(grContextSize);
    }
  }

  CJBig2_CompoundKey key(pSegment->key_, pSegment->data_offset_);
  bool cache_hit = false;
  pSegment->result_type_ = JBIG2_SYMBOL_DICT_POINTER;
  if (is_global_ && key.first != 0) {
    for (auto it = symbol_dict_cache_->begin(); it != symbol_dict_cache_->end();
         ++it) {
      if (it->first == key) {
        pSegment->symbol_dict_ = it->second->DeepCopy();
        symbol_dict_cache_->emplace_front(key, std::move(it->second));
        symbol_dict_cache_->erase(it);
        cache_hit = true;
        break;
      }
    }
  }
  if (!cache_hit) {
    if (bUseGbContext) {
      auto pArithDecoder = std::make_unique<CJBig2_ArithDecoder>(stream_.get());
      pSegment->symbol_dict_ = pSymbolDictDecoder->DecodeArith(
          pArithDecoder.get(), gbContexts, grContexts);
      if (!pSegment->symbol_dict_) {
        return JBig2_Result::kFailure;
      }

      stream_->alignByte();
      stream_->addOffset(2);
    } else {
      pSegment->symbol_dict_ = pSymbolDictDecoder->DecodeHuffman(
          stream_.get(), gbContexts, grContexts);
      if (!pSegment->symbol_dict_) {
        return JBig2_Result::kFailure;
      }
      stream_->alignByte();
    }
    if (is_global_) {
      std::unique_ptr<CJBig2_SymbolDict> value =
          pSegment->symbol_dict_->DeepCopy();
      size_t size = symbol_dict_cache_->size();
      while (size >= kSymbolDictCacheMaxSize) {
        symbol_dict_cache_->pop_back();
        --size;
      }
      symbol_dict_cache_->emplace_front(key, std::move(value));
    }
  }
  if (wFlags & 0x0200) {
    if (bUseGbContext) {
      pSegment->symbol_dict_->SetGbContexts(std::move(gbContexts));
    }
    if (bUseGrContext) {
      pSegment->symbol_dict_->SetGrContexts(std::move(grContexts));
    }
  }
  return JBig2_Result::kSuccess;
}

JBig2_Result CJBig2_Context::ParseTextRegion(CJBig2_Segment* pSegment) {
  uint16_t wFlags;
  JBig2RegionInfo ri;
  if (ParseRegionInfo(&ri) != JBig2_Result::kSuccess ||
      stream_->readShortInteger(&wFlags) != 0) {
    return JBig2_Result::kFailure;
  }
  if (!CJBig2_Image::IsValidImageSize(ri.width, ri.height)) {
    return JBig2_Result::kFailure;
  }

  auto pTRD = std::make_unique<CJBig2_TRDProc>();
  pTRD->SBW = ri.width;
  pTRD->SBH = ri.height;
  pTRD->SBHUFF = wFlags & 0x0001;
  pTRD->SBREFINE = (wFlags >> 1) & 0x0001;
  uint32_t dwTemp = (wFlags >> 2) & 0x0003;
  pTRD->SBSTRIPS = 1 << dwTemp;
  pTRD->REFCORNER = (JBig2Corner)((wFlags >> 4) & 0x0003);
  pTRD->TRANSPOSED = (wFlags >> 6) & 0x0001;
  pTRD->SBCOMBOP = (JBig2ComposeOp)((wFlags >> 7) & 0x0003);
  pTRD->SBDEFPIXEL = (wFlags >> 9) & 0x0001;
  pTRD->SBDSOFFSET = (wFlags >> 10) & 0x001f;
  if (pTRD->SBDSOFFSET >= 0x0010) {
    pTRD->SBDSOFFSET = pTRD->SBDSOFFSET - 0x0020;
  }
  pTRD->SBRTEMPLATE = !!((wFlags >> 15) & 0x0001);

  if (pTRD->SBHUFF && stream_->readShortInteger(&wFlags) != 0) {
    return JBig2_Result::kFailure;
  }
  if (pTRD->SBREFINE && !pTRD->SBRTEMPLATE) {
    for (int32_t i = 0; i < 4; ++i) {
      if (stream_->read1Byte((uint8_t*)&pTRD->SBRAT[i]) != 0) {
        return JBig2_Result::kFailure;
      }
    }
  }
  if (stream_->readInteger(&pTRD->SBNUMINSTANCES) != 0) {
    return JBig2_Result::kFailure;
  }

  // Assume each instance takes at least 0.25 bits when encoded. That means for
  // a stream of length N bytes, there can be at most 32N instances. This is a
  // conservative estimate just to sanitize the |SBNUMINSTANCES| value.
  // Use FX_SAFE_INT32 to be safe, though it should never overflow because PDFs
  // have a maximum size of roughly 11 GB.
  FX_SAFE_INT32 nMaxStripInstances = stream_->getBufSpan().size();
  nMaxStripInstances *= 32;
  if (pTRD->SBNUMINSTANCES > nMaxStripInstances.ValueOrDie()) {
    return JBig2_Result::kFailure;
  }

  for (int32_t i = 0; i < pSegment->referred_to_segment_count_; ++i) {
    if (!FindSegmentByNumber(pSegment->referred_to_segment_numbers_[i])) {
      return JBig2_Result::kFailure;
    }
  }

  FX_SAFE_UINT32 dwNumSyms = 0;
  for (int32_t i = 0; i < pSegment->referred_to_segment_count_; ++i) {
    CJBig2_Segment* pSeg =
        FindSegmentByNumber(pSegment->referred_to_segment_numbers_[i]);
    if (pSeg->flags_.s.type == 0) {
      dwNumSyms += pSeg->symbol_dict_->NumImages();
    }
  }
  pTRD->SBNUMSYMS = dwNumSyms.ValueOrDie();

  std::vector<UnownedPtr<CJBig2_Image>> SBSYMS(pTRD->SBNUMSYMS);
  if (!SBSYMS.empty()) {
    dwNumSyms = 0;
    for (int32_t i = 0; i < pSegment->referred_to_segment_count_; ++i) {
      CJBig2_Segment* pSeg =
          FindSegmentByNumber(pSegment->referred_to_segment_numbers_[i]);
      if (pSeg->flags_.s.type == 0) {
        const CJBig2_SymbolDict& dict = *pSeg->symbol_dict_;
        for (uint32_t j = 0; j < dict.NumImages(); ++j) {
          uint32_t dwIndex = (dwNumSyms + j).ValueOrDie();
          SBSYMS[dwIndex] = dict.GetImage(j);
        }
        dwNumSyms += dict.NumImages();
      }
    }
  }
  pTRD->SBSYMS = std::move(SBSYMS);

  if (pTRD->SBHUFF) {
    std::vector<JBig2HuffmanCode> SBSYMCODES =
        DecodeSymbolIDHuffmanTable(pTRD->SBNUMSYMS);
    if (SBSYMCODES.empty()) {
      return JBig2_Result::kFailure;
    }

    stream_->alignByte();
    pTRD->SBSYMCODES = std::move(SBSYMCODES);
  } else {
    dwTemp = 0;
    while ((uint32_t)(1 << dwTemp) < pTRD->SBNUMSYMS) {
      ++dwTemp;
    }
    pTRD->SBSYMCODELEN = (uint8_t)dwTemp;
  }

  if (pTRD->SBHUFF) {
    uint8_t cSBHUFFFS = wFlags & 0x0003;
    uint8_t cSBHUFFDS = (wFlags >> 2) & 0x0003;
    uint8_t cSBHUFFDT = (wFlags >> 4) & 0x0003;
    uint8_t cSBHUFFRDW = (wFlags >> 6) & 0x0003;
    uint8_t cSBHUFFRDH = (wFlags >> 8) & 0x0003;
    uint8_t cSBHUFFRDX = (wFlags >> 10) & 0x0003;
    uint8_t cSBHUFFRDY = (wFlags >> 12) & 0x0003;
    uint8_t cSBHUFFRSIZE = (wFlags >> 14) & 0x0001;
    if (cSBHUFFFS == 2 || cSBHUFFRDW == 2 || cSBHUFFRDH == 2 ||
        cSBHUFFRDX == 2 || cSBHUFFRDY == 2) {
      return JBig2_Result::kFailure;
    }
    int32_t nIndex = 0;
    if (cSBHUFFFS == 0) {
      pTRD->SBHUFFFS = GetHuffmanTable(6);
    } else if (cSBHUFFFS == 1) {
      pTRD->SBHUFFFS = GetHuffmanTable(7);
    } else {
      CJBig2_Segment* pSeg =
          FindReferredTableSegmentByIndex(pSegment, nIndex++);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }
      pTRD->SBHUFFFS = pSeg->huffman_table_.get();
    }
    if (cSBHUFFDS == 0) {
      pTRD->SBHUFFDS = GetHuffmanTable(8);
    } else if (cSBHUFFDS == 1) {
      pTRD->SBHUFFDS = GetHuffmanTable(9);
    } else if (cSBHUFFDS == 2) {
      pTRD->SBHUFFDS = GetHuffmanTable(10);
    } else {
      CJBig2_Segment* pSeg =
          FindReferredTableSegmentByIndex(pSegment, nIndex++);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }
      pTRD->SBHUFFDS = pSeg->huffman_table_.get();
    }
    if (cSBHUFFDT == 0) {
      pTRD->SBHUFFDT = GetHuffmanTable(11);
    } else if (cSBHUFFDT == 1) {
      pTRD->SBHUFFDT = GetHuffmanTable(12);
    } else if (cSBHUFFDT == 2) {
      pTRD->SBHUFFDT = GetHuffmanTable(13);
    } else {
      CJBig2_Segment* pSeg =
          FindReferredTableSegmentByIndex(pSegment, nIndex++);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }
      pTRD->SBHUFFDT = pSeg->huffman_table_.get();
    }
    if (cSBHUFFRDW == 0) {
      pTRD->SBHUFFRDW = GetHuffmanTable(14);
    } else if (cSBHUFFRDW == 1) {
      pTRD->SBHUFFRDW = GetHuffmanTable(15);
    } else {
      CJBig2_Segment* pSeg =
          FindReferredTableSegmentByIndex(pSegment, nIndex++);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }
      pTRD->SBHUFFRDW = pSeg->huffman_table_.get();
    }
    if (cSBHUFFRDH == 0) {
      pTRD->SBHUFFRDH = GetHuffmanTable(14);
    } else if (cSBHUFFRDH == 1) {
      pTRD->SBHUFFRDH = GetHuffmanTable(15);
    } else {
      CJBig2_Segment* pSeg =
          FindReferredTableSegmentByIndex(pSegment, nIndex++);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }
      pTRD->SBHUFFRDH = pSeg->huffman_table_.get();
    }
    if (cSBHUFFRDX == 0) {
      pTRD->SBHUFFRDX = GetHuffmanTable(14);
    } else if (cSBHUFFRDX == 1) {
      pTRD->SBHUFFRDX = GetHuffmanTable(15);
    } else {
      CJBig2_Segment* pSeg =
          FindReferredTableSegmentByIndex(pSegment, nIndex++);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }
      pTRD->SBHUFFRDX = pSeg->huffman_table_.get();
    }
    if (cSBHUFFRDY == 0) {
      pTRD->SBHUFFRDY = GetHuffmanTable(14);
    } else if (cSBHUFFRDY == 1) {
      pTRD->SBHUFFRDY = GetHuffmanTable(15);
    } else {
      CJBig2_Segment* pSeg =
          FindReferredTableSegmentByIndex(pSegment, nIndex++);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }
      pTRD->SBHUFFRDY = pSeg->huffman_table_.get();
    }
    if (cSBHUFFRSIZE == 0) {
      pTRD->SBHUFFRSIZE = GetHuffmanTable(1);
    } else {
      CJBig2_Segment* pSeg =
          FindReferredTableSegmentByIndex(pSegment, nIndex++);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }
      pTRD->SBHUFFRSIZE = pSeg->huffman_table_.get();
    }
  }
  FixedSizeDataVector<JBig2ArithCtx> grContexts;
  if (pTRD->SBREFINE) {
    const size_t size = GetRefAggContextSize(pTRD->SBRTEMPLATE);
    grContexts = FixedSizeDataVector<JBig2ArithCtx>::Zeroed(size);
  }
  pSegment->result_type_ = JBIG2_IMAGE_POINTER;
  if (pTRD->SBHUFF) {
    pSegment->image_ = pTRD->DecodeHuffman(stream_.get(), grContexts);
    if (!pSegment->image_) {
      return JBig2_Result::kFailure;
    }
    stream_->alignByte();
  } else {
    auto pArithDecoder = std::make_unique<CJBig2_ArithDecoder>(stream_.get());
    pSegment->image_ =
        pTRD->DecodeArith(pArithDecoder.get(), grContexts, nullptr);
    if (!pSegment->image_) {
      return JBig2_Result::kFailure;
    }
    stream_->alignByte();
    stream_->addOffset(2);
  }
  if (pSegment->flags_.s.type != 4) {
    if (!buf_specified_) {
      const auto& pPageInfo = page_info_list_.back();
      if (pPageInfo->is_striped_ && ri.y + ri.height > page_->height()) {
        page_->Expand(ri.y + ri.height, pPageInfo->default_pixel_value_);
      }
    }
    page_->ComposeFrom(ri.x, ri.y, pSegment->image_.get(),
                       GetRegionInfoComposeOp(ri));
    pSegment->image_.reset();
  }
  return JBig2_Result::kSuccess;
}

JBig2_Result CJBig2_Context::ParsePatternDict(CJBig2_Segment* pSegment,
                                              PauseIndicatorIface* pPause) {
  uint8_t cFlags;
  auto pPDD = std::make_unique<CJBig2_PDDProc>();
  if (stream_->read1Byte(&cFlags) != 0 ||
      stream_->read1Byte(&pPDD->HDPW) != 0 ||
      stream_->read1Byte(&pPDD->HDPH) != 0 ||
      stream_->readInteger(&pPDD->GRAYMAX) != 0) {
    return JBig2_Result::kFailure;
  }
  if (pPDD->GRAYMAX > kJBig2MaxPatternIndex) {
    return JBig2_Result::kFailure;
  }

  pPDD->HDMMR = cFlags & 0x01;
  pPDD->HDTEMPLATE = (cFlags >> 1) & 0x03;
  pSegment->result_type_ = JBIG2_PATTERN_DICT_POINTER;
  if (pPDD->HDMMR) {
    pSegment->pattern_dict_ = pPDD->DecodeMMR(stream_.get());
    if (!pSegment->pattern_dict_) {
      return JBig2_Result::kFailure;
    }
    stream_->alignByte();
  } else {
    const size_t size = GetHuffContextSize(pPDD->HDTEMPLATE);
    auto gbContexts = FixedSizeDataVector<JBig2ArithCtx>::Zeroed(size);
    auto pArithDecoder = std::make_unique<CJBig2_ArithDecoder>(stream_.get());
    pSegment->pattern_dict_ =
        pPDD->DecodeArith(pArithDecoder.get(), gbContexts, pPause);
    if (!pSegment->pattern_dict_) {
      return JBig2_Result::kFailure;
    }

    stream_->alignByte();
    stream_->addOffset(2);
  }
  return JBig2_Result::kSuccess;
}

JBig2_Result CJBig2_Context::ParseHalftoneRegion(CJBig2_Segment* pSegment,
                                                 PauseIndicatorIface* pPause) {
  uint8_t cFlags;
  JBig2RegionInfo ri;
  auto pHRD = std::make_unique<CJBig2_HTRDProc>();
  if (ParseRegionInfo(&ri) != JBig2_Result::kSuccess ||
      stream_->read1Byte(&cFlags) != 0 ||
      stream_->readInteger(&pHRD->HGW) != 0 ||
      stream_->readInteger(&pHRD->HGH) != 0 ||
      stream_->readInteger((uint32_t*)&pHRD->HGX) != 0 ||
      stream_->readInteger((uint32_t*)&pHRD->HGY) != 0 ||
      stream_->readShortInteger(&pHRD->HRX) != 0 ||
      stream_->readShortInteger(&pHRD->HRY) != 0) {
    return JBig2_Result::kFailure;
  }

  if (!CJBig2_Image::IsValidImageSize(pHRD->HGW, pHRD->HGH)) {
    return JBig2_Result::kFailure;
  }

  if (!CJBig2_Image::IsValidImageSize(ri.width, ri.height)) {
    return JBig2_Result::kFailure;
  }

  pHRD->HBW = ri.width;
  pHRD->HBH = ri.height;
  pHRD->HMMR = cFlags & 0x01;
  pHRD->HTEMPLATE = (cFlags >> 1) & 0x03;
  pHRD->HENABLESKIP = (cFlags >> 3) & 0x01;
  pHRD->HCOMBOP = (JBig2ComposeOp)((cFlags >> 4) & 0x07);
  pHRD->HDEFPIXEL = (cFlags >> 7) & 0x01;
  if (pSegment->referred_to_segment_count_ != 1) {
    return JBig2_Result::kFailure;
  }

  CJBig2_Segment* pSeg =
      FindSegmentByNumber(pSegment->referred_to_segment_numbers_[0]);
  if (!pSeg || (pSeg->flags_.s.type != 16)) {
    return JBig2_Result::kFailure;
  }

  const CJBig2_PatternDict* pPatternDict = pSeg->pattern_dict_.get();
  if (!pPatternDict || (pPatternDict->NUMPATS == 0)) {
    return JBig2_Result::kFailure;
  }

  pHRD->HNUMPATS = pPatternDict->NUMPATS;
  pHRD->HPATS = &pPatternDict->HDPATS;
  pHRD->HPW = pPatternDict->HDPATS[0]->width();
  pHRD->HPH = pPatternDict->HDPATS[0]->height();
  pSegment->result_type_ = JBIG2_IMAGE_POINTER;
  if (pHRD->HMMR) {
    pSegment->image_ = pHRD->DecodeMMR(stream_.get());
    if (!pSegment->image_) {
      return JBig2_Result::kFailure;
    }
    stream_->alignByte();
  } else {
    const size_t size = GetHuffContextSize(pHRD->HTEMPLATE);
    auto gbContexts = FixedSizeDataVector<JBig2ArithCtx>::Zeroed(size);
    auto pArithDecoder = std::make_unique<CJBig2_ArithDecoder>(stream_.get());
    pSegment->image_ =
        pHRD->DecodeArith(pArithDecoder.get(), gbContexts, pPause);
    if (!pSegment->image_) {
      return JBig2_Result::kFailure;
    }

    stream_->alignByte();
    stream_->addOffset(2);
  }
  if (pSegment->flags_.s.type != 20) {
    if (!buf_specified_) {
      const auto& pPageInfo = page_info_list_.back();
      if (pPageInfo->is_striped_ && ri.y + ri.height > page_->height()) {
        page_->Expand(ri.y + ri.height, pPageInfo->default_pixel_value_);
      }
    }
    page_->ComposeFrom(ri.x, ri.y, pSegment->image_.get(),
                       GetRegionInfoComposeOp(ri));
    pSegment->image_.reset();
  }
  return JBig2_Result::kSuccess;
}

JBig2_Result CJBig2_Context::ParseGenericRegion(CJBig2_Segment* pSegment,
                                                PauseIndicatorIface* pPause) {
  if (!grd_) {
    auto pGRD = std::make_unique<CJBig2_GRDProc>();
    uint8_t cFlags;
    if (ParseRegionInfo(&ri_) != JBig2_Result::kSuccess ||
        stream_->read1Byte(&cFlags) != 0) {
      return JBig2_Result::kFailure;
    }
    if (ri_.height < 0 || ri_.width < 0) {
      return JBig2_Result::kFailure;
    }
    pGRD->GBW = ri_.width;
    pGRD->GBH = ri_.height;
    pGRD->MMR = cFlags & 0x01;
    pGRD->GBTEMPLATE = (cFlags >> 1) & 0x03;
    pGRD->TPGDON = (cFlags >> 3) & 0x01;
    if (!pGRD->MMR) {
      if (pGRD->GBTEMPLATE == 0) {
        for (int32_t i = 0; i < 8; ++i) {
          if (stream_->read1Byte((uint8_t*)&pGRD->GBAT[i]) != 0) {
            return JBig2_Result::kFailure;
          }
        }
      } else {
        for (int32_t i = 0; i < 2; ++i) {
          if (stream_->read1Byte((uint8_t*)&pGRD->GBAT[i]) != 0) {
            return JBig2_Result::kFailure;
          }
        }
      }
    }
    pGRD->USESKIP = false;
    grd_ = std::move(pGRD);
  }
  pSegment->result_type_ = JBIG2_IMAGE_POINTER;
  if (grd_->MMR) {
    grd_->StartDecodeMMR(&pSegment->image_, stream_.get());
    if (!pSegment->image_) {
      grd_.reset();
      return JBig2_Result::kFailure;
    }
    stream_->alignByte();
  } else {
    if (gb_contexts_.empty()) {
      gb_contexts_.resize(GetHuffContextSize(grd_->GBTEMPLATE));
    }

    bool bStart = !arith_decoder_;
    if (bStart) {
      arith_decoder_ = std::make_unique<CJBig2_ArithDecoder>(stream_.get());
    }
    {
      // |state.gbContexts| can't exist when gb_contexts_.clear() called below.
      CJBig2_GRDProc::ProgressiveArithDecodeState state;
      state.pImage = &pSegment->image_;
      state.pArithDecoder = arith_decoder_.get();
      state.gbContexts = gb_contexts_;
      state.pPause = pPause;
      processing_status_ = bStart ? grd_->StartDecodeArith(&state)
                                  : grd_->ContinueDecode(&state);
      if (processing_status_ == FXCODEC_STATUS::kDecodeToBeContinued) {
        if (pSegment->flags_.s.type != 36) {
          if (!buf_specified_) {
            const auto& pPageInfo = page_info_list_.back();
            if (pPageInfo->is_striped_ &&
                ri_.y + ri_.height > page_->height()) {
              page_->Expand(ri_.y + ri_.height,
                            pPageInfo->default_pixel_value_);
            }
          }
          const FX_RECT& rect = grd_->GetReplaceRect();
          page_->ComposeFromWithRect(ri_.x + rect.left, ri_.y + rect.top,
                                     pSegment->image_.get(), rect,
                                     GetRegionInfoComposeOp(ri_));
        }
        return JBig2_Result::kSuccess;
      }
    }
    arith_decoder_.reset();
    gb_contexts_.clear();
    if (!pSegment->image_) {
      processing_status_ = FXCODEC_STATUS::kError;
      grd_.reset();
      return JBig2_Result::kFailure;
    }
    stream_->alignByte();
    stream_->addOffset(2);
  }
  if (pSegment->flags_.s.type != 36) {
    if (!buf_specified_) {
      JBig2PageInfo* pPageInfo = page_info_list_.back().get();
      if (pPageInfo->is_striped_ && ri_.y + ri_.height > page_->height()) {
        page_->Expand(ri_.y + ri_.height, pPageInfo->default_pixel_value_);
      }
    }
    const FX_RECT& rect = grd_->GetReplaceRect();
    page_->ComposeFromWithRect(ri_.x + rect.left, ri_.y + rect.top,
                               pSegment->image_.get(), rect,
                               GetRegionInfoComposeOp(ri_));
    pSegment->image_.reset();
  }
  grd_.reset();
  return JBig2_Result::kSuccess;
}

JBig2_Result CJBig2_Context::ParseGenericRefinementRegion(
    CJBig2_Segment* pSegment) {
  JBig2RegionInfo ri;
  uint8_t cFlags;
  if (ParseRegionInfo(&ri) != JBig2_Result::kSuccess ||
      stream_->read1Byte(&cFlags) != 0) {
    return JBig2_Result::kFailure;
  }
  if (!CJBig2_Image::IsValidImageSize(ri.width, ri.height)) {
    return JBig2_Result::kFailure;
  }

  auto pGRRD = std::make_unique<CJBig2_GRRDProc>();
  pGRRD->GRW = ri.width;
  pGRRD->GRH = ri.height;
  pGRRD->GRTEMPLATE = !!(cFlags & 0x01);
  pGRRD->TPGRON = (cFlags >> 1) & 0x01;
  if (!pGRRD->GRTEMPLATE) {
    for (int32_t i = 0; i < 4; ++i) {
      if (stream_->read1Byte((uint8_t*)&pGRRD->GRAT[i]) != 0) {
        return JBig2_Result::kFailure;
      }
    }
  }
  CJBig2_Segment* pSeg = nullptr;
  if (pSegment->referred_to_segment_count_ > 0) {
    int32_t i;
    for (i = 0; i < pSegment->referred_to_segment_count_; ++i) {
      pSeg = FindSegmentByNumber(pSegment->referred_to_segment_numbers_[0]);
      if (!pSeg) {
        return JBig2_Result::kFailure;
      }

      if (pSeg->flags_.s.type == 4 || pSeg->flags_.s.type == 20 ||
          pSeg->flags_.s.type == 36 || pSeg->flags_.s.type == 40) {
        break;
      }
    }
    if (i >= pSegment->referred_to_segment_count_) {
      return JBig2_Result::kFailure;
    }

    pGRRD->GRREFERENCE = pSeg->image_.get();
  } else {
    pGRRD->GRREFERENCE = page_.get();
  }
  pGRRD->GRREFERENCEDX = 0;
  pGRRD->GRREFERENCEDY = 0;
  const size_t size = GetRefAggContextSize(pGRRD->GRTEMPLATE);
  auto grContexts = FixedSizeDataVector<JBig2ArithCtx>::Zeroed(size);
  auto pArithDecoder = std::make_unique<CJBig2_ArithDecoder>(stream_.get());
  pSegment->result_type_ = JBIG2_IMAGE_POINTER;
  pSegment->image_ = pGRRD->Decode(pArithDecoder.get(), grContexts);
  if (!pSegment->image_) {
    return JBig2_Result::kFailure;
  }

  stream_->alignByte();
  stream_->addOffset(2);
  if (pSegment->flags_.s.type != 40) {
    if (!buf_specified_) {
      JBig2PageInfo* pPageInfo = page_info_list_.back().get();
      if (pPageInfo->is_striped_ && ri.y + ri.height > page_->height()) {
        page_->Expand(ri.y + ri.height, pPageInfo->default_pixel_value_);
      }
    }
    page_->ComposeFrom(ri.x, ri.y, pSegment->image_.get(),
                       GetRegionInfoComposeOp(ri));
    pSegment->image_.reset();
  }
  return JBig2_Result::kSuccess;
}

JBig2_Result CJBig2_Context::ParseTable(CJBig2_Segment* pSegment) {
  pSegment->result_type_ = JBIG2_HUFFMAN_TABLE_POINTER;
  pSegment->huffman_table_.reset();
  auto pHuff = std::make_unique<CJBig2_HuffmanTable>(stream_.get());
  if (!pHuff->IsOK()) {
    return JBig2_Result::kFailure;
  }

  pSegment->huffman_table_ = std::move(pHuff);
  stream_->alignByte();
  return JBig2_Result::kSuccess;
}

JBig2_Result CJBig2_Context::ParseRegionInfo(JBig2RegionInfo* pRI) {
  if (stream_->readInteger((uint32_t*)&pRI->width) != 0 ||
      stream_->readInteger((uint32_t*)&pRI->height) != 0 ||
      stream_->readInteger((uint32_t*)&pRI->x) != 0 ||
      stream_->readInteger((uint32_t*)&pRI->y) != 0 ||
      stream_->read1Byte(&pRI->flags) != 0) {
    return JBig2_Result::kFailure;
  }

  if (reject_large_regions_when_fuzzing_ &&
      (pRI->width > 4096 || pRI->height > 4096)) {
    return JBig2_Result::kFailure;
  }

  return JBig2_Result::kSuccess;
}

std::vector<JBig2HuffmanCode> CJBig2_Context::DecodeSymbolIDHuffmanTable(
    uint32_t SBNUMSYMS) {
  const size_t kRunCodesSize = 35;
  std::array<JBig2HuffmanCode, kRunCodesSize> huffman_codes;
  for (size_t i = 0; i < kRunCodesSize; ++i) {
    if (stream_->readNBits(4, &huffman_codes[i].codelen) != 0) {
      return std::vector<JBig2HuffmanCode>();
    }
  }
  if (!HuffmanAssignCode(huffman_codes)) {
    return std::vector<JBig2HuffmanCode>();
  }

  std::vector<JBig2HuffmanCode> SBSYMCODES(SBNUMSYMS);
  int32_t run = 0;
  int32_t i = 0;
  while (i < static_cast<int>(SBNUMSYMS)) {
    size_t j;
    FX_SAFE_INT32 nSafeVal = 0;
    int32_t nBits = 0;
    uint32_t nTemp;
    while (true) {
      if (stream_->read1Bit(&nTemp) != 0) {
        return std::vector<JBig2HuffmanCode>();
      }

      nSafeVal <<= 1;
      if (!nSafeVal.IsValid()) {
        return std::vector<JBig2HuffmanCode>();
      }

      nSafeVal |= nTemp;
      ++nBits;
      const int32_t nVal = nSafeVal.ValueOrDie();
      for (j = 0; j < kRunCodesSize; ++j) {
        if (nBits == huffman_codes[j].codelen &&
            nVal == huffman_codes[j].code) {
          break;
        }
      }
      if (j < kRunCodesSize) {
        break;
      }
    }
    int32_t runcode = static_cast<int32_t>(j);
    if (runcode < 32) {
      SBSYMCODES[i].codelen = runcode;
      run = 0;
    } else if (runcode == 32) {
      if (stream_->readNBits(2, &nTemp) != 0) {
        return std::vector<JBig2HuffmanCode>();
      }
      run = nTemp + 3;
    } else if (runcode == 33) {
      if (stream_->readNBits(3, &nTemp) != 0) {
        return std::vector<JBig2HuffmanCode>();
      }
      run = nTemp + 3;
    } else if (runcode == 34) {
      if (stream_->readNBits(7, &nTemp) != 0) {
        return std::vector<JBig2HuffmanCode>();
      }
      run = nTemp + 11;
    }
    if (run > 0) {
      if (i + run > (int)SBNUMSYMS) {
        return std::vector<JBig2HuffmanCode>();
      }
      for (int32_t k = 0; k < run; ++k) {
        if (runcode == 32 && i > 0) {
          SBSYMCODES[i + k].codelen = SBSYMCODES[i - 1].codelen;
        } else {
          SBSYMCODES[i + k].codelen = 0;
        }
      }
      i += run;
    } else {
      ++i;
    }
  }
  if (!HuffmanAssignCode(SBSYMCODES)) {
    return std::vector<JBig2HuffmanCode>();
  }
  return SBSYMCODES;
}

const CJBig2_HuffmanTable* CJBig2_Context::GetHuffmanTable(size_t idx) {
  DCHECK(idx > 0);
  DCHECK(idx < CJBig2_HuffmanTable::kNumHuffmanTables);
  if (!huffman_tables_[idx].get()) {
    huffman_tables_[idx] = std::make_unique<CJBig2_HuffmanTable>(idx);
  }
  return huffman_tables_[idx].get();
}

// static
bool CJBig2_Context::HuffmanAssignCode(
    pdfium::span<JBig2HuffmanCode> symcodes) {
  int lenmax = 0;
  for (const auto& symcode : symcodes) {
    lenmax = std::max(symcode.codelen, lenmax);
  }
  std::vector<int> lencounts(lenmax + 1);
  std::vector<int> firstcodes(lenmax + 1);
  for (const auto& symcode : symcodes) {
    ++lencounts[symcode.codelen];
  }
  lencounts[0] = 0;
  for (int i = 1; i <= lenmax; ++i) {
    FX_SAFE_INT32 shifted = firstcodes[i - 1];
    shifted += lencounts[i - 1];
    shifted <<= 1;
    if (!shifted.IsValid()) {
      return false;
    }
    firstcodes[i] = shifted.ValueOrDie();
    int curcode = firstcodes[i];
    for (auto& symcode : symcodes) {
      if (symcode.codelen == i) {
        symcode.code = curcode++;
      }
    }
  }
  return true;
}
