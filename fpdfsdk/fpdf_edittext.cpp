// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_textobject.h"
#include "core/fpdfapi/page/cpdf_textstate.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/render/charposlist.h"
#include "core/fpdfapi/render/cpdf_pagerendercontext.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fpdfapi/render/cpdf_renderstatus.h"
#include "core/fpdfapi/render/cpdf_textrenderer.h"
#include "core/fpdftext/cpdf_textpage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_string_wrappers.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/text_char_pos.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/fpdf_edit.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/numerics/safe_conversions.h"

// These checks are here because core/ and public/ cannot depend on each other.
static_assert(static_cast<int>(TextRenderingMode::MODE_UNKNOWN) ==
                  FPDF_TEXTRENDERMODE_UNKNOWN,
              "TextRenderingMode::MODE_UNKNOWN value mismatch");
static_assert(static_cast<int>(TextRenderingMode::MODE_FILL) ==
                  FPDF_TEXTRENDERMODE_FILL,
              "TextRenderingMode::MODE_FILL value mismatch");
static_assert(static_cast<int>(TextRenderingMode::MODE_STROKE) ==
                  FPDF_TEXTRENDERMODE_STROKE,
              "TextRenderingMode::MODE_STROKE value mismatch");
static_assert(static_cast<int>(TextRenderingMode::MODE_FILL_STROKE) ==
                  FPDF_TEXTRENDERMODE_FILL_STROKE,
              "TextRenderingMode::MODE_FILL_STROKE value mismatch");
static_assert(static_cast<int>(TextRenderingMode::MODE_INVISIBLE) ==
                  FPDF_TEXTRENDERMODE_INVISIBLE,
              "TextRenderingMode::MODE_INVISIBLE value mismatch");
static_assert(static_cast<int>(TextRenderingMode::MODE_FILL_CLIP) ==
                  FPDF_TEXTRENDERMODE_FILL_CLIP,
              "TextRenderingMode::MODE_FILL_CLIP value mismatch");
static_assert(static_cast<int>(TextRenderingMode::MODE_STROKE_CLIP) ==
                  FPDF_TEXTRENDERMODE_STROKE_CLIP,
              "TextRenderingMode::MODE_STROKE_CLIP value mismatch");
static_assert(static_cast<int>(TextRenderingMode::MODE_FILL_STROKE_CLIP) ==
                  FPDF_TEXTRENDERMODE_FILL_STROKE_CLIP,
              "TextRenderingMode::MODE_FILL_STROKE_CLIP value mismatch");
static_assert(static_cast<int>(TextRenderingMode::MODE_CLIP) ==
                  FPDF_TEXTRENDERMODE_CLIP,
              "TextRenderingMode::MODE_CLIP value mismatch");
static_assert(static_cast<int>(TextRenderingMode::MODE_LAST) ==
                  FPDF_TEXTRENDERMODE_LAST,
              "TextRenderingMode::MODE_LAST value mismatch");

