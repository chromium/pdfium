// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPVT_VARIABLETEXT_H_
#define CORE_FPDFDOC_CPVT_VARIABLETEXT_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "core/fpdfdoc/cpvt_floatrect.h"
#include "core/fpdfdoc/cpvt_line.h"
#include "core/fpdfdoc/cpvt_lineinfo.h"
#include "core/fpdfdoc/cpvt_wordplace.h"
#include "core/fpdfdoc/cpvt_wordrange.h"
#include "core/fxcrt/fx_codepage_forward.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"

class CPVT_Section;
class CPVT_Word;
class IPVT_FontMap;
struct CPVT_WordInfo;

class CPVT_VariableText {
 public:
  class Iterator {
   public:
    explicit Iterator(CPVT_VariableText* pVT);
    ~Iterator();

    bool NextWord();
    bool NextLine();
    bool GetWord(CPVT_Word& word) const;
    bool GetLine(CPVT_Line& line) const;
    void SetAt(int32_t nWordIndex);
    void SetAt(const CPVT_WordPlace& place);
    const CPVT_WordPlace& GetWordPlace() const { return cur_pos_; }

   private:
    CPVT_WordPlace cur_pos_;
    UnownedPtr<const CPVT_VariableText> const vt_;
  };

  class Provider {
   public:
    explicit Provider(IPVT_FontMap* font_map);
    virtual ~Provider();

    virtual int GetCharWidth(int32_t nFontIndex, uint16_t word);
    virtual int32_t GetTypeAscent(int32_t nFontIndex);
    virtual int32_t GetTypeDescent(int32_t nFontIndex);
    virtual int32_t GetWordFontIndex(uint16_t word,
                                     FX_Charset charset,
                                     int32_t nFontIndex);
    virtual int32_t GetDefaultFontIndex();

    IPVT_FontMap* GetFontMap() { return font_map_; }

   private:
    UnownedPtr<IPVT_FontMap> const font_map_;
  };

  explicit CPVT_VariableText(Provider* Provider);
  ~CPVT_VariableText();

  void SetProvider(Provider* pProvider);
  Provider* GetProvider();
  CPVT_VariableText::Iterator* GetIterator();

  CFX_FloatRect GetContentRect() const;
  void SetPlateRect(const CFX_FloatRect& rect);
  const CFX_FloatRect& GetPlateRect() const;

  void SetAlignment(int32_t nFormat) { alignment_ = nFormat; }
  void SetPasswordChar(uint16_t wSubWord) { sub_word_ = wSubWord; }
  void SetLimitChar(int32_t nLimitChar) { limit_char_ = nLimitChar; }
  void SetMultiLine(bool bMultiLine) { multi_line_ = bMultiLine; }
  void SetAutoReturn(bool bAuto) { limit_width_ = bAuto; }
  void SetFontSize(float fFontSize) { font_size_ = fFontSize; }
  void SetCharArray(int32_t nCharArray) { char_array_ = nCharArray; }
  void SetAutoFontSize(bool bAuto) { auto_font_size_ = bAuto; }
  void Initialize();

  bool IsValid() const { return initialized_; }

  void RearrangeAll();
  void RearrangePart(const CPVT_WordRange& PlaceRange);
  void SetText(const WideString& text);
  CPVT_WordPlace InsertWord(const CPVT_WordPlace& place,
                            uint16_t word,
                            FX_Charset charset);
  CPVT_WordPlace InsertSection(const CPVT_WordPlace& place);
  CPVT_WordPlace DeleteWords(const CPVT_WordRange& PlaceRange);
  CPVT_WordPlace DeleteWord(const CPVT_WordPlace& place);
  CPVT_WordPlace BackSpaceWord(const CPVT_WordPlace& place);

  int32_t GetTotalWords() const;
  float GetFontSize() const { return font_size_; }
  int32_t GetAlignment() const { return alignment_; }
  uint16_t GetPasswordChar() const { return GetSubWord(); }
  int32_t GetCharArray() const { return char_array_; }
  int32_t GetLimitChar() const { return limit_char_; }
  bool IsMultiLine() const { return multi_line_; }
  bool IsAutoReturn() const { return limit_width_; }

