// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CFDF_DOCUMENT_H_
#define CORE_FPDFAPI_PARSER_CFDF_DOCUMENT_H_

#include <memory>

#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"

class CPDF_Dictionary;
class IFX_SeekableReadStream;

class CFDF_Document final : public CPDF_IndirectObjectHolder {
 public:
  static std::unique_ptr<CFDF_Document> CreateNewDoc();
  static std::unique_ptr<CFDF_Document> ParseMemory(
      pdfium::span<const uint8_t> span);

  CFDF_Document();
  ~CFDF_Document() override;

  ByteString WriteToString() const;
  const CPDF_Dictionary* GetRoot() const { return root_dict_.Get(); }
  RetainPtr<CPDF_Dictionary> GetMutableRoot() const { return root_dict_; }

 private:
  void ParseStream(RetainPtr<IFX_SeekableReadStream> pFile);

  RetainPtr<CPDF_Dictionary> root_dict_;
  RetainPtr<IFX_SeekableReadStream> file_;
};

#endif  // CORE_FPDFAPI_PARSER_CFDF_DOCUMENT_H_
