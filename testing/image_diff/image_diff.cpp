// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file input format is based loosely on
// Tools/DumpRenderTree/ImageDiff.m

// The exact format of this tool's output to stdout is important, to match
// what the run-webkit-tests script expects.

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "core/fxcrt/fx_memory.h"
#include "testing/image_diff/image_diff_png.h"
#include "testing/utils/path_service.h"
#include "third_party/base/logging.h"
#include "third_party/base/numerics/safe_conversions.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

// Return codes used by this utility.
constexpr int kStatusSame = 0;
constexpr int kStatusDifferent = 1;
constexpr int kStatusError = 2;

// Color codes.
constexpr uint32_t RGBA_RED = 0x000000ff;
constexpr uint32_t RGBA_ALPHA = 0xff000000;

class Image {
 public:
  Image() : w_(0), h_(0) {}
  Image(const Image& image) : w_(image.w_), h_(image.h_), data_(image.data_) {}

  bool has_image() const { return w_ > 0 && h_ > 0; }
  int w() const { return w_; }
  int h() const { return h_; }
  pdfium::span<const uint8_t> span() const { return data_; }

  // Creates the image from the given filename on disk, and returns true on
  // success.
  bool CreateFromFilename(const std::string& path) {
    return CreateFromFilenameImpl(path, /*reverse_byte_order=*/false);
  }

  // Same as CreateFromFilename(), but with BGRA instead of RGBA ordering.
  bool CreateFromFilenameWithReverseByteOrder(const std::string& path) {
    return CreateFromFilenameImpl(path, /*reverse_byte_order=*/true);
  }

  void Clear() {
    w_ = h_ = 0;
    data_.clear();
  }

  // Returns the RGBA value of the pixel at the given location
  uint32_t pixel_at(int x, int y) const {
    if (!pixel_in_bounds(x, y))
      return 0;
    return *reinterpret_cast<const uint32_t*>(&(data_[pixel_address(x, y)]));
  }

  void set_pixel_at(int x, int y, uint32_t color) {
    if (!pixel_in_bounds(x, y))
      return;

    void* addr = &data_[pixel_address(x, y)];
    *reinterpret_cast<uint32_t*>(addr) = color;
  }

 private:
  bool CreateFromFilenameImpl(const std::string& path,
                              bool reverse_byte_order) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f)
      return false;

    std::vector<uint8_t> compressed;
    const size_t kBufSize = 1024;
    uint8_t buf[kBufSize];
    size_t num_read = 0;
    while ((num_read = fread(buf, 1, kBufSize, f)) > 0) {
      compressed.insert(compressed.end(), buf, buf + num_read);
    }

    fclose(f);

    data_ = image_diff_png::DecodePNG(compressed, reverse_byte_order, &w_, &h_);
    if (data_.empty()) {
      Clear();
      return false;
    }
    return true;
  }

  bool pixel_in_bounds(int x, int y) const {
    return x >= 0 && x < w_ && y >= 0 && y < h_;
  }

  size_t pixel_address(int x, int y) const { return (y * w_ + x) * 4; }

  // Pixel dimensions of the image.
  int w_;
  int h_;

  std::vector<uint8_t> data_;
};

float CalculateDifferencePercentage(const Image& actual, int pixels_different) {
  // Like the WebKit ImageDiff tool, we define percentage different in terms
  // of the size of the 'actual' bitmap.
  float total_pixels =
      static_cast<float>(actual.w()) * static_cast<float>(actual.h());
  if (total_pixels == 0) {
    // When the bitmap is empty, they are 100% different.
    return 100.0f;
  }
  return 100.0f * pixels_different / total_pixels;
}

void CountImageSizeMismatchAsPixelDifference(const Image& baseline,
                                             const Image& actual,
                                             int* pixels_different) {
  int w = std::min(baseline.w(), actual.w());
  int h = std::min(baseline.h(), actual.h());

  // Count pixels that are a difference in size as also being different.
  int max_w = std::max(baseline.w(), actual.w());
  int max_h = std::max(baseline.h(), actual.h());
  // These pixels are off the right side, not including the lower right corner.
  *pixels_different += (max_w - w) * h;
  // These pixels are along the bottom, including the lower right corner.
  *pixels_different += (max_h - h) * max_w;
}

