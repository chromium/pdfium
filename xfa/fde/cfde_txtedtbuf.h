// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_TXTEDTBUF_H_
#define XFA_FDE_CFDE_TXTEDTBUF_H_

#include <memory>
#include <tuple>
#include <vector>

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fde/ifx_chariter.h"

class IFX_Pause;

class CFDE_TxtEdtBuf {
 public:
  class Iterator : public IFX_CharIter {
   public:
    explicit Iterator(CFDE_TxtEdtBuf* pBuf, FX_WCHAR wcAlias = 0);
    ~Iterator() override;

    bool Next(bool bPrev = false) override;
    FX_WCHAR GetChar() override;

    void SetAt(int32_t nIndex) override;
    int32_t GetAt() const override;

    bool IsEOF(bool bTail = true) const override;
    IFX_CharIter* Clone() override;

   private:
    CFDE_TxtEdtBuf* m_pBuf;
    int32_t m_nCurChunk;
    int32_t m_nCurIndex;
    int32_t m_nIndex;
    FX_WCHAR m_Alias;
  };

  CFDE_TxtEdtBuf();
  ~CFDE_TxtEdtBuf();

  int32_t GetChunkSize() const;
  int32_t GetTextLength() const;

  void SetText(const CFX_WideString& wsText);
  CFX_WideString GetText() const;

  FX_WCHAR GetCharByIndex(int32_t nIndex) const;
  CFX_WideString GetRange(int32_t nBegin, int32_t nCount) const;

  void Insert(int32_t nPos, const FX_WCHAR* lpText, int32_t nLength);
  void Delete(int32_t nIndex, int32_t nLength);
  void Clear(bool bRelease);

 private:
  friend class Iterator;
  friend class CFDE_TxtEdtBufTest;

  class ChunkHeader {
   public:
    ChunkHeader();
    ~ChunkHeader();

    int32_t nUsed;
    std::unique_ptr<FX_WCHAR, FxFreeDeleter> wChars;
  };

  void SetChunkSizeForTesting(size_t size);
  std::tuple<int32_t, int32_t> Index2CP(int32_t nIndex) const;
  std::unique_ptr<ChunkHeader> NewChunk();

  size_t m_chunkSize;
  int32_t m_nTotal;
  std::vector<std::unique_ptr<ChunkHeader>> m_chunks;
};

#endif  // XFA_FDE_CFDE_TXTEDTBUF_H_
