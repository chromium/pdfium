// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxcrt/fx_basic.h"
#include "core/include/fxcrt/fx_ext.h"

#include <cctype>

#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
#include <sys/types.h>
#include <dirent.h>
#else
#include <direct.h>
#endif

CFX_PrivateData::~CFX_PrivateData() {
  ClearAll();
}
void FX_PRIVATEDATA::FreeData() {
  if (!m_pData) {
    return;
  }
  if (m_bSelfDestruct) {
    delete (CFX_DestructObject*)m_pData;
  } else if (m_pCallback) {
    m_pCallback(m_pData);
  }
}
void CFX_PrivateData::AddData(void* pModuleId,
                              void* pData,
                              PD_CALLBACK_FREEDATA callback,
                              FX_BOOL bSelfDestruct) {
  if (!pModuleId) {
    return;
  }
  FX_PRIVATEDATA* pList = m_DataList.GetData();
  int count = m_DataList.GetSize();
  for (int i = 0; i < count; i++) {
    if (pList[i].m_pModuleId == pModuleId) {
      pList[i].FreeData();
      pList[i].m_pData = pData;
      pList[i].m_pCallback = callback;
      return;
    }
  }
  FX_PRIVATEDATA data = {pModuleId, pData, callback, bSelfDestruct};
  m_DataList.Add(data);
}
void CFX_PrivateData::SetPrivateData(void* pModuleId,
                                     void* pData,
                                     PD_CALLBACK_FREEDATA callback) {
  AddData(pModuleId, pData, callback, FALSE);
}
void CFX_PrivateData::SetPrivateObj(void* pModuleId, CFX_DestructObject* pObj) {
  AddData(pModuleId, pObj, NULL, TRUE);
}
FX_BOOL CFX_PrivateData::RemovePrivateData(void* pModuleId) {
  if (!pModuleId) {
    return FALSE;
  }
  FX_PRIVATEDATA* pList = m_DataList.GetData();
  int count = m_DataList.GetSize();
  for (int i = 0; i < count; i++) {
    if (pList[i].m_pModuleId == pModuleId) {
      m_DataList.RemoveAt(i);
      return TRUE;
    }
  }
  return FALSE;
}
void* CFX_PrivateData::GetPrivateData(void* pModuleId) {
  if (!pModuleId) {
    return NULL;
  }
  FX_PRIVATEDATA* pList = m_DataList.GetData();
  int count = m_DataList.GetSize();
  for (int i = 0; i < count; i++) {
    if (pList[i].m_pModuleId == pModuleId) {
      return pList[i].m_pData;
    }
  }
  return NULL;
}
void CFX_PrivateData::ClearAll() {
  FX_PRIVATEDATA* pList = m_DataList.GetData();
  int count = m_DataList.GetSize();
  for (int i = 0; i < count; i++) {
    pList[i].FreeData();
  }
  m_DataList.RemoveAll();
}
void FX_atonum(const CFX_ByteStringC& strc, FX_BOOL& bInteger, void* pData) {
  if (!FXSYS_memchr(strc.GetPtr(), '.', strc.GetLength())) {
    bInteger = TRUE;
    int cc = 0, integer = 0;
    const FX_CHAR* str = strc.GetCStr();
    int len = strc.GetLength();
    FX_BOOL bNegative = FALSE;
    if (str[0] == '+') {
      cc++;
    } else if (str[0] == '-') {
      bNegative = TRUE;
      cc++;
    }
    while (cc < len && std::isdigit(str[cc])) {
      // TODO(dsinclair): This is not the right way to handle overflow.
      integer = integer * 10 + FXSYS_toDecimalDigit(str[cc]);
      if (integer < 0)
        break;
      cc++;
    }
    if (bNegative) {
      integer = -integer;
    }
    *(int*)pData = integer;
  } else {
    bInteger = FALSE;
    *(FX_FLOAT*)pData = FX_atof(strc);
  }
}
FX_FLOAT FX_atof(const CFX_ByteStringC& strc) {
  if (strc.GetLength() == 0) {
    return 0.0;
  }
  int cc = 0;
  FX_BOOL bNegative = FALSE;
  const FX_CHAR* str = strc.GetCStr();
  int len = strc.GetLength();
  if (str[0] == '+') {
    cc++;
  } else if (str[0] == '-') {
    bNegative = TRUE;
    cc++;
  }
  while (cc < len) {
    if (str[cc] != '+' && str[cc] != '-') {
      break;
    }
    cc++;
  }
  FX_FLOAT value = 0;
  while (cc < len) {
    if (str[cc] == '.') {
      break;
    }
    value = value * 10 + FXSYS_toDecimalDigit(str[cc]);
    cc++;
  }
  static const FX_FLOAT fraction_scales[] = {
      0.1f,         0.01f,         0.001f,        0.0001f,
      0.00001f,     0.000001f,     0.0000001f,    0.00000001f,
      0.000000001f, 0.0000000001f, 0.00000000001f};
  int scale = 0;
  if (cc < len && str[cc] == '.') {
    cc++;
    while (cc < len) {
      value += fraction_scales[scale] * FXSYS_toDecimalDigit(str[cc]);
      scale++;
      if (scale == sizeof fraction_scales / sizeof(FX_FLOAT)) {
        break;
      }
      cc++;
    }
  }
  return bNegative ? -value : value;
}

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_ && _MSC_VER < 1900
void FXSYS_snprintf(char* str,
                    size_t size,
                    _Printf_format_string_ const char* fmt,
                    ...) {
  va_list ap;
  va_start(ap, fmt);
  FXSYS_vsnprintf(str, size, fmt, ap);
  va_end(ap);
}
void FXSYS_vsnprintf(char* str, size_t size, const char* fmt, va_list ap) {
  (void)_vsnprintf(str, size, fmt, ap);
  if (size) {
    str[size - 1] = 0;
  }
}
#endif  // _FXM_PLATFORM_WINDOWS_ && _MSC_VER < 1900

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
class CFindFileData {
 public:
  virtual ~CFindFileData() {}
  HANDLE m_Handle;
  FX_BOOL m_bEnd;
};
class CFindFileDataA : public CFindFileData {
 public:
  virtual ~CFindFileDataA() {}
  WIN32_FIND_DATAA m_FindData;
};
class CFindFileDataW : public CFindFileData {
 public:
  virtual ~CFindFileDataW() {}
  WIN32_FIND_DATAW m_FindData;
};
#endif
void* FX_OpenFolder(const FX_CHAR* path) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#ifndef _WIN32_WCE
  CFindFileDataA* pData = new CFindFileDataA;
