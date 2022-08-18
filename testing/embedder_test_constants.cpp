// Copyright 2020 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test_constants.h"

#include "build/build_config.h"

namespace pdfium {

const char* AnnotationStampWithApChecksum() {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kAnnotationStampWithApChecksum[] =
      "a31381406d0b95049e418720750b78dd";
#elif BUILDFLAG(IS_APPLE)
  static constexpr char kAnnotationStampWithApChecksum[] =
      "d243b5d64752be0f45b86df7bd2e2708";
#else
  static constexpr char kAnnotationStampWithApChecksum[] =
      "cdde6c161679ab10b07c38c1ef04b7e8";
#endif

  return kAnnotationStampWithApChecksum;
}

const char kBlankPage612By792Checksum[] = "1940568c9ba33bac5d0b1ee9558c76b3";

const char* Bug890322Checksum() {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kBug890322Checksum[] =
      "793689536cf64fe792c2f241888c0cf3";
#else
  static constexpr char kBug890322Checksum[] =
      "6c674642154408e877d88c6c082d67e9";
#endif

  return kBug890322Checksum;
}

const char* HelloWorldChecksum() {
#if BUILDFLAG(IS_APPLE) && !defined(_SKIA_SUPPORT_) && \
    !defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kHelloWorldChecksum[] =
      "6eef7237f7591f07616e238422086737";
#else
  static constexpr char kHelloWorldChecksum[] =
      "c1c548442e0e0f949c5550d89bf8ae3b";
#endif

  return kHelloWorldChecksum;
}

const char* HelloWorldRemovedChecksum() {
#if BUILDFLAG(IS_APPLE) && !defined(_SKIA_SUPPORT_) && \
    !defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kHelloWorldRemovedChecksum[] =
      "6e1cae48a2e35c521dee4ca502f48af6";
#else
  static constexpr char kHelloWorldRemovedChecksum[] =
      "4a9b80f675f7f3bf2da1b02f12449e4b";
#endif

  return kHelloWorldRemovedChecksum;
}

const char* ManyRectanglesChecksum() {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kManyRectanglesChecksum[] =
      "4e7e280c1597222afcb0ee3bb90ec119";
#else
  static constexpr char kManyRectanglesChecksum[] =
      "b0170c575b65ecb93ebafada0ff0f038";
#endif

  return kManyRectanglesChecksum;
}

const char* RectanglesChecksum() {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kRectanglesChecksum[] =
      "b4e411a6b5ffa59a50efede2efece597";
#else
  static constexpr char kRectanglesChecksum[] =
      "0a90de37f52127619c3dfb642b5fa2fe";
#endif

  return kRectanglesChecksum;
}

const char* TextFormChecksum() {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kTextFormChecksum[] =
      "e6d2eb75f18d773f0dad938b1bb22e23";
#elif BUILDFLAG(IS_APPLE)
  static constexpr char kTextFormChecksum[] =
      "fa2bf756942a950101fc147fc4ef3f82";
#else
  static constexpr char kTextFormChecksum[] =
      "6f86fe1dbed5965d91aec6e0b829e29f";
#endif

  return kTextFormChecksum;
}

}  // namespace pdfium
