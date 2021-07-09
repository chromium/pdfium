// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_STREAM_H_
#define CORE_FXCRT_FX_STREAM_H_

#include <stddef.h>
#include <stdint.h>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_types.h"
#include "core/fxcrt/retain_ptr.h"

constexpr uint32_t FX_FILEMODE_ReadOnly = 1 << 0;
constexpr uint32_t FX_FILEMODE_Truncate = 1 << 1;

class IFX_WriteStream {
 public:
  virtual bool WriteBlock(const void* pData, size_t size) = 0;

  bool WriteString(ByteStringView str);
  bool WriteByte(uint8_t byte);
  bool WriteDWord(uint32_t i);
  bool WriteFilesize(FX_FILESIZE size);

 protected:
  virtual ~IFX_WriteStream() = default;
};

class IFX_ArchiveStream : public IFX_WriteStream {
 public:
  virtual FX_FILESIZE CurrentOffset() const = 0;
};

class IFX_StreamWithSize {
 public:
  virtual FX_FILESIZE GetSize() = 0;
};

class IFX_RetainableWriteStream : virtual public Retainable,
                                  public IFX_WriteStream {};

class IFX_SeekableWriteStream : virtual public IFX_StreamWithSize,
                                public IFX_RetainableWriteStream {
 public:
  // IFX_WriteStream:
  bool WriteBlock(const void* pData, size_t size) override;

  virtual bool Flush() = 0;
  virtual bool WriteBlockAtOffset(const void* pData,
                                  FX_FILESIZE offset,
                                  size_t size) = 0;
};

class IFX_SeekableReadStream : virtual public Retainable,
                               virtual public IFX_StreamWithSize {
 public:
  static RetainPtr<IFX_SeekableReadStream> CreateFromFilename(
      const char* filename);

  virtual bool IsEOF();
  virtual FX_FILESIZE GetPosition();
  virtual size_t ReadBlock(void* buffer, size_t size);

  virtual bool ReadBlockAtOffset(void* buffer,
                                 FX_FILESIZE offset,
                                 size_t size) WARN_UNUSED_RESULT = 0;
};

class IFX_SeekableStream : public IFX_SeekableReadStream,
                           public IFX_SeekableWriteStream {
 public:
  // dwModes is a mask of FX_FILEMODE_* from above.
  static RetainPtr<IFX_SeekableStream> CreateFromFilename(const char* filename,
                                                          uint32_t dwModes);

  // dwModes is a mask of FX_FILEMODE_* from above.
  static RetainPtr<IFX_SeekableStream> CreateFromFilename(
      const wchar_t* filename,
      uint32_t dwModes);

  // IFX_SeekableWriteStream:
  bool WriteBlock(const void* buffer, size_t size) override;
};

#endif  // CORE_FXCRT_FX_STREAM_H_
