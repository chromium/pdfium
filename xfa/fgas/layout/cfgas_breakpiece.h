// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_BREAKPIECE_H_
#define XFA_FGAS_LAYOUT_CFGAS_BREAKPIECE_H_

#include <vector>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "xfa/fgas/layout/cfgas_char.h"

class CFGAS_TextUserData;

class CFGAS_BreakPiece {
 public:
  CFGAS_BreakPiece();
  CFGAS_BreakPiece(const CFGAS_BreakPiece& other);
  ~CFGAS_BreakPiece();

  int32_t GetEndPos() const;
  int32_t GetLength() const { return m_iCharCount; }

  CFGAS_Char* GetChar(int32_t index) const;
  WideString GetString() const;
  std::vector<int32_t> GetWidths() const;

  CFGAS_Char::BreakType m_dwStatus = CFGAS_Char::BreakType::kPiece;
  int32_t m_iStartPos = 0;
  int32_t m_iWidth = -1;
  int32_t m_iStartChar = 0;
  int32_t m_iCharCount = 0;
  int32_t m_iBidiLevel = 0;
  int32_t m_iBidiPos = 0;
  int32_t m_iFontSize = 0;
  int32_t m_iHorizontalScale = 100;
  int32_t m_iVerticalScale = 100;
  uint32_t m_dwIdentity = 0;
  uint32_t m_dwCharStyles = 0;
  UnownedPtr<std::vector<CFGAS_Char>> m_pChars;
  RetainPtr<CFGAS_TextUserData> m_pUserData;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_BREAKPIECE_H_
