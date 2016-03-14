// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_PDF417_BC_PDF417DIMENSIONS_H_
#define XFA_FXBARCODE_PDF417_BC_PDF417DIMENSIONS_H_

#include <stdint.h>

class CBC_Dimensions {
 public:
  CBC_Dimensions(int32_t minCols,
                 int32_t maxCols,
                 int32_t minRows,
                 int32_t maxRows);
  virtual ~CBC_Dimensions();
  int32_t getMinCols();
  int32_t getMaxCols();
  int32_t getMinRows();
  int32_t getMaxRows();

 private:
  int32_t m_minCols;
  int32_t m_maxCols;
  int32_t m_minRows;
  int32_t m_maxRows;
};

#endif  // XFA_FXBARCODE_PDF417_BC_PDF417DIMENSIONS_H_
