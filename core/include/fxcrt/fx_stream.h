// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_STREAM_H_
#define CORE_INCLUDE_FXCRT_FX_STREAM_H_

#include "fx_memory.h"
#include "fx_string.h"

void* FX_OpenFolder(const FX_CHAR* path);
void* FX_OpenFolder(const FX_WCHAR* path);
FX_BOOL FX_GetNextFile(void* handle, CFX_ByteString& filename, FX_BOOL& bFolder);
FX_BOOL FX_GetNextFile(void* handle, CFX_WideString& filename, FX_BOOL& bFolder);
void FX_CloseFolder(void* handle);
FX_WCHAR FX_GetFolderSeparator();
typedef struct FX_HFILE_ {
    void* pData;
}* FX_HFILE;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define FX_FILESIZE			int32_t
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#ifndef O_BINARY
#define O_BINARY 		0
#endif
#ifndef O_LARGEFILE
#define O_LARGEFILE		0
#endif
#define FX_FILESIZE			off_t
#endif
#define FX_GETBYTEOFFSET32(a)	0
#define FX_GETBYTEOFFSET40(a)	0
#define FX_GETBYTEOFFSET48(a)	0
#define FX_GETBYTEOFFSET56(a)	0
#define FX_GETBYTEOFFSET24(a)  ((uint8_t)(a>>24))
#define FX_GETBYTEOFFSET16(a)  ((uint8_t)(a>>16))
#define FX_GETBYTEOFFSET8(a)   ((uint8_t)(a>>8))
#define FX_GETBYTEOFFSET0(a)   ((uint8_t)(a))
#define FX_FILEMODE_Write		0
#define FX_FILEMODE_ReadOnly	1
#define FX_FILEMODE_Truncate	2
FX_HFILE	FX_File_Open(const CFX_ByteStringC& fileName, FX_DWORD dwMode);
FX_HFILE	FX_File_Open(const CFX_WideStringC& fileName, FX_DWORD dwMode);
void		FX_File_Close(FX_HFILE hFile);
FX_FILESIZE	FX_File_GetSize(FX_HFILE hFile);
FX_FILESIZE	FX_File_GetPosition(FX_HFILE hFile);
FX_FILESIZE	FX_File_SetPosition(FX_HFILE hFile, FX_FILESIZE pos);
size_t		FX_File_Read(FX_HFILE hFile, void* pBuffer, size_t szBuffer);
size_t		FX_File_ReadPos(FX_HFILE hFile, void* pBuffer, size_t szBuffer, FX_FILESIZE pos);
size_t		FX_File_Write(FX_HFILE hFile, const void* pBuffer, size_t szBuffer);
size_t		FX_File_WritePos(FX_HFILE hFile, const void* pBuffer, size_t szBuffer, FX_FILESIZE pos);
FX_BOOL		FX_File_Flush(FX_HFILE hFile);
FX_BOOL		FX_File_Truncate(FX_HFILE hFile, FX_FILESIZE szFile);
FX_BOOL		FX_File_Exist(const CFX_ByteStringC& fileName);
FX_BOOL		FX_File_Exist(const CFX_WideStringC& fileName);
FX_BOOL		FX_File_Delete(const CFX_ByteStringC& fileName);
FX_BOOL		FX_File_Delete(const CFX_WideStringC& fileName);
FX_BOOL		FX_File_Copy(const CFX_ByteStringC& fileNameSrc, const CFX_ByteStringC& fileNameDst);
FX_BOOL		FX_File_Copy(const CFX_WideStringC& fileNameSrc, const CFX_WideStringC& fileNameDst);
FX_BOOL		FX_File_Move(const CFX_ByteStringC& fileNameSrc, const CFX_ByteStringC& fileNameDst);
FX_BOOL		FX_File_Move(const CFX_WideStringC& fileNameSrc, const CFX_WideStringC& fileNameDst);
class IFX_StreamWrite
{
public:
    virtual ~IFX_StreamWrite() { }
    virtual void		Release() = 0;

    virtual	FX_BOOL		WriteBlock(const void* pData, size_t size) = 0;
};
class IFX_FileWrite : public IFX_StreamWrite
{
public:

    virtual void			Release() = 0;