#ifdef _FX_WINAPI_PARTITION_DESKTOP_
  pData->m_Handle =
      FindFirstFileA(CFX_ByteString(path) + "/*.*", &pData->m_FindData);
#else
  pData->m_Handle =
      FindFirstFileExA(CFX_ByteString(path) + "/*.*", FindExInfoStandard,
                       &pData->m_FindData, FindExSearchNameMatch, NULL, 0);
#endif
#else
  CFindFileDataW* pData = new CFindFileDataW;
  pData->m_Handle = FindFirstFileW(CFX_WideString::FromLocal(path) + L"/*.*",
                                   &pData->m_FindData);
#endif
  if (pData->m_Handle == INVALID_HANDLE_VALUE) {
    delete pData;
    return NULL;
  }
  pData->m_bEnd = FALSE;
  return pData;
#else
  DIR* dir = opendir(path);
  return dir;
#endif
}
void* FX_OpenFolder(const FX_WCHAR* path) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  CFindFileDataW* pData = new CFindFileDataW;
#ifdef _FX_WINAPI_PARTITION_DESKTOP_
  pData->m_Handle = FindFirstFileW((CFX_WideString(path) + L"/*.*").c_str(),
                                   &pData->m_FindData);
#else
  pData->m_Handle = FindFirstFileExW((CFX_WideString(path) + L"/*.*").c_str(),
                                     FindExInfoStandard, &pData->m_FindData,
                                     FindExSearchNameMatch, NULL, 0);
#endif
  if (pData->m_Handle == INVALID_HANDLE_VALUE) {
    delete pData;
    return NULL;
  }
  pData->m_bEnd = FALSE;
  return pData;
#else
  DIR* dir = opendir(CFX_ByteString::FromUnicode(path));
  return dir;
