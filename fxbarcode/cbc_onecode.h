// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_CBC_ONECODE_H_
#define FXBARCODE_CBC_ONECODE_H_

#include <stdint.h>

#include <memory>

#include "core/fxge/dib/fx_dib.h"
#include "fxbarcode/cbc_codebase.h"

class CBC_OneDimWriter;
class CFX_Font;

class CBC_OneCode : public CBC_CodeBase {
 public:
  explicit CBC_OneCode(std::unique_ptr<CBC_Writer> pWriter);
  ~CBC_OneCode() override;

  void SetPrintChecksum(bool checksum);
  void SetDataLength(int32_t length);
  void SetCalChecksum(bool calc);
  bool SetFont(CFX_Font* cFont);
  void SetFontSize(float size);
  void SetFontStyle(int32_t style);
  void SetFontColor(FX_ARGB color);

 private:
  CBC_OneDimWriter* GetOneDimWriter();
};

#endif  // FXBARCODE_CBC_ONECODE_H_
