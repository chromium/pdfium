// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DECODERRESULT_H_
#define _BC_DECODERRESULT_H_
class CBC_DecoderResult;
class CBC_DecoderResult {
 public:
  CBC_DecoderResult(CFX_ByteArray* rawBytes,
                    CFX_ByteString text,
                    CFX_ByteString ecLevel);
  virtual ~CBC_DecoderResult();
  CFX_ByteArray* getRawBytes();
  CFX_ByteString getText();
  CFX_ByteString getECLevel();
  int32_t getErrorsCorrected();
  void setErrorsCorrected(int32_t errorsCorrected);
  int32_t getErasures();
  void setErasures(int32_t erasures);
  void* getOther();
  void setOther(void* other);

 private:
  CFX_ByteArray* m_rawBytes;
  CFX_ByteString m_text;
  CFX_ByteString m_ecLevel;
  int32_t m_errorsCorrected;
  int32_t m_erasures;
  void* m_other;
};
#endif
