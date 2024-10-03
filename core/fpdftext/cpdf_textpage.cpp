// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdftext/cpdf_textpage.h"

#include <math.h>
#include <stdint.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_formobject.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_textobject.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdftext/unicodenormalizationdata.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_bidi.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_unicode.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/stl_util.h"

namespace {

constexpr float kDefaultFontSize = 1.0f;
constexpr float kSizeEpsilon = 0.01f;
constexpr std::array<pdfium::span<const uint16_t>, 3>
    kUnicodeDataNormalizationMaps = {{kUnicodeDataNormalizationMap2,
                                      kUnicodeDataNormalizationMap3,
                                      kUnicodeDataNormalizationMap4}};

float NormalizeThreshold(float threshold, int t1, int t2, int t3) {
  DCHECK(t1 < t2);
  DCHECK(t2 < t3);
  if (threshold < t1)
    return threshold / 2.0f;
  if (threshold < t2)
    return threshold / 4.0f;
  if (threshold < t3)
    return threshold / 5.0f;
  return threshold / 6.0f;
}

float CalculateBaseSpace(const CPDF_TextObject* pTextObj,
                         const CFX_Matrix& matrix) {
  const size_t nItems = pTextObj->CountItems();
  const float char_space = pTextObj->text_state().GetCharSpace();
  if (char_space == 0.0f || nItems < 3) {
    return 0.0f;
  }

  bool bAllChar = true;
  const float spacing = matrix.TransformDistance(char_space);
  const float fontsize_h = pTextObj->text_state().GetFontSizeH();
  float base_space = spacing;
  for (size_t i = 0; i < nItems; ++i) {
    CPDF_TextObject::Item item = pTextObj->GetItemInfo(i);
    if (item.m_CharCode == 0xffffffff) {
      float kerning = -fontsize_h * item.m_Origin.x / 1000;
      base_space = std::min(base_space, kerning + spacing);
      bAllChar = false;
    }
  }
  if (base_space < 0.0 || (nItems == 3 && !bAllChar)) {
    return 0.0f;
  }

  return base_space;
}

float CalculateBaseSpaceAdjustment(const CPDF_TextObject* pTextObj,
                                   const CFX_Matrix& matrix) {
  float char_space = pTextObj->text_state().GetCharSpace();
  if (char_space > 0.001f) {
    return -matrix.TransformDistance(char_space);
  }
  if (char_space < -0.001f) {
    return matrix.TransformDistance(fabs(char_space));
  }
  return 0.0f;
}

DataVector<wchar_t> GetUnicodeNormalization(wchar_t wch) {
  wch = wch & 0xFFFF;
  wchar_t wFind = kUnicodeDataNormalization[wch];
  if (!wFind) {
    return DataVector<wchar_t>(1, wch);
  }
  if (wFind >= 0x8000) {
    return DataVector<wchar_t>(1,
                               kUnicodeDataNormalizationMap1[wFind - 0x8000]);
  }
  wch = wFind & 0x0FFF;
  wFind >>= 12;
  auto pMap = kUnicodeDataNormalizationMaps[wFind - 2].subspan(wch);
  if (wFind == 4) {
    wFind = pMap.front();
    pMap = pMap.subspan(1);
  }
  const auto range = pMap.first(wFind);
  return DataVector<wchar_t>(range.begin(), range.end());
}

float MaskPercentFilled(const std::vector<bool>& mask,
                        int32_t start,
                        int32_t end) {
  if (start >= end)
    return 0;
  float count = std::count_if(mask.begin() + start, mask.begin() + end,
                              [](bool r) { return r; });
  return count / (end - start);
}

bool IsControlChar(const CPDF_TextPage::CharInfo& char_info) {
  switch (char_info.unicode()) {
    case 0x2:
    case 0x3:
    case 0x93:
    case 0x94:
    case 0x96:
    case 0x97:
    case 0x98:
    case 0xfffe:
      return char_info.char_type() != CPDF_TextPage::CharType::kHyphen;
    default:
      return false;
  }
}

bool IsHyphenCode(wchar_t c) {
  return c == 0x2D || c == 0xAD;
}

bool IsRectIntersect(const CFX_FloatRect& rect1, const CFX_FloatRect& rect2) {
  CFX_FloatRect rect = rect1;
  rect.Intersect(rect2);
  return !rect.IsEmpty();
}

bool IsRightToLeft(const CPDF_TextObject& text_obj) {
  RetainPtr<const CPDF_Font> font = text_obj.GetFont();
  const size_t nItems = text_obj.CountItems();
  WideString str;
  str.Reserve(nItems);
  for (size_t i = 0; i < nItems; ++i) {
    CPDF_TextObject::Item item = text_obj.GetItemInfo(i);
    if (item.m_CharCode == 0xffffffff)
      continue;
    WideString unicode = font->UnicodeFromCharCode(item.m_CharCode);
    wchar_t wChar = !unicode.IsEmpty() ? unicode[0] : 0;
    if (wChar == 0)
      wChar = item.m_CharCode;
    if (wChar)
      str += wChar;
  }
  return CFX_BidiString(str).OverallDirection() ==
         CFX_BidiChar::Direction::kRight;
}

int GetCharWidth(uint32_t charCode, CPDF_Font* pFont) {
  if (charCode == CPDF_Font::kInvalidCharCode)
    return 0;

  int w = pFont->GetCharWidthF(charCode);
  if (w > 0)
    return w;

  ByteString str;
  pFont->AppendChar(&str, charCode);
  w = pFont->GetStringWidth(str.AsStringView());
  if (w > 0)
    return w;

  FX_RECT rect = pFont->GetCharBBox(charCode);
  if (!rect.Valid())
    return 0;

  return std::max(rect.Width(), 0);
}

float CalculateSpaceThreshold(CPDF_Font* font,
                              float fontsize_h,
                              uint32_t char_code) {
  const uint32_t space_charcode = font->CharCodeFromUnicode(' ');
  float threshold = 0;
  if (space_charcode != CPDF_Font::kInvalidCharCode) {
    threshold = fontsize_h * font->GetCharWidthF(space_charcode) / 1000;
  }
  if (threshold > fontsize_h / 3) {
    threshold = 0;
  } else {
    threshold /= 2;
  }
  if (threshold == 0) {
    threshold = GetCharWidth(char_code, font);
    threshold = NormalizeThreshold(threshold, 300, 500, 700);
    threshold = fontsize_h * threshold / 1000;
  }
  return threshold;
}

bool GenerateSpace(const CFX_PointF& pos,
                   float last_pos,
                   float this_width,
                   float last_width,
                   float threshold) {
  if (fabs(last_pos + last_width - pos.x) <= threshold)
    return false;

  float threshold_pos = threshold + last_width;
  float pos_difference = pos.x - last_pos;
  if (fabs(pos_difference) > threshold_pos)
    return true;
  if (pos.x < 0 && -threshold_pos > pos_difference)
    return true;
  return pos_difference > this_width + last_width;
}

bool EndHorizontalLine(const CFX_FloatRect& this_rect,
                       const CFX_FloatRect& prev_rect) {
  if (this_rect.Height() <= 4.5 || prev_rect.Height() <= 4.5)
    return false;

  float top = std::min(this_rect.top, prev_rect.top);
  float bottom = std::max(this_rect.bottom, prev_rect.bottom);
  return bottom >= top;
}

bool EndVerticalLine(const CFX_FloatRect& this_rect,
                     const CFX_FloatRect& prev_rect,
                     const CFX_FloatRect& curline_rect,
                     float this_fontsize,
                     float prev_fontsize) {
  if (this_rect.Width() <= this_fontsize * 0.1f ||
      prev_rect.Width() <= prev_fontsize * 0.1f) {
    return false;
  }

  float left = std::max(this_rect.left, curline_rect.left);
  float right = std::min(this_rect.right, curline_rect.right);
  return right <= left;
}

CFX_Matrix GetPageMatrix(const CPDF_Page* pPage) {
  const FX_RECT rect(0, 0, static_cast<int>(pPage->GetPageWidth()),
                     static_cast<int>(pPage->GetPageHeight()));
  return pPage->GetDisplayMatrix(rect, 0);
}

float GetFontSize(const CPDF_TextObject* text_object) {
  bool has_font = text_object && text_object->GetFont();
  return has_font ? text_object->GetFontSize() : kDefaultFontSize;
}

CFX_FloatRect GetLooseBounds(const CPDF_TextPage::CharInfo& charinfo) {
  float font_size = GetFontSize(charinfo.text_object());
  if (charinfo.text_object() && !FXSYS_IsFloatZero(font_size)) {
    bool is_vert_writing = charinfo.text_object()->GetFont()->IsVertWriting();
    if (is_vert_writing && charinfo.text_object()->GetFont()->IsCIDFont()) {
      CPDF_CIDFont* pCIDFont = charinfo.text_object()->GetFont()->AsCIDFont();
      uint16_t cid = pCIDFont->CIDFromCharCode(charinfo.char_code());

      CFX_Point16 vertical_origin = pCIDFont->GetVertOrigin(cid);
      double offsetx = (vertical_origin.x - 500) * font_size / 1000.0;
      double offsety = vertical_origin.y * font_size / 1000.0;
      int16_t vert_width = pCIDFont->GetVertWidth(cid);
      double height = vert_width * font_size / 1000.0;

      float left = charinfo.origin().x + offsetx;
      float right = left + font_size;
      float bottom = charinfo.origin().y + offsety;
      float top = bottom + height;
      return CFX_FloatRect(left, bottom, right, top);
    }

    int ascent = charinfo.text_object()->GetFont()->GetTypeAscent();
    int descent = charinfo.text_object()->GetFont()->GetTypeDescent();
    if (ascent != descent) {
      float width = charinfo.matrix().a *
                    charinfo.text_object()->GetCharWidth(charinfo.char_code());
      float font_scale = charinfo.matrix().a * font_size / (ascent - descent);

      float left = charinfo.origin().x;
      float right = charinfo.origin().x + (is_vert_writing ? -width : width);
      float bottom = charinfo.origin().y + descent * font_scale;
      float top = charinfo.origin().y + ascent * font_scale;
      return CFX_FloatRect(left, bottom, right, top);
    }
  }

  // Fallback to the tight bounds in empty text scenarios, or bad font metrics
  return charinfo.char_box();
}

}  // namespace

