// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_BIDI_H_
#define CORE_INCLUDE_FXCRT_FX_BIDI_H_

#include "fx_system.h"

// Processes characters and group them into segments based on text direction.
class CFX_BidiChar {
 public:
  enum Direction { NEUTRAL, LEFT, RIGHT };

  CFX_BidiChar();
  ~CFX_BidiChar();

  // Append a character and classify it as left, right, or neutral.
  // Returns true if the character has a different direction than the
  // existing direction to indicate there is a segment to process.
  bool AppendChar(FX_WCHAR wch);

  // Call this after the last character has been appended. AppendChar()
  // must not be called after this.
  // Returns true if there is still a segment to process.
  bool EndChar();

  // Get information about the segment to process.
  // The segment's start position and character count is returned in |iStart|
  // and |iCount|, respectively. Pass in null pointers if the information is
  // not needed.
  // Returns the segment direction.
  Direction GetBidiInfo(int32_t* iStart, int32_t* iCount) const;

 private:
  void SaveCurrentStateToLastState();

  // Position of the current segment.
  int32_t m_iCurStart;

  // Number of characters in the current segment.
  int32_t m_iCurCount;

  // Direction of the current segment.
  Direction m_CurBidi;

  // Number of characters in the last segment.
  int32_t m_iLastStart;

  // Number of characters in the last segment.
  int32_t m_iLastCount;

  // Direction of the last segment.
  Direction m_LastBidi;
};

#endif  // CORE_INCLUDE_FXCRT_FX_BIDI_H_
