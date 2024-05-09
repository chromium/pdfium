// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_HELPERS_PAGE_RENDERER_H_
#define TESTING_HELPERS_PAGE_RENDERER_H_

#include <string>

#include "public/fpdfview.h"

// Renderer for a single page.
class PageRenderer {
 public:
  virtual ~PageRenderer();

  // Returns `true` if the rendered output exists. Must call `Finish()` first.
  virtual bool HasOutput() const = 0;

  // Starts rendering the page, returning `false` on failure.
  virtual bool Start() = 0;

  // Continues rendering the page, returning `false` when complete.
  virtual bool Continue();

  // Finishes rendering the page.
  virtual void Finish(FPDF_FORMHANDLE form) = 0;

  // Writes rendered output to a file, returning `false` on failure.
  virtual bool Write(const std::string& name, int page_index, bool md5) = 0;

 protected:
  PageRenderer(FPDF_PAGE page, int width, int height, int flags);

  FPDF_PAGE page() { return page_; }
  int width() const { return width_; }
  int height() const { return height_; }
  int flags() const { return flags_; }

 private:
  FPDF_PAGE page_;
  int width_;
  int height_;
  int flags_;
};

#endif  // TESTING_HELPERS_PAGE_RENDERER_H_
