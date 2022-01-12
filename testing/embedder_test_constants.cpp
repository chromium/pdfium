// Copyright 2020 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test_constants.h"

#include "build/build_config.h"

namespace pdfium {

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kAnnotationStampWithApChecksum[] =
    "e4e7dc6446fa763a245e03eb5de6ed28";
#else
#if BUILDFLAG(IS_APPLE)
const char kAnnotationStampWithApChecksum[] =
    "d243b5d64752be0f45b86df7bd2e2708";
#else
const char kAnnotationStampWithApChecksum[] =
    "cdde6c161679ab10b07c38c1ef04b7e8";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)

const char kBlankPage612By792Checksum[] = "1940568c9ba33bac5d0b1ee9558c76b3";

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kBug890322Checksum[] = "793689536cf64fe792c2f241888c0cf3";
#else
const char kBug890322Checksum[] = "6c674642154408e877d88c6c082d67e9";
#endif

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kHelloWorldChecksum[] = "fea3e59b7ac7b7a6940018497034f6cf";
#elif BUILDFLAG(IS_APPLE)
const char kHelloWorldChecksum[] = "6eef7237f7591f07616e238422086737";
#else
const char kHelloWorldChecksum[] = "c1c548442e0e0f949c5550d89bf8ae3b";
#endif

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kHelloWorldRemovedChecksum[] = "e51fe51cc5f03ad66f603030df9b0400";
#elif BUILDFLAG(IS_APPLE)
const char kHelloWorldRemovedChecksum[] = "6e1cae48a2e35c521dee4ca502f48af6";
#else
const char kHelloWorldRemovedChecksum[] = "4a9b80f675f7f3bf2da1b02f12449e4b";
#endif

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kManyRectanglesChecksum[] = "4e7e280c1597222afcb0ee3bb90ec119";
const char kRectanglesChecksum[] = "b4e411a6b5ffa59a50efede2efece597";
#else
const char kManyRectanglesChecksum[] = "b0170c575b65ecb93ebafada0ff0f038";
const char kRectanglesChecksum[] = "0a90de37f52127619c3dfb642b5fa2fe";
#endif

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kTextFormChecksum[] = "f8f0817b19ef07d0404caf008964b7f2";
#elif BUILDFLAG(IS_APPLE)
const char kTextFormChecksum[] = "fa2bf756942a950101fc147fc4ef3f82";
#else
const char kTextFormChecksum[] = "6f86fe1dbed5965d91aec6e0b829e29f";
#endif

}  // namespace pdfium
