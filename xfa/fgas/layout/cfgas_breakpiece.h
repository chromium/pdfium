// Copyright 2017 The PDFium Authors
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

  // TODO(thestig): When GetCharCount() returns size_t, remove this.
  size_t GetLength() const;

  CFGAS_Char* GetChar(int32_t index) const;
  CFGAS_Char* GetChar(size_t index) const;
  WideString GetString() const;
  std::vector<int32_t> GetWidths() const;

  CFGAS_Char::BreakType GetStatus() const { return m_dwStatus; }
  void SetStatus(CFGAS_Char::BreakType status) { m_dwStatus = status; }

  int32_t GetStartPos() const { return m_iStartPos; }
  void SetStartPos(int32_t pos) { m_iStartPos = pos; }
  void IncrementStartPos(int32_t count) { m_iStartPos += count; }

  int32_t GetWidth() const { return m_iWidth; }
  void SetWidth(int32_t width) { m_iWidth = width; }
  void IncrementWidth(int32_t width) { m_iWidth += width; }

  int32_t GetStartChar() const { return m_iStartChar; }
  void SetStartChar(int32_t pos) { m_iStartChar = pos; }

  int32_t GetCharCount() const { return m_iCharCount; }
  void SetCharCount(int32_t count) { m_iCharCount = count; }

  int32_t GetBidiLevel() const { return m_iBidiLevel; }
  void SetBidiLevel(int32_t level) { m_iBidiLevel = level; }

  int32_t GetBidiPos() const { return m_iBidiPos; }
  void SetBidiPos(int32_t pos) { m_iBidiPos = pos; }

  int32_t GetFontSize() const { return m_iFontSize; }
  void SetFontSize(int32_t font_size) { m_iFontSize = font_size; }

  int32_t GetHorizontalScale() const { return m_iHorizontalScale; }
  void SetHorizontalScale(int32_t scale) { m_iHorizontalScale = scale; }

  int32_t GetVerticalScale() const { return m_iVerticalScale; }
  void SetVerticalScale(int32_t scale) { m_iVerticalScale = scale; }

  uint32_t GetCharStyles() const { return m_dwCharStyles; }
  void SetCharStyles(uint32_t styles) { m_dwCharStyles = styles; }

  void SetChars(std::vector<CFGAS_Char>* chars) { m_pChars = chars; }

  const CFGAS_TextUserData* GetUserData() const { return m_pUserData.Get(); }
  void SetUserData(const RetainPtr<CFGAS_TextUserData>& user_data) {
    m_pUserData = user_data;
  }

 private:
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
  uint32_t m_dwCharStyles = 0;
  UnownedPtr<std::vector<CFGAS_Char>> m_pChars;
  RetainPtr<CFGAS_TextUserData> m_pUserData;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_BREAKPIECE_H_
