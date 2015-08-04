// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BARCODEMETADATA_H_
#define _BC_BARCODEMETADATA_H_
class CBC_BarcodeMetadata;
class CBC_BarcodeMetadata {
 public:
  CBC_BarcodeMetadata(int32_t columnCount,
                      int32_t rowCountUpperPart,
                      int32_t rowCountLowerPart,
                      int32_t errorCorrectionLevel);
  virtual ~CBC_BarcodeMetadata();
  int32_t getColumnCount();
  int32_t getErrorCorrectionLevel();
  int32_t getRowCount();
  int32_t getRowCountUpperPart();
  int32_t getRowCountLowerPart();

 private:
  int32_t m_columnCount;
  int32_t m_errorCorrectionLevel;
  int32_t m_rowCountUpperPart;
  int32_t m_rowCountLowerPart;
  int32_t m_rowCount;
};
#endif
