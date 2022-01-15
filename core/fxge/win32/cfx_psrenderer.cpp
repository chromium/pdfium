// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/win32/cfx_psrenderer.h"

#include <math.h>
#include <string.h>

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <utility>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_fontcache.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_glyphcache.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_dibextractor.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "core/fxge/fx_freetype.h"
#include "core/fxge/text_char_pos.h"
#include "core/fxge/win32/cfx_psfonttracker.h"
#include "third_party/base/check_op.h"
#include "third_party/base/numerics/safe_conversions.h"

namespace {

bool CanEmbed(CFX_Font* font) {
  FT_UShort fstype = FT_Get_FSType_Flags(font->GetFaceRec());
  return (fstype & (FT_FSTYPE_RESTRICTED_LICENSE_EMBEDDING |
                    FT_FSTYPE_BITMAP_EMBEDDING_ONLY)) == 0;
}

absl::optional<ByteString> GenerateType42SfntData(
    const ByteString& psname,
    pdfium::span<const uint8_t> font_data) {
  if (font_data.empty())
    return absl::nullopt;

  // Per Type 42 font spec.
  constexpr size_t kMaxSfntStringSize = 65535;
  if (font_data.size() > kMaxSfntStringSize) {
    // TODO(thestig): Fonts that are too big need to be written out in sections.
    return absl::nullopt;
  }

  // Each byte is written as 2 ASCIIHex characters, so really 64 chars per line.
  constexpr size_t kMaxBytesPerLine = 32;
  fxcrt::ostringstream output;
  output << "/" << psname << "_sfnts [\n<\n";
  size_t bytes_per_line = 0;
  char buf[2];
  for (uint8_t datum : font_data) {
    FXSYS_IntToTwoHexChars(datum, buf);
    output << buf[0];
    output << buf[1];
    bytes_per_line++;
    if (bytes_per_line == kMaxBytesPerLine) {
      output << "\n";
      bytes_per_line = 0;
    }
  }

  // Pad with ASCIIHex NUL character per Type 42 font spec if needed.
  if (!FX_IsOdd(font_data.size()))
    output << "00";

  output << "\n>\n] def\n";
  return ByteString(output);
}

// The value to use with GenerateType42FontDictionary() below, and the max
// number of entries supported for non-CID fonts.
// Also used to avoid buggy fonts by writing out at least this many entries,
// per note in Poppler's Type 42 generation code.
constexpr size_t kGlyphsPerDescendantFont = 256;

ByteString GenerateType42FontDictionary(const ByteString& psname,
                                        const FX_RECT& bbox,
                                        size_t num_glyphs,
                                        size_t glyphs_per_descendant_font) {
  DCHECK_LE(glyphs_per_descendant_font, kGlyphsPerDescendantFont);
  CHECK_GT(glyphs_per_descendant_font, 0u);

  const size_t descendant_font_count =
      (num_glyphs + glyphs_per_descendant_font - 1) /
      glyphs_per_descendant_font;

  fxcrt::ostringstream output;
  for (size_t i = 0; i < descendant_font_count; ++i) {
    output << "8 dict begin\n";
    output << "/FontType 42 def\n";
    output << "/FontMatrix [1 0 0 1 0 0] def\n";
    output << "/FontName /" << psname << "_" << i << " def\n";

    output << "/Encoding " << glyphs_per_descendant_font << " array\n";
    for (size_t j = 0, pos = i * glyphs_per_descendant_font;
         j < glyphs_per_descendant_font; ++j, ++pos) {
      if (pos >= num_glyphs)
        break;

      output << ByteString::Format("dup %d /c%02x put\n", j, j);
    }
    output << "readonly def\n";

    // Note: `bbox` is LTRB, while /FontBBox is LBRT. Writing it out as LTRB
    // gets the correct values.
    output << "/FontBBox [" << bbox.left << " " << bbox.top << " " << bbox.right
           << " " << bbox.bottom << "] def\n";

    output << "/PaintType 0 def\n";

    output << "/CharStrings " << glyphs_per_descendant_font + 1
           << " dict dup begin\n";
    output << "/.notdef 0 def\n";
    for (size_t j = 0, pos = i * glyphs_per_descendant_font;
         j < glyphs_per_descendant_font; ++j, ++pos) {
      if (pos >= num_glyphs)
        break;

      output << ByteString::Format("/c%02x %d def\n", j, pos);
    }
    output << "end readonly def\n";

    output << "/sfnts " << psname << "_sfnts def\n";
    output << "FontName currentdict end definefont pop\n";
  }

  output << "6 dict begin\n";
  output << "/FontName /" << psname << " def\n";
  output << "/FontType 0 def\n";
  output << "/FontMatrix [1 0 0 1 0 0] def\n";
  output << "/FMapType 2 def\n";

  output << "/Encoding [\n";
  for (size_t i = 0; i < descendant_font_count; ++i)
    output << i << "\n";
  output << "] def\n";

  output << "/FDepVector [\n";
  for (size_t i = 0; i < descendant_font_count; ++i)
    output << "/" << psname << "_" << i << " findfont\n";
  output << "] def\n";

  output << "FontName currentdict end definefont pop\n";
  output << "%%EndResource\n";

  return ByteString(output);
}

ByteString GenerateType42FontData(const CFX_Font* font) {
  const FXFT_FaceRec* font_face_rec = font->GetFaceRec();
  if (!font_face_rec)
    return ByteString();

  const ByteString psname = font->GetPsName();
  DCHECK(!psname.IsEmpty());

  absl::optional<ByteString> sfnt_data =
      GenerateType42SfntData(psname, font->GetFontSpan());
  if (!sfnt_data.has_value())
    return ByteString();

  ByteString output = "%%BeginResource: font ";
  output += psname;
  output += "\n";
  output += sfnt_data.value();
  output += GenerateType42FontDictionary(psname, font->GetRawBBox().value(),
                                         font_face_rec->num_glyphs,
                                         kGlyphsPerDescendantFont);
  return output;
}

}  // namespace

