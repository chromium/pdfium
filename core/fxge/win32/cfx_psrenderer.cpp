// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/win32/cfx_psrenderer.h"

#include <math.h>

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <utility>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_fontcache.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_glyphcache.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "core/fxge/text_char_pos.h"
#include "core/fxge/win32/cfx_psfonttracker.h"

namespace {

std::optional<ByteString> GenerateType42SfntData(
    const ByteString& psname,
    pdfium::span<const uint8_t> font_data) {
  if (font_data.empty()) {
    return std::nullopt;
  }

  // Per Type 42 font spec.
  static constexpr size_t kMaxSfntStringSize = 65535;
  if (font_data.size() > kMaxSfntStringSize) {
    // TODO(thestig): Fonts that are too big need to be written out in sections.
    return std::nullopt;
  }

  // Each byte is written as 2 ASCIIHex characters, so really 64 chars per line.
  static constexpr size_t kMaxBytesPerLine = 32;
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
  if (!FX_IsOdd(font_data.size())) {
    output << "00";
  }

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
      if (pos >= num_glyphs) {
        break;
      }

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
      if (pos >= num_glyphs) {
        break;
      }

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
  for (size_t i = 0; i < descendant_font_count; ++i) {
    output << i << "\n";
  }
  output << "] def\n";

  output << "/FDepVector [\n";
  for (size_t i = 0; i < descendant_font_count; ++i) {
    output << "/" << psname << "_" << i << " findfont\n";
  }
  output << "] def\n";

  output << "FontName currentdict end definefont pop\n";
  output << "%%EndResource\n";

  return ByteString(output);
}

ByteString GenerateType42FontData(const CFX_Font* font) {
  RetainPtr<const CFX_Face> face = font->GetFace();
  if (!face) {
    return ByteString();
  }

  int num_glyphs = face->GetGlyphCount();
  if (num_glyphs < 0) {
    return ByteString();
  }

  const ByteString psname = font->GetPsName();
  DCHECK(!psname.IsEmpty());

  std::optional<ByteString> sfnt_data =
      GenerateType42SfntData(psname, font->GetFontSpan());
  if (!sfnt_data.has_value()) {
    return ByteString();
  }

  ByteString output = "%%BeginResource: font ";
  output += psname;
  output += "\n";
  output += sfnt_data.value();
  output += GenerateType42FontDictionary(psname, font->GetRawBBox().value(),
                                         num_glyphs, kGlyphsPerDescendantFont);
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
  std::optional<std::array<float, 4>> adjust_matrix;
};

CFX_PSRenderer::FaxCompressResult::FaxCompressResult() = default;

CFX_PSRenderer::FaxCompressResult::FaxCompressResult(
    FaxCompressResult&&) noexcept = default;

CFX_PSRenderer::FaxCompressResult& CFX_PSRenderer::FaxCompressResult::operator=(
    FaxCompressResult&&) noexcept = default;

CFX_PSRenderer::FaxCompressResult::~FaxCompressResult() = default;

CFX_PSRenderer::PSCompressResult::PSCompressResult() = default;

CFX_PSRenderer::PSCompressResult::PSCompressResult(
    PSCompressResult&&) noexcept = default;

CFX_PSRenderer::PSCompressResult& CFX_PSRenderer::PSCompressResult::operator=(
    PSCompressResult&&) noexcept = default;

CFX_PSRenderer::PSCompressResult::~PSCompressResult() = default;

CFX_PSRenderer::CFX_PSRenderer(CFX_PSFontTracker* font_tracker,
                               const EncoderIface* encoder_iface)
    : font_tracker_(font_tracker), encoder_iface_(encoder_iface) {
  DCHECK(font_tracker_);
}

CFX_PSRenderer::~CFX_PSRenderer() {
  EndRendering();
}

void CFX_PSRenderer::Init(const RetainPtr<IFX_RetainableWriteStream>& pStream,
                          RenderingLevel level,
                          int width,
                          int height) {
  DCHECK(pStream);

  level_ = level;
  stream_ = pStream;
  clip_box_.left = 0;
  clip_box_.top = 0;
  clip_box_.right = width;
  clip_box_.bottom = height;
}

void CFX_PSRenderer::StartRendering() {
  if (inited_) {
    return;
  }

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
  inited_ = true;
}

void CFX_PSRenderer::EndRendering() {
  if (!inited_) {
    return;
  }

  WriteString("\nrestore\n");
  inited_ = false;

  // Flush `preamble_output_` if it is not empty.
  std::streamoff preamble_pos = preamble_output_.tellp();
  if (preamble_pos > 0) {
    stream_->WriteBlock(pdfium::as_byte_span(preamble_output_.str())
                            .first(pdfium::checked_cast<size_t>(preamble_pos)));
    preamble_output_.str("");
  }

  // Flush `output_`. It's never empty because of the WriteString() call above.
  stream_->WriteBlock(pdfium::as_byte_span(output_.str())
                          .first(pdfium::checked_cast<size_t>(
                              std::streamoff(output_.tellp()))));
  output_.str("");
}

void CFX_PSRenderer::SaveState() {
  StartRendering();
  WriteString("q\n");
  clip_box_stack_.push_back(clip_box_);
}

void CFX_PSRenderer::RestoreState(bool bKeepSaved) {
  StartRendering();
  WriteString("Q\n");
  if (bKeepSaved) {
    WriteString("q\n");
  }

  color_set_ = false;
  graph_state_set_ = false;
  if (clip_box_stack_.empty()) {
    return;
  }

  clip_box_ = clip_box_stack_.back();
  if (!bKeepSaved) {
    clip_box_stack_.pop_back();
  }
}

void CFX_PSRenderer::OutputPath(const CFX_Path& path,
                                const CFX_Matrix* pObject2Device) {
  fxcrt::ostringstream buf;
  size_t size = path.GetPoints().size();

  for (size_t i = 0; i < size; i++) {
    CFX_Path::Point::Type type = path.GetType(i);
    bool closing = path.IsClosingFigure(i);
    CFX_PointF pos = path.GetPoint(i);
    if (pObject2Device) {
      pos = pObject2Device->Transform(pos);
    }

    buf << pos.x << " " << pos.y;
    switch (type) {
      case CFX_Path::Point::Type::kMove:
        buf << " m ";
        break;
      case CFX_Path::Point::Type::kLine:
        buf << " l ";
        if (closing) {
          buf << "h ";
        }
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
        if (closing) {
          buf << " h";
        }
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
  if (pObject2Device) {
    rect = pObject2Device->TransformRect(rect);
  }

  clip_box_.left = static_cast<int>(rect.left);
  clip_box_.right = static_cast<int>(rect.left + rect.right);
  clip_box_.top = static_cast<int>(rect.top + rect.bottom);
  clip_box_.bottom = static_cast<int>(rect.bottom);

  WriteString("W");
  if (fill_options.fill_type != CFX_FillRenderOptions::FillType::kWinding) {
    WriteString("*");
  }
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
      pGraphState->line_width(), pGraphState->miter_limit());
  clip_box_.Intersect(pObject2Device->TransformRect(rect).GetOuterRect());

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
  if (fill_alpha && fill_alpha < 255) {
    return false;
  }
  if (stroke_alpha && stroke_alpha < 255) {
    return false;
  }
  if (fill_alpha == 0 && stroke_alpha == 0) {
    return false;
  }

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
      if (stroke_alpha) {
        WriteString("q f Q ");
      } else {
        WriteString("f");
      }
    } else if (fill_options.fill_type ==
               CFX_FillRenderOptions::FillType::kEvenOdd) {
      if (stroke_alpha) {
        WriteString("q F Q ");
      } else {
        WriteString("F");
      }
    }
  }

