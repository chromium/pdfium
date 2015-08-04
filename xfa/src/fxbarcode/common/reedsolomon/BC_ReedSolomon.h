// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_READSOLOMON_H_
#define _BC_READSOLOMON_H_
class CBC_ReedSolomonGF256;
class CBC_ReedSolomonGF256Poly;
class CBC_ReedSolomonEncoder {
 private:
  CBC_ReedSolomonGF256* m_field;
  CFX_PtrArray m_cachedGenerators;
  CBC_ReedSolomonGF256Poly* BuildGenerator(int32_t degree, int32_t& e);

 public:
  CBC_ReedSolomonEncoder(CBC_ReedSolomonGF256* field);
  virtual ~CBC_ReedSolomonEncoder();

  void Encode(CFX_Int32Array* toEncode, int32_t ecBytes, int32_t& e);
  virtual void Init();
};
#endif
