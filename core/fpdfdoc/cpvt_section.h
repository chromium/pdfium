// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPVT_SECTION_H_
#define CORE_FPDFDOC_CPVT_SECTION_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "core/fpdfdoc/cpvt_floatrect.h"
#include "core/fpdfdoc/cpvt_lineinfo.h"
#include "core/fpdfdoc/cpvt_wordinfo.h"
#include "core/fpdfdoc/cpvt_wordrange.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/unowned_ptr.h"

class CPVT_VariableText;
struct CPVT_LineInfo;
struct CPVT_WordPlace;

class CPVT_Section final {
 public:
  class Line {
   public:
    explicit Line(const CPVT_LineInfo& lineinfo);
    ~Line();

    CPVT_WordPlace GetBeginWordPlace() const;
    CPVT_WordPlace GetEndWordPlace() const;
    CPVT_WordPlace GetPrevWordPlace(const CPVT_WordPlace& place) const;
    CPVT_WordPlace GetNextWordPlace(const CPVT_WordPlace& place) const;
    CPVT_WordPlace line_place_;
    CPVT_LineInfo line_info_;
  };

  explicit CPVT_Section(CPVT_VariableText* pVT);
  ~CPVT_Section();

  void ResetLinePlace();
  CPVT_WordPlace AddWord(const CPVT_WordPlace& place,
                         const CPVT_WordInfo& wordinfo);
  CPVT_WordPlace AddLine(const CPVT_LineInfo& lineinfo);
  void ClearWords(const CPVT_WordRange& PlaceRange);
  void ClearWord(const CPVT_WordPlace& place);
  CPVT_FloatRect Rearrange();
  CFX_SizeF GetSectionSize(float fFontSize);
  CPVT_WordPlace GetBeginWordPlace() const;
  CPVT_WordPlace GetEndWordPlace() const;
  CPVT_WordPlace GetPrevWordPlace(const CPVT_WordPlace& place) const;
  CPVT_WordPlace GetNextWordPlace(const CPVT_WordPlace& place) const;
  void UpdateWordPlace(CPVT_WordPlace& place) const;
  CPVT_WordPlace SearchWordPlace(const CFX_PointF& point) const;
  CPVT_WordPlace SearchWordPlace(float fx,
                                 const CPVT_WordPlace& lineplace) const;
  CPVT_WordPlace SearchWordPlace(float fx, const CPVT_WordRange& range) const;

  void SetPlace(const CPVT_WordPlace& place) { sec_place_ = place; }
  void SetPlaceIndex(int32_t index) { sec_place_.nSecIndex = index; }
  const CPVT_FloatRect& GetRect() const { return rect_; }
  void SetRect(const CPVT_FloatRect& rect) { rect_ = rect; }

  int32_t GetLineArraySize() const;
  const Line* GetLineFromArray(int32_t index) const;
  int32_t GetWordArraySize() const;
  const CPVT_WordInfo* GetWordFromArray(int32_t index) const;
  void EraseWordsFrom(int32_t index);

 private:
  CPVT_FloatRect RearrangeCharArray() const;
  CPVT_FloatRect RearrangeTypeset();
  CPVT_FloatRect SplitLines(bool bTypeset, float fFontSize);
  CPVT_FloatRect OutputLines(const CPVT_FloatRect& rect) const;

  void ClearLeftWords(int32_t nWordIndex);
  void ClearRightWords(int32_t nWordIndex);
  void ClearMidWords(int32_t nBeginIndex, int32_t nEndIndex);

  CPVT_WordPlace sec_place_;
  CPVT_FloatRect rect_;
  std::vector<std::unique_ptr<Line>> line_array_;
  std::vector<std::unique_ptr<CPVT_WordInfo>> word_array_;
  UnownedPtr<CPVT_VariableText> const vt_;
};

#endif  // CORE_FPDFDOC_CPVT_SECTION_H_
