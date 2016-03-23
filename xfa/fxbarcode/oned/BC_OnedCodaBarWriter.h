// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_ONED_BC_ONEDCODABARWRITER_H_
#define XFA_FXBARCODE_ONED_BC_ONEDCODABARWRITER_H_

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fxbarcode/include/BC_Library.h"
#include "xfa/fxbarcode/oned/BC_OneDimWriter.h"

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
  static const FX_CHAR START_END_CHARS[];
  static const FX_CHAR CONTENT_CHARS[];

  void RenderResult(const CFX_WideStringC& contents,
                    uint8_t* code,
                    int32_t codeLength,
                    FX_BOOL isDevice,
                    int32_t& e);

  FX_CHAR m_chStart;
  FX_CHAR m_chEnd;
  int32_t m_iWideNarrRatio;
};

#endif  // XFA_FXBARCODE_ONED_BC_ONEDCODABARWRITER_H_