  if (stroke_alpha) {
    SetColor(stroke_color);
    WriteString("s");
    if (pObject2Device) {
      WriteString(" sm");
    }
  }

  WriteString("\n");
  return true;
}

void CFX_PSRenderer::SetGraphState(const CFX_GraphStateData* pGraphState) {
  fxcrt::ostringstream buf;
  if (!graph_state_set_ ||
      cur_graph_state_.line_cap() != pGraphState->line_cap()) {
    buf << static_cast<int>(pGraphState->line_cap()) << " J\n";
  }
  if (!graph_state_set_ ||
      cur_graph_state_.dash_array() != pGraphState->dash_array()) {
    buf << "[";
    for (float dash : pGraphState->dash_array()) {
      buf << dash << " ";
    }
    buf << "]" << pGraphState->dash_phase() << " d\n";
  }
  if (!graph_state_set_ ||
      cur_graph_state_.line_join() != pGraphState->line_join()) {
    buf << static_cast<int>(pGraphState->line_join()) << " j\n";
  }
  if (!graph_state_set_ ||
      cur_graph_state_.line_width() != pGraphState->line_width()) {
    buf << pGraphState->line_width() << " w\n";
  }
  if (!graph_state_set_ ||
      cur_graph_state_.miter_limit() != pGraphState->miter_limit()) {
    buf << pGraphState->miter_limit() << " M\n";
  }
  cur_graph_state_ = *pGraphState;
  graph_state_set_ = true;
  WriteStream(buf);
}

