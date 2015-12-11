// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417WRITER_H_
#define _BC_PDF417WRITER_H_
class CBC_TwoDimWriter;
class CBC_PDF417Writer;
class CBC_PDF417Writer : public CBC_TwoDimWriter {
 public:
  CBC_PDF417Writer();
  virtual ~CBC_PDF417Writer();
  uint8_t* Encode(const CFX_WideString& contents,
                  int32_t& outWidth,
                  int32_t& outHeight,
                  int32_t& e);
  FX_BOOL SetErrorCorrectionLevel(int32_t level);
  void SetTruncated(FX_BOOL truncated);

 private:
  void rotateArray(CFX_ByteArray& bitarray, int32_t width, int32_t height);
  FX_BOOL m_bTruncated;
};
#endif
