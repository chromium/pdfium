// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_SCOPED_LOCALE_H_
#define TESTING_SCOPED_LOCALE_H_

#include <string>

namespace pdfium {

// Sets the given |locale| on construction, and restores the previous locale
// on destruction.
class ScopedLocale {
 public:
  explicit ScopedLocale(const std::string& locale);
  ~ScopedLocale();

 private:
  std::string prev_locale_;

  ScopedLocale(const ScopedLocale&) = delete;
  ScopedLocale& operator=(const ScopedLocale&) = delete;
};

}  // namespace pdfium

#endif  // TESTING_SCOPED_LOCALE_H_
