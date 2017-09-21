// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_EDIT_CPDF_FLATEENCODER_H_
#define CORE_FPDFAPI_EDIT_CPDF_FLATEENCODER_H_

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/maybe_owned.h"
#include "core/fxcrt/retain_ptr.h"

class CPDF_Stream;

class CPDF_FlateEncoder {
 public:
  CPDF_FlateEncoder(CPDF_Stream* pStream, bool bFlateEncode);
  ~CPDF_FlateEncoder();

  void CloneDict();

  uint32_t GetSize() const { return m_dwSize; }
  uint8_t* GetData() const { return m_pData.Get(); }

  CPDF_Dictionary* GetDict() { return m_pDict.Get(); }

 private:
  uint32_t m_dwSize;
  MaybeOwned<uint8_t, FxFreeDeleter> m_pData;
  MaybeOwned<CPDF_Dictionary> m_pDict;
  RetainPtr<CPDF_StreamAcc> m_pAcc;
};

#endif  // CORE_FPDFAPI_EDIT_CPDF_FLATEENCODER_H_