  CPVT_WordPlace GetBeginWordPlace() const;
  CPVT_WordPlace GetEndWordPlace() const;
  CPVT_WordPlace GetPrevWordPlace(const CPVT_WordPlace& place) const;
  CPVT_WordPlace GetNextWordPlace(const CPVT_WordPlace& place) const;
  CPVT_WordPlace SearchWordPlace(const CFX_PointF& point) const;
  CPVT_WordPlace GetUpWordPlace(const CPVT_WordPlace& place,
                                const CFX_PointF& point) const;
  CPVT_WordPlace GetDownWordPlace(const CPVT_WordPlace& place,
                                  const CFX_PointF& point) const;
  CPVT_WordPlace GetLineBeginPlace(const CPVT_WordPlace& place) const;
  CPVT_WordPlace GetLineEndPlace(const CPVT_WordPlace& place) const;
  CPVT_WordPlace GetSectionBeginPlace(const CPVT_WordPlace& place) const;
  CPVT_WordPlace GetSectionEndPlace(const CPVT_WordPlace& place) const;
  void UpdateWordPlace(CPVT_WordPlace& place) const;
  CPVT_WordPlace PrevLineHeaderPlace(const CPVT_WordPlace& place) const;
  CPVT_WordPlace NextLineHeaderPlace(const CPVT_WordPlace& place) const;
  int32_t WordPlaceToWordIndex(const CPVT_WordPlace& place) const;
  CPVT_WordPlace WordIndexToWordPlace(int32_t index) const;

  uint16_t GetSubWord() const { return sub_word_; }

  float GetPlateWidth() const { return plate_rect_.right - plate_rect_.left; }
  float GetPlateHeight() const { return plate_rect_.top - plate_rect_.bottom; }
  CFX_PointF GetBTPoint() const;
  CFX_PointF GetETPoint() const;

  CFX_PointF InToOut(const CFX_PointF& point) const;
  CFX_PointF OutToIn(const CFX_PointF& point) const;
  CFX_FloatRect InToOut(const CPVT_FloatRect& rect) const;

  float GetFontAscent(int32_t nFontIndex, float fFontSize) const;
  float GetFontDescent(int32_t nFontIndex, float fFontSize) const;
  int32_t GetDefaultFontIndex();
  float GetLineLeading();
  float GetWordWidth(const CPVT_WordInfo& WordInfo) const;
  float GetWordWidth(int32_t nFontIndex,
                     uint16_t Word,
                     uint16_t SubWord,
                     float fFontSize,
                     float fWordTail) const;
  float GetWordAscent(const CPVT_WordInfo& WordInfo) const;
  float GetWordDescent(const CPVT_WordInfo& WordInfo) const;
  float GetWordAscent(const CPVT_WordInfo& WordInfo, float fFontSize) const;
  float GetWordDescent(const CPVT_WordInfo& WordInfo, float fFontSize) const;
  float GetLineAscent();
  float GetLineDescent();
  float GetLineIndent();

 private:
  int GetCharWidth(int32_t nFontIndex, uint16_t Word, uint16_t SubWord) const;
  int32_t GetWordFontIndex(uint16_t word,
                           FX_Charset charset,
                           int32_t nFontIndex);

  CPVT_WordPlace AddSection(const CPVT_WordPlace& place);
  CPVT_WordPlace AddLine(const CPVT_WordPlace& place,
                         const CPVT_LineInfo& lineinfo);
  CPVT_WordPlace AddWord(const CPVT_WordPlace& place,
                         const CPVT_WordInfo& wordinfo);
  float GetWordFontSize() const;

  void ClearSectionRightWords(const CPVT_WordPlace& place);

  void ClearEmptySection(const CPVT_WordPlace& place);
  void ClearEmptySections(const CPVT_WordRange& PlaceRange);
  void LinkLatterSection(const CPVT_WordPlace& place);
  void ClearWords(const CPVT_WordRange& PlaceRange);
  CPVT_WordPlace ClearLeftWord(const CPVT_WordPlace& place);
  CPVT_WordPlace ClearRightWord(const CPVT_WordPlace& place);

  void Rearrange(const CPVT_WordRange& PlaceRange);
  float GetAutoFontSize();
  bool IsBigger(float fFontSize) const;
  CPVT_FloatRect RearrangeSections(const CPVT_WordRange& PlaceRange);

  bool initialized_ = false;
  bool multi_line_ = false;
  bool limit_width_ = false;
  bool auto_font_size_ = false;
  uint16_t sub_word_ = 0;
  int32_t limit_char_ = 0;
  int32_t char_array_ = 0;
  int32_t alignment_ = 0;
  float line_leading_ = 0.0f;
  float font_size_ = 0.0f;
  std::vector<std::unique_ptr<CPVT_Section>> section_array_;
  UnownedPtr<Provider> vt_provider_;
  std::unique_ptr<Iterator> vt_iterator_;
  CFX_FloatRect plate_rect_;
  CPVT_FloatRect content_rect_;
};

#endif  // CORE_FPDFDOC_CPVT_VARIABLETEXT_H_
