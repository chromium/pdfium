// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_FLATEENCODER_H_
#define CORE_FPDFAPI_PARSER_CPDF_FLATEENCODER_H_

#include <stdint.h>

#include <memory>

#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/retain_ptr.h"
#include "third_party/abseil-cpp/absl/types/variant.h"
#include "third_party/base/span.h"

class CPDF_Dictionary;
class CPDF_Stream;
class CPDF_StreamAcc;

class CPDF_FlateEncoder {
 public:
  CPDF_FlateEncoder(const CPDF_Stream* pStream, bool bFlateEncode);
  ~CPDF_FlateEncoder();

  void CloneDict();
  CPDF_Dictionary* GetClonedDict();

  // Returns |m_pClonedDict| if it is valid. Otherwise returns |m_pDict|.
  const CPDF_Dictionary* GetDict() const;

  pdfium::span<const uint8_t> GetSpan() const;

 private:
  // TODO(crbug.com/pdfium/1872): Replace with fxcrt::DataVector.
  struct OwnedData {
    OwnedData(std::unique_ptr<uint8_t, FxFreeDeleter> buffer, uint32_t size);
    ~OwnedData();

    std::unique_ptr<uint8_t, FxFreeDeleter> buffer;
    uint32_t size;
  };

  bool is_owned() const { return m_Data.index() == 1; }

  RetainPtr<CPDF_StreamAcc> m_pAcc;

  absl::variant<pdfium::span<const uint8_t>, OwnedData> m_Data;

  // Only one of these two pointers is valid at any time.
  RetainPtr<const CPDF_Dictionary> m_pDict;
  RetainPtr<CPDF_Dictionary> m_pClonedDict;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_FLATEENCODER_H_
