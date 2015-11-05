// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_TXTEDTBUF_H
#define _FDE_TXTEDTBUF_H
class IFX_CharIter;
class CFDE_TxtEdtBufIter;
class CFDE_TxtEdtBuf;
class CFDE_TxtEdtBufIter : public IFX_CharIter {
 public:
#ifdef FDE_USEFORMATBLOCK
  CFDE_TxtEdtBufIter(CFDE_TxtEdtBuf* pBuf, FX_BOOL bForDisplay = TRUE);
#else
  CFDE_TxtEdtBufIter(CFDE_TxtEdtBuf* pBuf, FX_WCHAR wcAlias = 0);
#endif

  virtual void Release();
  virtual FX_BOOL Next(FX_BOOL bPrev = FALSE);
  virtual FX_WCHAR GetChar();
  virtual void SetAt(int32_t nIndex);
  virtual int32_t GetAt() const;
  virtual FX_BOOL IsEOF(FX_BOOL bTail = TRUE) const;
  virtual IFX_CharIter* Clone();

 protected:
  ~CFDE_TxtEdtBufIter();

 private:
  CFDE_TxtEdtBuf* m_pBuf;
  int32_t m_nCurChunk;
  int32_t m_nCurIndex;
  int32_t m_nIndex;
#ifdef FDE_USEFORMATBLOCK
  FX_BOOL m_bForDisplay;
  int32_t m_nAliasCount;
#endif
  FX_WCHAR m_Alias;
};
class CFDE_TxtEdtBuf : public IFDE_TxtEdtBuf {
  friend class CFDE_TxtEdtBufIter;
  struct _FDE_CHUNKHEADER {
    int32_t nUsed;
    FX_WCHAR wChars[1];
  };
  typedef _FDE_CHUNKHEADER FDE_CHUNKHEADER;
  typedef _FDE_CHUNKHEADER* FDE_LPCHUNKHEADER;
  struct _FDE_CHUNKPLACE {
    int32_t nChunkIndex;
    int32_t nCharIndex;
  };
  typedef _FDE_CHUNKPLACE FDE_CHUNKPLACE;
  typedef _FDE_CHUNKPLACE* FDE_LPCHUNKPLACE;

 public:
  CFDE_TxtEdtBuf(int32_t nDefChunkSize = FDE_DEFCHUNKLENGTH);

  virtual void Release();
  virtual FX_BOOL SetChunkSize(int32_t nChunkSize);
  virtual int32_t GetChunkSize() const;
  virtual int32_t GetTextLength() const;
  virtual void SetText(const CFX_WideString& wsText);
  virtual void GetText(CFX_WideString& wsText) const;
  virtual FX_WCHAR GetCharByIndex(int32_t nIndex) const;
  virtual void GetRange(CFX_WideString& wsText,
                        int32_t nBegine,
                        int32_t nCount = -1) const;

  virtual void Insert(int32_t nPos,
                      const FX_WCHAR* lpText,
                      int32_t nLength = 1);
  virtual void Delete(int32_t nIndex, int32_t nLength = 1);
  virtual void Clear(FX_BOOL bRelease = TRUE);

  virtual FX_BOOL Optimize(IFX_Pause* pPause = NULL);

 protected:
  virtual ~CFDE_TxtEdtBuf();

 private:
  void ResetChunkBuffer(int32_t nDefChunkCount, int32_t nChunkSize);
  int32_t CP2Index(const FDE_CHUNKPLACE& cp) const;
  void Index2CP(int32_t nIndex, FDE_CHUNKPLACE& cp) const;

  int32_t m_nChunkSize;

  int32_t m_nTotal;
  FX_BOOL m_bChanged;
  CFX_PtrArray m_Chunks;
  IFX_MEMAllocator* m_pAllocator;
};
#endif
