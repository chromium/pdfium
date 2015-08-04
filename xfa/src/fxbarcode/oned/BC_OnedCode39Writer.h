// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDCODA39WRITER_H_
#define _BC_ONEDCODA39WRITER_H_
enum BC_TEXT_LOC;
class CBC_OneDimWriter;
class CBC_OnedCoda39Writer;
class CBC_OnedCode39Writer : public CBC_OneDimWriter {
 public:
  CBC_OnedCode39Writer();
  CBC_OnedCode39Writer(FX_BOOL extendedMode);
  virtual ~CBC_OnedCode39Writer();
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
  uint8_t* Encode(const CFX_ByteString& contents,
                  int32_t& outLength,
                  int32_t& e);
  void RenderResult(const CFX_WideStringC& contents,
                    uint8_t* code,
                    int32_t codeLength,
                    FX_BOOL isDevice,
                    int32_t& e);
  CFX_WideString encodedContents(const CFX_WideStringC& contents, int32_t& e);
  FX_BOOL CheckContentValidity(const CFX_WideStringC& contents);
  FX_BOOL CheckExtendedContentValidity(const CFX_WideStringC& contents);
  CFX_WideString FilterContents(const CFX_WideStringC& contents);
  CFX_WideString FilterExtendedContents(const CFX_WideStringC& contents);
  CFX_WideString RenderTextContents(const CFX_WideStringC& contents);
  CFX_WideString RenderExtendedTextContents(const CFX_WideStringC& contents);
  FX_BOOL SetTextLocation(BC_TEXT_LOC loction);
  FX_BOOL SetWideNarrowRatio(int32_t ratio);

 private:
  void ToIntArray(int32_t a, int32_t* toReturn);
  FX_CHAR CalcCheckSum(const CFX_ByteString& contents, int32_t& e);
  int32_t m_iWideNarrRatio;
  FX_BOOL m_extendedMode;
};
#endif
