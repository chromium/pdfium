// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_sampledfunc.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fxcrt/cfx_fixedbufgrow.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"

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

CPDF_SampledFunc::~CPDF_SampledFunc() {}

bool CPDF_SampledFunc::v_Init(CPDF_Object* pObj) {
  CPDF_Stream* pStream = pObj->AsStream();
  if (!pStream)
    return false;

  CPDF_Dictionary* pDict = pStream->GetDict();
  CPDF_Array* pSize = pDict->GetArrayFor("Size");
  CPDF_Array* pEncode = pDict->GetArrayFor("Encode");
  CPDF_Array* pDecode = pDict->GetArrayFor("Decode");
  m_nBitsPerSample = pDict->GetIntegerFor("BitsPerSample");
  if (!IsValidBitsPerSample(m_nBitsPerSample))
    return false;

  m_SampleMax = 0xffffffff >> (32 - m_nBitsPerSample);
  m_pSampleStream = pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
  m_pSampleStream->LoadAllDataFiltered();
  FX_SAFE_UINT32 nTotalSampleBits = 1;
  m_EncodeInfo.resize(m_nInputs);
  for (uint32_t i = 0; i < m_nInputs; i++) {
    m_EncodeInfo[i].sizes = pSize ? pSize->GetIntegerAt(i) : 0;
    if (!pSize && i == 0)
      m_EncodeInfo[i].sizes = pDict->GetIntegerFor("Size");
    nTotalSampleBits *= m_EncodeInfo[i].sizes;
    if (pEncode) {
      m_EncodeInfo[i].encode_min = pEncode->GetFloatAt(i * 2);
      m_EncodeInfo[i].encode_max = pEncode->GetFloatAt(i * 2 + 1);
    } else {
      m_EncodeInfo[i].encode_min = 0;
      m_EncodeInfo[i].encode_max =
          m_EncodeInfo[i].sizes == 1 ? 1 : (float)m_EncodeInfo[i].sizes - 1;
    }
  }
  nTotalSampleBits *= m_nBitsPerSample;
  nTotalSampleBits *= m_nOutputs;
  FX_SAFE_UINT32 nTotalSampleBytes = nTotalSampleBits;
  nTotalSampleBytes += 7;
  nTotalSampleBytes /= 8;
  if (!nTotalSampleBytes.IsValid() || nTotalSampleBytes.ValueOrDie() == 0 ||
      nTotalSampleBytes.ValueOrDie() > m_pSampleStream->GetSize()) {
    return false;
  }
  m_DecodeInfo.resize(m_nOutputs);
  for (uint32_t i = 0; i < m_nOutputs; i++) {
    if (pDecode) {
      m_DecodeInfo[i].decode_min = pDecode->GetFloatAt(2 * i);
      m_DecodeInfo[i].decode_max = pDecode->GetFloatAt(2 * i + 1);
    } else {
      m_DecodeInfo[i].decode_min = m_pRanges[i * 2];
      m_DecodeInfo[i].decode_max = m_pRanges[i * 2 + 1];
    }
  }
  return true;
}

bool CPDF_SampledFunc::v_Call(float* inputs, float* results) const {
  int pos = 0;
  CFX_FixedBufGrow<float, 16> encoded_input_buf(m_nInputs);
  float* encoded_input = encoded_input_buf;
  CFX_FixedBufGrow<uint32_t, 32> int_buf(m_nInputs * 2);
  uint32_t* index = int_buf;
  uint32_t* blocksize = index + m_nInputs;
  for (uint32_t i = 0; i < m_nInputs; i++) {
    if (i == 0)
      blocksize[i] = 1;
    else
      blocksize[i] = blocksize[i - 1] * m_EncodeInfo[i - 1].sizes;
    encoded_input[i] =
        Interpolate(inputs[i], m_pDomains[i * 2], m_pDomains[i * 2 + 1],
                    m_EncodeInfo[i].encode_min, m_EncodeInfo[i].encode_max);
    index[i] = pdfium::clamp(static_cast<uint32_t>(encoded_input[i]), 0U,
                             m_EncodeInfo[i].sizes - 1);
    pos += index[i] * blocksize[i];
  }
  FX_SAFE_INT32 bits_to_output = m_nOutputs;
  bits_to_output *= m_nBitsPerSample;
  if (!bits_to_output.IsValid())
    return false;

  FX_SAFE_INT32 bitpos = pos;
  bitpos *= bits_to_output.ValueOrDie();
  if (!bitpos.IsValid())
    return false;

  FX_SAFE_INT32 range_check = bitpos;
  range_check += bits_to_output.ValueOrDie();
  if (!range_check.IsValid())
    return false;

  const uint8_t* pSampleData = m_pSampleStream->GetData();
  if (!pSampleData)
    return false;

  for (uint32_t j = 0; j < m_nOutputs; j++, bitpos += m_nBitsPerSample) {
    uint32_t sample =
        GetBits32(pSampleData, bitpos.ValueOrDie(), m_nBitsPerSample);
    float encoded = (float)sample;
    for (uint32_t i = 0; i < m_nInputs; i++) {
      if (index[i] == m_EncodeInfo[i].sizes - 1) {
        if (index[i] == 0)
          encoded = encoded_input[i] * (float)sample;
      } else {
        FX_SAFE_INT32 bitpos2 = blocksize[i];
        bitpos2 += pos;
        bitpos2 *= m_nOutputs;
        bitpos2 += j;
        bitpos2 *= m_nBitsPerSample;
        if (!bitpos2.IsValid())
          return false;
        uint32_t sample1 =
            GetBits32(pSampleData, bitpos2.ValueOrDie(), m_nBitsPerSample);
        encoded +=
            (encoded_input[i] - index[i]) * ((float)sample1 - (float)sample);
      }
    }
    results[j] =
        Interpolate(encoded, 0, (float)m_SampleMax, m_DecodeInfo[j].decode_min,
                    m_DecodeInfo[j].decode_max);
  }
  return true;
}