namespace {

ByteString BaseFontNameForType(CFX_Font* pFont, int font_type) {
  ByteString name = font_type == FPDF_FONT_TYPE1 ? pFont->GetPsName()
                                                 : pFont->GetBaseFontName();
  if (!name.IsEmpty())
    return name;

  return CFX_Font::kUntitledFontName;
}

RetainPtr<CPDF_Dictionary> LoadFontDesc(CPDF_Document* pDoc,
                                        const ByteString& font_name,
                                        CFX_Font* pFont,
                                        pdfium::span<const uint8_t> span,
                                        int font_type) {
  auto pFontDesc = pDoc->NewIndirect<CPDF_Dictionary>();
  pFontDesc->SetNewFor<CPDF_Name>("Type", "FontDescriptor");
  pFontDesc->SetNewFor<CPDF_Name>("FontName", font_name);
  int flags = 0;
  if (FXFT_Is_Face_fixedwidth(pFont->GetFaceRec()))
    flags |= FXFONT_FIXED_PITCH;
  if (font_name.Contains("Serif"))
    flags |= FXFONT_SERIF;
  if (FXFT_Is_Face_Italic(pFont->GetFaceRec()))
    flags |= FXFONT_ITALIC;
  if (FXFT_Is_Face_Bold(pFont->GetFaceRec()))
    flags |= FXFONT_FORCE_BOLD;

  // TODO(npm): How do I know if a  font is symbolic, script, allcap, smallcap
  flags |= FXFONT_NONSYMBOLIC;

  pFontDesc->SetNewFor<CPDF_Number>("Flags", flags);
  FX_RECT bbox = pFont->GetBBox().value_or(FX_RECT());
  pFontDesc->SetRectFor("FontBBox", CFX_FloatRect(bbox));

  // TODO(npm): calculate italic angle correctly
  pFontDesc->SetNewFor<CPDF_Number>("ItalicAngle", pFont->IsItalic() ? -12 : 0);

  pFontDesc->SetNewFor<CPDF_Number>("Ascent", pFont->GetAscent());
  pFontDesc->SetNewFor<CPDF_Number>("Descent", pFont->GetDescent());

  // TODO(npm): calculate the capheight, stemV correctly
  pFontDesc->SetNewFor<CPDF_Number>("CapHeight", pFont->GetAscent());
  pFontDesc->SetNewFor<CPDF_Number>("StemV", pFont->IsBold() ? 120 : 70);

  auto pStream = pDoc->NewIndirect<CPDF_Stream>();
  pStream->SetData(span);
  // TODO(npm): Lengths for Type1 fonts.
  if (font_type == FPDF_FONT_TRUETYPE) {
    pStream->GetMutableDict()->SetNewFor<CPDF_Number>(
        "Length1", static_cast<int>(span.size()));
  }
  ByteString fontFile = font_type == FPDF_FONT_TYPE1 ? "FontFile" : "FontFile2";
  pFontDesc->SetNewFor<CPDF_Reference>(fontFile, pDoc, pStream->GetObjNum());
  return pFontDesc;
}

const char ToUnicodeStart[] =
    "/CIDInit /ProcSet findresource begin\n"
    "12 dict begin\n"
    "begincmap\n"
    "/CIDSystemInfo\n"
    "<</Registry (Adobe)\n"
    "/Ordering (Identity)\n"
    "/Supplement 0\n"
    ">> def\n"
    "/CMapName /Adobe-Identity-H def\n"
    "CMapType 2 def\n"
    "1 begincodespacerange\n"
    "<0000> <FFFFF>\n"
    "endcodespacerange\n";

const char ToUnicodeEnd[] =
    "endcmap\n"
    "CMapName currentdict /CMap defineresource pop\n"
    "end\n"
    "end\n";

void AddCharcode(fxcrt::ostringstream* pBuffer, uint32_t number) {
  DCHECK(number <= 0xFFFF);
  *pBuffer << "<";
  char ans[4];
  FXSYS_IntToFourHexChars(number, ans);
  for (size_t i = 0; i < 4; ++i)
    *pBuffer << ans[i];
  *pBuffer << ">";
}

// PDF spec 1.7 Section 5.9.2: "Unicode character sequences as expressed in
// UTF-16BE encoding." See https://en.wikipedia.org/wiki/UTF-16#Description
void AddUnicode(fxcrt::ostringstream* pBuffer, uint32_t unicode) {
  if (unicode >= 0xD800 && unicode <= 0xDFFF)
    unicode = 0;

  char ans[8];
  *pBuffer << "<";
  size_t numChars = FXSYS_ToUTF16BE(unicode, ans);
  for (size_t i = 0; i < numChars; ++i)
    *pBuffer << ans[i];
  *pBuffer << ">";
}

// Loads the charcode to unicode mapping into a stream
RetainPtr<CPDF_Stream> LoadUnicode(
    CPDF_Document* pDoc,
    const std::multimap<uint32_t, uint32_t>& to_unicode) {
  // A map charcode->unicode
  std::map<uint32_t, uint32_t> char_to_uni;
  // A map <char_start, char_end> to vector v of unicode characters of size (end
  // - start + 1). This abbreviates: start->v[0], start+1->v[1], etc. PDF spec
  // 1.7 Section 5.9.2 says that only the last byte of the unicode may change.
  std::map<std::pair<uint32_t, uint32_t>, std::vector<uint32_t>>
      map_range_vector;
  // A map <start, end> -> unicode
  // This abbreviates: start->unicode, start+1->unicode+1, etc.
  // PDF spec 1.7 Section 5.9.2 says that only the last byte of the unicode may
  // change.
  std::map<std::pair<uint32_t, uint32_t>, uint32_t> map_range;

  // Calculate the maps
  for (auto iter = to_unicode.begin(); iter != to_unicode.end(); ++iter) {
    uint32_t firstCharcode = iter->first;
    uint32_t firstUnicode = iter->second;
    if (std::next(iter) == to_unicode.end() ||
        firstCharcode + 1 != std::next(iter)->first) {
      char_to_uni[firstCharcode] = firstUnicode;
      continue;
    }
    ++iter;
    uint32_t curCharcode = iter->first;
    uint32_t curUnicode = iter->second;
    if (curCharcode % 256 == 0) {
      char_to_uni[firstCharcode] = firstUnicode;
      char_to_uni[curCharcode] = curUnicode;
      continue;
    }
    const size_t maxExtra = 255 - (curCharcode % 256);
    auto next_it = std::next(iter);
    if (firstUnicode + 1 != curUnicode) {
      // Consecutive charcodes mapping to non-consecutive unicodes
      std::vector<uint32_t> unicodes;
      unicodes.push_back(firstUnicode);
      unicodes.push_back(curUnicode);
      for (size_t i = 0; i < maxExtra; ++i) {
        if (next_it == to_unicode.end() || curCharcode + 1 != next_it->first)
          break;
        ++iter;
        ++curCharcode;
        unicodes.push_back(iter->second);
        next_it = std::next(iter);
      }
      DCHECK_EQ(iter->first - firstCharcode + 1, unicodes.size());
      map_range_vector[std::make_pair(firstCharcode, iter->first)] = unicodes;
      continue;
    }
    // Consecutive charcodes mapping to consecutive unicodes
    for (size_t i = 0; i < maxExtra; ++i) {
      if (next_it == to_unicode.end() || curCharcode + 1 != next_it->first ||
          curUnicode + 1 != next_it->second) {
        break;
      }
      ++iter;
      ++curCharcode;
      ++curUnicode;
      next_it = std::next(iter);
    }
    map_range[std::make_pair(firstCharcode, curCharcode)] = firstUnicode;
  }
  fxcrt::ostringstream buffer;
  buffer << ToUnicodeStart;
  // Add maps to buffer
  buffer << static_cast<uint32_t>(char_to_uni.size()) << " beginbfchar\n";
  for (const auto& iter : char_to_uni) {
    AddCharcode(&buffer, iter.first);
    buffer << " ";
    AddUnicode(&buffer, iter.second);
    buffer << "\n";
  }
  buffer << "endbfchar\n"
         << static_cast<uint32_t>(map_range_vector.size() + map_range.size())
         << " beginbfrange\n";
  for (const auto& iter : map_range_vector) {
    const std::pair<uint32_t, uint32_t>& charcodeRange = iter.first;
    AddCharcode(&buffer, charcodeRange.first);
    buffer << " ";
    AddCharcode(&buffer, charcodeRange.second);
    buffer << " [";
    const std::vector<uint32_t>& unicodes = iter.second;
    for (size_t i = 0; i < unicodes.size(); ++i) {
      uint32_t uni = unicodes[i];
      AddUnicode(&buffer, uni);
      if (i != unicodes.size() - 1)
        buffer << " ";
    }
    buffer << "]\n";
  }
  for (const auto& iter : map_range) {
    const std::pair<uint32_t, uint32_t>& charcodeRange = iter.first;
    AddCharcode(&buffer, charcodeRange.first);
    buffer << " ";
    AddCharcode(&buffer, charcodeRange.second);
    buffer << " ";
    AddUnicode(&buffer, iter.second);
    buffer << "\n";
  }
  buffer << "endbfrange\n";
  buffer << ToUnicodeEnd;
  // TODO(npm): Encrypt / Compress?
  auto stream = pDoc->NewIndirect<CPDF_Stream>();
  stream->SetDataFromStringstream(&buffer);
  return stream;
}

RetainPtr<CPDF_Font> LoadSimpleFont(CPDF_Document* pDoc,
                                    std::unique_ptr<CFX_Font> pFont,
                                    pdfium::span<const uint8_t> span,
                                    int font_type) {
  auto pFontDict = pDoc->NewIndirect<CPDF_Dictionary>();
  pFontDict->SetNewFor<CPDF_Name>("Type", "Font");
  pFontDict->SetNewFor<CPDF_Name>(
      "Subtype", font_type == FPDF_FONT_TYPE1 ? "Type1" : "TrueType");
  ByteString name = BaseFontNameForType(pFont.get(), font_type);
  pFontDict->SetNewFor<CPDF_Name>("BaseFont", name);

  uint32_t dwGlyphIndex;
  uint32_t dwCurrentChar = static_cast<uint32_t>(
      FT_Get_First_Char(pFont->GetFaceRec(), &dwGlyphIndex));
  static constexpr uint32_t kMaxSimpleFontChar = 0xFF;
  if (dwCurrentChar > kMaxSimpleFontChar || dwGlyphIndex == 0)
    return nullptr;
  pFontDict->SetNewFor<CPDF_Number>("FirstChar",
                                    static_cast<int>(dwCurrentChar));
  auto widthsArray = pDoc->NewIndirect<CPDF_Array>();
  while (true) {
    widthsArray->AppendNew<CPDF_Number>(pFont->GetGlyphWidth(dwGlyphIndex));
    uint32_t nextChar = static_cast<uint32_t>(
        FT_Get_Next_Char(pFont->GetFaceRec(), dwCurrentChar, &dwGlyphIndex));
    // Simple fonts have 1-byte charcodes only.
    if (nextChar > kMaxSimpleFontChar || dwGlyphIndex == 0)
      break;
    for (uint32_t i = dwCurrentChar + 1; i < nextChar; i++)
      widthsArray->AppendNew<CPDF_Number>(0);
    dwCurrentChar = nextChar;
  }
  pFontDict->SetNewFor<CPDF_Number>("LastChar",
                                    static_cast<int>(dwCurrentChar));
  pFontDict->SetNewFor<CPDF_Reference>("Widths", pDoc,
                                       widthsArray->GetObjNum());
  RetainPtr<CPDF_Dictionary> pFontDesc =
      LoadFontDesc(pDoc, name, pFont.get(), span, font_type);

  pFontDict->SetNewFor<CPDF_Reference>("FontDescriptor", pDoc,
                                       pFontDesc->GetObjNum());
  return CPDF_DocPageData::FromDocument(pDoc)->GetFont(std::move(pFontDict));
}

RetainPtr<CPDF_Font> LoadCompositeFont(CPDF_Document* pDoc,
                                       std::unique_ptr<CFX_Font> pFont,
                                       pdfium::span<const uint8_t> span,
                                       int font_type) {
  auto pFontDict = pDoc->NewIndirect<CPDF_Dictionary>();
  pFontDict->SetNewFor<CPDF_Name>("Type", "Font");
  pFontDict->SetNewFor<CPDF_Name>("Subtype", "Type0");
  // TODO(npm): Get the correct encoding, if it's not identity.
  ByteString encoding = "Identity-H";
  pFontDict->SetNewFor<CPDF_Name>("Encoding", encoding);
  ByteString name = BaseFontNameForType(pFont.get(), font_type);
  pFontDict->SetNewFor<CPDF_Name>(
      "BaseFont", font_type == FPDF_FONT_TYPE1 ? name + "-" + encoding : name);

  auto pCIDFont = pDoc->NewIndirect<CPDF_Dictionary>();
  pCIDFont->SetNewFor<CPDF_Name>("Type", "Font");
  pCIDFont->SetNewFor<CPDF_Name>("Subtype", font_type == FPDF_FONT_TYPE1
                                                ? "CIDFontType0"
                                                : "CIDFontType2");
  pCIDFont->SetNewFor<CPDF_Name>("BaseFont", name);

  // TODO(npm): Maybe use FT_Get_CID_Registry_Ordering_Supplement to get the
  // CIDSystemInfo
  auto pCIDSystemInfo = pDoc->NewIndirect<CPDF_Dictionary>();
  pCIDSystemInfo->SetNewFor<CPDF_String>("Registry", "Adobe", false);
  pCIDSystemInfo->SetNewFor<CPDF_String>("Ordering", "Identity", false);
  pCIDSystemInfo->SetNewFor<CPDF_Number>("Supplement", 0);
  pCIDFont->SetNewFor<CPDF_Reference>("CIDSystemInfo", pDoc,
                                      pCIDSystemInfo->GetObjNum());

  RetainPtr<CPDF_Dictionary> pFontDesc =
      LoadFontDesc(pDoc, name, pFont.get(), span, font_type);
  pCIDFont->SetNewFor<CPDF_Reference>("FontDescriptor", pDoc,
                                      pFontDesc->GetObjNum());

  uint32_t dwGlyphIndex;
  uint32_t dwCurrentChar = static_cast<uint32_t>(
      FT_Get_First_Char(pFont->GetFaceRec(), &dwGlyphIndex));
  static constexpr uint32_t kMaxUnicode = 0x10FFFF;
  // If it doesn't have a single char, just fail
  if (dwGlyphIndex == 0 || dwCurrentChar > kMaxUnicode)
    return nullptr;

  std::multimap<uint32_t, uint32_t> to_unicode;
  std::map<uint32_t, uint32_t> widths;
  while (true) {
    if (dwCurrentChar > kMaxUnicode)
      break;

    if (!pdfium::Contains(widths, dwGlyphIndex))
      widths[dwGlyphIndex] = pFont->GetGlyphWidth(dwGlyphIndex);
    to_unicode.emplace(dwGlyphIndex, dwCurrentChar);
    dwCurrentChar = static_cast<uint32_t>(
        FT_Get_Next_Char(pFont->GetFaceRec(), dwCurrentChar, &dwGlyphIndex));
    if (dwGlyphIndex == 0)
      break;
  }
  auto widthsArray = pDoc->NewIndirect<CPDF_Array>();
  for (auto it = widths.begin(); it != widths.end(); ++it) {
    int ch = it->first;
    int w = it->second;
    if (std::next(it) == widths.end()) {
      // Only one char left, use format c [w]
      auto oneW = pdfium::MakeRetain<CPDF_Array>();
      oneW->AppendNew<CPDF_Number>(w);
      widthsArray->AppendNew<CPDF_Number>(ch);
      widthsArray->Append(oneW);
      break;
    }
    ++it;
    int next_ch = it->first;
    int next_w = it->second;
    if (next_ch == ch + 1 && next_w == w) {
      // The array can have a group c_first c_last w: all CIDs in the range from
      // c_first to c_last will have width w
      widthsArray->AppendNew<CPDF_Number>(ch);
      ch = next_ch;
      while (true) {
        auto next_it = std::next(it);
        if (next_it == widths.end() || next_it->first != it->first + 1 ||
            next_it->second != it->second) {
          break;
        }
        ++it;
        ch = it->first;
      }
      widthsArray->AppendNew<CPDF_Number>(ch);
      widthsArray->AppendNew<CPDF_Number>(w);
      continue;
    }
    // Otherwise we can have a group of the form c [w1 w2 ...]: c has width
    // w1, c+1 has width w2, etc.
    widthsArray->AppendNew<CPDF_Number>(ch);
    auto curWidthArray = pdfium::MakeRetain<CPDF_Array>();
    curWidthArray->AppendNew<CPDF_Number>(w);
    curWidthArray->AppendNew<CPDF_Number>(next_w);
    while (true) {
      auto next_it = std::next(it);
      if (next_it == widths.end() || next_it->first != it->first + 1)
        break;
      ++it;
      curWidthArray->AppendNew<CPDF_Number>(static_cast<int>(it->second));
    }
    widthsArray->Append(curWidthArray);
  }
  pCIDFont->SetNewFor<CPDF_Reference>("W", pDoc, widthsArray->GetObjNum());

  // TODO(npm): Support vertical writing

  auto pDescendant = pFontDict->SetNewFor<CPDF_Array>("DescendantFonts");
  pDescendant->AppendNew<CPDF_Reference>(pDoc, pCIDFont->GetObjNum());

  RetainPtr<CPDF_Stream> toUnicodeStream = LoadUnicode(pDoc, to_unicode);
  pFontDict->SetNewFor<CPDF_Reference>("ToUnicode", pDoc,
                                       toUnicodeStream->GetObjNum());
  return CPDF_DocPageData::FromDocument(pDoc)->GetFont(pFontDict);
}

CPDF_TextObject* CPDFTextObjectFromFPDFPageObject(FPDF_PAGEOBJECT page_object) {
  auto* obj = CPDFPageObjectFromFPDFPageObject(page_object);
  return obj ? obj->AsText() : nullptr;
}

FPDF_GLYPHPATH FPDFGlyphPathFromCFXPath(const CFX_Path* path) {
  return reinterpret_cast<FPDF_GLYPHPATH>(path);
}
const CFX_Path* CFXPathFromFPDFGlyphPath(FPDF_GLYPHPATH path) {
  return reinterpret_cast<const CFX_Path*>(path);
}

}  // namespace

