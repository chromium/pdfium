// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <utility>

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/font/cpdf_type1font.h"
#include "core/fpdfapi/page/cpdf_textobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/fx_font.h"
#include "fpdfsdk/fsdk_define.h"
#include "public/fpdf_edit.h"

namespace {

CPDF_Dictionary* LoadFontDesc(CPDF_Document* pDoc,
                              const CFX_ByteString& font_name,
                              CFX_Font* pFont,
                              const uint8_t* data,
                              uint32_t size,
                              int font_type) {
  CPDF_Dictionary* fontDesc = pDoc->NewIndirect<CPDF_Dictionary>();
  fontDesc->SetNewFor<CPDF_Name>("Type", "FontDescriptor");
  fontDesc->SetNewFor<CPDF_Name>("FontName", font_name);
  int flags = 0;
  if (FXFT_Is_Face_fixedwidth(pFont->GetFace()))
    flags |= FXFONT_FIXED_PITCH;
  if (font_name.Find("Serif") > -1)
    flags |= FXFONT_SERIF;
  if (FXFT_Is_Face_Italic(pFont->GetFace()))
    flags |= FXFONT_ITALIC;
  if (FXFT_Is_Face_Bold(pFont->GetFace()))
    flags |= FXFONT_BOLD;

  // TODO(npm): How do I know if a  font is symbolic, script, allcap, smallcap
  flags |= FXFONT_NONSYMBOLIC;

  fontDesc->SetNewFor<CPDF_Number>("Flags", flags);
  FX_RECT bbox;
  pFont->GetBBox(bbox);
  auto pBBox = pdfium::MakeUnique<CPDF_Array>();
  pBBox->AddNew<CPDF_Number>(bbox.left);
  pBBox->AddNew<CPDF_Number>(bbox.bottom);
  pBBox->AddNew<CPDF_Number>(bbox.right);
  pBBox->AddNew<CPDF_Number>(bbox.top);
  fontDesc->SetFor("FontBBox", std::move(pBBox));

  // TODO(npm): calculate italic angle correctly
  fontDesc->SetNewFor<CPDF_Number>("ItalicAngle", pFont->IsItalic() ? -12 : 0);

  fontDesc->SetNewFor<CPDF_Number>("Ascent", pFont->GetAscent());
  fontDesc->SetNewFor<CPDF_Number>("Descent", pFont->GetDescent());

  // TODO(npm): calculate the capheight, stemV correctly
  fontDesc->SetNewFor<CPDF_Number>("CapHeight", pFont->GetAscent());
  fontDesc->SetNewFor<CPDF_Number>("StemV", pFont->IsBold() ? 120 : 70);

  CPDF_Stream* pStream = pDoc->NewIndirect<CPDF_Stream>();
  pStream->SetData(data, size);
  CFX_ByteString fontFile =
      font_type == FPDF_FONT_TYPE1 ? "FontFile" : "FontFile2";
  fontDesc->SetNewFor<CPDF_Reference>(fontFile, pDoc, pStream->GetObjNum());
  return fontDesc;
}

void* LoadSimpleFont(CPDF_Document* pDoc,
                     std::unique_ptr<CFX_Font> pFont,
                     const uint8_t* data,
                     uint32_t size,
                     int font_type) {
  CPDF_Dictionary* fontDict = pDoc->NewIndirect<CPDF_Dictionary>();
  fontDict->SetNewFor<CPDF_Name>("Type", "Font");
  fontDict->SetNewFor<CPDF_Name>(
      "Subtype", font_type == FPDF_FONT_TYPE1 ? "Type1" : "TrueType");
  CFX_ByteString name = pFont->GetFaceName();
  if (name.IsEmpty())
    name = "Unnamed";
  fontDict->SetNewFor<CPDF_Name>("BaseFont", name);

  uint32_t glyphIndex;
  int currentChar = FXFT_Get_First_Char(pFont->GetFace(), &glyphIndex);
  fontDict->SetNewFor<CPDF_Number>("FirstChar", currentChar);
  CPDF_Array* widthsArray = pDoc->NewIndirect<CPDF_Array>();
  while (true) {
    int width = pFont->GetGlyphWidth(glyphIndex);
    widthsArray->AddNew<CPDF_Number>(width);
    int nextChar =
        FXFT_Get_Next_Char(pFont->GetFace(), currentChar, &glyphIndex);
    // Simple fonts have 1-byte charcodes only.
    if (nextChar > 0xff || glyphIndex == 0)
      break;
    for (int i = currentChar + 1; i < nextChar; i++)
      widthsArray->AddNew<CPDF_Number>(0);
    currentChar = nextChar;
  }
  fontDict->SetNewFor<CPDF_Number>("LastChar", currentChar);
  fontDict->SetNewFor<CPDF_Reference>("Widths", pDoc, widthsArray->GetObjNum());
  CPDF_Dictionary* fontDesc =
      LoadFontDesc(pDoc, name, pFont.get(), data, size, font_type);

  fontDict->SetNewFor<CPDF_Reference>("FontDescriptor", pDoc,
                                      fontDesc->GetObjNum());
  return pDoc->LoadFont(fontDict);
}

void* LoadCompositeFont(CPDF_Document* pDoc,
                        std::unique_ptr<CFX_Font> pFont,
                        const uint8_t* data,
                        uint32_t size,
                        int font_type) {
  CPDF_Dictionary* fontDict = pDoc->NewIndirect<CPDF_Dictionary>();
  fontDict->SetNewFor<CPDF_Name>("Type", "Font");
  fontDict->SetNewFor<CPDF_Name>("Subtype", "Type0");
  // TODO(npm): Get the correct encoding, if it's not identity.
  CFX_ByteString encoding = "Identity-H";
  fontDict->SetNewFor<CPDF_Name>("Encoding", encoding);
  CFX_ByteString name = pFont->GetFaceName();
  if (name.IsEmpty())
    name = "Unnamed";
  fontDict->SetNewFor<CPDF_Name>(
      "BaseFont", font_type == FPDF_FONT_TYPE1 ? name + "-" + encoding : name);

  CPDF_Dictionary* pCIDFont = pDoc->NewIndirect<CPDF_Dictionary>();
  pCIDFont->SetNewFor<CPDF_Name>("Type", "Font");
  pCIDFont->SetNewFor<CPDF_Name>("Subtype", font_type == FPDF_FONT_TYPE1
                                                ? "CIDFontType0"
                                                : "CIDFontType2");
  pCIDFont->SetNewFor<CPDF_Name>("BaseFont", name);

  // TODO(npm): Maybe use FT_Get_CID_Registry_Ordering_Supplement to get the
  // CIDSystemInfo
  CPDF_Dictionary* pCIDSystemInfo = pDoc->NewIndirect<CPDF_Dictionary>();
  pCIDSystemInfo->SetNewFor<CPDF_Name>("Registry", "Adobe");
  pCIDSystemInfo->SetNewFor<CPDF_Name>("Ordering", "Identity");
  pCIDSystemInfo->SetNewFor<CPDF_Number>("Supplement", 0);
  pCIDFont->SetNewFor<CPDF_Reference>("CIDSystemInfo", pDoc,
                                      pCIDSystemInfo->GetObjNum());

  CPDF_Dictionary* fontDesc =
      LoadFontDesc(pDoc, name, pFont.get(), data, size, font_type);
  pCIDFont->SetNewFor<CPDF_Reference>("FontDescriptor", pDoc,
                                      fontDesc->GetObjNum());

  uint32_t glyphIndex;
  int currentChar = FXFT_Get_First_Char(pFont->GetFace(), &glyphIndex);
  CPDF_Array* widthsArray = pDoc->NewIndirect<CPDF_Array>();
  while (true) {
    int width = pFont->GetGlyphWidth(glyphIndex);
    int nextChar =
        FXFT_Get_Next_Char(pFont->GetFace(), currentChar, &glyphIndex);
    if (glyphIndex == 0) {
      // Only one char left, use format c [w]
      auto oneW = pdfium::MakeUnique<CPDF_Array>();
      oneW->AddNew<CPDF_Number>(width);
      widthsArray->AddNew<CPDF_Number>(currentChar);
      widthsArray->Add(std::move(oneW));
      break;
    }
    int nextWidth = pFont->GetGlyphWidth(glyphIndex);
    if (nextChar == currentChar + 1 && nextWidth == width) {
      // The array can have a group c_first c_last w: all CIDs in the range from
      // c_first to c_last will have width w
      widthsArray->AddNew<CPDF_Number>(currentChar);
      currentChar = nextChar;
      while (true) {
        nextChar =
            FXFT_Get_Next_Char(pFont->GetFace(), currentChar, &glyphIndex);
        if (glyphIndex == 0)
          break;
        nextWidth = pFont->GetGlyphWidth(glyphIndex);
        if (nextChar != currentChar + 1 || nextWidth != width)
          break;
        currentChar = nextChar;
      }
      widthsArray->AddNew<CPDF_Number>(currentChar);
      widthsArray->AddNew<CPDF_Number>(width);
    } else {
      // Otherwise we can have a group of the form c [w1 w2 ...]: c has width
      // w1, c+1 has width w2, etc.
      widthsArray->AddNew<CPDF_Number>(currentChar);
      auto curWidthArray = pdfium::MakeUnique<CPDF_Array>();
      curWidthArray->AddNew<CPDF_Number>(width);
      while (nextChar == currentChar + 1) {
        curWidthArray->AddNew<CPDF_Number>(nextWidth);
        currentChar = nextChar;
        nextChar =
            FXFT_Get_Next_Char(pFont->GetFace(), currentChar, &glyphIndex);
        if (glyphIndex == 0)
          break;
        nextWidth = pFont->GetGlyphWidth(glyphIndex);
      }
      widthsArray->Add(std::move(curWidthArray));
    }
    if (glyphIndex == 0)
      break;
    currentChar = nextChar;
  }
  pCIDFont->SetNewFor<CPDF_Reference>("W", pDoc, widthsArray->GetObjNum());
  // TODO(npm): Support vertical writing

  auto pDescendant = pdfium::MakeUnique<CPDF_Array>();
  pDescendant->AddNew<CPDF_Reference>(pDoc, pCIDFont->GetObjNum());
  fontDict->SetFor("DescendantFonts", std::move(pDescendant));
  // TODO(npm): do we need a ToUnicode?
  return pDoc->LoadFont(fontDict);
}

}  // namespace

DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPageObj_NewTextObj(FPDF_DOCUMENT document,
                                                         FPDF_BYTESTRING font,
                                                         float font_size) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return nullptr;

  CPDF_Font* pFont = CPDF_Font::GetStockFont(pDoc, CFX_ByteStringC(font));
  if (!pFont)
    return nullptr;

  CPDF_TextObject* pTextObj = new CPDF_TextObject;
  pTextObj->m_TextState.SetFont(pFont);
  pTextObj->m_TextState.SetFontSize(font_size);
  pTextObj->DefaultStates();
  return pTextObj;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFText_SetText(FPDF_PAGEOBJECT text_object,
                                             FPDF_BYTESTRING text) {
  if (!text_object)
    return false;

  auto* pTextObj = reinterpret_cast<CPDF_TextObject*>(text_object);
  pTextObj->SetText(CFX_ByteString(text));
  return true;
}

DLLEXPORT FPDF_FONT STDCALL FPDFText_LoadFont(FPDF_DOCUMENT document,
                                              const uint8_t* data,
                                              uint32_t size,
                                              int font_type,
                                              FPDF_BOOL cid) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc || !data || size == 0 ||
      (font_type != FPDF_FONT_TYPE1 && font_type != FPDF_FONT_TRUETYPE)) {
    return nullptr;
  }

  auto pFont = pdfium::MakeUnique<CFX_Font>();

  // TODO(npm): Maybe use FT_Get_X11_Font_Format to check format? Otherwise, we
  // are allowing giving any font that can be loaded on freetype and setting it
  // as any font type.
  if (!pFont->LoadEmbedded(data, size))
    return nullptr;

  return cid ? LoadCompositeFont(pDoc, std::move(pFont), data, size, font_type)
             : LoadSimpleFont(pDoc, std::move(pFont), data, size, font_type);
}
