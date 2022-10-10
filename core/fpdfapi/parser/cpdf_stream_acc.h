// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_STREAM_ACC_H_
#define CORE_FPDFAPI_PARSER_CPDF_STREAM_ACC_H_

#include <stdint.h>

#include <memory>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/retain_ptr.h"
#include "third_party/abseil-cpp/absl/types/variant.h"
#include "third_party/base/span.h"

class CPDF_Dictionary;
class CPDF_Stream;

class CPDF_StreamAcc final : public Retainable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  CPDF_StreamAcc(const CPDF_StreamAcc&) = delete;
  CPDF_StreamAcc& operator=(const CPDF_StreamAcc&) = delete;

  void LoadAllDataFiltered();
  void LoadAllDataFilteredWithEstimatedSize(uint32_t estimated_size);
  void LoadAllDataImageAcc(uint32_t estimated_size);
  void LoadAllDataRaw();

  RetainPtr<const CPDF_Stream> GetStream() const;
  RetainPtr<const CPDF_Dictionary> GetImageParam() const;

  uint32_t GetSize() const;
  pdfium::span<const uint8_t> GetSpan() const;
  uint64_t KeyForCache() const;
  ByteString ComputeDigest() const;
  ByteString GetImageDecoder() const { return m_ImageDecoder; }
  std::unique_ptr<uint8_t, FxFreeDeleter> DetachData();

  int GetLength1ForTest() const;

 private:
  // TODO(crbug.com/pdfium/1872): Replace with fxcrt::DataVector.
  struct OwnedData {
    OwnedData(std::unique_ptr<uint8_t, FxFreeDeleter> buffer, uint32_t size);
    OwnedData(OwnedData&&);
    OwnedData& operator=(OwnedData&&);
    ~OwnedData();

    std::unique_ptr<uint8_t, FxFreeDeleter> buffer;
    uint32_t size;
  };

  explicit CPDF_StreamAcc(RetainPtr<const CPDF_Stream> pStream);
  ~CPDF_StreamAcc() override;

  void LoadAllData(bool bRawAccess, uint32_t estimated_size, bool bImageAcc);
  void ProcessRawData();
  void ProcessFilteredData(uint32_t estimated_size, bool bImageAcc);
  const uint8_t* GetData() const;

  // Reads the raw data from |m_pStream|, or return nullptr on failure.
  std::unique_ptr<uint8_t, FxFreeDeleter> ReadRawStream() const;

  bool is_owned() const { return m_Data.index() == 1; }

  ByteString m_ImageDecoder;
  RetainPtr<const CPDF_Dictionary> m_pImageParam;
  // Needs to outlive `m_Data` when the data is not owned.
  RetainPtr<const CPDF_Stream> const m_pStream;
  absl::variant<pdfium::span<const uint8_t>, OwnedData> m_Data;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_STREAM_ACC_H_