CPDF_TextPage::TransformedTextObject::TransformedTextObject() = default;

CPDF_TextPage::TransformedTextObject::TransformedTextObject(
    const TransformedTextObject& that) = default;

CPDF_TextPage::TransformedTextObject::~TransformedTextObject() = default;

CPDF_TextPage::CharInfo::CharInfo() = default;

CPDF_TextPage::CharInfo::CharInfo(CharType char_type,
                                  uint32_t char_code,
                                  wchar_t unicode,
                                  CFX_PointF origin,
                                  CFX_FloatRect char_box,
                                  CFX_Matrix matrix,
                                  CPDF_TextObject* text_object)
    : char_type_(char_type),
      unicode_(unicode),
      char_code_(char_code),
      origin_(origin),
      char_box_(char_box),
      matrix_(matrix),
      text_object_(text_object) {}

CPDF_TextPage::CharInfo::CharInfo(const CharInfo&) = default;

CPDF_TextPage::CharInfo::~CharInfo() = default;

CPDF_TextPage::CPDF_TextPage(const CPDF_Page* pPage, bool rtl)
    : m_pPage(pPage), m_rtl(rtl), m_DisplayMatrix(GetPageMatrix(pPage)) {
  Init();
}

CPDF_TextPage::~CPDF_TextPage() = default;

void CPDF_TextPage::Init() {
  m_TextBuf.SetAllocStep(10240);
  ProcessObject();

  const int nCount = CountChars();
  if (nCount)
    m_CharIndices.push_back({0, 0});

  bool skipped = false;
  for (int i = 0; i < nCount; ++i) {
    const CharInfo& charinfo = m_CharList[i];
    if (charinfo.char_type() == CharType::kGenerated ||
        (charinfo.unicode() != 0 && !IsControlChar(charinfo)) ||
        (charinfo.unicode() == 0 && charinfo.char_code() != 0)) {
      m_CharIndices.back().count++;
      skipped = true;
    } else {
      if (skipped) {
        m_CharIndices.push_back({i + 1, 0});
        skipped = false;
      } else {
        m_CharIndices.back().index = i + 1;
      }
    }
  }
}

int CPDF_TextPage::CountChars() const {
  return fxcrt::CollectionSize<int>(m_CharList);
}

int CPDF_TextPage::CharIndexFromTextIndex(int text_index) const {
  int count = 0;
  for (const auto& info : m_CharIndices) {
    count += info.count;
    if (count > text_index)
      return text_index - count + info.count + info.index;
  }
  return -1;
}

int CPDF_TextPage::TextIndexFromCharIndex(int char_index) const {
  int count = 0;
  for (const auto& info : m_CharIndices) {
    int text_index = char_index - info.index;
    if (text_index < info.count)
      return text_index >= 0 ? text_index + count : -1;

    count += info.count;
  }
  return -1;
}

std::vector<CFX_FloatRect> CPDF_TextPage::GetRectArray(int start,
                                                       int count) const {
  std::vector<CFX_FloatRect> rects;
  if (start < 0 || count == 0)
    return rects;

  const int number_of_chars = CountChars();
  if (start >= number_of_chars)
    return rects;

  if (count < 0 || start + count > number_of_chars)
    count = number_of_chars - start;
  DCHECK(count > 0);

  const CPDF_TextObject* text_object = nullptr;
  CFX_FloatRect rect;
  int pos = start;
  bool is_new_rect = true;
  while (count--) {
    const CharInfo& charinfo = m_CharList[pos++];
    if (charinfo.char_type() == CharType::kGenerated) {
      continue;
    }
    if (charinfo.char_box().Width() < kSizeEpsilon ||
        charinfo.char_box().Height() < kSizeEpsilon) {
      continue;
    }
    if (!text_object)
      text_object = charinfo.text_object();
    if (text_object != charinfo.text_object()) {
      rects.push_back(rect);
      text_object = charinfo.text_object();
      is_new_rect = true;
    }
    if (is_new_rect) {
      is_new_rect = false;
      rect = charinfo.char_box();
      rect.Normalize();
      continue;
    }
    rect.Union(charinfo.char_box());
  }
  rects.push_back(rect);
  return rects;
}

