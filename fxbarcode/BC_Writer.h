// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_BC_WRITER_H_
#define FXBARCODE_BC_WRITER_H_

#include "core/fxge/dib/fx_dib.h"
#include "fxbarcode/BC_Library.h"

class CBC_Writer {
 public:
  CBC_Writer();
  virtual ~CBC_Writer();

  void SetCharEncoding(BC_CHAR_ENCODING encoding);
  bool SetModuleHeight(int32_t moduleHeight);
  bool SetModuleWidth(int32_t moduleWidth);
  void SetHeight(int32_t height);
  void SetWidth(int32_t width);

  virtual void SetTextLocation(BC_TEXT_LOC location);
  virtual bool SetWideNarrowRatio(int8_t ratio);
  virtual bool SetStartChar(char start);
  virtual bool SetEndChar(char end);
  virtual bool SetErrorCorrectionLevel(int32_t level);

 protected:
  static const FX_ARGB kBarColor = 0xff000000;
  static const FX_ARGB kBackgroundColor = 0xffffffff;

  int32_t m_ModuleHeight = 1;
  int32_t m_ModuleWidth = 1;
  int32_t m_Height = 320;
  int32_t m_Width = 640;
  FXDIB_Format m_colorSpace = FXDIB_Format::kArgb;
  BC_CHAR_ENCODING m_CharEncoding = BC_CHAR_ENCODING::kUTF8;
};

#endif  // FXBARCODE_BC_WRITER_H_