#endif
}
FX_BOOL FX_GetNextFile(void* handle,
                       CFX_ByteString& filename,
                       FX_BOOL& bFolder) {
  if (!handle) {
    return FALSE;
  }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#ifndef _WIN32_WCE
  CFindFileDataA* pData = (CFindFileDataA*)handle;
  if (pData->m_bEnd) {
    return FALSE;
  }
  filename = pData->m_FindData.cFileName;
  bFolder = pData->m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
  if (!FindNextFileA(pData->m_Handle, &pData->m_FindData)) {
    pData->m_bEnd = TRUE;
  }
  return TRUE;
#else
  CFindFileDataW* pData = (CFindFileDataW*)handle;
  if (pData->m_bEnd) {
    return FALSE;
  }
  filename = CFX_ByteString::FromUnicode(pData->m_FindData.cFileName);
  bFolder = pData->m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
  if (!FindNextFileW(pData->m_Handle, &pData->m_FindData)) {
    pData->m_bEnd = TRUE;
  }
  return TRUE;
#endif
#elif defined(__native_client__)
  abort();
  return FALSE;
#else
  struct dirent* de = readdir((DIR*)handle);
  if (!de) {
    return FALSE;
  }
  filename = de->d_name;
  bFolder = de->d_type == DT_DIR;
  return TRUE;
#endif
}
FX_BOOL FX_GetNextFile(void* handle,
                       CFX_WideString& filename,
                       FX_BOOL& bFolder) {
  if (!handle) {
    return FALSE;
  }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  CFindFileDataW* pData = (CFindFileDataW*)handle;
  if (pData->m_bEnd) {
    return FALSE;
  }
  filename = pData->m_FindData.cFileName;
  bFolder = pData->m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
  if (!FindNextFileW(pData->m_Handle, &pData->m_FindData)) {
    pData->m_bEnd = TRUE;
  }
  return TRUE;
#elif defined(__native_client__)
  abort();
  return FALSE;
#else
  struct dirent* de = readdir((DIR*)handle);
  if (!de) {
    return FALSE;
  }
  filename = CFX_WideString::FromLocal(de->d_name);
  bFolder = de->d_type == DT_DIR;
  return TRUE;
#endif
}
void FX_CloseFolder(void* handle) {
  if (!handle) {
    return;
  }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  CFindFileData* pData = (CFindFileData*)handle;
  FindClose(pData->m_Handle);
  delete pData;
#else
  closedir((DIR*)handle);
#endif
}
FX_WCHAR FX_GetFolderSeparator() {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  return '\\';
#else
  return '/';
#endif
}

CFX_Matrix_3by3 CFX_Matrix_3by3::Inverse() {
  FX_FLOAT det =
      a * (e * i - f * h) - b * (i * d - f * g) + c * (d * h - e * g);
  if (FXSYS_fabs(det) < 0.0000001)
    return CFX_Matrix_3by3();

  return CFX_Matrix_3by3(
      (e * i - f * h) / det, -(b * i - c * h) / det, (b * f - c * e) / det,
      -(d * i - f * g) / det, (a * i - c * g) / det, -(a * f - c * d) / det,
      (d * h - e * g) / det, -(a * h - b * g) / det, (a * e - b * d) / det);
}

CFX_Matrix_3by3 CFX_Matrix_3by3::Multiply(const CFX_Matrix_3by3& m) {
  return CFX_Matrix_3by3(
      a * m.a + b * m.d + c * m.g, a * m.b + b * m.e + c * m.h,
      a * m.c + b * m.f + c * m.i, d * m.a + e * m.d + f * m.g,
      d * m.b + e * m.e + f * m.h, d * m.c + e * m.f + f * m.i,
      g * m.a + h * m.d + i * m.g, g * m.b + h * m.e + i * m.h,
      g * m.c + h * m.f + i * m.i);
}

CFX_Vector_3by1 CFX_Matrix_3by3::TransformVector(const CFX_Vector_3by1& v) {
  return CFX_Vector_3by1(a * v.a + b * v.b + c * v.c,
                         d * v.a + e * v.b + f * v.c,
                         g * v.a + h * v.b + i * v.c);
}
