// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_SUBSTFONT_H_
#define CORE_FXGE_CFX_SUBSTFONT_H_

#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_string.h"

class CFX_SubstFont {
 public:
  CFX_SubstFont();
  ~CFX_SubstFont();

  ByteString m_Family;
  int m_Charset = FX_CHARSET_ANSI;
  int m_Weight = 0;
  int m_ItalicAngle = 0;
  int m_WeightCJK = 0;
  bool m_bSubstCJK = false;
  bool m_bItalicCJK = false;
  bool m_bFlagMM = false;
};

#endif  // CORE_FXGE_CFX_SUBSTFONT_H_
