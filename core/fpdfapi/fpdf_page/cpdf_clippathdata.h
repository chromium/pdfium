// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_CLIPPATHDATA_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_CLIPPATHDATA_H_

#include <stdint.h>

class CPDF_Path;
class CPDF_TextObject;

class CPDF_ClipPathData {
 public:
  CPDF_ClipPathData();
  CPDF_ClipPathData(const CPDF_ClipPathData&);
  ~CPDF_ClipPathData();

  void SetCount(int path_count, int text_count);

  int m_PathCount;
  CPDF_Path* m_pPathList;
  uint8_t* m_pTypeList;
  int m_TextCount;
  CPDF_TextObject** m_pTextList;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_CLIPPATHDATA_H_