float PercentageDifferent(const Image& baseline, const Image& actual) {
  int w = std::min(baseline.w(), actual.w());
  int h = std::min(baseline.h(), actual.h());

  // Compute pixels different in the overlap.
  int pixels_different = 0;
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      if (baseline.pixel_at(x, y) != actual.pixel_at(x, y))
        ++pixels_different;
    }
  }

  CountImageSizeMismatchAsPixelDifference(baseline, actual, &pixels_different);
  return CalculateDifferencePercentage(actual, pixels_different);
}

float HistogramPercentageDifferent(const Image& baseline, const Image& actual) {
  // TODO(johnme): Consider using a joint histogram instead, as described in
  // "Comparing Images Using Joint Histograms" by Pass & Zabih
  // http://www.cs.cornell.edu/~rdz/papers/pz-jms99.pdf

  int w = std::min(baseline.w(), actual.w());
  int h = std::min(baseline.h(), actual.h());

  // Count occurrences of each RGBA pixel value of baseline in the overlap.
  std::map<uint32_t, int32_t> baseline_histogram;
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      // hash_map operator[] inserts a 0 (default constructor) if key not found.
      ++baseline_histogram[baseline.pixel_at(x, y)];
    }
  }

  // Compute pixels different in the histogram of the overlap.
  int pixels_different = 0;
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      uint32_t actual_rgba = actual.pixel_at(x, y);
      auto it = baseline_histogram.find(actual_rgba);
      if (it != baseline_histogram.end() && it->second > 0)
        --it->second;
      else
        ++pixels_different;
    }
  }

  CountImageSizeMismatchAsPixelDifference(baseline, actual, &pixels_different);
  return CalculateDifferencePercentage(actual, pixels_different);
}

void PrintHelp(const std::string& binary_name) {
  fprintf(
      stderr,
      "Usage:\n"
      "  %s OPTIONS <compare file> <reference file>\n"
      "    Compares two files on disk, returning 0 when they are the same;\n"
      "    Passing \"--histogram\" additionally calculates a diff of the\n"
      "    RGBA value histograms. (which is resistant to shifts in layout)\n"
      "    Passing \"--reverse-byte-order\" additionally assumes the compare\n"
      "    file has BGRA byte ordering.\n"
      "  %s --diff <compare file> <reference file> <output file>\n"
      "    Compares two files on disk, outputs an image that visualizes the\n"
      "    difference to <output file>\n",
      binary_name.c_str(), binary_name.c_str());
}

int CompareImages(const std::string& binary_name,
                  const std::string& file1,
                  const std::string& file2,
                  bool compare_histograms,
                  bool reverse_byte_order) {
  Image actual_image;
  Image baseline_image;

  bool actual_load_result =
      reverse_byte_order
          ? actual_image.CreateFromFilenameWithReverseByteOrder(file1)
          : actual_image.CreateFromFilename(file1);
  if (!actual_load_result) {
    fprintf(stderr, "%s: Unable to open file \"%s\"\n", binary_name.c_str(),
            file1.c_str());
    return kStatusError;
  }
  if (!baseline_image.CreateFromFilename(file2)) {
    fprintf(stderr, "%s: Unable to open file \"%s\"\n", binary_name.c_str(),
            file2.c_str());
    return kStatusError;
  }

  if (compare_histograms) {
    float percent = HistogramPercentageDifferent(actual_image, baseline_image);
    const char* passed = percent > 0.0 ? "failed" : "passed";
    printf("histogram diff: %01.2f%% %s\n", percent, passed);
  }

  const char* const diff_name = compare_histograms ? "exact diff" : "diff";
  float percent = PercentageDifferent(actual_image, baseline_image);
  const char* const passed = percent > 0.0 ? "failed" : "passed";
  printf("%s: %01.2f%% %s\n", diff_name, percent, passed);

  if (percent > 0.0) {
    // failure: The WebKit version also writes the difference image to
    // stdout, which seems excessive for our needs.
    return kStatusDifferent;
  }
  // success
  return kStatusSame;
}