    virtual FX_FILESIZE		GetSize() = 0;

    virtual FX_BOOL			Flush() = 0;

    virtual	FX_BOOL			WriteBlock(const void* pData, FX_FILESIZE offset, size_t size) = 0;
    virtual	FX_BOOL			WriteBlock(const void* pData, size_t size)
    {
        return WriteBlock(pData, GetSize(), size);
    }
};
IFX_FileWrite* FX_CreateFileWrite(const FX_CHAR* filename);
IFX_FileWrite* FX_CreateFileWrite(const FX_WCHAR* filename);
class IFX_StreamRead
{
public:
    virtual ~IFX_StreamRead() { }

    virtual void			Release() = 0;

    virtual FX_BOOL			IsEOF() = 0;

    virtual FX_FILESIZE		GetPosition() = 0;

    virtual size_t			ReadBlock(void* buffer, size_t size) = 0;
};
class IFX_FileRead : IFX_StreamRead
{
public:
    virtual void			Release() = 0;

    virtual FX_FILESIZE		GetSize() = 0;

    virtual FX_BOOL			IsEOF()
    {
        return FALSE;
    }

    virtual FX_FILESIZE		GetPosition()
    {
        return 0;
    }

    virtual FX_BOOL			SetRange(FX_FILESIZE offset, FX_FILESIZE size)
    {
        return FALSE;
    }

    virtual void			ClearRange() {}

    virtual FX_BOOL			ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) = 0;

    virtual size_t			ReadBlock(void* buffer, size_t size)
    {
        return 0;
    }
};
IFX_FileRead* FX_CreateFileRead(const FX_CHAR* filename);
IFX_FileRead* FX_CreateFileRead(const FX_WCHAR* filename);
class IFX_FileStream : public IFX_FileRead, public IFX_FileWrite
{
public:

    virtual IFX_FileStream*		Retain() = 0;

    virtual void				Release() = 0;

    virtual FX_FILESIZE			GetSize() = 0;

    virtual FX_BOOL				IsEOF() = 0;

    virtual FX_FILESIZE			GetPosition() = 0;

    virtual FX_BOOL				ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) = 0;

    virtual size_t				ReadBlock(void* buffer, size_t size) = 0;

    virtual	FX_BOOL				WriteBlock(const void* buffer, FX_FILESIZE offset, size_t size) = 0;
    virtual	FX_BOOL				WriteBlock(const void* buffer, size_t size)
    {
        return WriteBlock(buffer, GetSize(), size);
    }

    virtual FX_BOOL				Flush() = 0;
};
IFX_FileStream*		FX_CreateFileStream(const FX_CHAR* filename, FX_DWORD dwModes);
IFX_FileStream*		FX_CreateFileStream(const FX_WCHAR* filename, FX_DWORD dwModes);
class IFX_MemoryStream : public IFX_FileStream
{
public:

    virtual FX_BOOL			IsConsecutive() const = 0;

    virtual void			EstimateSize(size_t nInitSize, size_t nGrowSize) = 0;

    virtual uint8_t*		GetBuffer() const = 0;

    virtual void			AttachBuffer(uint8_t* pBuffer, size_t nSize, FX_BOOL bTakeOver = FALSE) = 0;

    virtual void			DetachBuffer() = 0;
};
IFX_MemoryStream*	FX_CreateMemoryStream(uint8_t* pBuffer, size_t nSize, FX_BOOL bTakeOver = FALSE);
IFX_MemoryStream*	FX_CreateMemoryStream(FX_BOOL bConsecutive = FALSE);
class IFX_BufferRead : public IFX_StreamRead
{
public:

    virtual void			Release() = 0;

    virtual FX_BOOL			IsEOF() = 0;

    virtual FX_FILESIZE		GetPosition() = 0;

    virtual size_t			ReadBlock(void* buffer, size_t size) = 0;

    virtual FX_BOOL			ReadNextBlock(FX_BOOL bRestart = FALSE) = 0;

    virtual const uint8_t*		GetBlockBuffer() = 0;

    virtual size_t			GetBlockSize() = 0;

    virtual FX_FILESIZE		GetBlockOffset() = 0;
};

#endif  // CORE_INCLUDE_FXCRT_FX_STREAM_H_