struct CFX_PSRenderer::Glyph {
  Glyph(CFX_Font* font, uint32_t glyph_index)
      : font(font), glyph_index(glyph_index) {}
  Glyph(const Glyph& other) = delete;
  Glyph& operator=(const Glyph&) = delete;
  ~Glyph() = default;

  UnownedPtr<CFX_Font> const font;
  const uint32_t glyph_index;
  absl::optional<std::array<float, 4>> adjust_matrix;
};

CFX_PSRenderer::CFX_PSRenderer(CFX_PSFontTracker* font_tracker,
                               const EncoderIface* encoder_iface)
    : m_pFontTracker(font_tracker), m_pEncoderIface(encoder_iface) {
  DCHECK(m_pFontTracker);
}

CFX_PSRenderer::~CFX_PSRenderer() {
  EndRendering();
}

void CFX_PSRenderer::Init(const RetainPtr<IFX_RetainableWriteStream>& pStream,
                          RenderingLevel level,
                          int width,
                          int height) {
  DCHECK(pStream);

  m_Level = level;
  m_pStream = pStream;
  m_ClipBox.left = 0;
  m_ClipBox.top = 0;
  m_ClipBox.right = width;
  m_ClipBox.bottom = height;
}

void CFX_PSRenderer::StartRendering() {
  if (m_bInited)
    return;

  static const char kInitStr[] =
      "\nsave\n/im/initmatrix load def\n"
      "/n/newpath load def/m/moveto load def/l/lineto load def/c/curveto load "
      "def/h/closepath load def\n"
      "/f/fill load def/F/eofill load def/s/stroke load def/W/clip load "
      "def/W*/eoclip load def\n"
      "/rg/setrgbcolor load def/k/setcmykcolor load def\n"
      "/J/setlinecap load def/j/setlinejoin load def/w/setlinewidth load "
      "def/M/setmiterlimit load def/d/setdash load def\n"
      "/q/gsave load def/Q/grestore load def/iM/imagemask load def\n"
      "/Tj/show load def/Ff/findfont load def/Fs/scalefont load def/Sf/setfont "
      "load def\n"
      "/cm/concat load def/Cm/currentmatrix load def/mx/matrix load "
      "def/sm/setmatrix load def\n";
  WriteString(kInitStr);
  m_bInited = true;
}

