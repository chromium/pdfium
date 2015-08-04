// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DETECTIONRESULTCOLUMN_H_
#define _BC_DETECTIONRESULTCOLUMN_H_
class CBC_Codeword;
class CBC_BoundingBox;
class CBC_DetectionResultColumn {
 public:
  CBC_DetectionResultColumn(CBC_BoundingBox* boundingBox);
  virtual ~CBC_DetectionResultColumn();
  CBC_Codeword* getCodewordNearby(int32_t imageRow);
  int32_t imageRowToCodewordIndex(int32_t imageRow);
  int32_t codewordIndexToImageRow(int32_t codewordIndex);
  void setCodeword(int32_t imageRow, CBC_Codeword* codeword);
  CBC_Codeword* getCodeword(int32_t imageRow);
  CBC_BoundingBox* getBoundingBox();
  CFX_PtrArray* getCodewords();
  CFX_ByteString toString();

 public:
  CBC_BoundingBox* m_boundingBox;
  CFX_PtrArray* m_codewords;

 private:
  static int32_t MAX_NEARBY_DISTANCE;
};
#endif
