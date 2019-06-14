// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BASE_TEST_SCOPED_LOCALE_H_
#define THIRD_PARTY_BASE_TEST_SCOPED_LOCALE_H_

#include <string>

namespace pdfium {
namespace base {

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

}  // namespace base
}  // namespace pdfium

#endif  // THIRD_PARTY_BASE_TEST_SCOPED_LOCALE_H_