void CFX_PSRenderer::EndRendering() {
  if (!m_bInited)
    return;

  WriteString("\nrestore\n");
  m_bInited = false;

  // Flush `m_PreambleOutput` if it is not empty.
  std::streamoff preamble_pos = m_PreambleOutput.tellp();
  if (preamble_pos > 0) {
    m_pStream->WriteBlock(m_PreambleOutput.str().c_str(),
                          pdfium::base::checked_cast<size_t>(preamble_pos));
    m_PreambleOutput.str("");
  }

  // Flush `m_Output`. It's never empty because of the WriteString() call above.
  m_pStream->WriteBlock(
      m_Output.str().c_str(),
      pdfium::base::checked_cast<size_t>(std::streamoff(m_Output.tellp())));
  m_Output.str("");
}

void CFX_PSRenderer::SaveState() {
  StartRendering();
  WriteString("q\n");
  m_ClipBoxStack.push_back(m_ClipBox);
}

void CFX_PSRenderer::RestoreState(bool bKeepSaved) {
  StartRendering();
  WriteString("Q\n");
  if (bKeepSaved)
    WriteString("q\n");

  m_bColorSet = false;
  m_bGraphStateSet = false;
  if (m_ClipBoxStack.empty())
    return;

  m_ClipBox = m_ClipBoxStack.back();
  if (!bKeepSaved)
    m_ClipBoxStack.pop_back();
}

void CFX_PSRenderer::OutputPath(const CFX_Path& path,
                                const CFX_Matrix* pObject2Device) {
  fxcrt::ostringstream buf;
  size_t size = path.GetPoints().size();

  for (size_t i = 0; i < size; i++) {
    CFX_Path::Point::Type type = path.GetType(i);
    bool closing = path.IsClosingFigure(i);
    CFX_PointF pos = path.GetPoint(i);
    if (pObject2Device)
      pos = pObject2Device->Transform(pos);

    buf << pos.x << " " << pos.y;
    switch (type) {
      case CFX_Path::Point::Type::kMove:
        buf << " m ";
        break;
      case CFX_Path::Point::Type::kLine:
        buf << " l ";
        if (closing)
          buf << "h ";
        break;
      case CFX_Path::Point::Type::kBezier: {
        CFX_PointF pos1 = path.GetPoint(i + 1);
        CFX_PointF pos2 = path.GetPoint(i + 2);
        if (pObject2Device) {
          pos1 = pObject2Device->Transform(pos1);
          pos2 = pObject2Device->Transform(pos2);
        }
        buf << " " << pos1.x << " " << pos1.y << " " << pos2.x << " " << pos2.y
            << " c";
        if (closing)
          buf << " h";
        buf << "\n";
        i += 2;
        break;
      }
    }
  }
  WriteStream(buf);
}

void CFX_PSRenderer::SetClip_PathFill(
    const CFX_Path& path,
    const CFX_Matrix* pObject2Device,
    const CFX_FillRenderOptions& fill_options) {
  StartRendering();
  OutputPath(path, pObject2Device);
  CFX_FloatRect rect = path.GetBoundingBox();
  if (pObject2Device)
    rect = pObject2Device->TransformRect(rect);

  m_ClipBox.left = static_cast<int>(rect.left);
  m_ClipBox.right = static_cast<int>(rect.left + rect.right);
  m_ClipBox.top = static_cast<int>(rect.top + rect.bottom);
  m_ClipBox.bottom = static_cast<int>(rect.bottom);

  WriteString("W");
  if (fill_options.fill_type != CFX_FillRenderOptions::FillType::kWinding)
    WriteString("*");
  WriteString(" n\n");
}

void CFX_PSRenderer::SetClip_PathStroke(const CFX_Path& path,
                                        const CFX_Matrix* pObject2Device,
                                        const CFX_GraphStateData* pGraphState) {
  StartRendering();
  SetGraphState(pGraphState);

  fxcrt::ostringstream buf;
  buf << "mx Cm [" << pObject2Device->a << " " << pObject2Device->b << " "
      << pObject2Device->c << " " << pObject2Device->d << " "
      << pObject2Device->e << " " << pObject2Device->f << "]cm ";
  WriteStream(buf);

  OutputPath(path, nullptr);
  CFX_FloatRect rect = path.GetBoundingBoxForStrokePath(
      pGraphState->m_LineWidth, pGraphState->m_MiterLimit);
  m_ClipBox.Intersect(pObject2Device->TransformRect(rect).GetOuterRect());

  WriteString("strokepath W n sm\n");
}

