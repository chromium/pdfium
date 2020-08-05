// Copyright 2020 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test_constants.h"

#include "build/build_config.h"

namespace pdfium {

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
const char kAnnotationStampWithApChecksum[] =
    "db83eaadc92967e3ac9bebfc6178ca75";
#else
const char kAnnotationStampWithApChecksum[] =
    "3c87b4a8e51245964357fb5f5fbc612b";
#endif  // defined(OS_LINUX) || defined(OS_CHROMEOS)
#else
#if defined(OS_WIN)
const char kAnnotationStampWithApChecksum[] =
    "6aa001a77ec05d0f1b0d1d22e28744d4";
#elif defined(OS_APPLE)
const char kAnnotationStampWithApChecksum[] =
    "80d7b6cc7b13a78d77a6151bc846e80b";
#else
const char kAnnotationStampWithApChecksum[] =
    "b42cef463483e668eaf4055a65e4f1f5";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)

const char kBlankPage612By792Checksum[] = "1940568c9ba33bac5d0b1ee9558c76b3";

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kBug890322Checksum[] = "793689536cf64fe792c2f241888c0cf3";
#else
const char kBug890322Checksum[] = "6c674642154408e877d88c6c082d67e9";
#endif

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#if defined(OS_WIN)
const char kHelloWorldChecksum[] = "7fca5790ce81c715d74d955ea9939fd8";
#else
const char kHelloWorldChecksum[] = "66ecb880a880dd263ff495b28aeda0d1";
#endif  // defined(OS_WIN)
#else
#if defined(OS_WIN)
const char kHelloWorldChecksum[] = "795b7ce1626931aa06af0fa23b7d80bb";
#elif defined(OS_APPLE)
const char kHelloWorldChecksum[] = "c38b75e16a13852aee3b97d77a0f0ee7";
#else
const char kHelloWorldChecksum[] = "2baa4c0e1758deba1b9c908e1fbd04ed";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kHelloWorldRemovedChecksum[] = "f87c63cbbc83fbb0f5b7b1d9e67448d0";
#else
#if defined(OS_WIN)
const char kHelloWorldRemovedChecksum[] = "93db13099042bafefb3c22a165bad684";
#elif defined(OS_APPLE)
const char kHelloWorldRemovedChecksum[] = "572b1022bb3e8f43dc671162fc62cf7f";
#else
const char kHelloWorldRemovedChecksum[] = "93dcc09055f87a2792c8e3065af99a1b";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kManyRectanglesChecksum[] = "4e7e280c1597222afcb0ee3bb90ec119";
const char kRectanglesChecksum[] = "b4e411a6b5ffa59a50efede2efece597";
#else
const char kManyRectanglesChecksum[] = "b0170c575b65ecb93ebafada0ff0f038";
const char kRectanglesChecksum[] = "0a90de37f52127619c3dfb642b5fa2fe";
#endif

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kTextFormChecksum[] = "17efe329169f5b7681fbe939894a35de";
#else
#if defined(OS_WIN)
const char kTextFormChecksum[] = "d3204faa62b607f0bd3893c9c22cabcb";
#elif defined(OS_APPLE)
const char kTextFormChecksum[] = "d485541d958fef08d24e8eca3e537023";
#else
const char kTextFormChecksum[] = "b890950d4b9bc163b1a96797f3004b53";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)

}  // namespace pdfium