int CPDF_TextPage::GetIndexAtPos(const CFX_PointF& point,
                                 const CFX_SizeF& tolerance) const {
  int pos;
  int NearPos = -1;
  double xdif = 5000;
  double ydif = 5000;
  const int nCount = CountChars();
  for (pos = 0; pos < nCount; ++pos) {
    const CFX_FloatRect& orig_charrect = m_CharList[pos].char_box();
    if (orig_charrect.Contains(point))
      break;

    if (tolerance.width <= 0 && tolerance.height <= 0)
      continue;

    CFX_FloatRect charrect = orig_charrect;
    charrect.Normalize();
    CFX_FloatRect char_rect_ext(charrect.left - tolerance.width / 2,
                                charrect.bottom - tolerance.height / 2,
                                charrect.right + tolerance.width / 2,
                                charrect.top + tolerance.height / 2);
    if (!char_rect_ext.Contains(point))
      continue;

    double curXdif =
        std::min(fabs(point.x - charrect.left), fabs(point.x - charrect.right));
    double curYdif =
        std::min(fabs(point.y - charrect.bottom), fabs(point.y - charrect.top));
    if (curYdif + curXdif < xdif + ydif) {
      ydif = curYdif;
      xdif = curXdif;
      NearPos = pos;
    }
  }
  return pos < nCount ? pos : NearPos;
}

WideString CPDF_TextPage::GetTextByPredicate(
    const std::function<bool(const CharInfo&)>& predicate) const {
  float posy = 0;
  bool IsContainPreChar = false;
  bool IsAddLineFeed = false;
  WideString strText;
  for (const auto& charinfo : m_CharList) {
    if (predicate(charinfo)) {
      if (fabs(posy - charinfo.origin().y) > 0 && !IsContainPreChar &&
          IsAddLineFeed) {
        posy = charinfo.origin().y;
        if (!strText.IsEmpty())
          strText += L"\r\n";
      }
      IsContainPreChar = true;
      IsAddLineFeed = false;
      if (charinfo.unicode()) {
        strText += charinfo.unicode();
      }
    } else if (charinfo.unicode() == L' ') {
      if (IsContainPreChar) {
        strText += L' ';
        IsContainPreChar = false;
        IsAddLineFeed = false;
      }
    } else {
      IsContainPreChar = false;
      IsAddLineFeed = true;
    }
  }
  return strText;
}

WideString CPDF_TextPage::GetTextByRect(const CFX_FloatRect& rect) const {
  return GetTextByPredicate([&rect](const CharInfo& charinfo) {
    return IsRectIntersect(rect, charinfo.char_box());
  });
}

WideString CPDF_TextPage::GetTextByObject(
    const CPDF_TextObject* pTextObj) const {
  return GetTextByPredicate([pTextObj](const CharInfo& charinfo) {
    return charinfo.text_object() == pTextObj;
  });
}

const CPDF_TextPage::CharInfo& CPDF_TextPage::GetCharInfo(size_t index) const {
  CHECK_LT(index, m_CharList.size());
  return m_CharList[index];
}

CPDF_TextPage::CharInfo& CPDF_TextPage::GetCharInfo(size_t index) {
  CHECK_LT(index, m_CharList.size());
  return m_CharList[index];
}

float CPDF_TextPage::GetCharFontSize(size_t index) const {
  CHECK_LT(index, m_CharList.size());
  return GetFontSize(m_CharList[index].text_object());
}

CFX_FloatRect CPDF_TextPage::GetCharLooseBounds(size_t index) const {
  return GetLooseBounds(GetCharInfo(index));
}

WideString CPDF_TextPage::GetPageText(int start, int count) const {
  if (start < 0 || start >= CountChars() || count <= 0 || m_CharList.empty() ||
      m_TextBuf.IsEmpty()) {
    return WideString();
  }

  const int count_chars = CountChars();
  int text_start = TextIndexFromCharIndex(start);

  // If the character at |start| is a non-printing character, then
  // TextIndexFromCharIndex will return -1, so scan ahead to the first printing
  // character.
  while (text_start < 0) {
    if (start >= count_chars)
      return WideString();
    start++;
    text_start = TextIndexFromCharIndex(start);
  }

  count = std::min(count, count_chars - start);

  int last = start + count - 1;
  int text_last = TextIndexFromCharIndex(last);

  // If the character at |last| is a non-printing character, then
  // TextIndexFromCharIndex will return -1, so scan back to the last printing
  // character.
  while (text_last < 0) {
    if (last < text_start)
      return WideString();

    last--;
    text_last = TextIndexFromCharIndex(last);
  }

  if (text_last < text_start)
    return WideString();

  int text_count = text_last - text_start + 1;

  return WideString(m_TextBuf.AsStringView().Substr(text_start, text_count));
}

int CPDF_TextPage::CountRects(int start, int nCount) {
  if (start < 0)
    return -1;

  m_SelRects = GetRectArray(start, nCount);
  return fxcrt::CollectionSize<int>(m_SelRects);
}

bool CPDF_TextPage::GetRect(int rectIndex, CFX_FloatRect* pRect) const {
  if (!fxcrt::IndexInBounds(m_SelRects, rectIndex))
    return false;

  *pRect = m_SelRects[rectIndex];
  return true;
}

CPDF_TextPage::TextOrientation CPDF_TextPage::FindTextlineFlowOrientation()
    const {
  DCHECK_NE(m_pPage->GetPageObjectCount(), 0u);

  const int32_t nPageWidth = static_cast<int32_t>(m_pPage->GetPageWidth());
  const int32_t nPageHeight = static_cast<int32_t>(m_pPage->GetPageHeight());
  if (nPageWidth <= 0 || nPageHeight <= 0)
    return TextOrientation::kUnknown;

  std::vector<bool> nHorizontalMask(nPageWidth);
  std::vector<bool> nVerticalMask(nPageHeight);
  float fLineHeight = 0.0f;
  int32_t nStartH = nPageWidth;
  int32_t nEndH = 0;
  int32_t nStartV = nPageHeight;
  int32_t nEndV = 0;
  for (const auto& pPageObj : *m_pPage) {
    if (!pPageObj->IsText())
      continue;

    int32_t minH = static_cast<int32_t>(
        std::clamp<float>(pPageObj->GetRect().left, 0.0f, nPageWidth));
    int32_t maxH = static_cast<int32_t>(
        std::clamp<float>(pPageObj->GetRect().right, 0.0f, nPageWidth));
    int32_t minV = static_cast<int32_t>(
        std::clamp<float>(pPageObj->GetRect().bottom, 0.0f, nPageHeight));
    int32_t maxV = static_cast<int32_t>(
        std::clamp<float>(pPageObj->GetRect().top, 0.0f, nPageHeight));
    if (minH >= maxH || minV >= maxV)
      continue;

    for (int32_t i = minH; i < maxH; ++i)
      nHorizontalMask[i] = true;
    for (int32_t i = minV; i < maxV; ++i)
      nVerticalMask[i] = true;

    nStartH = std::min(nStartH, minH);
    nEndH = std::max(nEndH, maxH);
    nStartV = std::min(nStartV, minV);
    nEndV = std::max(nEndV, maxV);

    if (fLineHeight <= 0.0f)
      fLineHeight = pPageObj->GetRect().Height();
  }
  const int32_t nDoubleLineHeight = 2 * fLineHeight;
  if ((nEndV - nStartV) < nDoubleLineHeight)
    return TextOrientation::kHorizontal;
  if ((nEndH - nStartH) < nDoubleLineHeight)
    return TextOrientation::kVertical;

  const float nSumH = MaskPercentFilled(nHorizontalMask, nStartH, nEndH);
  if (nSumH > 0.8f)
    return TextOrientation::kHorizontal;

  const float nSumV = MaskPercentFilled(nVerticalMask, nStartV, nEndV);
  if (nSumH > nSumV)
    return TextOrientation::kHorizontal;
  if (nSumH < nSumV)
    return TextOrientation::kVertical;
  return TextOrientation::kUnknown;
}

