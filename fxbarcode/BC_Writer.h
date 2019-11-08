// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_BC_WRITER_H_
#define FXBARCODE_BC_WRITER_H_

#include "core/fxge/fx_dib.h"
#include "fxbarcode/BC_Library.h"

class CBC_Writer {
 public:
  CBC_Writer();
  virtual ~CBC_Writer();
  virtual bool SetCharEncoding(int32_t encoding);
  virtual bool SetModuleHeight(int32_t moduleHeight);
  virtual bool SetModuleWidth(int32_t moduleWidth);
  virtual bool SetHeight(int32_t height);
  virtual bool SetWidth(int32_t width);
  virtual bool SetTextLocation(BC_TEXT_LOC location);
  virtual bool SetWideNarrowRatio(int8_t ratio);
  virtual bool SetStartChar(char start);
  virtual bool SetEndChar(char end);
  virtual bool SetErrorCorrectionLevel(int32_t level);

 protected:
  static const FX_ARGB kBarColor = 0xff000000;
  static const FX_ARGB kBackgroundColor = 0xffffffff;

  int32_t m_CharEncoding = 0;
  int32_t m_ModuleHeight = 1;
  int32_t m_ModuleWidth = 1;
  int32_t m_Height = 320;
  int32_t m_Width = 640;
  FXDIB_Format m_colorSpace = FXDIB_Argb;
};

#endif  // FXBARCODE_BC_WRITER_H_
