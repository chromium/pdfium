// Copyright 2020 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test_constants.h"

#include "build/build_config.h"

namespace pdfium {

const char kBlankPage612By792Checksum[] = "1940568c9ba33bac5d0b1ee9558c76b3";

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kBug890322Checksum[] = "793689536cf64fe792c2f241888c0cf3";
#else
const char kBug890322Checksum[] = "6c674642154408e877d88c6c082d67e9";
#endif

#if defined(OS_WIN)
const char kHelloWorldChecksum[] = "795b7ce1626931aa06af0fa23b7d80bb";
const char kHelloWorldRemovedChecksum[] = "93db13099042bafefb3c22a165bad684";
#elif defined(OS_MACOSX)
const char kHelloWorldChecksum[] = "c38b75e16a13852aee3b97d77a0f0ee7";
const char kHelloWorldRemovedChecksum[] = "572b1022bb3e8f43dc671162fc62cf7f";
#else
const char kHelloWorldChecksum[] = "2baa4c0e1758deba1b9c908e1fbd04ed";
const char kHelloWorldRemovedChecksum[] = "93dcc09055f87a2792c8e3065af99a1b";
#endif

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kManyRectanglesChecksum[] = "4e7e280c1597222afcb0ee3bb90ec119";
const char kRectanglesChecksum[] = "b4e411a6b5ffa59a50efede2efece597";
#else
const char kManyRectanglesChecksum[] = "b0170c575b65ecb93ebafada0ff0f038";
const char kRectanglesChecksum[] = "0a90de37f52127619c3dfb642b5fa2fe";
#endif

}  // namespace pdfium