FPDF_EXPORT FPDF_PAGEOBJECT FPDF_CALLCONV
FPDFPageObj_NewTextObj(FPDF_DOCUMENT document,
                       FPDF_BYTESTRING font,
                       float font_size) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  RetainPtr<CPDF_Font> pFont =
      CPDF_Font::GetStockFont(pDoc, ByteStringView(font));
  if (!pFont)
    return nullptr;

  auto pTextObj = std::make_unique<CPDF_TextObject>();
  pTextObj->m_TextState.SetFont(std::move(pFont));
  pTextObj->m_TextState.SetFontSize(font_size);
  pTextObj->DefaultStates();

  // Caller takes ownership.
  return FPDFPageObjectFromCPDFPageObject(pTextObj.release());
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFText_SetText(FPDF_PAGEOBJECT text_object, FPDF_WIDESTRING text) {
  CPDF_TextObject* pTextObj = CPDFTextObjectFromFPDFPageObject(text_object);
  if (!pTextObj)
    return false;

  WideString encodedText = WideStringFromFPDFWideString(text);
  ByteString byteText;
  for (wchar_t wc : encodedText) {
    pTextObj->GetFont()->AppendChar(
        &byteText, pTextObj->GetFont()->CharCodeFromUnicode(wc));
  }
  pTextObj->SetText(byteText);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFText_SetCharcodes(FPDF_PAGEOBJECT text_object,
                      const uint32_t* charcodes,
                      size_t count) {
  CPDF_TextObject* pTextObj = CPDFTextObjectFromFPDFPageObject(text_object);
  if (!pTextObj)
    return false;

  if (!charcodes && count)
    return false;

  ByteString byte_text;
  if (charcodes) {
    for (size_t i = 0; i < count; ++i) {
      pTextObj->GetFont()->AppendChar(&byte_text, charcodes[i]);
    }
  }
  pTextObj->SetText(byte_text);
  return true;
}

FPDF_EXPORT FPDF_FONT FPDF_CALLCONV FPDFText_LoadFont(FPDF_DOCUMENT document,
                                                      const uint8_t* data,
                                                      uint32_t size,
                                                      int font_type,
                                                      FPDF_BOOL cid) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc || !data || size == 0 ||
      (font_type != FPDF_FONT_TYPE1 && font_type != FPDF_FONT_TRUETYPE)) {
    return nullptr;
  }

  auto span = pdfium::make_span(data, size);
  auto pFont = std::make_unique<CFX_Font>();

  // TODO(npm): Maybe use FT_Get_X11_Font_Format to check format? Otherwise, we
  // are allowing giving any font that can be loaded on freetype and setting it
  // as any font type.
  if (!pFont->LoadEmbedded(span, /*force_vertical=*/false, /*object_tag=*/0))
    return nullptr;

  // Caller takes ownership.
  return FPDFFontFromCPDFFont(
      cid ? LoadCompositeFont(pDoc, std::move(pFont), span, font_type).Leak()
          : LoadSimpleFont(pDoc, std::move(pFont), span, font_type).Leak());
}

