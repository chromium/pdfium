// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_COMMONDECODERRESULT_H_
#define _BC_COMMONDECODERRESULT_H_
class CBC_QRCoderErrorCorrectionLevel;
class CBC_PDF417ResultMetadata;
class CBC_CommonDecoderResult {
 public:
  CBC_CommonDecoderResult();
  virtual ~CBC_CommonDecoderResult();
  const CFX_ByteArray& GetRawBytes();
  const CFX_ByteString& GetText();
  const CFX_Int32Array& GetByteSegments();
  CBC_QRCoderErrorCorrectionLevel* GetECLevel();
  virtual void Init(const CFX_ByteArray& rawBytes,
                    const CFX_ByteString& text,
                    const CFX_Int32Array& byteSegments,
                    CBC_QRCoderErrorCorrectionLevel* ecLevel,
                    int32_t& e);
  virtual void Init(const CFX_ByteArray& rawBytes,
                    const CFX_ByteString& text,
                    const CFX_PtrArray& byteSegments,
                    const CFX_ByteString& ecLevel,
                    int32_t& e);
  void setOther(CBC_PDF417ResultMetadata* other);

 private:
  CFX_ByteArray m_rawBytes;
  CFX_ByteString m_text;
  CFX_Int32Array m_byteSegments;
  CFX_PtrArray m_pdf417byteSegments;
  CBC_QRCoderErrorCorrectionLevel* m_ecLevel;
  CFX_ByteString m_pdf417ecLevel;
  CBC_PDF417ResultMetadata* m_other;
};
#endif
