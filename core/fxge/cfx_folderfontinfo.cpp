// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_folderfontinfo.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/byteorder.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/debug/alias.h"
#include "core/fxcrt/fixed_size_data_vector.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_folder.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/fx_font.h"

namespace {

struct FontSubst {
  const char* name_;
  const char* subst_name_;
};

constexpr auto kBase14Substs = std::to_array<const FontSubst>({
    {"Courier", "Courier New"},
    {"Courier-Bold", "Courier New Bold"},
    {"Courier-BoldOblique", "Courier New Bold Italic"},
    {"Courier-Oblique", "Courier New Italic"},
    {"Helvetica", "Arial"},
    {"Helvetica-Bold", "Arial Bold"},
    {"Helvetica-BoldOblique", "Arial Bold Italic"},
    {"Helvetica-Oblique", "Arial Italic"},
    {"Times-Roman", "Times New Roman"},
    {"Times-Bold", "Times New Roman Bold"},
    {"Times-BoldItalic", "Times New Roman Bold Italic"},
    {"Times-Italic", "Times New Roman Italic"},
});

// Used with std::unique_ptr to automatically call fclose().
struct FxFileCloser {
  inline void operator()(FILE* h) const {
    if (h) {
      fclose(h);
    }
  }
};

bool FindFamilyNameMatch(ByteStringView family_name,
                         const ByteString& installed_font_name) {
  std::optional<size_t> result = installed_font_name.Find(family_name, 0);
  if (!result.has_value()) {
    return false;
  }

  size_t next_index = result.value() + family_name.GetLength();
  // Rule out the case that |family_name| is a substring of
  // |installed_font_name| but their family names are actually different words.
  // For example: "Univers" and "Universal" are not a match because they have
  // different family names, but "Univers" and "Univers Bold" are a match.
  if (installed_font_name.IsValidIndex(next_index) &&
      FXSYS_IsLowerASCII(installed_font_name[next_index])) {
    return false;
  }

  return true;
}

ByteString ReadStringFromFile(FILE* pFile, uint32_t size) {
  ByteString result;
  {
    // Span's lifetime must end before ReleaseBuffer() below.
    pdfium::span<char> buffer = result.GetBuffer(size);

    // SAFETY: GetBuffer(size) ensures size bytes available.
    if (!UNSAFE_BUFFERS(fread(buffer.data(), size, 1, pFile))) {
      return ByteString();
    }
  }
  result.ReleaseBuffer(size);
  return result;
}

ByteString LoadTableFromTT(FILE* pFile,
                           const uint8_t* pTables,
                           uint32_t nTables,
                           uint32_t tag,
                           FX_FILESIZE fileSize) {
  UNSAFE_TODO({
    for (uint32_t i = 0; i < nTables; i++) {
      // TODO(tsepez): use actual span.
      auto p = pdfium::span(pTables + i * 16, 16u);
      if (fxcrt::GetUInt32MSBFirst(p.first<4u>()) == tag) {
        uint32_t offset = fxcrt::GetUInt32MSBFirst(p.subspan<8u, 4u>());
        uint32_t size = fxcrt::GetUInt32MSBFirst(p.subspan<12u, 4u>());
        if (offset > std::numeric_limits<uint32_t>::max() - size ||
            static_cast<FX_FILESIZE>(offset + size) > fileSize ||
            fseek(pFile, offset, SEEK_SET) < 0) {
          return ByteString();
        }
        return ReadStringFromFile(pFile, size);
      }
    }
  });
  return ByteString();
}

uint32_t GetCharset(FX_Charset charset) {
  switch (charset) {
    case FX_Charset::kShiftJIS:
      return CHARSET_FLAG_SHIFTJIS;
    case FX_Charset::kChineseSimplified:
      return CHARSET_FLAG_GB;
    case FX_Charset::kChineseTraditional:
      return CHARSET_FLAG_BIG5;
    case FX_Charset::kHangul:
      return CHARSET_FLAG_KOREAN;
    case FX_Charset::kSymbol:
      return CHARSET_FLAG_SYMBOL;
    case FX_Charset::kANSI:
      return CHARSET_FLAG_ANSI;
    default:
      break;
  }
  return 0;
}

}  // namespace

CFX_FolderFontInfo::CFX_FolderFontInfo() = default;

CFX_FolderFontInfo::~CFX_FolderFontInfo() = default;

void CFX_FolderFontInfo::AddPath(const ByteString& path) {
  path_list_.push_back(path);
}

