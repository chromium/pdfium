// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_sampledfunc.h"

#include <algorithm>
#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/cfx_bitstream.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/abseil-cpp/absl/container/inlined_vector.h"

namespace {

// See PDF Reference 1.7, page 170, table 3.36.
bool IsValidBitsPerSample(uint32_t x) {
  switch (x) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 12:
    case 16:
    case 24:
    case 32:
      return true;
    default:
      return false;
  }
}

}  // namespace

CPDF_SampledFunc::CPDF_SampledFunc() : CPDF_Function(Type::kType0Sampled) {}

CPDF_SampledFunc::~CPDF_SampledFunc() = default;

bool CPDF_SampledFunc::v_Init(const CPDF_Object* pObj, VisitedSet* pVisited) {
  RetainPtr<const CPDF_Stream> pStream(pObj->AsStream());
  if (!pStream) {
    return false;
  }

  RetainPtr<const CPDF_Dictionary> dict = pStream->GetDict();
  RetainPtr<const CPDF_Array> pSize = dict->GetArrayFor("Size");
  if (!pSize || pSize->IsEmpty()) {
    return false;
  }

  bits_per_sample_ = dict->GetIntegerFor("BitsPerSample");
  if (!IsValidBitsPerSample(bits_per_sample_)) {
    return false;
  }

  FX_SAFE_UINT32 nTotalSampleBits = bits_per_sample_;
  nTotalSampleBits *= outputs_;
  RetainPtr<const CPDF_Array> pEncode = dict->GetArrayFor("Encode");
  encode_info_.resize(inputs_);
  for (uint32_t i = 0; i < inputs_; i++) {
    int size = pSize->GetIntegerAt(i);
    if (size <= 0) {
      return false;
    }

    encode_info_[i].sizes = size;
    nTotalSampleBits *= encode_info_[i].sizes;
    if (pEncode) {
      encode_info_[i].encode_min = pEncode->GetFloatAt(i * 2);
      encode_info_[i].encode_max = pEncode->GetFloatAt(i * 2 + 1);
    } else {
      encode_info_[i].encode_min = 0;
      encode_info_[i].encode_max =
          encode_info_[i].sizes == 1 ? 1 : encode_info_[i].sizes - 1;
    }
  }
  FX_SAFE_UINT32 nTotalSampleBytes = (nTotalSampleBits + 7) / 8;
  if (!nTotalSampleBytes.IsValid() || nTotalSampleBytes.ValueOrDie() == 0) {
    return false;
  }

  sample_max_ = 0xffffffff >> (32 - bits_per_sample_);
  sample_stream_ = pdfium::MakeRetain<CPDF_StreamAcc>(std::move(pStream));
  sample_stream_->LoadAllDataFiltered();
  if (nTotalSampleBytes.ValueOrDie() > sample_stream_->GetSize()) {
    return false;
  }

  RetainPtr<const CPDF_Array> pDecode = dict->GetArrayFor("Decode");
  decode_info_.resize(outputs_);
  for (uint32_t i = 0; i < outputs_; i++) {
    if (pDecode) {
      decode_info_[i].decode_min = pDecode->GetFloatAt(2 * i);
      decode_info_[i].decode_max = pDecode->GetFloatAt(2 * i + 1);
    } else {
      decode_info_[i].decode_min = ranges_[i * 2];
      decode_info_[i].decode_max = ranges_[i * 2 + 1];
    }
  }
  return true;
}

bool CPDF_SampledFunc::v_Call(pdfium::span<const float> inputs,
                              pdfium::span<float> results) const {
  int pos = 0;
  absl::InlinedVector<float, 16, FxAllocAllocator<float>> encoded_input_buf(
      inputs_);
  absl::InlinedVector<uint32_t, 32, FxAllocAllocator<uint32_t>> int_buf(
      inputs_ * 2);
  UNSAFE_TODO({
    float* encoded_input = encoded_input_buf.data();
    uint32_t* index = int_buf.data();
    uint32_t* blocksize = index + inputs_;
    for (uint32_t i = 0; i < inputs_; i++) {
      if (i == 0) {
        blocksize[i] = 1;
      } else {
        blocksize[i] = blocksize[i - 1] * encode_info_[i - 1].sizes;
      }
      encoded_input[i] =
          Interpolate(inputs[i], domains_[i * 2], domains_[i * 2 + 1],
                      encode_info_[i].encode_min, encode_info_[i].encode_max);
      index[i] = std::clamp(static_cast<uint32_t>(encoded_input[i]), 0U,
                            encode_info_[i].sizes - 1);
      pos += index[i] * blocksize[i];
    }
    FX_SAFE_INT32 bits_to_output = outputs_;
    bits_to_output *= bits_per_sample_;
    if (!bits_to_output.IsValid()) {
      return false;
    }

    int bits_to_skip;
    {
      FX_SAFE_INT32 bitpos = pos;
      bitpos *= bits_to_output.ValueOrDie();
      bits_to_skip = bitpos.ValueOrDefault(-1);
      if (bits_to_skip < 0) {
        return false;
      }

      FX_SAFE_INT32 range_check = bitpos;
      range_check += bits_to_output.ValueOrDie();
      if (!range_check.IsValid()) {
        return false;
      }
    }

    pdfium::span<const uint8_t> pSampleData = sample_stream_->GetSpan();
    if (pSampleData.empty()) {
      return false;
    }

    CFX_BitStream bitstream(pSampleData);
    bitstream.SkipBits(bits_to_skip);
    for (uint32_t i = 0; i < outputs_; ++i) {
      uint32_t sample = bitstream.GetBits(bits_per_sample_);
      float encoded = sample;
      for (uint32_t j = 0; j < inputs_; ++j) {
        if (index[j] == encode_info_[j].sizes - 1) {
          if (index[j] == 0) {
            encoded = encoded_input[j] * sample;
          }
        } else {
          FX_SAFE_INT32 bitpos2 = blocksize[j];
          bitpos2 += pos;
          bitpos2 *= outputs_;
          bitpos2 += i;
          bitpos2 *= bits_per_sample_;
          int bits_to_skip2 = bitpos2.ValueOrDefault(-1);
          if (bits_to_skip2 < 0) {
            return false;
          }

          CFX_BitStream bitstream2(pSampleData);
          bitstream2.SkipBits(bits_to_skip2);
          float sample2 =
              static_cast<float>(bitstream2.GetBits(bits_per_sample_));
          encoded += (encoded_input[j] - index[j]) * (sample2 - sample);
        }
      }
      results[i] =
          Interpolate(encoded, 0, sample_max_, decode_info_[i].decode_min,
                      decode_info_[i].decode_max);
    }
  });
  return true;
}

#if defined(PDF_USE_SKIA)
RetainPtr<CPDF_StreamAcc> CPDF_SampledFunc::GetSampleStream() const {
  return sample_stream_;
}
#endif