void CPDF_TextPage::AppendGeneratedCharacter(wchar_t unicode,
                                             const CFX_Matrix& form_matrix,
                                             bool use_temp_buffer) {
  std::optional<CharInfo> charinfo = GenerateCharInfo(unicode, form_matrix);
  if (!charinfo.has_value()) {
    return;
  }

  if (use_temp_buffer) {
    m_TempTextBuf.AppendChar(unicode);
    m_TempCharList.push_back(charinfo.value());
  } else {
    m_TextBuf.AppendChar(unicode);
    m_CharList.push_back(charinfo.value());
  }
}

void CPDF_TextPage::ProcessObject() {
  if (m_pPage->GetPageObjectCount() == 0)
    return;

  m_TextlineDir = FindTextlineFlowOrientation();
  for (auto it = m_pPage->begin(); it != m_pPage->end(); ++it) {
    CPDF_PageObject* pObj = it->get();
    if (pObj->IsText()) {
      ProcessTextObject(pObj->AsText(), CFX_Matrix(), m_pPage, it);
    } else if (pObj->IsForm()) {
      ProcessFormObject(pObj->AsForm(), CFX_Matrix());
    }
  }
  for (const auto& obj : mTextObjects)
    ProcessTextObject(obj);

  mTextObjects.clear();
  CloseTempLine();
}

void CPDF_TextPage::ProcessFormObject(CPDF_FormObject* pFormObj,
                                      const CFX_Matrix& form_matrix) {
  CFX_Matrix curFormMatrix = pFormObj->form_matrix() * form_matrix;
  const CPDF_PageObjectHolder* pHolder = pFormObj->form();
  for (auto it = pHolder->begin(); it != pHolder->end(); ++it) {
    CPDF_PageObject* pPageObj = it->get();
    if (pPageObj->IsText()) {
      ProcessTextObject(pPageObj->AsText(), curFormMatrix, pHolder, it);
    } else if (pPageObj->IsForm()) {
      ProcessFormObject(pPageObj->AsForm(), curFormMatrix);
    }
  }
}

void CPDF_TextPage::AddCharInfoByLRDirection(wchar_t wChar,
                                             const CharInfo& info) {
  CharInfo info2 = info;
  if (IsControlChar(info2)) {
    m_CharList.push_back(info2);
    return;
  }
  DataVector<wchar_t> normalized;
  if (wChar >= 0xFB00 && wChar <= 0xFB06)
    normalized = GetUnicodeNormalization(wChar);
  if (normalized.empty()) {
    m_TextBuf.AppendChar(wChar);
    m_CharList.push_back(info2);
    return;
  }
  info2.set_char_type(CharType::kPiece);
  for (wchar_t normalized_char : normalized) {
    info2.set_unicode(normalized_char);
    m_TextBuf.AppendChar(normalized_char);
    m_CharList.push_back(info2);
  }
}

void CPDF_TextPage::AddCharInfoByRLDirection(wchar_t wChar,
                                             const CharInfo& info) {
  CharInfo info2 = info;
  if (IsControlChar(info2)) {
    m_CharList.push_back(info2);
    return;
  }
  wChar = pdfium::unicode::GetMirrorChar(wChar);
  DataVector<wchar_t> normalized = GetUnicodeNormalization(wChar);
  if (normalized.empty()) {
    info2.set_unicode(wChar);
    m_TextBuf.AppendChar(wChar);
    m_CharList.push_back(info2);
    return;
  }
  info2.set_char_type(CharType::kPiece);
  for (wchar_t normalized_char : normalized) {
    info2.set_unicode(normalized_char);
    m_TextBuf.AppendChar(normalized_char);
    m_CharList.push_back(info2);
  }
}

void CPDF_TextPage::CloseTempLine() {
  if (m_TempCharList.empty())
    return;

  WideString str = m_TempTextBuf.MakeString();
  bool bPrevSpace = false;
  for (size_t i = 0; i < str.GetLength(); ++i) {
    if (str[i] != ' ') {
      bPrevSpace = false;
      continue;
    }
    if (bPrevSpace) {
      m_TempTextBuf.Delete(i, 1);
      m_TempCharList.erase(m_TempCharList.begin() + i);
      str.Delete(i);
      --i;
    }
    bPrevSpace = true;
  }
  CFX_BidiString bidi(str);
  if (m_rtl)
    bidi.SetOverallDirectionRight();
  CFX_BidiChar::Direction eCurrentDirection = bidi.OverallDirection();
  for (const auto& segment : bidi) {
    if (segment.direction == CFX_BidiChar::Direction::kRight ||
        (segment.direction == CFX_BidiChar::Direction::kNeutral &&
         eCurrentDirection == CFX_BidiChar::Direction::kRight)) {
      eCurrentDirection = CFX_BidiChar::Direction::kRight;
      for (int m = segment.start + segment.count; m > segment.start; --m)
        AddCharInfoByRLDirection(str[m - 1], m_TempCharList[m - 1]);
    } else {
      if (segment.direction != CFX_BidiChar::Direction::kLeftWeak) {
        eCurrentDirection = CFX_BidiChar::Direction::kLeft;
      }
      for (int m = segment.start; m < segment.start + segment.count; ++m)
        AddCharInfoByLRDirection(str[m], m_TempCharList[m]);
    }
  }
  m_TempCharList.clear();
  m_TempTextBuf.Delete(0, m_TempTextBuf.GetLength());
}