bool CFX_PSRenderer::DrawPath(const CFX_Path& path,
                              const CFX_Matrix* pObject2Device,
                              const CFX_GraphStateData* pGraphState,
                              uint32_t fill_color,
                              uint32_t stroke_color,
                              const CFX_FillRenderOptions& fill_options) {
  StartRendering();
  int fill_alpha = FXARGB_A(fill_color);
  int stroke_alpha = FXARGB_A(stroke_color);
  if (fill_alpha && fill_alpha < 255)
    return false;
  if (stroke_alpha && stroke_alpha < 255)
    return false;
  if (fill_alpha == 0 && stroke_alpha == 0)
    return false;

  if (stroke_alpha) {
    SetGraphState(pGraphState);
    if (pObject2Device) {
      fxcrt::ostringstream buf;
      buf << "mx Cm [" << pObject2Device->a << " " << pObject2Device->b << " "
          << pObject2Device->c << " " << pObject2Device->d << " "
          << pObject2Device->e << " " << pObject2Device->f << "]cm ";
      WriteStream(buf);
    }
  }

  OutputPath(path, stroke_alpha ? nullptr : pObject2Device);
  if (fill_options.fill_type != CFX_FillRenderOptions::FillType::kNoFill &&
      fill_alpha) {
    SetColor(fill_color);
    if (fill_options.fill_type == CFX_FillRenderOptions::FillType::kWinding) {
      if (stroke_alpha)
        WriteString("q f Q ");
      else
        WriteString("f");
    } else if (fill_options.fill_type ==
               CFX_FillRenderOptions::FillType::kEvenOdd) {
      if (stroke_alpha)
        WriteString("q F Q ");
      else
        WriteString("F");
    }
  }

  if (stroke_alpha) {
    SetColor(stroke_color);
    WriteString("s");
    if (pObject2Device)
      WriteString(" sm");
  }

  WriteString("\n");
  return true;
}

void CFX_PSRenderer::SetGraphState(const CFX_GraphStateData* pGraphState) {
  fxcrt::ostringstream buf;
  if (!m_bGraphStateSet ||
      m_CurGraphState.m_LineCap != pGraphState->m_LineCap) {
    buf << static_cast<int>(pGraphState->m_LineCap) << " J\n";
  }
  if (!m_bGraphStateSet ||
      m_CurGraphState.m_DashArray != pGraphState->m_DashArray) {
    buf << "[";
    for (const auto& dash : pGraphState->m_DashArray)
      buf << dash << " ";
    buf << "]" << pGraphState->m_DashPhase << " d\n";
  }
  if (!m_bGraphStateSet ||
      m_CurGraphState.m_LineJoin != pGraphState->m_LineJoin) {
    buf << static_cast<int>(pGraphState->m_LineJoin) << " j\n";
  }
  if (!m_bGraphStateSet ||
      m_CurGraphState.m_LineWidth != pGraphState->m_LineWidth) {
    buf << pGraphState->m_LineWidth << " w\n";
  }
  if (!m_bGraphStateSet ||
      m_CurGraphState.m_MiterLimit != pGraphState->m_MiterLimit) {
    buf << pGraphState->m_MiterLimit << " M\n";
  }
  m_CurGraphState = *pGraphState;
  m_bGraphStateSet = true;
  WriteStream(buf);
}

bool CFX_PSRenderer::SetDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                               uint32_t color,
                               int left,
                               int top) {
  StartRendering();
  CFX_Matrix matrix = CFX_RenderDevice::GetFlipMatrix(
      pSource->GetWidth(), pSource->GetHeight(), left, top);
  return DrawDIBits(pSource, color, matrix, FXDIB_ResampleOptions());
}

bool CFX_PSRenderer::StretchDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                   uint32_t color,
                                   int dest_left,
                                   int dest_top,
                                   int dest_width,
                                   int dest_height,
                                   const FXDIB_ResampleOptions& options) {
  StartRendering();
  CFX_Matrix matrix = CFX_RenderDevice::GetFlipMatrix(dest_width, dest_height,
                                                      dest_left, dest_top);
  return DrawDIBits(pSource, color, matrix, options);
}