FPDF_EXPORT FPDF_FONT FPDF_CALLCONV
FPDFText_LoadStandardFont(FPDF_DOCUMENT document, FPDF_BYTESTRING font) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  // Caller takes ownership.
  return FPDFFontFromCPDFFont(
      CPDF_Font::GetStockFont(pDoc, ByteStringView(font)).Leak());
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFTextObj_GetFontSize(FPDF_PAGEOBJECT text, float* size) {
  if (!size)
    return false;

  CPDF_TextObject* pTextObj = CPDFTextObjectFromFPDFPageObject(text);
  if (!pTextObj)
    return false;

  *size = pTextObj->GetFontSize();
  return true;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFTextObj_GetText(FPDF_PAGEOBJECT text_object,
                    FPDF_TEXTPAGE text_page,
                    FPDF_WCHAR* buffer,
                    unsigned long length) {
  CPDF_TextObject* pTextObj = CPDFTextObjectFromFPDFPageObject(text_object);
  if (!pTextObj)
    return 0;

  CPDF_TextPage* pTextPage = CPDFTextPageFromFPDFTextPage(text_page);
  if (!pTextPage)
    return 0;

  WideString text = pTextPage->GetTextByObject(pTextObj);
  return Utf16EncodeMaybeCopyAndReturnLength(text, buffer, length);
}

FPDF_EXPORT FPDF_BITMAP FPDF_CALLCONV
FPDFTextObj_GetRenderedBitmap(FPDF_DOCUMENT document,
                              FPDF_PAGE page,
                              FPDF_PAGEOBJECT text_object,
                              float scale) {
  CPDF_Document* doc = CPDFDocumentFromFPDFDocument(document);
  if (!doc)
    return nullptr;

  CPDF_Page* optional_page = CPDFPageFromFPDFPage(page);
  if (optional_page && optional_page->GetDocument() != doc)
    return nullptr;

  CPDF_TextObject* text = CPDFTextObjectFromFPDFPageObject(text_object);
  if (!text)
    return nullptr;

  if (scale <= 0)
    return nullptr;

  const CFX_Matrix scale_matrix(scale, 0, 0, scale, 0, 0);
  const CFX_FloatRect& text_rect = text->GetRect();
  const CFX_FloatRect scaled_text_rect = scale_matrix.TransformRect(text_rect);

  // `rect` has to use integer values. Round up as needed.
  const FX_RECT rect = scaled_text_rect.GetOuterRect();
  if (rect.IsEmpty())
    return nullptr;

  auto result_bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!result_bitmap->Create(rect.Width(), rect.Height(), FXDIB_Format::kArgb))
    return nullptr;

  auto render_context = std::make_unique<CPDF_PageRenderContext>();
  CPDF_PageRenderContext* render_context_ptr = render_context.get();
  CPDF_Page::RenderContextClearer clearer(optional_page);
  if (optional_page)
    optional_page->SetRenderContext(std::move(render_context));

  RetainPtr<CPDF_Dictionary> page_resources =
      optional_page ? optional_page->GetMutablePageResources() : nullptr;

  auto device = std::make_unique<CFX_DefaultRenderDevice>();
  CFX_DefaultRenderDevice* device_ptr = device.get();
  render_context_ptr->m_pDevice = std::move(device);
  render_context_ptr->m_pContext = std::make_unique<CPDF_RenderContext>(
      doc, std::move(page_resources), /*pPageCache=*/nullptr);

  device_ptr->Attach(result_bitmap);

  CFX_Matrix device_matrix(rect.Width(), 0, 0, rect.Height(), 0, 0);
  CPDF_RenderStatus status(render_context_ptr->m_pContext.get(), device_ptr);
  status.SetDeviceMatrix(device_matrix);
  status.Initialize(nullptr, nullptr);

  // Need to flip the rendering and also move it to fit within `result_bitmap`.
  CFX_Matrix render_matrix(1, 0, 0, -1, -text_rect.left, text_rect.top);
  render_matrix *= scale_matrix;
  status.RenderSingleObject(text, render_matrix);

  // Caller takes ownership.
  return FPDFBitmapFromCFXDIBitmap(result_bitmap.Leak());
}

