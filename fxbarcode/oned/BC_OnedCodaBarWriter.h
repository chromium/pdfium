// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_ONED_BC_ONEDCODABARWRITER_H_
#define FXBARCODE_ONED_BC_ONEDCODABARWRITER_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "fxbarcode/BC_Library.h"
#include "fxbarcode/oned/BC_OneDimWriter.h"

class CBC_OnedCodaBarWriter : public CBC_OneDimWriter {
 public:
  CBC_OnedCodaBarWriter();
  ~CBC_OnedCodaBarWriter() override;

  // CBC_OneDimWriter
  uint8_t* EncodeImpl(const CFX_ByteString& contents,
                      int32_t& outLength) override;
  uint8_t* EncodeWithHint(const CFX_ByteString& contents,
                          BCFORMAT format,
                          int32_t& outWidth,
                          int32_t& outHeight,
                          int32_t hints) override;
  bool RenderResult(const CFX_WideStringC& contents,
                    uint8_t* code,
                    int32_t codeLength,
                    bool isDevice) override;
  bool CheckContentValidity(const CFX_WideStringC& contents) override;
  CFX_WideString FilterContents(const CFX_WideStringC& contents) override;
  void SetDataLength(int32_t length) override;

  virtual bool SetStartChar(char start);
  virtual bool SetEndChar(char end);
  virtual bool SetTextLocation(BC_TEXT_LOC location);
  virtual bool SetWideNarrowRatio(int8_t ratio);
  virtual bool FindChar(wchar_t ch, bool isContent);

  CFX_WideString encodedContents(const CFX_WideStringC& contents);

 private:
  char m_chStart;
  char m_chEnd;
  int8_t m_iWideNarrRatio;
};

#endif  // FXBARCODE_ONED_BC_ONEDCODABARWRITER_H_
