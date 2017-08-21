// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_TXTEDTBUF_H_
#define XFA_FDE_CFDE_TXTEDTBUF_H_

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/ifx_chariter.h"

class CFDE_TxtEdtBuf {
 public:
  class Iterator : public IFX_CharIter {
   public:
    Iterator(CFDE_TxtEdtBuf* pBuf, wchar_t wcAlias);
    ~Iterator() override;

    bool Next(bool bPrev = false) override;
    wchar_t GetChar() const override;

    void SetAt(int32_t nIndex) override;
    int32_t GetAt() const override;

    bool IsEOF(bool bTail = true) const override;
    std::unique_ptr<IFX_CharIter> Clone() const override;

   private:
    CFDE_TxtEdtBuf* m_pBuf;
    int32_t m_nCurChunk;
    int32_t m_nCurIndex;
    int32_t m_nIndex;
    wchar_t m_Alias;
  };

  CFDE_TxtEdtBuf();
  ~CFDE_TxtEdtBuf();

  int32_t GetTextLength() const { return m_nTotal; }
  void SetText(const CFX_WideString& wsText);
  CFX_WideString GetText() const { return GetRange(0, m_nTotal); }

  wchar_t GetCharByIndex(int32_t nIndex) const;
  CFX_WideString GetRange(int32_t nBegin, int32_t nCount) const;

  void Insert(int32_t nPos, const CFX_WideString& wsText);
  void Delete(int32_t nIndex, int32_t nLength);
  void Clear();

  void SetChunkSizeForTesting(size_t size);
  size_t GetChunkCountForTesting() const { return m_chunks.size(); }

 private:
  class ChunkHeader {
   public:
    explicit ChunkHeader(int32_t chunkSize);
    ~ChunkHeader();

    int32_t nUsed;
    std::vector<wchar_t> wChars;
  };

  std::pair<int32_t, int32_t> Index2CP(int32_t nIndex) const;

  int32_t m_chunkSize;
  int32_t m_nTotal;
  std::vector<std::unique_ptr<ChunkHeader>> m_chunks;
};

#endif  // XFA_FDE_CFDE_TXTEDTBUF_H_
