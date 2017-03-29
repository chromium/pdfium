// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_ONED_BC_ONEDCODE128WRITER_H_
#define FXBARCODE_ONED_BC_ONEDCODE128WRITER_H_

#include <vector>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "fxbarcode/oned/BC_OneDimWriter.h"

class CBC_OnedCode128Writer : public CBC_OneDimWriter {
 public:
  CBC_OnedCode128Writer();
  explicit CBC_OnedCode128Writer(BC_TYPE type);
  ~CBC_OnedCode128Writer() override;

  // CBC_OneDimWriter
  uint8_t* Encode(const CFX_ByteString& contents,
                  BCFORMAT format,
                  int32_t& outWidth,
                  int32_t& outHeight,
                  int32_t hints,
                  int32_t& e) override;
  uint8_t* Encode(const CFX_ByteString& contents,
                  BCFORMAT format,
                  int32_t& outWidth,
                  int32_t& outHeight,
                  int32_t& e) override;
  uint8_t* Encode(const CFX_ByteString& contents,
                  int32_t& outLength,
                  int32_t& e) override;

  bool CheckContentValidity(const CFX_WideStringC& contents) override;
  CFX_WideString FilterContents(const CFX_WideStringC& contents) override;

  bool SetTextLocation(BC_TEXT_LOC location);

  BC_TYPE GetType();

 private:
  bool IsDigits(const CFX_ByteString& contents, int32_t start, int32_t length);
  int32_t Encode128B(const CFX_ByteString& contents,
                     std::vector<const int32_t*>* patterns);
  int32_t Encode128C(const CFX_ByteString& contents,
                     std::vector<const int32_t*>* patterns);

  BC_TYPE m_codeFormat;
};

#endif  // FXBARCODE_ONED_BC_ONEDCODE128WRITER_H_
