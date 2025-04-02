// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_folder.h"

#include <memory>

#include "build/build_config.h"
#include "core/fxcrt/ptr_util.h"

#if !BUILDFLAG(IS_WIN)
#error "built on wrong platform"
#endif

#include <direct.h>

class FX_WindowsFolder : public FX_Folder {
 public:
  ~FX_WindowsFolder() override;
  bool GetNextFile(ByteString* filename, bool* bFolder) override;

 private:
  friend class FX_Folder;
  FX_WindowsFolder();

  HANDLE handle_ = INVALID_HANDLE_VALUE;
  bool reached_end_ = false;
  WIN32_FIND_DATAA find_data_;
};

std::unique_ptr<FX_Folder> FX_Folder::OpenFolder(const ByteString& path) {
  // Private ctor.
  auto handle = pdfium::WrapUnique(new FX_WindowsFolder());
  ByteString search_path = path + "/*.*";
  handle->handle_ =
      FindFirstFileExA(search_path.c_str(), FindExInfoStandard,
                       &handle->find_data_, FindExSearchNameMatch, nullptr, 0);
  if (handle->handle_ == INVALID_HANDLE_VALUE) {
    return nullptr;
  }

  return handle;
}

FX_WindowsFolder::FX_WindowsFolder() = default;

FX_WindowsFolder::~FX_WindowsFolder() {
  if (handle_ != INVALID_HANDLE_VALUE) {
    FindClose(handle_);
  }
}

bool FX_WindowsFolder::GetNextFile(ByteString* filename, bool* bFolder) {
  if (reached_end_) {
    return false;
  }

  *filename = find_data_.cFileName;
  *bFolder = !!(find_data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
  if (!FindNextFileA(handle_, &find_data_)) {
    reached_end_ = true;
  }
  return true;
}