bool CFX_PSRenderer::DrawDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                uint32_t color,
                                const CFX_Matrix& matrix,
                                const FXDIB_ResampleOptions& options) {
  StartRendering();
  if ((matrix.a == 0 && matrix.b == 0) || (matrix.c == 0 && matrix.d == 0))
    return true;

  if (pSource->IsAlphaFormat())
    return false;

  int alpha = FXARGB_A(color);
  if (pSource->IsMaskFormat() && (alpha < 255 || pSource->GetBPP() != 1))
    return false;

  WriteString("q\n");

  fxcrt::ostringstream buf;
  buf << "[" << matrix.a << " " << matrix.b << " " << matrix.c << " "
      << matrix.d << " " << matrix.e << " " << matrix.f << "]cm ";

  int width = pSource->GetWidth();
  int height = pSource->GetHeight();
  buf << width << " " << height;

  if (pSource->GetBPP() == 1 && !pSource->HasPalette()) {
    int pitch = (width + 7) / 8;
    uint32_t src_size = height * pitch;
    std::unique_ptr<uint8_t, FxFreeDeleter> src_buf(
        FX_Alloc(uint8_t, src_size));
    for (int row = 0; row < height; row++) {
      const uint8_t* src_scan = pSource->GetScanline(row).data();
      memcpy(src_buf.get() + row * pitch, src_scan, pitch);
    }

    std::unique_ptr<uint8_t, FxFreeDeleter> output_buf;
    uint32_t output_size;
    bool compressed = FaxCompressData(std::move(src_buf), width, height,
                                      &output_buf, &output_size);
    if (pSource->IsMaskFormat()) {
      SetColor(color);
      m_bColorSet = false;
      buf << " true[";
    } else {
      buf << " 1[";
    }
    buf << width << " 0 0 -" << height << " 0 " << height
        << "]currentfile/ASCII85Decode filter ";

    if (compressed) {
      buf << "<</K -1/EndOfBlock false/Columns " << width << "/Rows " << height
          << ">>/CCITTFaxDecode filter ";
    }
    if (pSource->IsMaskFormat())
      buf << "iM\n";
    else
      buf << "false 1 colorimage\n";

    WriteStream(buf);
    WritePSBinary({output_buf.get(), output_size});
  } else {
    CFX_DIBExtractor source_extractor(pSource);
    RetainPtr<CFX_DIBBase> pConverted = source_extractor.GetBitmap();
    if (!pConverted)
      return false;
    switch (pSource->GetFormat()) {
      case FXDIB_Format::k1bppRgb:
      case FXDIB_Format::kRgb32:
        pConverted = pConverted->ConvertTo(FXDIB_Format::kRgb);
        break;
      case FXDIB_Format::k8bppRgb:
        if (pSource->HasPalette())
          pConverted = pConverted->ConvertTo(FXDIB_Format::kRgb);
        break;
      default:
        break;
    }
    if (!pConverted) {
      WriteString("\nQ\n");
      return false;
    }

    int bpp = pConverted->GetBPP() / 8;
    uint8_t* output_buf = nullptr;
    size_t output_size = 0;
    const char* filter = nullptr;
    if ((m_Level.value() == RenderingLevel::kLevel2 || options.bLossy) &&
        m_pEncoderIface->pJpegEncodeFunc(pConverted, &output_buf,
                                         &output_size)) {
      filter = "/DCTDecode filter ";
    }
    if (!filter) {
      int src_pitch = width * bpp;
      output_size = height * src_pitch;
      output_buf = FX_Alloc(uint8_t, output_size);
      for (int row = 0; row < height; row++) {
        const uint8_t* src_scan = pConverted->GetScanline(row).data();
        uint8_t* dest_scan = output_buf + row * src_pitch;
        if (bpp == 3) {
          for (int col = 0; col < width; col++) {
            *dest_scan++ = src_scan[2];
            *dest_scan++ = src_scan[1];
            *dest_scan++ = *src_scan;
            src_scan += 3;
          }
        } else {
          memcpy(dest_scan, src_scan, src_pitch);
        }
      }
      uint8_t* compressed_buf;
      uint32_t compressed_size;
      PSCompressData(output_buf,
                     pdfium::base::checked_cast<uint32_t>(output_size),
                     &compressed_buf, &compressed_size, &filter);
      if (output_buf != compressed_buf)
        FX_Free(output_buf);

      output_buf = compressed_buf;
      output_size = compressed_size;
    }
    buf << " 8[";
    buf << width << " 0 0 -" << height << " 0 " << height << "]";
    buf << "currentfile/ASCII85Decode filter ";
    if (filter)
      buf << filter;

    buf << "false " << bpp;
    buf << " colorimage\n";
    WriteStream(buf);

    WritePSBinary({output_buf, output_size});
    FX_Free(output_buf);
  }
  WriteString("\nQ\n");
  return true;
}

