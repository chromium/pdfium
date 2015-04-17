// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_ext.h"
#include "fxcrt_windows.h"
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
FX_BOOL FX_File_Exist(FX_BSTR fileName)
{
    FX_DWORD dwAttri = ::GetFileAttributesA(fileName.GetCStr());
    if (dwAttri == -1) {
        return FALSE;
    }
    return (dwAttri & FILE_ATTRIBUTE_DIRECTORY) == 0;
}
FX_BOOL FX_File_Exist(FX_WSTR fileName)
{
    FX_DWORD dwAttri = ::GetFileAttributesW((LPCWSTR)fileName.GetPtr());
    if (dwAttri == -1) {
        return FALSE;
    }
    return (dwAttri & FILE_ATTRIBUTE_DIRECTORY) == 0;
}
IFXCRT_FileAccess* FXCRT_FileAccess_Create()
{
    return new CFXCRT_FileAccess_Win64;
}
void FXCRT_Windows_GetFileMode(FX_DWORD dwMode, FX_DWORD &dwAccess, FX_DWORD &dwShare, FX_DWORD &dwCreation)
{
    dwAccess = GENERIC_READ;
    dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
    if (!(dwMode & FX_FILEMODE_ReadOnly)) {
        dwAccess |= GENERIC_WRITE;
        dwCreation = (dwMode & FX_FILEMODE_Truncate) ? CREATE_ALWAYS : OPEN_ALWAYS;
    } else {
        dwCreation = OPEN_EXISTING;
    }
}
#ifdef __cplusplus
extern "C" {
#endif
WINBASEAPI BOOL WINAPI GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize);
WINBASEAPI BOOL WINAPI SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod);
#ifdef __cplusplus
}
#endif
CFXCRT_FileAccess_Win64::CFXCRT_FileAccess_Win64()
    : m_hFile(NULL)
{
}
CFXCRT_FileAccess_Win64::~CFXCRT_FileAccess_Win64()
{
    Close();
}
FX_BOOL CFXCRT_FileAccess_Win64::Open(FX_BSTR fileName, FX_DWORD dwMode)
{
    if (m_hFile) {
        return FALSE;
    }
    FX_DWORD dwAccess, dwShare, dwCreation;
    FXCRT_Windows_GetFileMode(dwMode, dwAccess, dwShare, dwCreation);
    m_hFile = ::CreateFileA(fileName.GetCStr(), dwAccess, dwShare, NULL, dwCreation, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE) {
        m_hFile = NULL;
    }
    return m_hFile != NULL;
}
FX_BOOL CFXCRT_FileAccess_Win64::Open(FX_WSTR fileName, FX_DWORD dwMode)
{
    if (m_hFile) {
        return FALSE;
    }
    FX_DWORD dwAccess, dwShare, dwCreation;
    FXCRT_Windows_GetFileMode(dwMode, dwAccess, dwShare, dwCreation);
    m_hFile = ::CreateFileW((LPCWSTR)fileName.GetPtr(), dwAccess, dwShare, NULL, dwCreation, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE) {
        m_hFile = NULL;
    }
    return m_hFile != NULL;
}
void CFXCRT_FileAccess_Win64::Close()
{
    if (!m_hFile) {
        return;
    }
    ::CloseHandle(m_hFile);
    m_hFile = NULL;
}
void CFXCRT_FileAccess_Win64::Release()
{
    delete this;
}
FX_FILESIZE CFXCRT_FileAccess_Win64::GetSize() const
{
    if (!m_hFile) {
        return 0;
    }
    LARGE_INTEGER size = {0, 0};
    if (!::GetFileSizeEx(m_hFile, &size)) {
        return 0;
    }
    return (FX_FILESIZE)size.QuadPart;
}
FX_FILESIZE CFXCRT_FileAccess_Win64::GetPosition() const
{
    if (!m_hFile) {
        return (FX_FILESIZE) - 1;
    }
    LARGE_INTEGER dist = {0, 0};
    LARGE_INTEGER newPos = {0, 0};
    if (!::SetFilePointerEx(m_hFile, dist, &newPos, FILE_CURRENT)) {
        return (FX_FILESIZE) - 1;
    }
    return (FX_FILESIZE)newPos.QuadPart;
}
FX_FILESIZE CFXCRT_FileAccess_Win64::SetPosition(FX_FILESIZE pos)
{
    if (!m_hFile) {
        return (FX_FILESIZE) - 1;
    }
    LARGE_INTEGER dist;
    dist.QuadPart = pos;
    LARGE_INTEGER newPos = {0, 0};
    if (!::SetFilePointerEx(m_hFile, dist, &newPos, FILE_BEGIN)) {
        return (FX_FILESIZE) - 1;
    }
    return (FX_FILESIZE)newPos.QuadPart;
}
size_t CFXCRT_FileAccess_Win64::Read(void* pBuffer, size_t szBuffer)
{
    if (!m_hFile) {
        return 0;
    }
    size_t szRead = 0;
    if (!::ReadFile(m_hFile, pBuffer, (DWORD)szBuffer, (LPDWORD)&szRead, NULL)) {
        return 0;
    }
    return szRead;
}
size_t CFXCRT_FileAccess_Win64::Write(const void* pBuffer, size_t szBuffer)
{
    if (!m_hFile) {
        return 0;
    }
    size_t szWrite = 0;
    if (!::WriteFile(m_hFile, pBuffer, (DWORD)szBuffer, (LPDWORD)&szWrite, NULL)) {
        return 0;
    }
    return szWrite;
}
size_t CFXCRT_FileAccess_Win64::ReadPos(void* pBuffer, size_t szBuffer, FX_FILESIZE pos)
{
    if (!m_hFile) {
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
size_t CFXCRT_FileAccess_Win64::WritePos(const void* pBuffer, size_t szBuffer, FX_FILESIZE pos)
{
    if (!m_hFile) {
        return 0;
    }
    if (SetPosition(pos) == (FX_FILESIZE) - 1) {
        return 0;
    }
    return Write(pBuffer, szBuffer);
}
FX_BOOL CFXCRT_FileAccess_Win64::Flush()
{
    if (!m_hFile) {
        return FALSE;
    }
    return ::FlushFileBuffers(m_hFile);
}
FX_BOOL CFXCRT_FileAccess_Win64::Truncate(FX_FILESIZE szFile)
{
    if (SetPosition(szFile) == (FX_FILESIZE) - 1) {
        return FALSE;
    }
    return ::SetEndOfFile(m_hFile);
}
FX_BOOL FX_File_Delete(FX_BSTR fileName)
{
    return ::DeleteFileA(fileName.GetCStr());
}
FX_BOOL FX_File_Delete(FX_WSTR fileName)
{
    return ::DeleteFileW((LPCWSTR)fileName.GetPtr());
}
FX_BOOL FX_File_Copy(FX_BSTR fileNameSrc, FX_BSTR fileNameDst)
{
    return ::CopyFileA(fileNameSrc.GetCStr(), fileNameDst.GetCStr(), FALSE);
}
FX_BOOL FX_File_Copy(FX_WSTR fileNameSrc, FX_WSTR fileNameDst)
{
    return ::CopyFileW((LPCWSTR)fileNameSrc.GetPtr(), (LPCWSTR)fileNameDst.GetPtr(), FALSE);
}
FX_BOOL FX_File_Move(FX_BSTR fileNameSrc, FX_BSTR fileNameDst)
{
    return ::MoveFileA(fileNameSrc.GetCStr(), fileNameDst.GetCStr());
}
FX_BOOL FX_File_Move(FX_WSTR fileNameSrc, FX_WSTR fileNameDst)
{
    return ::MoveFileW((LPCWSTR)fileNameSrc.GetPtr(), (LPCWSTR)fileNameDst.GetPtr());
}
#endif
