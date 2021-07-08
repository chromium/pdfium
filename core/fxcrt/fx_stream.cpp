// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_stream.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/fileaccess_iface.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/unowned_ptr.h"

#if defined(OS_WIN)
#include <direct.h>

struct FX_FolderHandle {
  HANDLE m_Handle;
  bool m_bReachedEnd;
  WIN32_FIND_DATAA m_FindData;
};
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

struct FX_FolderHandle {
  ByteString m_Path;
  UnownedPtr<DIR> m_Dir;
};
#endif

namespace {

class CFX_CRTFileStream final : public IFX_SeekableStream {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  // IFX_SeekableStream:
  FX_FILESIZE GetSize() override { return m_pFile->GetSize(); }
  bool IsEOF() override { return GetPosition() >= GetSize(); }
  FX_FILESIZE GetPosition() override { return m_pFile->GetPosition(); }
  bool ReadBlockAtOffset(void* buffer,
                         FX_FILESIZE offset,
                         size_t size) override {
    return m_pFile->ReadPos(buffer, size, offset) > 0;
  }
  size_t ReadBlock(void* buffer, size_t size) override {
    return m_pFile->Read(buffer, size);
  }
  bool WriteBlockAtOffset(const void* buffer,
                          FX_FILESIZE offset,
                          size_t size) override {
    return !!m_pFile->WritePos(buffer, size, offset);
  }
  bool Flush() override { return m_pFile->Flush(); }

 private:
  explicit CFX_CRTFileStream(std::unique_ptr<FileAccessIface> pFA)
      : m_pFile(std::move(pFA)) {}
  ~CFX_CRTFileStream() override = default;

  std::unique_ptr<FileAccessIface> m_pFile;
};

}  // namespace

bool IFX_WriteStream::WriteString(ByteStringView str) {
  return WriteBlock(str.unterminated_c_str(), str.GetLength());
}

bool IFX_WriteStream::WriteByte(uint8_t byte) {
  return WriteBlock(&byte, 1);
}

bool IFX_WriteStream::WriteDWord(uint32_t i) {
  char buf[32];
  FXSYS_itoa(i, buf, 10);
  return WriteBlock(buf, strlen(buf));
}

// static
RetainPtr<IFX_SeekableStream> IFX_SeekableStream::CreateFromFilename(
    const char* filename,
    uint32_t dwModes) {
  std::unique_ptr<FileAccessIface> pFA = FileAccessIface::Create();
  if (!pFA->Open(filename, dwModes))
    return nullptr;
  return pdfium::MakeRetain<CFX_CRTFileStream>(std::move(pFA));
}

// static
RetainPtr<IFX_SeekableStream> IFX_SeekableStream::CreateFromFilename(
    const wchar_t* filename,
    uint32_t dwModes) {
  std::unique_ptr<FileAccessIface> pFA = FileAccessIface::Create();
  if (!pFA->Open(filename, dwModes))
    return nullptr;
  return pdfium::MakeRetain<CFX_CRTFileStream>(std::move(pFA));
}

// static
RetainPtr<IFX_SeekableReadStream> IFX_SeekableReadStream::CreateFromFilename(
    const char* filename) {
  return IFX_SeekableStream::CreateFromFilename(filename, FX_FILEMODE_ReadOnly);
}

bool IFX_SeekableWriteStream::WriteBlock(const void* pData, size_t size) {
  return WriteBlockAtOffset(pData, GetSize(), size);
}

bool IFX_SeekableReadStream::IsEOF() {
  return false;
}

FX_FILESIZE IFX_SeekableReadStream::GetPosition() {
  return 0;
}

size_t IFX_SeekableReadStream::ReadBlock(void* buffer, size_t size) {
  return 0;
}

bool IFX_SeekableStream::WriteBlock(const void* buffer, size_t size) {
  return WriteBlockAtOffset(buffer, GetSize(), size);
}

FX_FolderHandle* FX_OpenFolder(const char* path) {
  auto handle = std::make_unique<FX_FolderHandle>();
#if defined(OS_WIN)
  handle->m_Handle =
      FindFirstFileExA((ByteString(path) + "/*.*").c_str(), FindExInfoStandard,
                       &handle->m_FindData, FindExSearchNameMatch, nullptr, 0);
  if (handle->m_Handle == INVALID_HANDLE_VALUE)
    return nullptr;

  handle->m_bReachedEnd = false;
#else
  DIR* dir = opendir(path);
  if (!dir)
    return nullptr;

  handle->m_Path = path;
  handle->m_Dir = dir;
#endif
  return handle.release();
}

bool FX_GetNextFile(FX_FolderHandle* handle,
                    ByteString* filename,
                    bool* bFolder) {
  if (!handle)
    return false;

#if defined(OS_WIN)
  if (handle->m_bReachedEnd)
    return false;

  *filename = handle->m_FindData.cFileName;
  *bFolder =
      (handle->m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  if (!FindNextFileA(handle->m_Handle, &handle->m_FindData))
    handle->m_bReachedEnd = true;
  return true;
#else
  struct dirent* de = readdir(handle->m_Dir);
  if (!de)
    return false;
  ByteString fullpath = handle->m_Path + "/" + de->d_name;
  struct stat deStat;
  if (stat(fullpath.c_str(), &deStat) < 0)
    return false;

  *filename = de->d_name;
  *bFolder = S_ISDIR(deStat.st_mode);
  return true;
#endif
}

void FX_CloseFolder(FX_FolderHandle* handle) {
  if (!handle)
    return;

#if defined(OS_WIN)
  FindClose(handle->m_Handle);
#else
  closedir(handle->m_Dir.Release());
#endif
  delete handle;
}