void CPDF_TextPage::ProcessTextObject(
    CPDF_TextObject* pTextObj,
    const CFX_Matrix& form_matrix,
    const CPDF_PageObjectHolder* pObjList,
    CPDF_PageObjectHolder::const_iterator ObjPos) {
  if (fabs(pTextObj->GetRect().Width()) < kSizeEpsilon)
    return;

  size_t count = mTextObjects.size();
  TransformedTextObject new_obj;
  new_obj.m_pTextObj = pTextObj;
  new_obj.m_formMatrix = form_matrix;
  if (count == 0) {
    mTextObjects.push_back(new_obj);
    return;
  }
  if (IsSameAsPreTextObject(pTextObj, pObjList, ObjPos))
    return;

  TransformedTextObject prev_obj = mTextObjects[count - 1];
  size_t nItem = prev_obj.m_pTextObj->CountItems();
  if (nItem == 0)
    return;

  CPDF_TextObject::Item item = prev_obj.m_pTextObj->GetItemInfo(nItem - 1);
  float prev_width =
      GetCharWidth(item.m_CharCode, prev_obj.m_pTextObj->GetFont().Get()) *
      prev_obj.m_pTextObj->GetFontSize() / 1000;

  CFX_Matrix prev_matrix =
      prev_obj.m_pTextObj->GetTextMatrix() * prev_obj.m_formMatrix;
  prev_width = prev_matrix.TransformDistance(fabs(prev_width));
  item = pTextObj->GetItemInfo(0);
  float this_width = GetCharWidth(item.m_CharCode, pTextObj->GetFont().Get()) *
                     pTextObj->GetFontSize() / 1000;
  this_width = fabs(this_width);

  CFX_Matrix this_matrix = pTextObj->GetTextMatrix() * form_matrix;
  this_width = this_matrix.TransformDistance(fabs(this_width));

  float threshold = std::max(prev_width, this_width) / 4;
  CFX_PointF prev_pos = m_DisplayMatrix.Transform(
      prev_obj.m_formMatrix.Transform(prev_obj.m_pTextObj->GetPos()));
  CFX_PointF this_pos =
      m_DisplayMatrix.Transform(form_matrix.Transform(pTextObj->GetPos()));
  if (fabs(this_pos.y - prev_pos.y) > threshold * 2) {
    for (size_t i = 0; i < count; ++i)
      ProcessTextObject(mTextObjects[i]);
    mTextObjects.clear();
    mTextObjects.push_back(new_obj);
    return;
  }

  for (size_t i = count; i > 0; --i) {
    TransformedTextObject prev_text_obj = mTextObjects[i - 1];
    CFX_PointF new_prev_pos =
        m_DisplayMatrix.Transform(prev_text_obj.m_formMatrix.Transform(
            prev_text_obj.m_pTextObj->GetPos()));
    if (this_pos.x >= new_prev_pos.x) {
      mTextObjects.insert(mTextObjects.begin() + i, new_obj);
      return;
    }
  }
  mTextObjects.insert(mTextObjects.begin(), new_obj);
}

CPDF_TextPage::MarkedContentState CPDF_TextPage::PreMarkedContent(
    const CPDF_TextObject* pTextObj) {
  const CPDF_ContentMarks* pMarks = pTextObj->GetContentMarks();
  const size_t nContentMarks = pMarks->CountItems();
  if (nContentMarks == 0)
    return MarkedContentState::kPass;

  WideString actual_text;
  bool bExist = false;
  RetainPtr<const CPDF_Dictionary> pDict;
  for (size_t i = 0; i < nContentMarks; ++i) {
    const CPDF_ContentMarkItem* item = pMarks->GetItem(i);
    pDict = item->GetParam();
    if (!pDict)
      continue;
    RetainPtr<const CPDF_String> temp = pDict->GetStringFor("ActualText");
    if (temp) {
      bExist = true;
      actual_text = temp->GetUnicodeText();
    }
  }
  if (!bExist)
    return MarkedContentState::kPass;

  if (m_pPrevTextObj) {
    const CPDF_ContentMarks* pPrevMarks = m_pPrevTextObj->GetContentMarks();
    if (pPrevMarks->CountItems() == nContentMarks &&
        pPrevMarks->GetItem(nContentMarks - 1)->GetParam() == pDict) {
      return MarkedContentState::kDone;
    }
  }

  if (actual_text.IsEmpty()) {
    return MarkedContentState::kPass;
  }

  RetainPtr<CPDF_Font> pFont = pTextObj->GetFont();
  bExist = false;
  for (size_t i = 0; i < actual_text.GetLength(); ++i) {
    if (pFont->CharCodeFromUnicode(actual_text[i]) !=
        CPDF_Font::kInvalidCharCode) {
      bExist = true;
      break;
    }
  }
  if (!bExist)
    return MarkedContentState::kPass;

  bExist = false;
  for (size_t i = 0; i < actual_text.GetLength(); ++i) {
    wchar_t wChar = actual_text[i];
    if ((wChar > 0x80 && wChar < 0xFFFD) || (wChar <= 0x80 && isprint(wChar))) {
      bExist = true;
      break;
    }
  }
  if (!bExist)
    return MarkedContentState::kDone;

  return MarkedContentState::kDelay;
}

void CPDF_TextPage::ProcessMarkedContent(const TransformedTextObject& obj) {
  CPDF_TextObject* const pTextObj = obj.m_pTextObj;
  const CPDF_ContentMarks* pMarks = pTextObj->GetContentMarks();
  const size_t nContentMarks = pMarks->CountItems();
  WideString actual_text;
  for (size_t n = 0; n < nContentMarks; ++n) {
    const CPDF_ContentMarkItem* item = pMarks->GetItem(n);
    RetainPtr<const CPDF_Dictionary> pDict = item->GetParam();
    if (pDict) {
      actual_text = pDict->GetUnicodeTextFor("ActualText");
    }
  }
  if (actual_text.IsEmpty()) {
    return;
  }

  const bool bR2L = IsRightToLeft(*pTextObj);
  CFX_Matrix matrix = pTextObj->GetTextMatrix() * obj.m_formMatrix;
  CFX_FloatRect rect = pTextObj->GetRect();
  float step = 0;

  if (bR2L) {
    rect.left = rect.right - (rect.Width() / actual_text.GetLength());
    step = -rect.Width();
  } else {
    rect.right = rect.left + (rect.Width() / actual_text.GetLength());
    step = rect.Width();
  }

  RetainPtr<CPDF_Font> const font = pTextObj->GetFont();
  for (size_t k = 0; k < actual_text.GetLength(); ++k) {
    wchar_t wChar = actual_text[k];
    if (wChar <= 0x80 && !isprint(wChar)) {
      wChar = 0x20;
    }
    if (wChar >= 0xFFFD) {
      continue;
    }

    CFX_FloatRect char_box(rect);
    char_box.Translate(k * step, 0);
    m_TempCharList.push_back(
        CharInfo(CharType::kPiece, font->CharCodeFromUnicode(wChar), wChar,
                 pTextObj->GetPos(), char_box, matrix, pTextObj));
  }
}

void CPDF_TextPage::FindPreviousTextObject() {
  const CharInfo* pPrevCharInfo = GetPrevCharInfo();
  if (!pPrevCharInfo)
    return;

  if (pPrevCharInfo->text_object()) {
    m_pPrevTextObj = pPrevCharInfo->text_object();
  }
}

void CPDF_TextPage::SwapTempTextBuf(size_t iCharListStartAppend,
                                    size_t iBufStartAppend) {
  DCHECK(!m_TempCharList.empty());
  if (iCharListStartAppend < m_TempCharList.size()) {
    auto fwd = m_TempCharList.begin() + iCharListStartAppend;
    auto rev = m_TempCharList.end() - 1;
    for (; fwd < rev; ++fwd, --rev) {
      std::swap(*fwd, *rev);
    }
  }
  pdfium::span<wchar_t> temp_span = m_TempTextBuf.GetWideSpan();
  DCHECK(!temp_span.empty());
  if (iBufStartAppend < temp_span.size()) {
    pdfium::span<wchar_t> reverse_span = temp_span.subspan(iBufStartAppend);
    std::reverse(reverse_span.begin(), reverse_span.end());
  }
}

