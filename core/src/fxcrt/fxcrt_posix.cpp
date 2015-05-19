// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_ext.h"
#include "fxcrt_posix.h"
#if _FXM_PLATFORM_ == _FXM_PLATFORM_LINUX_ || _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_ || _FXM_PLATFORM_ == _FXM_PLATFORM_ANDROID_
IFXCRT_FileAccess* FXCRT_FileAccess_Create()
{
    return new CFXCRT_FileAccess_Posix;
}
void FXCRT_Posix_GetFileMode(FX_DWORD dwModes, FX_INT32 &nFlags, FX_INT32 &nMasks)
{
    nFlags = O_BINARY | O_LARGEFILE;
    if (dwModes & FX_FILEMODE_ReadOnly) {
        nFlags |= O_RDONLY;
        nMasks = 0;
    } else {
        nFlags |= O_RDWR | O_CREAT;
        if (dwModes & FX_FILEMODE_Truncate) {
            nFlags |= O_TRUNC;
        }
        nMasks = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    }
}
CFXCRT_FileAccess_Posix::CFXCRT_FileAccess_Posix()
    : m_nFD(-1)
{
}
CFXCRT_FileAccess_Posix::~CFXCRT_FileAccess_Posix()
{
    Close();
}
FX_BOOL CFXCRT_FileAccess_Posix::Open(FX_BSTR fileName, FX_DWORD dwMode)
{
    if (m_nFD > -1) {
        return FALSE;
    }
    FX_INT32 nFlags, nMasks;
    FXCRT_Posix_GetFileMode(dwMode, nFlags, nMasks);
    m_nFD = open(fileName.GetCStr(), nFlags, nMasks);
    return m_nFD > -1;
}
FX_BOOL CFXCRT_FileAccess_Posix::Open(FX_WSTR fileName, FX_DWORD dwMode)
{
    return Open(FX_UTF8Encode(fileName), dwMode);
}
void CFXCRT_FileAccess_Posix::Close()
{
    if (m_nFD < 0) {
        return;
    }
    close(m_nFD);
    m_nFD = -1;
}
void CFXCRT_FileAccess_Posix::Release()
{
    delete this;
}
FX_FILESIZE CFXCRT_FileAccess_Posix::GetSize() const
{
    if (m_nFD < 0) {
        return 0;
    }
    struct stat s;
    FXSYS_memset32(&s, 0, sizeof(s));
    fstat(m_nFD, &s);
    return s.st_size;
}
FX_FILESIZE CFXCRT_FileAccess_Posix::GetPosition() const
{
    if (m_nFD < 0) {
        return (FX_FILESIZE) - 1;
    }
    return lseek(m_nFD, 0, SEEK_CUR);
}
FX_FILESIZE CFXCRT_FileAccess_Posix::SetPosition(FX_FILESIZE pos)
{
    if (m_nFD < 0) {
        return (FX_FILESIZE) - 1;
    }
    return lseek(m_nFD, pos, SEEK_SET);
}
size_t CFXCRT_FileAccess_Posix::Read(void* pBuffer, size_t szBuffer)
{
    if (m_nFD < 0) {
        return 0;
    }
    return read(m_nFD, pBuffer, szBuffer);
}
size_t CFXCRT_FileAccess_Posix::Write(const void* pBuffer, size_t szBuffer)
{
    if (m_nFD < 0) {
        return 0;
    }
    return write(m_nFD, pBuffer, szBuffer);
}
size_t CFXCRT_FileAccess_Posix::ReadPos(void* pBuffer, size_t szBuffer, FX_FILESIZE pos)
{
    if (m_nFD < 0) {
        return 0;
    }
    if (pos >= GetSize()) {
        return 0;
    }
    if (SetPosition(pos) == (FX_FILESIZE) - 1) {
        return 0;
    }
    return Read(pBuffer, szBuffer);
}
size_t CFXCRT_FileAccess_Posix::WritePos(const void* pBuffer, size_t szBuffer, FX_FILESIZE pos)
{
    if (m_nFD < 0) {
        return 0;
    }
    if (SetPosition(pos) == (FX_FILESIZE) - 1) {
        return 0;
    }
    return Write(pBuffer, szBuffer);
}
FX_BOOL CFXCRT_FileAccess_Posix::Flush()
{
    if (m_nFD < 0) {
        return FALSE;
    }
    return fsync(m_nFD) > -1;
}
FX_BOOL CFXCRT_FileAccess_Posix::Truncate(FX_FILESIZE szFile)
{
    if (m_nFD < 0) {
        return FALSE;
    }
    return !ftruncate(m_nFD, szFile);
}
FX_BOOL FX_File_Exist(FX_BSTR fileName)
{
    return access(fileName.GetCStr(), F_OK) > -1;
}
FX_BOOL FX_File_Exist(FX_WSTR fileName)
{
    return FX_File_Exist(FX_UTF8Encode(fileName));
}
FX_BOOL FX_File_Delete(FX_BSTR fileName)
{
    return remove(fileName.GetCStr()) > -1;
}
FX_BOOL FX_File_Delete(FX_WSTR fileName)
{
    return FX_File_Delete(FX_UTF8Encode(fileName));
}
FX_BOOL FX_File_Copy(FX_BSTR fileNameSrc, FX_BSTR fileNameDst)
{
    CFXCRT_FileAccess_Posix src, dst;
    if (!src.Open(fileNameSrc, FX_FILEMODE_ReadOnly)) {
        return FALSE;
    }
    FX_FILESIZE size = src.GetSize();
    if (!size) {
        return FALSE;
    }
    if (!dst.Open(fileNameDst, FX_FILEMODE_Truncate)) {
        return FALSE;
    }
    size_t num = 0;
    FX_LPBYTE pBuffer = FX_Alloc(FX_BYTE, 32768);
    num = src.Read(pBuffer, 32768);
    while (num) {
        if (dst.Write(pBuffer, num) != num) {
            break;
        }
        num = src.Read(pBuffer, 32768);
    }
    FX_Free(pBuffer);
    return TRUE;
}
FX_BOOL FX_File_Copy(FX_WSTR fileNameSrc, FX_WSTR fileNameDst)
{
    return FX_File_Copy(FX_UTF8Encode(fileNameSrc), FX_UTF8Encode(fileNameDst));
}
FX_BOOL FX_File_Move(FX_BSTR fileNameSrc, FX_BSTR fileNameDst)
{
    return rename(fileNameSrc.GetCStr(), fileNameDst.GetCStr());
}
FX_BOOL FX_File_Move(FX_WSTR fileNameSrc, FX_WSTR fileNameDst)
{
    return FX_File_Move(FX_UTF8Encode(fileNameSrc), FX_UTF8Encode(fileNameDst));
}
#endif
