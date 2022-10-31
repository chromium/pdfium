// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_TEST_FONTS_H_
#define TESTING_TEST_FONTS_H_

#include <memory>
#include <string>

class TestFonts {
 public:
  TestFonts();
  ~TestFonts();

  const char** font_paths() const { return font_paths_.get(); }

  void InstallFontMapper();

  static std::string RenameFont(const char* face);

 private:
  std::string font_path_;
  std::unique_ptr<const char*[]> font_paths_;
};

#endif  // TESTING_TEST_FONTS_H_