void CPDF_TextPage::ProcessTextObject(const TransformedTextObject& obj) {
  CPDF_TextObject* const pTextObj = obj.m_pTextObj;
  if (fabs(pTextObj->GetRect().Width()) < kSizeEpsilon) {
    return;
  }

  const CFX_Matrix form_matrix = obj.m_formMatrix;
  const MarkedContentState ePreMKC = PreMarkedContent(pTextObj);
  if (ePreMKC == MarkedContentState::kDone) {
    m_pPrevTextObj = pTextObj;
    m_PrevMatrix = form_matrix;
    return;
  }

  if (m_pPrevTextObj) {
    GenerateCharacter type = ProcessInsertObject(pTextObj, form_matrix);
    if (type == GenerateCharacter::kLineBreak) {
      m_CurlineRect = pTextObj->GetRect();
    } else {
      m_CurlineRect.Union(pTextObj->GetRect());
    }

    if (!ProcessGenerateCharacter(type, pTextObj, form_matrix)) {
      return;
    }
  } else {
    m_CurlineRect = pTextObj->GetRect();
  }

  if (ePreMKC == MarkedContentState::kDelay) {
    ProcessMarkedContent(obj);
    m_pPrevTextObj = pTextObj;
    m_PrevMatrix = form_matrix;
    return;
  }

  m_pPrevTextObj = pTextObj;
  m_PrevMatrix = form_matrix;

  const bool bR2L = IsRightToLeft(*pTextObj);
  const CFX_Matrix matrix = pTextObj->GetTextMatrix() * form_matrix;
  const bool bIsBidiAndMirrorInverse =
      bR2L && (matrix.a * matrix.d - matrix.b * matrix.c) < 0;
  const size_t iBufStartAppend = m_TempTextBuf.GetLength();
  const size_t iCharListStartAppend = m_TempCharList.size();

  ProcessTextObjectItems(pTextObj, form_matrix, matrix);
  if (bIsBidiAndMirrorInverse) {
    SwapTempTextBuf(iCharListStartAppend, iBufStartAppend);
  }
}

CPDF_TextPage::TextOrientation CPDF_TextPage::GetTextObjectWritingMode(
    const CPDF_TextObject* pTextObj) const {
  size_t nChars = pTextObj->CountChars();
  if (nChars <= 1)
    return m_TextlineDir;

  CPDF_TextObject::Item first = pTextObj->GetCharInfo(0);
  CPDF_TextObject::Item last = pTextObj->GetCharInfo(nChars - 1);
  CFX_Matrix text_matrix = pTextObj->GetTextMatrix();
  first.m_Origin = text_matrix.Transform(first.m_Origin);
  last.m_Origin = text_matrix.Transform(last.m_Origin);

  static constexpr float kEpsilon = 0.0001f;
  float dX = fabs(last.m_Origin.x - first.m_Origin.x);
  float dY = fabs(last.m_Origin.y - first.m_Origin.y);
  if (dX <= kEpsilon && dY <= kEpsilon)
    return TextOrientation::kUnknown;

  static constexpr float kThreshold = 0.0872f;
  CFX_VectorF v(dX, dY);
  v.Normalize();
  bool bXUnderThreshold = v.x <= kThreshold;
  if (v.y <= kThreshold)
    return bXUnderThreshold ? m_TextlineDir : TextOrientation::kHorizontal;
  return bXUnderThreshold ? TextOrientation::kVertical : m_TextlineDir;
}

bool CPDF_TextPage::IsHyphen(wchar_t curChar) const {
  WideStringView curText = m_TempTextBuf.AsStringView();
  if (curText.IsEmpty())
    curText = m_TextBuf.AsStringView();

  if (curText.IsEmpty())
    return false;

  auto iter = curText.rbegin();
  for (; (iter + 1) != curText.rend() && *iter == 0x20; ++iter) {
    // Do nothing
  }

  if (!IsHyphenCode(*iter))
    return false;

  if ((iter + 1) != curText.rend()) {
    iter++;
    if (FXSYS_iswalpha(*iter) && FXSYS_iswalnum(curChar))
      return true;
  }

  const CharInfo* pPrevCharInfo = GetPrevCharInfo();
  return pPrevCharInfo && pPrevCharInfo->char_type() == CharType::kPiece &&
         IsHyphenCode(pPrevCharInfo->unicode());
}

const CPDF_TextPage::CharInfo* CPDF_TextPage::GetPrevCharInfo() const {
  if (!m_TempCharList.empty())
    return &m_TempCharList.back();
  return !m_CharList.empty() ? &m_CharList.back() : nullptr;
}