void CFX_PSRenderer::SetColor(uint32_t color) {
  if (m_bColorSet && m_LastColor == color)
    return;

  fxcrt::ostringstream buf;
  buf << FXARGB_R(color) / 255.0 << " " << FXARGB_G(color) / 255.0 << " "
      << FXARGB_B(color) / 255.0 << " rg\n";
  m_bColorSet = true;
  m_LastColor = color;
  WriteStream(buf);
}

void CFX_PSRenderer::FindPSFontGlyph(CFX_GlyphCache* pGlyphCache,
                                     CFX_Font* pFont,
                                     const TextCharPos& charpos,
                                     int* ps_fontnum,
                                     int* ps_glyphindex) {
  for (size_t i = 0; i < m_PSFontList.size(); ++i) {
    const Glyph& glyph = *m_PSFontList[i];
    if (glyph.font == pFont && glyph.glyph_index == charpos.m_GlyphIndex &&
        glyph.adjust_matrix.has_value() == charpos.m_bGlyphAdjust) {
      bool found;
      if (glyph.adjust_matrix.has_value()) {
        constexpr float kEpsilon = 0.01f;
        const auto& adjust_matrix = glyph.adjust_matrix.value();
        found = fabs(adjust_matrix[0] - charpos.m_AdjustMatrix[0]) < kEpsilon &&
                fabs(adjust_matrix[1] - charpos.m_AdjustMatrix[1]) < kEpsilon &&
                fabs(adjust_matrix[2] - charpos.m_AdjustMatrix[2]) < kEpsilon &&
                fabs(adjust_matrix[3] - charpos.m_AdjustMatrix[3]) < kEpsilon;
      } else {
        found = true;
      }
      if (found) {
        *ps_fontnum = pdfium::base::checked_cast<int>(i / 256);
        *ps_glyphindex = i % 256;
        return;
      }
    }
  }

  m_PSFontList.push_back(std::make_unique<Glyph>(pFont, charpos.m_GlyphIndex));
  *ps_fontnum =
      pdfium::base::checked_cast<int>((m_PSFontList.size() - 1) / 256);
  *ps_glyphindex = (m_PSFontList.size() - 1) % 256;
  if (*ps_glyphindex == 0) {
    fxcrt::ostringstream buf;
    buf << "8 dict begin/FontType 3 def/FontMatrix[1 0 0 1 0 0]def\n"
           "/FontBBox[0 0 0 0]def/Encoding 256 array def 0 1 255{Encoding "
           "exch/.notdef put}for\n"
           "/CharProcs 1 dict def CharProcs begin/.notdef {} def end\n"
           "/BuildGlyph{1 0 -10 -10 10 10 setcachedevice exch/CharProcs get "
           "exch 2 copy known not{pop/.notdef}if get exec}bind def\n"
           "/BuildChar{1 index/Encoding get exch get 1 index/BuildGlyph get "
           "exec}bind def\n"
           "currentdict end\n";
    buf << "/X" << *ps_fontnum << " exch definefont pop\n";
    WriteStream(buf);
  }

  if (charpos.m_bGlyphAdjust) {
    m_PSFontList.back()->adjust_matrix = std::array<float, 4>{
        charpos.m_AdjustMatrix[0], charpos.m_AdjustMatrix[1],
        charpos.m_AdjustMatrix[2], charpos.m_AdjustMatrix[3]};
  }

  CFX_Matrix matrix;
  if (charpos.m_bGlyphAdjust) {
    matrix =
        CFX_Matrix(charpos.m_AdjustMatrix[0], charpos.m_AdjustMatrix[1],
                   charpos.m_AdjustMatrix[2], charpos.m_AdjustMatrix[3], 0, 0);
  }
  const CFX_Path* pPath = pGlyphCache->LoadGlyphPath(
      pFont, charpos.m_GlyphIndex, charpos.m_FontCharWidth);
  if (!pPath)
    return;

  CFX_Path TransformedPath(*pPath);
  if (charpos.m_bGlyphAdjust)
    TransformedPath.Transform(matrix);

  fxcrt::ostringstream buf;
  buf << "/X" << *ps_fontnum << " Ff/CharProcs get begin/" << *ps_glyphindex
      << "{n ";
  for (size_t p = 0; p < TransformedPath.GetPoints().size(); p++) {
    CFX_PointF point = TransformedPath.GetPoint(p);
    switch (TransformedPath.GetType(p)) {
      case CFX_Path::Point::Type::kMove: {
        buf << point.x << " " << point.y << " m\n";
        break;
      }
      case CFX_Path::Point::Type::kLine: {
        buf << point.x << " " << point.y << " l\n";
        break;
      }
      case CFX_Path::Point::Type::kBezier: {
        CFX_PointF point1 = TransformedPath.GetPoint(p + 1);
        CFX_PointF point2 = TransformedPath.GetPoint(p + 2);
        buf << point.x << " " << point.y << " " << point1.x << " " << point1.y
            << " " << point2.x << " " << point2.y << " c\n";
        p += 2;
        break;
      }
    }
  }
  buf << "f}bind def end\n";
  buf << "/X" << *ps_fontnum << " Ff/Encoding get " << *ps_glyphindex << "/"
      << *ps_glyphindex << " put\n";
  WriteStream(buf);
}