FPDF_EXPORT void FPDF_CALLCONV FPDFFont_Close(FPDF_FONT font) {
  // Take back ownership from caller and release.
  RetainPtr<CPDF_Font>().Unleak(CPDFFontFromFPDFFont(font));
}

FPDF_EXPORT FPDF_PAGEOBJECT FPDF_CALLCONV
FPDFPageObj_CreateTextObj(FPDF_DOCUMENT document,
                          FPDF_FONT font,
                          float font_size) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  CPDF_Font* pFont = CPDFFontFromFPDFFont(font);
  if (!pDoc || !pFont)
    return nullptr;

  auto pTextObj = std::make_unique<CPDF_TextObject>();
  pTextObj->m_TextState.SetFont(CPDF_DocPageData::FromDocument(pDoc)->GetFont(
      pFont->GetMutableFontDict()));
  pTextObj->m_TextState.SetFontSize(font_size);
  pTextObj->DefaultStates();
  return FPDFPageObjectFromCPDFPageObject(pTextObj.release());
}

FPDF_EXPORT FPDF_TEXT_RENDERMODE FPDF_CALLCONV
FPDFTextObj_GetTextRenderMode(FPDF_PAGEOBJECT text) {
  CPDF_TextObject* pTextObj = CPDFTextObjectFromFPDFPageObject(text);
  if (!pTextObj)
    return FPDF_TEXTRENDERMODE_UNKNOWN;
  return static_cast<FPDF_TEXT_RENDERMODE>(pTextObj->GetTextRenderMode());
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFTextObj_SetTextRenderMode(FPDF_PAGEOBJECT text,
                              FPDF_TEXT_RENDERMODE render_mode) {
  if (render_mode <= FPDF_TEXTRENDERMODE_UNKNOWN ||
      render_mode > FPDF_TEXTRENDERMODE_LAST) {
    return false;
  }

  CPDF_TextObject* pTextObj = CPDFTextObjectFromFPDFPageObject(text);
  if (!pTextObj)
    return false;

  pTextObj->SetTextRenderMode(static_cast<TextRenderingMode>(render_mode));
  return true;
}

FPDF_EXPORT FPDF_FONT FPDF_CALLCONV FPDFTextObj_GetFont(FPDF_PAGEOBJECT text) {
  CPDF_TextObject* pTextObj = CPDFTextObjectFromFPDFPageObject(text);
  if (!pTextObj)
    return nullptr;

  // Unretained reference in public API. NOLINTNEXTLINE
  return FPDFFontFromCPDFFont(pTextObj->GetFont());
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFFont_GetFontName(FPDF_FONT font, char* buffer, unsigned long length) {
  auto* pFont = CPDFFontFromFPDFFont(font);
  if (!pFont)
    return 0;

  CFX_Font* pCfxFont = pFont->GetFont();
  ByteString name = pCfxFont->GetFamilyName();
  const unsigned long dwStringLen =
      pdfium::base::checked_cast<unsigned long>(name.GetLength() + 1);
  if (buffer && length >= dwStringLen)
    memcpy(buffer, name.c_str(), dwStringLen);

  return dwStringLen;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFFont_GetFontData(FPDF_FONT font,
                                                         uint8_t* buffer,
                                                         size_t buflen,
                                                         size_t* out_buflen) {
  auto* cfont = CPDFFontFromFPDFFont(font);
  if (!cfont || !out_buflen)
    return false;

  pdfium::span<uint8_t> data = cfont->GetFont()->GetFontSpan();
  if (buffer && buflen >= data.size())
    fxcrt::spancpy(pdfium::make_span(buffer, buflen), data);
  *out_buflen = data.size();
  return true;
}

FPDF_EXPORT int FPDF_CALLCONV FPDFFont_GetIsEmbedded(FPDF_FONT font) {
  auto* cfont = CPDFFontFromFPDFFont(font);
  if (!cfont)
    return -1;
  return cfont->IsEmbedded() ? 1 : 0;
}

FPDF_EXPORT int FPDF_CALLCONV FPDFFont_GetFlags(FPDF_FONT font) {
  auto* pFont = CPDFFontFromFPDFFont(font);
  if (!pFont)
    return -1;

  // Return only flags from ISO 32000-1:2008, table 123.
  return pFont->GetFontFlags() & 0x7ffff;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFFont_GetWeight(FPDF_FONT font) {
  auto* pFont = CPDFFontFromFPDFFont(font);
  return pFont ? pFont->GetFontWeight() : -1;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFFont_GetItalicAngle(FPDF_FONT font,
                                                            int* angle) {
  auto* pFont = CPDFFontFromFPDFFont(font);
  if (!pFont || !angle)
    return false;

  *angle = pFont->GetItalicAngle();
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFFont_GetAscent(FPDF_FONT font,
                                                       float font_size,
                                                       float* ascent) {
  auto* pFont = CPDFFontFromFPDFFont(font);
  if (!pFont || !ascent)
    return false;

  *ascent = pFont->GetTypeAscent() * font_size / 1000.f;
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFFont_GetDescent(FPDF_FONT font,
                                                        float font_size,
                                                        float* descent) {
  auto* pFont = CPDFFontFromFPDFFont(font);
  if (!pFont || !descent)
    return false;

  *descent = pFont->GetTypeDescent() * font_size / 1000.f;
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFFont_GetGlyphWidth(FPDF_FONT font,
                                                           uint32_t glyph,
                                                           float font_size,
                                                           float* width) {
  auto* pFont = CPDFFontFromFPDFFont(font);
  if (!pFont || !width)
    return false;

  uint32_t charcode = pFont->CharCodeFromUnicode(static_cast<wchar_t>(glyph));

  CPDF_CIDFont* pCIDFont = pFont->AsCIDFont();
  if (pCIDFont && pCIDFont->IsVertWriting()) {
    uint16_t cid = pCIDFont->CIDFromCharCode(charcode);
    *width = pCIDFont->GetVertWidth(cid) * font_size / 1000.f;
  } else {
    *width = pFont->GetCharWidthF(charcode) * font_size / 1000.f;
  }

  return true;
}

FPDF_EXPORT FPDF_GLYPHPATH FPDF_CALLCONV
FPDFFont_GetGlyphPath(FPDF_FONT font, uint32_t glyph, float font_size) {
  auto* pFont = CPDFFontFromFPDFFont(font);
  if (!pFont)
    return nullptr;

  if (!pdfium::base::IsValueInRangeForNumericType<wchar_t>(glyph))
    return nullptr;

  uint32_t charcode = pFont->CharCodeFromUnicode(static_cast<wchar_t>(glyph));
  std::vector<TextCharPos> pos =
      GetCharPosList(pdfium::make_span(&charcode, 1),
                     pdfium::span<const float>(), pFont, font_size);
  if (pos.empty())
    return nullptr;

  CFX_Font* pCfxFont;
  if (pos[0].m_FallbackFontPosition == -1) {
    pCfxFont = pFont->GetFont();
    DCHECK(pCfxFont);  // Never null.
  } else {
    pCfxFont = pFont->GetFontFallback(pos[0].m_FallbackFontPosition);
    if (!pCfxFont)
      return nullptr;
  }

  const CFX_Path* pPath =
      pCfxFont->LoadGlyphPath(pos[0].m_GlyphIndex, pos[0].m_FontCharWidth);

  return FPDFGlyphPathFromCFXPath(pPath);
}

FPDF_EXPORT int FPDF_CALLCONV
FPDFGlyphPath_CountGlyphSegments(FPDF_GLYPHPATH glyphpath) {
  auto* pPath = CFXPathFromFPDFGlyphPath(glyphpath);
  if (!pPath)
    return -1;

  return fxcrt::CollectionSize<int>(pPath->GetPoints());
}

FPDF_EXPORT FPDF_PATHSEGMENT FPDF_CALLCONV
FPDFGlyphPath_GetGlyphPathSegment(FPDF_GLYPHPATH glyphpath, int index) {
  auto* pPath = CFXPathFromFPDFGlyphPath(glyphpath);
  if (!pPath)
    return nullptr;

  pdfium::span<const CFX_Path::Point> points = pPath->GetPoints();
  if (!fxcrt::IndexInBounds(points, index))
    return nullptr;

  return FPDFPathSegmentFromFXPathPoint(&points[index]);
}