CPDF_TextPage::GenerateCharacter CPDF_TextPage::ProcessInsertObject(
    const CPDF_TextObject* pObj,
    const CFX_Matrix& form_matrix) {
  FindPreviousTextObject();
  TextOrientation WritingMode = GetTextObjectWritingMode(pObj);
  if (WritingMode == TextOrientation::kUnknown)
    WritingMode = GetTextObjectWritingMode(m_pPrevTextObj);

  size_t nItem = m_pPrevTextObj->CountItems();
  if (nItem == 0)
    return GenerateCharacter::kNone;

  CPDF_TextObject::Item PrevItem = m_pPrevTextObj->GetItemInfo(nItem - 1);
  CPDF_TextObject::Item item = pObj->GetItemInfo(0);
  const CFX_FloatRect& this_rect = pObj->GetRect();
  const CFX_FloatRect& prev_rect = m_pPrevTextObj->GetRect();
  WideString unicode = pObj->GetFont()->UnicodeFromCharCode(item.m_CharCode);
  if (unicode.IsEmpty()) {
    unicode += static_cast<wchar_t>(item.m_CharCode);
  }

  wchar_t curChar = unicode[0];
  if (WritingMode == TextOrientation::kHorizontal) {
    if (EndHorizontalLine(this_rect, prev_rect)) {
      return IsHyphen(curChar) ? GenerateCharacter::kHyphen
                               : GenerateCharacter::kLineBreak;
    }
  } else if (WritingMode == TextOrientation::kVertical) {
    if (EndVerticalLine(this_rect, prev_rect, m_CurlineRect,
                        pObj->GetFontSize(), m_pPrevTextObj->GetFontSize())) {
      return IsHyphen(curChar) ? GenerateCharacter::kHyphen
                               : GenerateCharacter::kLineBreak;
    }
  }

  float last_pos = PrevItem.m_Origin.x;
  int nLastWidth =
      GetCharWidth(PrevItem.m_CharCode, m_pPrevTextObj->GetFont().Get());
  float last_width = nLastWidth * m_pPrevTextObj->GetFontSize() / 1000;
  last_width = fabs(last_width);
  int nThisWidth = GetCharWidth(item.m_CharCode, pObj->GetFont().Get());
  float this_width = fabs(nThisWidth * pObj->GetFontSize() / 1000);
  float threshold = std::max(last_width, this_width) / 4;

  CFX_Matrix prev_matrix = m_pPrevTextObj->GetTextMatrix() * m_PrevMatrix;
  CFX_Matrix prev_reverse = prev_matrix.GetInverse();

  CFX_PointF pos =
      prev_reverse.Transform(form_matrix.Transform(pObj->GetPos()));
  if (last_width < this_width)
    threshold = prev_reverse.TransformDistance(threshold);

  bool bNewline = false;
  if (WritingMode == TextOrientation::kHorizontal) {
    CFX_FloatRect rect = m_pPrevTextObj->GetRect();
    float rect_height = rect.Height();
    rect.Normalize();
    if ((rect.IsEmpty() && rect_height > 5) ||
        ((pos.y > threshold * 2 || pos.y < threshold * -3) &&
         (fabs(pos.y) >= 1 || fabs(pos.y) > fabs(pos.x)))) {
      bNewline = true;
      if (nItem > 1) {
        CPDF_TextObject::Item tempItem = m_pPrevTextObj->GetItemInfo(0);
        CFX_Matrix m = m_pPrevTextObj->GetTextMatrix();
        if (PrevItem.m_Origin.x > tempItem.m_Origin.x &&
            m_DisplayMatrix.a > 0.9 && m_DisplayMatrix.b < 0.1 &&
            m_DisplayMatrix.c < 0.1 && m_DisplayMatrix.d < -0.9 && m.b < 0.1 &&
            m.c < 0.1) {
          CFX_FloatRect re(0, m_pPrevTextObj->GetRect().bottom, 1000,
                           m_pPrevTextObj->GetRect().top);
          if (re.Contains(pObj->GetPos())) {
            bNewline = false;
          } else {
            if (CFX_FloatRect(0, pObj->GetRect().bottom, 1000,
                              pObj->GetRect().top)
                    .Contains(m_pPrevTextObj->GetPos())) {
              bNewline = false;
            }
          }
        }
      }
    }
  }
  if (bNewline) {
    return IsHyphen(curChar) ? GenerateCharacter::kHyphen
                             : GenerateCharacter::kLineBreak;
  }

  if (pObj->CountChars() == 1 && IsHyphenCode(curChar) && IsHyphen(curChar))
    return GenerateCharacter::kHyphen;

  if (curChar == L' ')
    return GenerateCharacter::kNone;

  WideString PrevStr =
      m_pPrevTextObj->GetFont()->UnicodeFromCharCode(PrevItem.m_CharCode);
  wchar_t preChar = PrevStr.Back();
  if (preChar == L' ')
    return GenerateCharacter::kNone;

  CFX_Matrix matrix = pObj->GetTextMatrix() * form_matrix;
  float threshold2 = std::max(nLastWidth, nThisWidth);
  threshold2 = NormalizeThreshold(threshold2, 400, 700, 800);
  if (nLastWidth >= nThisWidth) {
    threshold2 *= fabs(m_pPrevTextObj->GetFontSize());
  } else {
    threshold2 *= fabs(pObj->GetFontSize());
    threshold2 = matrix.TransformDistance(threshold2);
    threshold2 = prev_reverse.TransformDistance(threshold2);
  }
  threshold2 /= 1000;
  if ((threshold2 < 1.4881 && threshold2 > 1.4879) ||
      (threshold2 < 1.39001 && threshold2 > 1.38999)) {
    threshold2 *= 1.5;
  }
  return GenerateSpace(pos, last_pos, this_width, last_width, threshold2)
             ? GenerateCharacter::kSpace
             : GenerateCharacter::kNone;
}

bool CPDF_TextPage::ProcessGenerateCharacter(GenerateCharacter type,
                                             const CPDF_TextObject* text_object,
                                             const CFX_Matrix& form_matrix) {
  switch (type) {
    case GenerateCharacter::kNone:
      return true;
    case GenerateCharacter::kSpace: {
      AppendGeneratedCharacter(L' ', form_matrix, /*use_temp_buffer=*/true);
      return true;
    }
    case GenerateCharacter::kLineBreak:
      CloseTempLine();
      if (m_TextBuf.GetSize()) {
        AppendGeneratedCharacter(L'\r', form_matrix, /*use_temp_buffer=*/false);
        AppendGeneratedCharacter(L'\n', form_matrix, /*use_temp_buffer=*/false);
      }
      return true;
    case GenerateCharacter::kHyphen:
      if (text_object->CountChars() == 1) {
        CPDF_TextObject::Item item = text_object->GetCharInfo(0);
        WideString unicode =
            text_object->GetFont()->UnicodeFromCharCode(item.m_CharCode);
        if (unicode.IsEmpty()) {
          unicode += static_cast<wchar_t>(item.m_CharCode);
        }
        wchar_t curChar = unicode[0];
        if (IsHyphenCode(curChar)) {
          return false;
        }
      }
      while (m_TempTextBuf.GetSize() > 0 &&
             m_TempTextBuf.AsStringView().Back() == 0x20) {
        m_TempTextBuf.Delete(m_TempTextBuf.GetLength() - 1, 1);
        m_TempCharList.pop_back();
      }
      CharInfo& charinfo = m_TempCharList.back();
      m_TempTextBuf.Delete(m_TempTextBuf.GetLength() - 1, 1);
      charinfo.set_char_type(CharType::kHyphen);
      charinfo.set_unicode(0x2);
      m_TempTextBuf.AppendChar(0xfffe);
      return true;
  }
  NOTREACHED_NORETURN();
}