void CFX_PSRenderer::DrawTextAsType3Font(int char_count,
                                         const TextCharPos* char_pos,
                                         CFX_Font* font,
                                         float font_size,
                                         fxcrt::ostringstream& buf) {
  CFX_FontCache* pCache = CFX_GEModule::Get()->GetFontCache();
  RetainPtr<CFX_GlyphCache> pGlyphCache = pCache->GetGlyphCache(font);
  int last_fontnum = -1;
  for (int i = 0; i < char_count; i++) {
    int ps_fontnum;
    int ps_glyphindex;
    FindPSFontGlyph(pGlyphCache.Get(), font, char_pos[i], &ps_fontnum,
                    &ps_glyphindex);
    if (last_fontnum != ps_fontnum) {
      buf << "/X" << ps_fontnum << " Ff " << font_size << " Fs Sf ";
      last_fontnum = ps_fontnum;
    }
    buf << char_pos[i].m_Origin.x << " " << char_pos[i].m_Origin.y << " m";
    ByteString hex = ByteString::Format("<%02X>", ps_glyphindex);
    buf << hex.AsStringView() << "Tj\n";
  }
}

bool CFX_PSRenderer::DrawTextAsType42Font(int char_count,
                                          const TextCharPos* char_pos,
                                          CFX_Font* font,
                                          float font_size,
                                          fxcrt::ostringstream& buf) {
  if (m_Level != RenderingLevel::kLevel3Type42 || !CanEmbed(font))
    return false;

  if (font->GetFontType() != CFX_Font::FontType::kCIDTrueType)
    return false;

  bool is_existing_font = m_pFontTracker->SeenFontObject(font);
  if (!is_existing_font) {
    ByteString font_data = GenerateType42FontData(font);
    if (font_data.IsEmpty())
      return false;

    m_pFontTracker->AddFontObject(font);
    WritePreambleString(font_data.AsStringView());
  }

  buf << "/" << font->GetPsName() << " " << font_size << " selectfont\n";
  for (int i = 0; i < char_count; ++i) {
    buf << char_pos[i].m_Origin.x << " " << char_pos[i].m_Origin.y << " m";
    uint8_t hi = char_pos[i].m_GlyphIndex / 256;
    uint8_t lo = char_pos[i].m_GlyphIndex % 256;
    ByteString hex = ByteString::Format("<%02X%02X>", hi, lo);
    buf << hex.AsStringView() << "Tj\n";
  }
  return true;
}

bool CFX_PSRenderer::DrawText(int nChars,
                              const TextCharPos* pCharPos,
                              CFX_Font* pFont,
                              const CFX_Matrix& mtObject2Device,
                              float font_size,
                              uint32_t color) {
  // Check object to device matrix first, since it can scale the font size.
  if ((mtObject2Device.a == 0 && mtObject2Device.b == 0) ||
      (mtObject2Device.c == 0 && mtObject2Device.d == 0)) {
    return true;
  }

  // Do not send near zero font sizes to printers. See crbug.com/767343.
  float scale =
      std::min(mtObject2Device.GetXUnit(), mtObject2Device.GetYUnit());
  static constexpr float kEpsilon = 0.01f;
  if (fabsf(font_size * scale) < kEpsilon)
    return true;

  StartRendering();
  int alpha = FXARGB_A(color);
  if (alpha < 255)
    return false;

  SetColor(color);
  fxcrt::ostringstream buf;
  buf << "q[" << mtObject2Device.a << " " << mtObject2Device.b << " "
      << mtObject2Device.c << " " << mtObject2Device.d << " "
      << mtObject2Device.e << " " << mtObject2Device.f << "]cm\n";

  if (!DrawTextAsType42Font(nChars, pCharPos, pFont, font_size, buf)) {
    DrawTextAsType3Font(nChars, pCharPos, pFont, font_size, buf);
  }

  buf << "Q\n";
  WriteStream(buf);
  return true;
}

