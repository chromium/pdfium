// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/font_renamer.h"

#include <string>

#include "testing/test_fonts.h"

namespace {

FPDF_SYSFONTINFO* GetImpl(FPDF_SYSFONTINFO* info) {
  return static_cast<FontRenamer*>(info)->impl();
}

void ReleaseImpl(FPDF_SYSFONTINFO* info) {
  FPDF_SYSFONTINFO* impl = GetImpl(info);
  impl->Release(impl);
}

void EnumFontsImpl(FPDF_SYSFONTINFO* info, void* mapper) {
  FPDF_SYSFONTINFO* impl = GetImpl(info);
  impl->EnumFonts(impl, mapper);
}

void* MapFontImpl(FPDF_SYSFONTINFO* info,
                  int weight,
                  FPDF_BOOL italic,
                  int charset,
                  int pitch_family,
                  const char* face,
                  FPDF_BOOL* exact) {
  std::string renamed_face = TestFonts::RenameFont(face);
  FPDF_SYSFONTINFO* impl = GetImpl(info);
  return impl->MapFont(impl, weight, italic, charset, pitch_family,
                       renamed_face.c_str(), exact);
}

void* GetFontImpl(FPDF_SYSFONTINFO* info, const char* face) {
  // Any non-null return will do.
  FPDF_SYSFONTINFO* impl = GetImpl(info);
  std::string renamed_face = TestFonts::RenameFont(face);
  return impl->GetFont(impl, renamed_face.c_str());
}

unsigned long GetFontDataImpl(FPDF_SYSFONTINFO* info,
                              void* font,
                              unsigned int table,
                              unsigned char* buffer,
                              unsigned long buf_size) {
  FPDF_SYSFONTINFO* impl = GetImpl(info);
  return impl->GetFontData(impl, font, table, buffer, buf_size);
}

unsigned long GetFaceNameImpl(FPDF_SYSFONTINFO* info,
                              void* font,
                              char* buffer,
                              unsigned long buf_size) {
  FPDF_SYSFONTINFO* impl = GetImpl(info);
  return impl->GetFaceName(impl, font, buffer, buf_size);
}

int GetFontCharsetImpl(FPDF_SYSFONTINFO* info, void* font) {
  FPDF_SYSFONTINFO* impl = GetImpl(info);
  return impl->GetFontCharset(impl, font);
}

void DeleteFontImpl(FPDF_SYSFONTINFO* info, void* font) {
  FPDF_SYSFONTINFO* impl = GetImpl(info);
  impl->DeleteFont(impl, font);
}

}  // namespace

FontRenamer::FontRenamer() : impl_(FPDF_GetDefaultSystemFontInfo()) {
  version = 1;
  Release = ReleaseImpl;
  EnumFonts = EnumFontsImpl;
  MapFont = MapFontImpl;
  GetFont = GetFontImpl;
  GetFontData = GetFontDataImpl;
  GetFaceName = GetFaceNameImpl;
  GetFontCharset = GetFontCharsetImpl;
  DeleteFont = DeleteFontImpl;
  FPDF_SetSystemFontInfo(this);
}

FontRenamer::~FontRenamer() {
  FPDF_SetSystemFontInfo(nullptr);
  FPDF_FreeDefaultSystemFontInfo(impl_.ExtractAsDangling());
}