bool CFX_PSRenderer::SetDIBits(RetainPtr<const CFX_DIBBase> bitmap,
                               uint32_t color,
                               int left,
                               int top) {
  StartRendering();
  CFX_Matrix matrix = CFX_RenderDevice::GetFlipMatrix(
      bitmap->GetWidth(), bitmap->GetHeight(), left, top);
  return DrawDIBits(std::move(bitmap), color, matrix, FXDIB_ResampleOptions());
}

bool CFX_PSRenderer::StretchDIBits(RetainPtr<const CFX_DIBBase> bitmap,
                                   uint32_t color,
                                   int dest_left,
                                   int dest_top,
                                   int dest_width,
                                   int dest_height,
                                   const FXDIB_ResampleOptions& options) {
  StartRendering();
  CFX_Matrix matrix = CFX_RenderDevice::GetFlipMatrix(dest_width, dest_height,
                                                      dest_left, dest_top);
  return DrawDIBits(std::move(bitmap), color, matrix, options);
}

bool CFX_PSRenderer::DrawDIBits(RetainPtr<const CFX_DIBBase> bitmap,
                                uint32_t color,
                                const CFX_Matrix& matrix,
                                const FXDIB_ResampleOptions& options) {
  StartRendering();
  if ((matrix.a == 0 && matrix.b == 0) || (matrix.c == 0 && matrix.d == 0)) {
    return true;
  }

  if (bitmap->IsAlphaFormat()) {
    return false;
  }

  int alpha = FXARGB_A(color);
  if (bitmap->IsMaskFormat() && (alpha < 255 || bitmap->GetBPP() != 1)) {
    return false;
  }

  WriteString("q\n");

  fxcrt::ostringstream buf;
  buf << "[" << matrix.a << " " << matrix.b << " " << matrix.c << " "
      << matrix.d << " " << matrix.e << " " << matrix.f << "]cm ";

  const int width = bitmap->GetWidth();
  const int height = bitmap->GetHeight();
  buf << width << " " << height;

  if (bitmap->GetBPP() == 1 && !bitmap->HasPalette()) {
    FaxCompressResult compress_result = FaxCompressData(bitmap);
    if (compress_result.data.empty()) {
      return false;
    }

    if (bitmap->IsMaskFormat()) {
      SetColor(color);
      color_set_ = false;
      buf << " true[";
    } else {
      buf << " 1[";
    }
    buf << width << " 0 0 -" << height << " 0 " << height
        << "]currentfile/ASCII85Decode filter ";

    if (compress_result.compressed) {
      buf << "<</K -1/EndOfBlock false/Columns " << width << "/Rows " << height
          << ">>/CCITTFaxDecode filter ";
    }
    if (bitmap->IsMaskFormat()) {
      buf << "iM\n";
    } else {
      buf << "false 1 colorimage\n";
    }

    WriteStream(buf);
    WritePSBinary(compress_result.data);
  } else {
    switch (bitmap->GetFormat()) {
      case FXDIB_Format::k1bppRgb:
      case FXDIB_Format::kBgrx:
        bitmap = bitmap->ConvertTo(FXDIB_Format::kBgr);
        break;
      case FXDIB_Format::k8bppRgb:
        if (bitmap->HasPalette()) {
          bitmap = bitmap->ConvertTo(FXDIB_Format::kBgr);
        }
        break;
      case FXDIB_Format::kInvalid:
      case FXDIB_Format::k1bppMask:
      case FXDIB_Format::k8bppMask:
      case FXDIB_Format::kBgr:
        break;
      case FXDIB_Format::kBgra:
#if defined(PDF_USE_SKIA)
      case FXDIB_Format::kBgraPremul:
#endif
        // Should have returned early due to IsAlphaFormat() check above.
        NOTREACHED();
    }
    if (!bitmap) {
      WriteString("\nQ\n");
      return false;
    }

    const int bytes_per_pixel = bitmap->GetBPP() / 8;
    uint8_t* output_buf = nullptr;
    size_t output_size = 0;
    bool output_buf_is_owned = true;
    std::optional<PSCompressResult> compress_result;
    ByteString filter;
    if ((level_.value() == RenderingLevel::kLevel2 || options.bLossy) &&
        encoder_iface_->pJpegEncodeFunc(bitmap, &output_buf, &output_size)) {
      filter = "/DCTDecode filter ";
    } else {
      int src_pitch = width * bytes_per_pixel;
      output_size = height * src_pitch;
      output_buf = FX_Alloc(uint8_t, output_size);
      for (int row = 0; row < height; row++) {
        const uint8_t* src_scan = bitmap->GetScanline(row).data();
        uint8_t* dest_scan = UNSAFE_TODO(output_buf + row * src_pitch);
        if (bytes_per_pixel == 3) {
          UNSAFE_TODO({
            for (int col = 0; col < width; col++) {
              *dest_scan++ = src_scan[2];
              *dest_scan++ = src_scan[1];
              *dest_scan++ = *src_scan;
              src_scan += 3;
            }
          });
        } else {
          UNSAFE_TODO(FXSYS_memcpy(dest_scan, src_scan, src_pitch));
        }
      }
      // SAFETY: `output_size` passed to FX_Alloc() of `output_buf`.
      compress_result =
          PSCompressData(UNSAFE_BUFFERS(pdfium::span(output_buf, output_size)));
      if (compress_result.has_value()) {
        FX_Free(output_buf);
        output_buf_is_owned = false;
        output_buf = compress_result.value().data.data();
        output_size = compress_result.value().data.size();
        filter = compress_result.value().filter;
      }
    }
    buf << " 8[";
    buf << width << " 0 0 -" << height << " 0 " << height << "]";
    buf << "currentfile/ASCII85Decode filter ";
    if (!filter.IsEmpty()) {
      buf << filter;
    }

    buf << "false " << bytes_per_pixel;
    buf << " colorimage\n";
    WriteStream(buf);

    // SAFETY: `output_size` passed to FX_Alloc() of `output_buf`.
    WritePSBinary(UNSAFE_BUFFERS(pdfium::span(output_buf, output_size)));
    if (output_buf_is_owned) {
      FX_Free(output_buf);
    }
  }
  WriteString("\nQ\n");
  return true;
}

