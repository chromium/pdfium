// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417READER_H_
#define _BC_PDF417READER_H_
class CBC_BinaryBitmap;
class CBC_ResultPoint;
class CBC_PDF417Reader;
class CBC_PDF417Reader : public CBC_Reader {
 public:
  CBC_PDF417Reader();
  virtual ~CBC_PDF417Reader();
  CFX_ByteString Decode(CBC_BinaryBitmap* image, int32_t& e);
  CFX_ByteString Decode(CBC_BinaryBitmap* image,
                        FX_BOOL multiple,
                        int32_t hints,
                        int32_t& e);
  CFX_ByteString Decode(CBC_BinaryBitmap* image, int32_t hints, int32_t& e);

 private:
  static int32_t getMaxWidth(CBC_ResultPoint* p1, CBC_ResultPoint* p2);
  static int32_t getMinWidth(CBC_ResultPoint* p1, CBC_ResultPoint* p2);
  static int32_t getMaxCodewordWidth(CFX_PtrArray& p);
  static int32_t getMinCodewordWidth(CFX_PtrArray& p);
};
#endif
