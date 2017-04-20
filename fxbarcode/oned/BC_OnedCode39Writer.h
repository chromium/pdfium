// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_ONED_BC_ONEDCODE39WRITER_H_
#define FXBARCODE_ONED_BC_ONEDCODE39WRITER_H_

#include "fxbarcode/BC_Library.h"
#include "fxbarcode/oned/BC_OneDimWriter.h"

class CBC_OnedCode39Writer : public CBC_OneDimWriter {
 public:
  CBC_OnedCode39Writer();
  ~CBC_OnedCode39Writer() override;

  // CBC_OneDimWriter
  uint8_t* EncodeWithHint(const CFX_ByteString& contents,
                          BCFORMAT format,
                          int32_t& outWidth,
                          int32_t& outHeight,
                          int32_t hints) override;
  uint8_t* EncodeImpl(const CFX_ByteString& contents,
                      int32_t& outLength) override;
  bool RenderResult(const CFX_WideStringC& contents,
                    uint8_t* code,
                    int32_t codeLength,
                    bool isDevice) override;
  bool CheckContentValidity(const CFX_WideStringC& contents) override;
  CFX_WideString FilterContents(const CFX_WideStringC& contents) override;
  CFX_WideString RenderTextContents(const CFX_WideStringC& contents) override;

  virtual bool SetTextLocation(BC_TEXT_LOC loction);
  virtual bool SetWideNarrowRatio(int8_t ratio);

  bool encodedContents(const CFX_WideStringC& contents, CFX_WideString* result);

 private:
  void ToIntArray(int32_t a, int8_t* toReturn);
  char CalcCheckSum(const CFX_ByteString& contents);

  int8_t m_iWideNarrRatio;
};

#endif  // FXBARCODE_ONED_BC_ONEDCODE39WRITER_H_