void CFX_PSRenderer::SetColor(uint32_t color) {
  if (color_set_ && last_color_ == color) {
    return;
  }

  fxcrt::ostringstream buf;
  buf << FXARGB_R(color) / 255.0 << " " << FXARGB_G(color) / 255.0 << " "
      << FXARGB_B(color) / 255.0 << " rg\n";
  color_set_ = true;
  last_color_ = color;
  WriteStream(buf);
}

void CFX_PSRenderer::FindPSFontGlyph(CFX_GlyphCache* pGlyphCache,
                                     CFX_Font* pFont,
                                     const TextCharPos& charpos,
                                     int* ps_fontnum,
                                     int* ps_glyphindex) {
  for (size_t i = 0; i < psfont_list_.size(); ++i) {
    const Glyph& glyph = *psfont_list_[i];
    if (glyph.font == pFont && glyph.glyph_index == charpos.glyph_index_ &&
        glyph.adjust_matrix.has_value() == charpos.glyph_adjust_) {
      bool found;
      if (glyph.adjust_matrix.has_value()) {
        static constexpr float kEpsilon = 0.01f;
        const auto& adjust_matrix = glyph.adjust_matrix.value();
        found = fabs(adjust_matrix[0] - charpos.adjust_matrix_[0]) < kEpsilon &&
                fabs(adjust_matrix[1] - charpos.adjust_matrix_[1]) < kEpsilon &&
                fabs(adjust_matrix[2] - charpos.adjust_matrix_[2]) < kEpsilon &&
                fabs(adjust_matrix[3] - charpos.adjust_matrix_[3]) < kEpsilon;
      } else {
        found = true;
      }
      if (found) {
        *ps_fontnum = pdfium::checked_cast<int>(i / 256);
        *ps_glyphindex = i % 256;
        return;
      }
    }
  }

  psfont_list_.push_back(std::make_unique<Glyph>(pFont, charpos.glyph_index_));
  *ps_fontnum = pdfium::checked_cast<int>((psfont_list_.size() - 1) / 256);
  *ps_glyphindex = (psfont_list_.size() - 1) % 256;
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

  if (charpos.glyph_adjust_) {
    psfont_list_.back()->adjust_matrix = std::array<float, 4>{
        charpos.adjust_matrix_[0], charpos.adjust_matrix_[1],
        charpos.adjust_matrix_[2], charpos.adjust_matrix_[3]};
  }

  CFX_Matrix matrix;
  if (charpos.glyph_adjust_) {
    matrix =
        CFX_Matrix(charpos.adjust_matrix_[0], charpos.adjust_matrix_[1],
                   charpos.adjust_matrix_[2], charpos.adjust_matrix_[3], 0, 0);
  }
  const CFX_Path* pPath = pGlyphCache->LoadGlyphPath(
      pFont, charpos.glyph_index_, charpos.font_char_width_);
  if (!pPath) {
    return;
  }

  CFX_Path TransformedPath(*pPath);
  if (charpos.glyph_adjust_) {
    TransformedPath.Transform(matrix);
  }

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
  UNSAFE_TODO({
    for (int i = 0; i < char_count; i++) {
      int ps_fontnum;
      int ps_glyphindex;
      FindPSFontGlyph(pGlyphCache.Get(), font, char_pos[i], &ps_fontnum,
                      &ps_glyphindex);
      if (last_fontnum != ps_fontnum) {
        buf << "/X" << ps_fontnum << " Ff " << font_size << " Fs Sf ";
        last_fontnum = ps_fontnum;
      }
      buf << char_pos[i].origin_.x << " " << char_pos[i].origin_.y << " m";
      ByteString hex = ByteString::Format("<%02X>", ps_glyphindex);
      buf << hex.AsStringView() << "Tj\n";
    }
  });
}

