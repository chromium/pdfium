// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/page/cpdf_pageimagecache.h"

#include <memory>
#include <string>
#include <utility>

#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/render/cpdf_docrenderdata.h"
#include "core/fxcrt/fx_stream.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/path_service.h"

TEST(CPDFPageImageCache, RenderBug1924) {
  // If you render a page with a JPEG2000 image as a thumbnail (small picture)
  // first, the image that gets cached has a low resolution. If you afterwards
  // render it full-size, you should get a larger image - the image cache will
  // be regenerate.

  CPDF_PageModule::Create();
  {
    std::string file_path;
    ASSERT_TRUE(PathService::GetTestFilePath("jpx_lzw.pdf", &file_path));
    auto document =
        std::make_unique<CPDF_Document>(std::make_unique<CPDF_DocRenderData>(),
                                        std::make_unique<CPDF_DocPageData>());
    ASSERT_EQ(document->LoadDoc(
                  IFX_SeekableReadStream::CreateFromFilename(file_path.c_str()),
                  nullptr),
              CPDF_Parser::SUCCESS);

    RetainPtr<CPDF_Dictionary> page_dict =
        document->GetMutablePageDictionary(0);
    ASSERT_TRUE(page_dict);
    auto page =
        pdfium::MakeRetain<CPDF_Page>(document.get(), std::move(page_dict));
    page->AddPageImageCache();
    page->ParseContent();

    CPDF_PageImageCache* page_image_cache = page->GetPageImageCache();
    ASSERT_TRUE(page_image_cache);

    CPDF_PageObject* page_obj = page->GetPageObjectByIndex(0);
    ASSERT_TRUE(page_obj);
    CPDF_ImageObject* image = page_obj->AsImage();
    ASSERT_TRUE(image);

    // Render with small scale.
    bool should_continue = page_image_cache->StartGetCachedBitmap(
        image->GetImage(), nullptr, page->GetMutablePageResources(), true,
        CPDF_ColorSpace::Family::kICCBased, false, {50, 50});
    while (should_continue)
      should_continue = page_image_cache->Continue(nullptr);

    RetainPtr<CFX_DIBBase> bitmap_small = page_image_cache->DetachCurBitmap();

    // And render with large scale.
    should_continue = page_image_cache->StartGetCachedBitmap(
        image->GetImage(), nullptr, page->GetMutablePageResources(), true,
        CPDF_ColorSpace::Family::kICCBased, false, {100, 100});
    while (should_continue)
      should_continue = page_image_cache->Continue(nullptr);

    RetainPtr<CFX_DIBBase> bitmap_large = page_image_cache->DetachCurBitmap();

    ASSERT_GT(bitmap_large->GetWidth(), bitmap_small->GetWidth());
    ASSERT_GT(bitmap_large->GetHeight(), bitmap_small->GetHeight());

    ASSERT_TRUE(page->AsPDFPage());
    page->AsPDFPage()->ClearView();
  }
  CPDF_PageModule::Destroy();
}
