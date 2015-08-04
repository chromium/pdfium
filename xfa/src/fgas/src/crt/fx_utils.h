// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_UTILS_IMP
#define _FX_UTILS_IMP
class CFX_BaseMassArrayImp : public CFX_Target {
 public:
  CFX_BaseMassArrayImp(int32_t iChunkSize, int32_t iBlockSize);
  ~CFX_BaseMassArrayImp();
  uint8_t* AddSpace() { return AddSpaceTo(m_iBlockCount); }
  uint8_t* AddSpaceTo(int32_t index);
  uint8_t* GetAt(int32_t index) const;
  int32_t Append(const CFX_BaseMassArrayImp& src,
                 int32_t iStart = 0,
                 int32_t iCount = -1);
  int32_t Copy(const CFX_BaseMassArrayImp& src,
               int32_t iStart = 0,
               int32_t iCount = -1);
  int32_t RemoveLast(int32_t iCount = -1);
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE);
  int32_t m_iChunkSize;
  int32_t m_iBlockSize;
  int32_t m_iChunkCount;
  int32_t m_iBlockCount;
  CFX_PtrArray* m_pData;

 protected:
  void Append(int32_t iDstStart,
              const CFX_BaseMassArrayImp& src,
              int32_t iSrcStart = 0,
              int32_t iSrcCount = -1);
};
#endif
