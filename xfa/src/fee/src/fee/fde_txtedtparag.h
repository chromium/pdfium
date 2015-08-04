// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_TXTEDTPARAG_H
#define _FDE_TXTEDTPARAG_H
class CFDE_TxtEdtEngine;
class CFDE_TxtEdtParag;
class CFDE_TxtEdtParag : public IFDE_TxtEdtParag {
 public:
  CFDE_TxtEdtParag(CFDE_TxtEdtEngine* pEngine);
  ~CFDE_TxtEdtParag();
  virtual int32_t GetTextLength() const { return m_nCharCount; }
  virtual int32_t GetStartIndex() const { return m_nCharStart; }
  virtual int32_t CountLines() const { return m_nLineCount; }
  virtual void GetLineRange(int32_t nLineIndex,
                            int32_t& nStart,
                            int32_t& nCount) const;
  void LoadParag();
  void UnloadParag();
  void CalcLines();
  int32_t m_nCharStart;
  int32_t m_nCharCount;
  int32_t m_nLineCount;

 private:
  void* m_lpData;
  CFDE_TxtEdtEngine* m_pEngine;
};
#endif