void CFX_FolderFontInfo::EnumFontList(CFX_FontMapper* pMapper) {
  mapper_ = pMapper;
  for (const auto& path : path_list_) {
    ScanPath(path);
  }
}

void CFX_FolderFontInfo::ScanPath(const ByteString& path) {
  std::unique_ptr<FX_Folder> handle = FX_Folder::OpenFolder(path);
  if (!handle) {
    return;
  }

  ByteString filename;
  bool bFolder;
  while (handle->GetNextFile(&filename, &bFolder)) {
    if (bFolder) {
      if (filename == "." || filename == "..") {
        continue;
      }
    } else {
      ByteString ext = filename.Last(4);
      ext.MakeLower();
      if (ext != ".ttf" && ext != ".ttc" && ext != ".otf") {
        continue;
      }
    }

    ByteString fullpath = path;
#if BUILDFLAG(IS_WIN)
    fullpath += "\\";
#else
    fullpath += "/";
#endif

    fullpath += filename;
    bFolder ? ScanPath(fullpath) : ScanFile(fullpath);
  }
}

void CFX_FolderFontInfo::ScanFile(const ByteString& path) {
  std::unique_ptr<FILE, FxFileCloser> pFile(fopen(path.c_str(), "rb"));
  if (!pFile) {
    return;
  }

  fseek(pFile.get(), 0, SEEK_END);

  FX_FILESIZE filesize = ftell(pFile.get());
  uint8_t buffer[16];
  fseek(pFile.get(), 0, SEEK_SET);

  // SAFETY: 12 byte read fits into 16 byte buffer,
  size_t items_read =
      UNSAFE_BUFFERS(fread(buffer, /*size=*/12, /*nmemb=*/1, pFile.get()));
  if (items_read != 1) {
    return;
  }
  uint32_t magic = fxcrt::GetUInt32MSBFirst(pdfium::span(buffer).first<4u>());
  if (magic != kTableTTCF) {
    ReportFace(path, pFile.get(), filesize, 0);
    return;
  }

  uint32_t nFaces =
      fxcrt::GetUInt32MSBFirst(pdfium::span(buffer).subspan<8u, 4u>());
  FX_SAFE_SIZE_T safe_face_bytes = nFaces;
  safe_face_bytes *= 4;
  if (!safe_face_bytes.IsValid()) {
    return;
  }

  auto offsets =
      FixedSizeDataVector<uint8_t>::Uninit(safe_face_bytes.ValueOrDie());
  pdfium::span<uint8_t> offsets_span = offsets.span();
  items_read = UNSAFE_TODO(fread(offsets_span.data(), /*size=*/1,
                                 /*nmemb=*/offsets_span.size(), pFile.get()));
  if (items_read != offsets_span.size()) {
    return;
  }

  for (uint32_t i = 0; i < nFaces; i++) {
    ReportFace(
        path, pFile.get(), filesize,
        fxcrt::GetUInt32MSBFirst(offsets_span.subspan(i * 4).first<4u>()));
  }
}

