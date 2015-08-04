// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDCODA128WRITER_H_
#define _BC_ONEDCODA128WRITER_H_
class CBC_OneDimWriter;
class CBC_OnedCoda128Writer;
class CBC_OnedCode128Writer : public CBC_OneDimWriter {
 public:
  CBC_OnedCode128Writer();
  CBC_OnedCode128Writer(BC_TYPE type);
  virtual ~CBC_OnedCode128Writer();
  uint8_t* Encode(const CFX_ByteString& contents,
                  BCFORMAT format,
                  int32_t& outWidth,
                  int32_t& outHeight,
                  int32_t hints,
                  int32_t& e);
  uint8_t* Encode(const CFX_ByteString& contents,
                  BCFORMAT format,
                  int32_t& outWidth,
                  int32_t& outHeight,
                  int32_t& e);
  uint8_t* Encode(const CFX_ByteString& contents,
                  int32_t& outLength,
                  int32_t& e);
  FX_BOOL CheckContentValidity(const CFX_WideStringC& contents);
  CFX_WideString FilterContents(const CFX_WideStringC& contents);
  FX_BOOL SetTextLocation(BC_TEXT_LOC location);
  BC_TYPE GetType();

 private:
  FX_BOOL IsDigits(const CFX_ByteString& contents,
                   int32_t start,
                   int32_t length);
  int32_t Encode128B(const CFX_ByteString& contents, CFX_PtrArray& patterns);
  int32_t Encode128C(const CFX_ByteString& contents, CFX_PtrArray& patterns);
  BC_TYPE m_codeFormat;
  const static int32_t CODE_START_B;
  const static int32_t CODE_START_C;
  const static int32_t CODE_CODE_B;
  const static int32_t CODE_CODE_C;
  const static int32_t CODE_STOP;
};
#endif