bool CFX_PSRenderer::DrawTextAsType42Font(int char_count,
                                          const TextCharPos* char_pos,
                                          CFX_Font* font,
                                          float font_size,
                                          fxcrt::ostringstream& buf) {
  if (level_ != RenderingLevel::kLevel3Type42) {
    return false;
  }

  RetainPtr<CFX_Face> face = font->GetFace();
  if (!face || !face->CanEmbed()) {
    return false;
  }

  if (font->GetFontType() != CFX_Font::FontType::kCIDTrueType) {
    return false;
  }

  bool is_existing_font = font_tracker_->SeenFontObject(font);
  if (!is_existing_font) {
    ByteString font_data = GenerateType42FontData(font);
    if (font_data.IsEmpty()) {
      return false;
    }

    font_tracker_->AddFontObject(font);
    WritePreambleString(font_data.AsStringView());
  }

  buf << "/" << font->GetPsName() << " " << font_size << " selectfont\n";
  UNSAFE_TODO({
    for (int i = 0; i < char_count; ++i) {
      buf << char_pos[i].origin_.x << " " << char_pos[i].origin_.y << " m";
      uint8_t hi = char_pos[i].glyph_index_ / 256;
      uint8_t lo = char_pos[i].glyph_index_ % 256;
      ByteString hex = ByteString::Format("<%02X%02X>", hi, lo);
      buf << hex.AsStringView() << "Tj\n";
    }
  });
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
  if (fabsf(font_size * scale) < kEpsilon) {
    return true;
  }

  StartRendering();
  int alpha = FXARGB_A(color);
  if (alpha < 255) {
    return false;
  }

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

CFX_PSRenderer::FaxCompressResult CFX_PSRenderer::FaxCompressData(
    RetainPtr<const CFX_DIBBase> src) const {
  DCHECK_EQ(1, src->GetBPP());

  FaxCompressResult result;
  const int width = src->GetWidth();
  const int height = src->GetHeight();
  const int pitch = src->GetPitch();
  DCHECK_GE(width, pitch);

  FX_SAFE_UINT32 safe_pixel_count = width;
  safe_pixel_count *= height;
  if (!safe_pixel_count.IsValid()) {
    return result;
  }

  if (safe_pixel_count.ValueOrDie() > 128) {
    result.data = encoder_iface_->pFaxEncodeFunc(std::move(src));
    result.compressed = true;
    return result;
  }

  FX_SAFE_UINT32 safe_size = pitch;
  safe_size *= height;
  result.data.resize(safe_size.ValueOrDie());
  auto dest_span = pdfium::span(result.data);
  for (int row = 0; row < height; row++) {
    fxcrt::Copy(src->GetScanline(row),
                dest_span.subspan(static_cast<size_t>(row * pitch),
                                  static_cast<size_t>(pitch)));
  }
  return result;
}

std::optional<CFX_PSRenderer::PSCompressResult> CFX_PSRenderer::PSCompressData(
    pdfium::span<const uint8_t> src_span) const {
  if (src_span.size() < 1024) {
    return std::nullopt;
  }

  DataVector<uint8_t> (*encode_func)(pdfium::span<const uint8_t> src_span);
  ByteString filter;
  if (level_.value() == RenderingLevel::kLevel3 ||
      level_.value() == RenderingLevel::kLevel3Type42) {
    encode_func = encoder_iface_->pFlateEncodeFunc;
    filter = "/FlateDecode filter ";
  } else {
    encode_func = encoder_iface_->pRunLengthEncodeFunc;
    filter = "/RunLengthDecode filter ";
  }

  DataVector<uint8_t> decode_result = encode_func(src_span);
  if (decode_result.size() == 0 || decode_result.size() >= src_span.size()) {
    return std::nullopt;
  }

  PSCompressResult result;
  result.data = std::move(decode_result);
  result.filter = filter;
  return result;
}

void CFX_PSRenderer::WritePreambleString(ByteStringView str) {
  preamble_output_ << str;
}

void CFX_PSRenderer::WritePSBinary(pdfium::span<const uint8_t> data) {
  DataVector<uint8_t> encoded_data = encoder_iface_->pA85EncodeFunc(data);
  pdfium::span<const uint8_t> result =
      encoded_data.empty() ? data : encoded_data;
  auto chars = pdfium::as_chars(result);
  output_.write(chars.data(), chars.size());
}

void CFX_PSRenderer::WriteStream(fxcrt::ostringstream& stream) {
  std::streamoff output_pos = stream.tellp();
  if (output_pos > 0) {
    output_.write(stream.str().c_str(),
                  pdfium::checked_cast<size_t>(output_pos));
  }
}

void CFX_PSRenderer::WriteString(ByteStringView str) {
  output_ << str;
}

// static
std::optional<ByteString> CFX_PSRenderer::GenerateType42SfntDataForTesting(
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