void CFX_FolderFontInfo::ReportFace(const ByteString& path,
                                    FILE* pFile,
                                    FX_FILESIZE filesize,
                                    uint32_t offset) {
  char buffer[16];
  if (fseek(pFile, offset, SEEK_SET) < 0) {
    return;
  }
  // SAFTEY: 12 byt read fits in 16 byte buffer.
  if (UNSAFE_BUFFERS(!fread(buffer, 12, 1, pFile))) {
    return;
  }

  uint32_t nTables =
      fxcrt::GetUInt16MSBFirst(pdfium::as_byte_span(buffer).subspan<4, 2>());
  ByteString tables = ReadStringFromFile(pFile, nTables * 16);
  if (tables.IsEmpty()) {
    return;
  }

  static constexpr uint32_t kNameTag =
      CFX_FontMapper::MakeTag('n', 'a', 'm', 'e');
  ByteString names = LoadTableFromTT(pFile, tables.unsigned_str(), nTables,
                                     kNameTag, filesize);
  if (names.IsEmpty()) {
    return;
  }

  ByteString facename = GetNameFromTT(names.unsigned_span(), 1);
  if (facename.IsEmpty()) {
    return;
  }

  ByteString style = GetNameFromTT(names.unsigned_span(), 2);
  if (style != "Regular") {
    facename += " " + style;
  }

  if (pdfium::Contains(font_list_, facename)) {
    return;
  }

  auto pInfo =
      std::make_unique<FontFaceInfo>(path, facename, tables, offset, filesize);
  static constexpr uint32_t kOs2Tag =
      CFX_FontMapper::MakeTag('O', 'S', '/', '2');
  ByteString os2 =
      LoadTableFromTT(pFile, tables.unsigned_str(), nTables, kOs2Tag, filesize);
  if (os2.GetLength() >= 86) {
    pdfium::span<const uint8_t> p = os2.unsigned_span().subspan(78u);
    uint32_t codepages = fxcrt::GetUInt32MSBFirst(p.first<4u>());
    if (codepages & (1U << 17)) {
      mapper_->AddInstalledFont(facename, FX_Charset::kShiftJIS);
      pInfo->charsets_ |= CHARSET_FLAG_SHIFTJIS;
    }
    if (codepages & (1U << 18)) {
      mapper_->AddInstalledFont(facename, FX_Charset::kChineseSimplified);
      pInfo->charsets_ |= CHARSET_FLAG_GB;
    }
    if (codepages & (1U << 20)) {
      mapper_->AddInstalledFont(facename, FX_Charset::kChineseTraditional);
      pInfo->charsets_ |= CHARSET_FLAG_BIG5;
    }
    if ((codepages & (1U << 19)) || (codepages & (1U << 21))) {
      mapper_->AddInstalledFont(facename, FX_Charset::kHangul);
      pInfo->charsets_ |= CHARSET_FLAG_KOREAN;
    }
    if (codepages & (1U << 31)) {
      mapper_->AddInstalledFont(facename, FX_Charset::kSymbol);
      pInfo->charsets_ |= CHARSET_FLAG_SYMBOL;
    }
  }
  mapper_->AddInstalledFont(facename, FX_Charset::kANSI);
  pInfo->charsets_ |= CHARSET_FLAG_ANSI;
  pInfo->styles_ = 0;
  if (style.Contains("Bold")) {
    pInfo->styles_ |= pdfium::kFontStyleForceBold;
  }
  if (style.Contains("Italic") || style.Contains("Oblique")) {
    pInfo->styles_ |= pdfium::kFontStyleItalic;
  }
  if (facename.Contains("Serif")) {
    pInfo->styles_ |= pdfium::kFontStyleSerif;
  }

  font_list_[facename] = std::move(pInfo);
}

void* CFX_FolderFontInfo::GetSubstFont(const ByteString& face) {
  for (size_t iBaseFont = 0; iBaseFont < std::size(kBase14Substs);
       iBaseFont++) {
    if (face == kBase14Substs[iBaseFont].name_) {
      return GetFont(kBase14Substs[iBaseFont].subst_name_);
    }
  }
  return nullptr;
}

void* CFX_FolderFontInfo::FindFont(int weight,
                                   bool bItalic,
                                   FX_Charset charset,
                                   int pitch_family,
                                   const ByteString& family,
                                   bool bMatchName) {
  FontFaceInfo* pFind = nullptr;
  uint32_t charset_flag = GetCharset(charset);

  int32_t iBestSimilar = 0;
  if (bMatchName) {
    // Try a direct lookup for either a perfect score or to determine a
    // baseline similarity score.
    auto direct_it = font_list_.find(family);
    if (direct_it != font_list_.end()) {
      FontFaceInfo* pFont = direct_it->second.get();
      if (pFont->IsEligibleForFindFont(charset_flag, charset)) {
        iBestSimilar =
            pFont->SimilarityScore(weight, bItalic, pitch_family, bMatchName);
        if (iBestSimilar == FontFaceInfo::kSimilarityScoreMax) {
          return pFont;
        }
        pFind = pFont;
      }
    }
  }
  // Try and find a better match. Since FindFamilyNameMatch() is expensive,
  // avoid calling it unless there might be a better match.
  ByteStringView bsFamily = family.AsStringView();
  for (const auto& it : font_list_) {
    const ByteString& bsName = it.first;
    FontFaceInfo* pFont = it.second.get();
    if (!pFont->IsEligibleForFindFont(charset_flag, charset)) {
      continue;
    }
    int32_t iSimilarValue = pFont->SimilarityScore(
        weight, bItalic, pitch_family,
        bMatchName && bsFamily.GetLength() == bsName.GetLength());
    if (iSimilarValue > iBestSimilar) {
      if (bMatchName && !FindFamilyNameMatch(bsFamily, bsName)) {
        continue;
      }
      iBestSimilar = iSimilarValue;
      pFind = pFont;
    }
  }

  if (pFind) {
    return pFind;
  }

  if (charset == FX_Charset::kANSI && FontFamilyIsFixedPitch(pitch_family)) {
    auto* courier_new = GetFont("Courier New");
    if (courier_new) {
      return courier_new;
    }
  }

  return nullptr;
}

