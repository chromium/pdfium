// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/cmaps/fpdf_cmaps.h"

#include <algorithm>

#include "core/fxcrt/check.h"
#include "core/fxcrt/span.h"

namespace fxcmap {

namespace {

struct SingleCmap {
  uint16_t code;
  uint16_t cid;
};

struct RangeCmap {
  uint16_t low;
  uint16_t high;
  uint16_t cid;
};

pdfium::span<const SingleCmap> GetSingleCmapSpan(const CMap* cmap) {
  // SAFETY: `CMap` uses manually audited constexpr data.
  return UNSAFE_BUFFERS(pdfium::span(
      reinterpret_cast<const SingleCmap*>(cmap->word_map_), cmap->word_count_));
}

pdfium::span<const RangeCmap> GetRangeCmapSpan(const CMap* cmap) {
  // SAFETY: `CMap` uses manually audited constexpr data.
  return UNSAFE_BUFFERS(pdfium::span(
      reinterpret_cast<const RangeCmap*>(cmap->word_map_), cmap->word_count_));
}

pdfium::span<const DWordCIDMap> GetDWordCIDMapCmapSpan(const CMap* cmap) {
  // SAFETY: `CMap` uses manually audited constexpr data.
  return UNSAFE_BUFFERS(pdfium::span(cmap->dword_map_, cmap->dword_count_));
}

const CMap* FindNextCMap(const CMap* cmap) {
  if (cmap->use_offset_ == 0) {
    return nullptr;
  }

  // SAFETY: `CMap` uses manually audited constexpr data.
  return UNSAFE_BUFFERS(cmap + cmap->use_offset_);
}

uint16_t CIDFromCharCodeForDword(const CMap* cmap, uint32_t charcode) {
  const uint16_t loword = static_cast<uint16_t>(charcode);
  while (cmap) {
    if (cmap->dword_map_) {
      auto dword_span = GetDWordCIDMapCmapSpan(cmap);
      const auto* found = std::lower_bound(
          dword_span.begin(), dword_span.end(), charcode,
          [](const DWordCIDMap& element, uint32_t charcode) {
            uint16_t hiword = static_cast<uint16_t>(charcode >> 16);
            if (element.hi_word_ != hiword) {
              return element.hi_word_ < hiword;
            }
            return element.lo_word_high_ < static_cast<uint16_t>(charcode);
          });
      if (found != dword_span.end() && loword >= found->lo_word_low_ &&
          loword <= found->lo_word_high_) {
        return found->cid_ + loword - found->lo_word_low_;
      }
    }
    cmap = FindNextCMap(cmap);
  }
  return 0;
}

}  // namespace

uint16_t CIDFromCharCode(const CMap* cmap, uint32_t charcode) {
  CHECK(cmap);
  if (charcode >> 16) {
    return CIDFromCharCodeForDword(cmap, charcode);
  }

  const uint16_t loword = static_cast<uint16_t>(charcode);
  while (cmap) {
    CHECK(cmap->word_map_);
    switch (cmap->word_map_type_) {
      case CMap::Type::kSingle: {
        auto single_span = GetSingleCmapSpan(cmap);
        const auto* found =
            std::lower_bound(single_span.begin(), single_span.end(), loword,
                             [](const SingleCmap& element, uint16_t code) {
                               return element.code < code;
                             });
        if (found != single_span.end() && found->code == loword) {
          return found->cid;
        }
        break;
      }
      case CMap::Type::kRange: {
        auto range_span = GetRangeCmapSpan(cmap);
        const auto* found =
            std::lower_bound(range_span.begin(), range_span.end(), loword,
                             [](const RangeCmap& element, uint16_t code) {
                               return element.high < code;
                             });
        if (found != range_span.end() && loword >= found->low &&
            loword <= found->high) {
          return found->cid + loword - found->low;
        }
        break;
      }
    }
    cmap = FindNextCMap(cmap);
  }

  return 0;
}

uint32_t CharCodeFromCID(const CMap* cmap, uint16_t cid) {
  // TODO(dsinclair): This should be checking both cmap->word_map_ and
  // cmap->dword_map_. There was a second while() but it was never reached as
  // the first always returns. Investigate and determine how this should
  // really be working. (https://codereview.chromium.org/2235743003 removed the
  // second while loop.)
  CHECK(cmap);
  CHECK(cmap->word_map_);
  while (cmap) {
    switch (cmap->word_map_type_) {
      case CMap::Type::kSingle: {
        for (const auto& single : GetSingleCmapSpan(cmap)) {
          if (single.cid == cid) {
            return single.code;
          }
        }
        break;
      }
      case CMap::Type::kRange: {
        for (const auto& range : GetRangeCmapSpan(cmap)) {
          if (cid >= range.cid && cid <= range.cid + range.high - range.low) {
            return range.low + cid - range.cid;
          }
        }
        break;
      }
    }
    cmap = FindNextCMap(cmap);
  }
  return 0;
}

}  // namespace fxcmap
