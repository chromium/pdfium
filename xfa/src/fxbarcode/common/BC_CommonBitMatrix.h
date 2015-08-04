// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_COMMONBITMATRIX_H_
#define _BC_COMMONBITMATRIX_H_
class CBC_CommonBitArray;
class CBC_CommonBitMatrix {
 public:
  CBC_CommonBitMatrix();
  virtual ~CBC_CommonBitMatrix();
  FX_BOOL Get(int32_t x, int32_t y);
  void Set(int32_t x, int32_t y);
  void Flip(int32_t x, int32_t y);
  void Clear();
  void SetRegion(int32_t left,
                 int32_t top,
                 int32_t width,
                 int32_t height,
                 int32_t& e);
  CBC_CommonBitArray* GetRow(int32_t y, CBC_CommonBitArray* row);
  void SetRow(int32_t y, CBC_CommonBitArray* row);
  CBC_CommonBitArray* GetCol(int32_t y, CBC_CommonBitArray* row);
  void SetCol(int32_t y, CBC_CommonBitArray* col);
  int32_t GetWidth();
  int32_t GetHeight();
  int32_t GetRowSize();
  int32_t GetDimension(int32_t& e);
  virtual void Init(int32_t dimension);
  virtual void Init(int32_t width, int32_t height);
  int32_t* GetBits();

 private:
  int32_t m_width;
  int32_t m_height;
  int32_t m_rowSize;
  int32_t* m_bits;
};
#endif
