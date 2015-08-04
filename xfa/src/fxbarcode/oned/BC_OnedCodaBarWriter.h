// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDCODABARWRITER_H_
#define _BC_ONEDCODABARWRITER_H_
enum BC_TEXT_LOC;
class CBC_OneDimWriter;
class CBC_OnedCodaBarWriter;
class CBC_OnedCodaBarWriter : public CBC_OneDimWriter {
 public:
  CBC_OnedCodaBarWriter();
  virtual ~CBC_OnedCodaBarWriter();
  uint8_t* Encode(const CFX_ByteString& contents,
                  int32_t& outLength,
                  int32_t& e);
  uint8_t* Encode(const CFX_ByteString& contents,
                  BCFORMAT format,
                  int32_t& outWidth,
                  int32_t& outHeight,
                  int32_t& e);
  uint8_t* Encode(const CFX_ByteString& contents,
                  BCFORMAT format,
                  int32_t& outWidth,
                  int32_t& outHeight,
                  int32_t hints,
                  int32_t& e);
  CFX_WideString encodedContents(const CFX_WideStringC& contents);
  FX_BOOL CheckContentValidity(const CFX_WideStringC& contents);
  CFX_WideString FilterContents(const CFX_WideStringC& contents);
  FX_BOOL SetStartChar(FX_CHAR start);
  FX_BOOL SetEndChar(FX_CHAR end);
  void SetDataLength(int32_t length);
  FX_BOOL SetTextLocation(BC_TEXT_LOC location);
  FX_BOOL SetWideNarrowRatio(int32_t ratio);
  FX_BOOL FindChar(FX_WCHAR ch, FX_BOOL isContent);

 private:
  void RenderResult(const CFX_WideStringC& contents,
                    uint8_t* code,
                    int32_t codeLength,
                    FX_BOOL isDevice,
                    int32_t& e);
  const static FX_CHAR START_END_CHARS[];
  const static FX_CHAR CONTENT_CHARS[];
  FX_CHAR m_chStart;
  FX_CHAR m_chEnd;
  int32_t m_iWideNarrRatio;
};
#endif