bool CreateImageDiff(const Image& image1, const Image& image2, Image* out) {
  int w = std::min(image1.w(), image2.w());
  int h = std::min(image1.h(), image2.h());
  *out = Image(image1);
  bool same = (image1.w() == image2.w()) && (image1.h() == image2.h());

  // TODO(estade): do something with the extra pixels if the image sizes
  // are different.
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      uint32_t base_pixel = image1.pixel_at(x, y);
      if (base_pixel != image2.pixel_at(x, y)) {
        // Set differing pixels red.
        out->set_pixel_at(x, y, RGBA_RED | RGBA_ALPHA);
        same = false;
      } else {
        // Set same pixels as faded.
        uint32_t alpha = base_pixel & RGBA_ALPHA;
        uint32_t new_pixel = base_pixel - ((alpha / 2) & RGBA_ALPHA);
        out->set_pixel_at(x, y, new_pixel);
      }
    }
  }

  return same;
}

int DiffImages(const std::string& binary_name,
               const std::string& file1,
               const std::string& file2,
               const std::string& out_file) {
  Image actual_image;
  Image baseline_image;

  if (!actual_image.CreateFromFilename(file1)) {
    fprintf(stderr, "%s: Unable to open file \"%s\"\n", binary_name.c_str(),
            file1.c_str());
    return kStatusError;
  }
  if (!baseline_image.CreateFromFilename(file2)) {
    fprintf(stderr, "%s: Unable to open file \"%s\"\n", binary_name.c_str(),
            file2.c_str());
    return kStatusError;
  }

  Image diff_image;
  bool same = CreateImageDiff(baseline_image, actual_image, &diff_image);
  if (same)
    return kStatusSame;

  std::vector<uint8_t> png_encoding = image_diff_png::EncodeRGBAPNG(
      diff_image.span(), diff_image.w(), diff_image.h(), diff_image.w() * 4);
  if (png_encoding.empty())
    return kStatusError;

  FILE* f = fopen(out_file.c_str(), "wb");
  if (!f)
    return kStatusError;

  size_t size = png_encoding.size();
  char* ptr = reinterpret_cast<char*>(&png_encoding.front());
  if (fwrite(ptr, 1, size, f) != size)
    return kStatusError;

  return kStatusDifferent;
}

int main(int argc, const char* argv[]) {
  FXMEM_InitializePartitionAlloc();

  bool histograms = false;
  bool produce_diff_image = false;
  bool reverse_byte_order = false;
  std::string filename1;
  std::string filename2;
  std::string diff_filename;

  // Strip the path from the first arg
  const char* last_separator = strrchr(argv[0], PATH_SEPARATOR);
  std::string binary_name = last_separator ? last_separator + 1 : argv[0];

  int i;
  for (i = 1; i < argc; ++i) {
    const char* arg = argv[i];
    if (strstr(arg, "--") != arg)
      break;
    if (strcmp(arg, "--histogram") == 0) {
      histograms = true;
    } else if (strcmp(arg, "--diff") == 0) {
      produce_diff_image = true;
    } else if (strcmp(arg, "--reverse-byte-order") == 0) {
      reverse_byte_order = true;
    }
  }
  if (i < argc)
    filename1 = argv[i++];
  if (i < argc)
    filename2 = argv[i++];
  if (i < argc)
    diff_filename = argv[i++];

  if (produce_diff_image) {
    if (!diff_filename.empty()) {
      return DiffImages(binary_name, filename1, filename2, diff_filename);
    }
  } else if (!filename2.empty()) {
    return CompareImages(binary_name, filename1, filename2, histograms,
                         reverse_byte_order);
  }

  PrintHelp(binary_name);
  return kStatusError;
}
