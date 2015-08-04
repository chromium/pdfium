// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERECB_H_
#define _BC_QRCODERECB_H_
class CBC_QRCoderECB {
 private:
  int32_t m_count;
  int32_t m_dataCodeWords;

 public:
  CBC_QRCoderECB(int32_t count, int32_t dataCodeWords);
  virtual ~CBC_QRCoderECB();
  int32_t GetCount();
  int32_t GetDataCodeWords();
};
#endif