bool CFX_PSRenderer::FaxCompressData(
    std::unique_ptr<uint8_t, FxFreeDeleter> src_buf,
    int width,
    int height,
    std::unique_ptr<uint8_t, FxFreeDeleter>* dest_buf,
    uint32_t* dest_size) const {
  if (width * height <= 128) {
    *dest_buf = std::move(src_buf);
    *dest_size = (width + 7) / 8 * height;
    return false;
  }

  m_pEncoderIface->pFaxEncodeFunc(src_buf.get(), width, height, (width + 7) / 8,
                                  dest_buf, dest_size);
  return true;
}

void CFX_PSRenderer::PSCompressData(uint8_t* src_buf,
                                    uint32_t src_size,
                                    uint8_t** output_buf,
                                    uint32_t* output_size,
                                    const char** filter) const {
  *output_buf = src_buf;
  *output_size = src_size;
  *filter = "";
  if (src_size < 1024)
    return;

  uint8_t* dest_buf = nullptr;
  uint32_t dest_size = src_size;
  if (m_Level.value() == RenderingLevel::kLevel3 ||
      m_Level.value() == RenderingLevel::kLevel3Type42) {
    std::unique_ptr<uint8_t, FxFreeDeleter> dest_buf_unique;
    if (m_pEncoderIface->pFlateEncodeFunc(pdfium::make_span(src_buf, src_size),
                                          &dest_buf_unique, &dest_size)) {
      dest_buf = dest_buf_unique.release();
      *filter = "/FlateDecode filter ";
    }
  } else {
    std::unique_ptr<uint8_t, FxFreeDeleter> dest_buf_unique;
    if (m_pEncoderIface->pRunLengthEncodeFunc({src_buf, src_size},
                                              &dest_buf_unique, &dest_size)) {
      dest_buf = dest_buf_unique.release();
      *filter = "/RunLengthDecode filter ";
    }
  }
  if (dest_size < src_size) {
    *output_buf = dest_buf;
    *output_size = dest_size;
  } else {
    *filter = nullptr;
    FX_Free(dest_buf);
  }
}

void CFX_PSRenderer::WritePreambleString(ByteStringView str) {
  m_PreambleOutput << str;
}

void CFX_PSRenderer::WritePSBinary(pdfium::span<const uint8_t> data) {
  std::unique_ptr<uint8_t, FxFreeDeleter> dest_buf;
  uint32_t dest_size;
  if (m_pEncoderIface->pA85EncodeFunc(data, &dest_buf, &dest_size)) {
    m_Output.write(reinterpret_cast<const char*>(dest_buf.get()), dest_size);
  } else {
    m_Output.write(reinterpret_cast<const char*>(data.data()), data.size());
  }
}

void CFX_PSRenderer::WriteStream(fxcrt::ostringstream& stream) {
  std::streamoff output_pos = stream.tellp();
  if (output_pos > 0) {
    m_Output.write(stream.str().c_str(),
                   pdfium::base::checked_cast<size_t>(output_pos));
  }
}

void CFX_PSRenderer::WriteString(ByteStringView str) {
  m_Output << str;
}

// static
absl::optional<ByteString> CFX_PSRenderer::GenerateType42SfntDataForTesting(
    const ByteString& psname,
    pdfium::span<const uint8_t> font_data) {
  return GenerateType42SfntData(psname, font_data);
}

// static
ByteString CFX_PSRenderer::GenerateType42FontDictionaryForTesting(
    const ByteString& psname,
    const FX_RECT& bbox,
    size_t num_glyphs,
    size_t glyphs_per_descendant_font) {
  return GenerateType42FontDictionary(psname, bbox, num_glyphs,
                                      glyphs_per_descendant_font);
}
