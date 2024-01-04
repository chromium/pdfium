// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_TEST_FONTS_H_
#define TESTING_TEST_FONTS_H_

#include <string>
#include <vector>

class TestFonts {
 public:
  TestFonts();
  ~TestFonts();

  const char** font_paths() { return font_paths_.data(); }
  void InstallFontMapper();

  static std::string RenameFont(const char* face);

 private:
  std::string font_path_;
  std::vector<const char*> font_paths_;
};

#endif  // TESTING_TEST_FONTS_H_