void* CFX_FolderFontInfo::MapFont(int weight,
                                  bool bItalic,
                                  FX_Charset charset,
                                  int pitch_family,
                                  const ByteString& face) {
  return nullptr;
}

void* CFX_FolderFontInfo::GetFont(const ByteString& face) {
  auto it = font_list_.find(face);
  return it != font_list_.end() ? it->second.get() : nullptr;
}

size_t CFX_FolderFontInfo::GetFontData(void* hFont,
                                       uint32_t table,
                                       pdfium::span<uint8_t> buffer) {
  if (!hFont) {
    return 0;
  }

  const FontFaceInfo* pFont = static_cast<FontFaceInfo*>(hFont);
  uint32_t datasize = 0;
  uint32_t offset = 0;
  if (table == 0) {
    datasize = pFont->font_offset_ ? 0 : pFont->file_size_;
  } else if (table == kTableTTCF) {
    datasize = pFont->font_offset_ ? pFont->file_size_ : 0;
  } else {
    size_t nTables = pFont->font_tables_.GetLength() / 16;
    for (size_t i = 0; i < nTables; i++) {
      // TODO(tsepez): iterate over span.
      pdfium::span<const uint8_t> p =
          pFont->font_tables_.unsigned_span().subspan(i * 16);
      if (fxcrt::GetUInt32MSBFirst(p.first<4u>()) == table) {
        offset = fxcrt::GetUInt32MSBFirst(p.subspan<8u, 4u>());
        datasize = fxcrt::GetUInt32MSBFirst(p.subspan<12u, 4u>());
      }
    }
  }

  if (!datasize || buffer.size() < datasize) {
    return datasize;
  }

  std::unique_ptr<FILE, FxFileCloser> pFile(
      fopen(pFont->file_path_.c_str(), "rb"));
  if (!pFile) {
    return 0;
  }

  if (fseek(pFile.get(), offset, SEEK_SET) < 0) {
    return 0;
  }

  // TODO(crbug.com/376633555): Remove debugging data after fixing the bug.
  pdfium::Alias(&datasize);
  char buf[256] = {};
  pdfium::Alias(&buf);
  ByteStringView font_path = pFont->file_path_.AsStringView();
  size_t path_char_count = std::min(font_path.GetLength(), std::size(buf));
  fxcrt::spancpy(pdfium::span(buf), font_path.Last(path_char_count).span());

  if (UNSAFE_TODO(fread(buffer.data(), datasize, 1, pFile.get())) != 1) {
    return 0;
  }
  return datasize;
}

void CFX_FolderFontInfo::DeleteFont(void* hFont) {}

bool CFX_FolderFontInfo::GetFaceName(void* hFont, ByteString* name) {
  if (!hFont) {
    return false;
  }
  *name = static_cast<FontFaceInfo*>(hFont)->face_name_;
  return true;
}

bool CFX_FolderFontInfo::GetFontCharset(void* hFont, FX_Charset* charset) {
  return false;
}

CFX_FolderFontInfo::FontFaceInfo::FontFaceInfo(ByteString filePath,
                                               ByteString faceName,
                                               ByteString fontTables,
                                               uint32_t fontOffset,
                                               uint32_t fileSize)
    : file_path_(filePath),
      face_name_(faceName),
      font_tables_(fontTables),
      font_offset_(fontOffset),
      file_size_(fileSize) {}

bool CFX_FolderFontInfo::FontFaceInfo::IsEligibleForFindFont(
    uint32_t flag,
    FX_Charset charset) const {
  return (charsets_ & flag) || charset == FX_Charset::kDefault;
}

int32_t CFX_FolderFontInfo::FontFaceInfo::SimilarityScore(
    int weight,
    bool italic,
    int pitch_family,
    bool exact_match_bonus) const {
  int32_t score = 0;
  if (FontStyleIsForceBold(styles_) == (weight > 400)) {
    score += 16;
  }
  if (FontStyleIsItalic(styles_) == italic) {
    score += 16;
  }
  if (FontStyleIsSerif(styles_) == FontFamilyIsRoman(pitch_family)) {
    score += 16;
  }
  if (FontStyleIsScript(styles_) == FontFamilyIsScript(pitch_family)) {
    score += 8;
  }
  if (FontStyleIsFixedPitch(styles_) == FontFamilyIsFixedPitch(pitch_family)) {
    score += 8;
  }
  if (exact_match_bonus) {
    score += 4;
  }
  DCHECK_LE(score, kSimilarityScoreMax);
  return score;
}
