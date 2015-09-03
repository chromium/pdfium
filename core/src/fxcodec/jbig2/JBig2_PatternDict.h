// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_PATTERN_DICT_H_
#define _JBIG2_PATTERN_DICT_H_
#include "JBig2_Define.h"
#include "JBig2_Image.h"
class CJBig2_PatternDict {
 public:
  CJBig2_PatternDict();

  ~CJBig2_PatternDict();

  FX_DWORD NUMPATS;
  CJBig2_Image** HDPATS;
};
#endif