void CPDF_TextPage::ProcessTextObjectItems(CPDF_TextObject* text_object,
                                           const CFX_Matrix& form_matrix,
                                           const CFX_Matrix& matrix) {
  const float base_space = CalculateBaseSpace(text_object, matrix) +
                           CalculateBaseSpaceAdjustment(text_object, matrix);
  RetainPtr<CPDF_Font> const font = text_object->GetFont();

  float spacing = 0;
  const size_t nItems = text_object->CountItems();
  for (size_t i = 0; i < nItems; ++i) {
    CPDF_TextObject::Item item = text_object->GetItemInfo(i);
    if (item.m_CharCode == 0xffffffff) {
      WideString str = m_TempTextBuf.MakeString();
      if (str.IsEmpty()) {
        str = m_TextBuf.AsStringView();
      }
      if (!str.IsEmpty() && str.Back() != L' ') {
        float fontsize_h = text_object->text_state().GetFontSizeH();
        spacing = -fontsize_h * item.m_Origin.x / 1000;
      }
      continue;
    }

    spacing -= base_space;

    if (spacing && i > 0) {
      const float threshold = CalculateSpaceThreshold(
          font, text_object->text_state().GetFontSizeH(), item.m_CharCode);
      if (threshold && spacing && spacing >= threshold) {
        m_TempTextBuf.AppendChar(L' ');
        CFX_PointF origin = matrix.Transform(item.m_Origin);
        m_TempCharList.push_back(CharInfo(
            CharType::kGenerated, CPDF_Font::kInvalidCharCode, L' ', origin,
            CFX_FloatRect(origin.x, origin.y, origin.x, origin.y), form_matrix,
            text_object));
      }
      if (item.m_CharCode == CPDF_Font::kInvalidCharCode) {
        continue;
      }
    }

    spacing = 0;
    WideString unicode = font->UnicodeFromCharCode(item.m_CharCode);
    CharType char_type = CharType::kNormal;
    if (unicode.IsEmpty() && item.m_CharCode) {
      unicode += static_cast<wchar_t>(item.m_CharCode);
      char_type = CharType::kNotUnicode;
    }

    const FX_RECT rect = font->GetCharBBox(item.m_CharCode);
    const float fFontSize = text_object->GetFontSize() / 1000;
    CFX_FloatRect char_box(rect.left * fFontSize + item.m_Origin.x,
                           rect.bottom * fFontSize + item.m_Origin.y,
                           rect.right * fFontSize + item.m_Origin.x,
                           rect.top * fFontSize + item.m_Origin.y);
    if (fabsf(char_box.top - char_box.bottom) < kSizeEpsilon) {
      char_box.top = char_box.bottom + fFontSize;
    }
    if (fabsf(char_box.right - char_box.left) < kSizeEpsilon) {
      char_box.right =
          char_box.left + text_object->GetCharWidth(item.m_CharCode);
    }
    char_box = matrix.TransformRect(char_box);

    CharInfo charinfo(char_type, item.m_CharCode, 0,
                      matrix.Transform(item.m_Origin), char_box, matrix,
                      text_object);
    if (unicode.IsEmpty()) {
      m_TempCharList.push_back(charinfo);
      m_TempTextBuf.AppendChar(0xfffe);
      continue;
    }

    bool add_unicode = true;
    const int count = std::min(fxcrt::CollectionSize<int>(m_TempCharList), 7);
    constexpr float kTextCharRatioGapDelta = 0.07f;
    float threshold = charinfo.matrix().TransformXDistance(
        kTextCharRatioGapDelta * text_object->GetFontSize());
    for (int n = fxcrt::CollectionSize<int>(m_TempCharList);
         n > fxcrt::CollectionSize<int>(m_TempCharList) - count; --n) {
      const CharInfo& charinfo1 = m_TempCharList[n - 1];
      CFX_PointF diff = charinfo1.origin() - charinfo.origin();
      if (charinfo1.char_code() == charinfo.char_code() &&
          charinfo1.text_object()->GetFont() ==
              charinfo.text_object()->GetFont() &&
          fabs(diff.x) < threshold && fabs(diff.y) < threshold) {
        add_unicode = false;
        break;
      }
    }
    if (add_unicode) {
      for (wchar_t c : unicode) {
        charinfo.set_unicode(c);
        m_TempTextBuf.AppendChar(c ? c : 0xfffe);
        m_TempCharList.push_back(charinfo);
      }
    } else if (i == 0) {
      WideString str = m_TempTextBuf.MakeString();
      if (!str.IsEmpty() && str.Back() == L' ') {
        m_TempTextBuf.Delete(m_TempTextBuf.GetLength() - 1, 1);
        m_TempCharList.pop_back();
      }
    }
  }
}

bool CPDF_TextPage::IsSameTextObject(CPDF_TextObject* pTextObj1,
                                     CPDF_TextObject* pTextObj2) const {
  if (!pTextObj1 || !pTextObj2)
    return false;

  CFX_FloatRect rcPreObj = pTextObj2->GetRect();
  const CFX_FloatRect& rcCurObj = pTextObj1->GetRect();
  if (rcPreObj.IsEmpty() && rcCurObj.IsEmpty()) {
    float dbXdif = fabs(rcPreObj.left - rcCurObj.left);
    size_t nCount = m_CharList.size();
    if (nCount >= 2) {
      float dbSpace = m_CharList[nCount - 2].char_box().Width();
      if (dbXdif > dbSpace)
        return false;
    }
  }
  if (!rcPreObj.IsEmpty() || !rcCurObj.IsEmpty()) {
    rcPreObj.Intersect(rcCurObj);
    if (rcPreObj.IsEmpty())
      return false;
    if (fabs(rcPreObj.Width() - rcCurObj.Width()) > rcCurObj.Width() / 2) {
      return false;
    }
    if (pTextObj2->GetFontSize() != pTextObj1->GetFontSize())
      return false;
  }

  size_t nPreCount = pTextObj2->CountItems();
  if (nPreCount != pTextObj1->CountItems())
    return false;

  // If both objects have no items, consider them same.
  if (nPreCount == 0)
    return true;

  CPDF_TextObject::Item itemPer;
  CPDF_TextObject::Item itemCur;
  for (size_t i = 0; i < nPreCount; ++i) {
    itemPer = pTextObj2->GetItemInfo(i);
    itemCur = pTextObj1->GetItemInfo(i);
    if (itemCur.m_CharCode != itemPer.m_CharCode)
      return false;
  }

  CFX_PointF diff = pTextObj1->GetPos() - pTextObj2->GetPos();
  float font_size = pTextObj2->GetFontSize();
  float char_size =
      GetCharWidth(itemPer.m_CharCode, pTextObj2->GetFont().Get());
  float max_pre_size =
      std::max(std::max(rcPreObj.Height(), rcPreObj.Width()), font_size);
  return fabs(diff.x) <= 0.9 * char_size * font_size / 1000 &&
         fabs(diff.y) <= max_pre_size / 8;
}

bool CPDF_TextPage::IsSameAsPreTextObject(
    CPDF_TextObject* pTextObj,
    const CPDF_PageObjectHolder* pObjList,
    CPDF_PageObjectHolder::const_iterator iter) const {
  int i = 0;
  while (i < 5 && iter != pObjList->begin()) {
    --iter;
    CPDF_PageObject* pOtherObj = iter->get();
    if (pOtherObj == pTextObj || !pOtherObj->IsText())
      continue;
    if (IsSameTextObject(pOtherObj->AsText(), pTextObj))
      return true;
    ++i;
  }
  return false;
}

std::optional<CPDF_TextPage::CharInfo> CPDF_TextPage::GenerateCharInfo(
    wchar_t unicode,
    const CFX_Matrix& form_matrix) {
  const CharInfo* pPrevCharInfo = GetPrevCharInfo();
  if (!pPrevCharInfo) {
    return std::nullopt;
  }

  int pre_width = 0;
  if (pPrevCharInfo->text_object() &&
      pPrevCharInfo->char_code() != CPDF_Font::kInvalidCharCode) {
    pre_width = GetCharWidth(pPrevCharInfo->char_code(),
                             pPrevCharInfo->text_object()->GetFont().Get());
  }

  float fFontSize = pPrevCharInfo->text_object()
                        ? pPrevCharInfo->text_object()->GetFontSize()
                        : pPrevCharInfo->char_box().Height();
  if (!fFontSize)
    fFontSize = kDefaultFontSize;

  CFX_PointF origin(pPrevCharInfo->origin().x + pre_width * (fFontSize) / 1000,
                    pPrevCharInfo->origin().y);
  return CharInfo(CharType::kGenerated, CPDF_Font::kInvalidCharCode, unicode,
                  origin, CFX_FloatRect(origin.x, origin.y, origin.x, origin.y),
                  form_matrix, /*text_object=*/nullptr);
}
