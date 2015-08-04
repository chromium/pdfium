// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DATAMATRIXVERSION_H_
#define _BC_DATAMATRIXVERSION_H_
class ECBlocks;
class CBC_DataMatrixVersion;
class ECB {
 public:
  ECB(int32_t count, int32_t dataCodewords) {
    m_count = count;
    m_dataCodewords = dataCodewords;
  }

  int32_t GetCount() { return m_count; }

  int32_t GetDataCodewords() { return m_dataCodewords; }

 private:
  int32_t m_count;
  int32_t m_dataCodewords;
};
class ECBlocks {
 public:
  ECBlocks(int32_t ecCodewords, ECB* ecBlocks) {
    m_ecCodewords = ecCodewords;
    m_ecBlocks.Add(ecBlocks);
  }

  ECBlocks(int32_t ecCodewords, ECB* ecBlocks1, ECB* ecBlocks2) {
    m_ecCodewords = ecCodewords;
    m_ecBlocks.Add(ecBlocks1);
    m_ecBlocks.Add(ecBlocks2);
  }
  ~ECBlocks() {
    for (int32_t i = 0; i < m_ecBlocks.GetSize(); i++) {
      delete (ECB*)m_ecBlocks[i];
    }
    m_ecBlocks.RemoveAll();
  }

  int32_t GetECCodewords() { return m_ecCodewords; }

  const CFX_PtrArray& GetECBlocks() { return m_ecBlocks; }

 private:
  int32_t m_ecCodewords;
  CFX_PtrArray m_ecBlocks;
};
class CBC_DataMatrixVersion {
 public:
  CBC_DataMatrixVersion(int32_t versionNumber,
                        int32_t symbolSizeRows,
                        int32_t symbolSizeColumns,
                        int32_t dataRegionSizeRows,
                        int32_t dataRegionSizeColumns,
                        ECBlocks* ecBlocks);
  virtual ~CBC_DataMatrixVersion();
  static void Initialize();
  static void Finalize();
  int32_t GetVersionNumber();
  int32_t GetSymbolSizeRows();
  int32_t GetSymbolSizeColumns();
  int32_t GetDataRegionSizeRows();
  int32_t GetDataRegionSizeColumns();
  int32_t GetTotalCodewords();
  ECBlocks* GetECBlocks();
  static CBC_DataMatrixVersion* GetVersionForDimensions(int32_t numRows,
                                                        int32_t numColumns,
                                                        int32_t& e);
  static void ReleaseAll();

 private:
  int32_t m_versionNumber;
  int32_t m_symbolSizeRows;
  int32_t m_symbolSizeColumns;
  int32_t m_dataRegionSizeRows;
  int32_t m_dataRegionSizeColumns;
  ECBlocks* m_ecBlocks;
  int32_t m_totalCodewords;
  static CFX_PtrArray* VERSIONS;
};
#endif
