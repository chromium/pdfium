// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_STREAM_H_
#define CORE_FPDFAPI_PARSER_CPDF_STREAM_H_

#include <memory>
#include <set>
#include <sstream>

#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_stream.h"

class CPDF_Stream final : public CPDF_Object {
 public:
  static const int kFileBufSize = 512;

  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // CPDF_Object:
  Type GetType() const override;
  RetainPtr<CPDF_Object> Clone() const override;
  CPDF_Dictionary* GetDict() override;
  const CPDF_Dictionary* GetDict() const override;
  WideString GetUnicodeText() const override;
  bool IsStream() const override;
  CPDF_Stream* AsStream() override;
  const CPDF_Stream* AsStream() const override;
  bool WriteTo(IFX_ArchiveStream* archive,
               const CPDF_Encryptor* encryptor) const override;

  uint32_t GetRawSize() const { return m_dwSize; }
  // Will be null in case when stream is not memory based.
  // Use CPDF_StreamAcc to data access in all cases.
  uint8_t* GetInMemoryRawData() const { return m_pDataBuf.get(); }

  // Copies span into internally-owned buffer.
  void SetData(pdfium::span<const uint8_t> pData);

  void TakeData(std::unique_ptr<uint8_t, FxFreeDeleter> pData, uint32_t size);

  void SetDataFromStringstream(std::ostringstream* stream);

  // Set data and remove "Filter" and "DecodeParms" fields from stream
  // dictionary.
  void SetDataAndRemoveFilter(pdfium::span<const uint8_t> pData);
  void SetDataFromStringstreamAndRemoveFilter(std::ostringstream* stream);

  void InitStream(pdfium::span<const uint8_t> pData,
                  RetainPtr<CPDF_Dictionary> pDict);
  void InitStreamFromFile(const RetainPtr<IFX_SeekableReadStream>& pFile,
                          RetainPtr<CPDF_Dictionary> pDict);

  bool ReadRawData(FX_FILESIZE offset, uint8_t* pBuf, uint32_t buf_size) const;

  bool IsMemoryBased() const { return m_bMemoryBased; }
  bool HasFilter() const;

 private:
  CPDF_Stream();
  CPDF_Stream(std::unique_ptr<uint8_t, FxFreeDeleter> pData,
              uint32_t size,
              RetainPtr<CPDF_Dictionary> pDict);
  ~CPDF_Stream() override;

  RetainPtr<CPDF_Object> CloneNonCyclic(
      bool bDirect,
      std::set<const CPDF_Object*>* pVisited) const override;

  bool m_bMemoryBased = true;
  uint32_t m_dwSize = 0;
  RetainPtr<CPDF_Dictionary> m_pDict;
  std::unique_ptr<uint8_t, FxFreeDeleter> m_pDataBuf;
  RetainPtr<IFX_SeekableReadStream> m_pFile;
};

inline CPDF_Stream* ToStream(CPDF_Object* obj) {
  return obj ? obj->AsStream() : nullptr;
}

inline const CPDF_Stream* ToStream(const CPDF_Object* obj) {
  return obj ? obj->AsStream() : nullptr;
}

inline RetainPtr<CPDF_Stream> ToStream(RetainPtr<CPDF_Object> obj) {
  return RetainPtr<CPDF_Stream>(ToStream(obj.Get()));
}

#endif  // CORE_FPDFAPI_PARSER_CPDF_STREAM_H_
