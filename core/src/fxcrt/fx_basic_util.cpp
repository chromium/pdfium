// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_basic.h"
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
#include <sys/types.h>
#include <dirent.h>
#else
#include <direct.h>
#endif
CFX_PrivateData::~CFX_PrivateData()
{
    ClearAll();
}
void FX_PRIVATEDATA::FreeData()
{
    if (m_pData == NULL) {
        return;
    }
    if (m_bSelfDestruct) {
        delete (CFX_DestructObject*)m_pData;
    } else if (m_pCallback) {
        m_pCallback(m_pData);
    }
}
void CFX_PrivateData::AddData(FX_LPVOID pModuleId, FX_LPVOID pData, PD_CALLBACK_FREEDATA callback, FX_BOOL bSelfDestruct)
{
    if (pModuleId == NULL) {
        return;
    }
    FX_PRIVATEDATA* pList = m_DataList.GetData();
    int count = m_DataList.GetSize();
    for (int i = 0; i < count; i ++) {
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
void CFX_PrivateData::SetPrivateData(FX_LPVOID pModuleId, FX_LPVOID pData, PD_CALLBACK_FREEDATA callback)
{
    AddData(pModuleId, pData, callback, FALSE);
}
void CFX_PrivateData::SetPrivateObj(FX_LPVOID pModuleId, CFX_DestructObject* pObj)
{
    AddData(pModuleId, pObj, NULL, TRUE);
}
FX_BOOL CFX_PrivateData::RemovePrivateData(FX_LPVOID pModuleId)
{
    if (pModuleId == NULL) {
        return FALSE;
    }
    FX_PRIVATEDATA* pList = m_DataList.GetData();
    int count = m_DataList.GetSize();
    for (int i = 0; i < count; i ++) {
        if (pList[i].m_pModuleId == pModuleId) {
            m_DataList.RemoveAt(i);
            return TRUE;
        }
    }
    return FALSE;
}
FX_LPVOID CFX_PrivateData::GetPrivateData(FX_LPVOID pModuleId)
{
    if (pModuleId == NULL) {
        return NULL;
    }
    FX_PRIVATEDATA* pList = m_DataList.GetData();
    int count = m_DataList.GetSize();
    for (int i = 0; i < count; i ++) {
        if (pList[i].m_pModuleId == pModuleId) {
            return pList[i].m_pData;
        }
    }
    return NULL;
}
void CFX_PrivateData::ClearAll()
{
    FX_PRIVATEDATA* pList = m_DataList.GetData();
    int count = m_DataList.GetSize();
    for (int i = 0; i < count; i ++) {
        pList[i].FreeData();
    }
    m_DataList.RemoveAll();
}
void FX_atonum(FX_BSTR strc, FX_BOOL& bInteger, void* pData)
{
    if (FXSYS_memchr(strc.GetPtr(), '.', strc.GetLength()) == NULL) {
        bInteger = TRUE;
        int cc = 0, integer = 0;
        FX_LPCSTR str = strc.GetCStr();
        int len = strc.GetLength();
        FX_BOOL bNegative = FALSE;
        if (str[0] == '+') {
            cc++;
        } else if (str[0] == '-') {
            bNegative = TRUE;
            cc++;
        }
        while (cc < len) {
            if (str[cc] < '0' || str[cc] > '9') {
                break;
            }
            integer = integer * 10 + str[cc] - '0';
            if (integer < 0) {
                break;
            }
            cc ++;
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
FX_FLOAT FX_atof(FX_BSTR strc)
{
    if (strc.GetLength() == 0) {
        return 0.0;
    }
    int cc = 0;
    FX_BOOL bNegative = FALSE;
    FX_LPCSTR str = strc.GetCStr();
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
        cc ++;
    }
    FX_FLOAT value = 0;
    while (cc < len) {
        if (str[cc] == '.') {
            break;
        }
        value = value * 10 + str[cc] - '0';
        cc ++;
    }
    static const FX_FLOAT fraction_scales[] = {0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f,
                                               0.0000001f, 0.00000001f, 0.000000001f, 0.0000000001f, 0.00000000001f
                                              };
    int scale = 0;
    if (cc < len && str[cc] == '.') {
        cc ++;
        while (cc < len) {
            value += fraction_scales[scale] * (str[cc] - '0');
            scale ++;
            if (scale == sizeof fraction_scales / sizeof(FX_FLOAT)) {
                break;
            }
            cc ++;
        }
    }
    return bNegative ? -value : value;
}

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_ && _MSC_VER < 1900
void FXSYS_snprintf(char *str, size_t size, _Printf_format_string_ const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    FXSYS_vsnprintf(str, size, fmt, ap);
    va_end(ap);
}
void FXSYS_vsnprintf(char *str, size_t size, const char* fmt, va_list ap)
{
    (void) _vsnprintf(str, size, fmt, ap);
    if (size) {
        str[size - 1] = 0;
    }
}
#endif  // _FXM_PLATFORM_WINDOWS_ && _MSC_VER < 1900

static FX_BOOL FX_IsDigit(FX_BYTE ch)
{
    return (ch >= '0' && ch <= '9') ? TRUE : FALSE;
}
static FX_BOOL FX_IsXDigit(FX_BYTE ch)
{
    return (FX_IsDigit(ch) || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) ? TRUE : FALSE;
}
static FX_BYTE FX_MakeUpper(FX_BYTE ch)
{
    if (ch < 'a' || ch > 'z') {
        return ch;
    }
    return ch - 32;
}
static int FX_HexToI(FX_BYTE ch)
{
    ch = FX_MakeUpper(ch);
    return FX_IsDigit(ch) ? (ch - '0') : (ch - 55);
}
static const unsigned char url_encodeTable[128] = {
    1,  1,  1,  1,		1,  1,  1,  1,
    1,  1,  1,  1,		1,  1,  1,  1,
    1,  1,  1,  1,		1,  1,  1,  1,
    1,  1,  1,  1,		1,  1,  1,  1,
    1,  0,  1,  1,		0,  1,  0,  0,
    0,  0,  0,  0,		0,  0,  0,  0,
    0,  0,  0,  0,		0,  0,  0,  0,
    0,  0,  0,  0,		1,  0,  1,  0,
    0,  0,  0,  0,		0,  0,  0,  0,
    0,  0,  0,  0,		0,  0,  0,  0,
    0,  0,  0,  0,		0,  0,  0,  0,
    0,  0,  0,  1,		1,  1,  1,  0,
    1,  0,  0,  0,		0,  0,  0,  0,
    0,  0,  0,  0,		0,  0,  0,  0,
    0,  0,  0,  0,		0,  0,  0,  0,
    0,  0,  0,  1,		1,  1,  1,  1,
};
CFX_ByteString FX_UrlEncode(const CFX_WideString& wsUrl)
{
    const char arDigits[] = "0123456789ABCDEF";
    CFX_ByteString rUrl;
    int nLength = wsUrl.GetLength();
    for (int i = 0; i < nLength; i++) {
        FX_DWORD word = wsUrl.GetAt(i);
        if (word > 0x7F || url_encodeTable[word] == 1) {
            CFX_ByteString bsUri = CFX_ByteString::FromUnicode((FX_WORD)word);
            int nByte = bsUri.GetLength();
            for (int j = 0; j < nByte; j++) {
                rUrl += '%';
                FX_BYTE code = bsUri.GetAt(j);
                rUrl += arDigits[code >> 4];
                rUrl += arDigits[code & 0x0F];
            }
        } else {
            rUrl += CFX_ByteString::FromUnicode((FX_WORD)word);
        }
    }
    return rUrl;
}
CFX_WideString FX_UrlDecode(const CFX_ByteString& bsUrl)
{
    CFX_ByteString rUrl;
    int nLength = bsUrl.GetLength();
    for (int i = 0; i < nLength; i++) {
        if (i < nLength - 2 && bsUrl[i] == '%' && FX_IsXDigit(bsUrl[i + 1]) && FX_IsXDigit(bsUrl[i + 2])) {
            rUrl += (FX_HexToI(bsUrl[i + 1]) << 4 | FX_HexToI(bsUrl[i + 2]));
            i += 2;
        } else {
            rUrl += bsUrl[i];
        }
    }
    return CFX_WideString::FromLocal(rUrl);
}
CFX_ByteString FX_EncodeURI(const CFX_WideString& wsURI)
{
    const char arDigits[] = "0123456789ABCDEF";
    CFX_ByteString rURI;
    CFX_ByteString bsUri = wsURI.UTF8Encode();
    int nLength = bsUri.GetLength();
    for (int i = 0; i < nLength; i++) {
        FX_BYTE code = bsUri.GetAt(i);
        if (code > 0x7F || url_encodeTable[code] == 1) {
            rURI += '%';
            rURI += arDigits[code >> 4];
            rURI += arDigits[code & 0x0F];
        } else {
            rURI += code;
        }
    }
    return rURI;
}
CFX_WideString FX_DecodeURI(const CFX_ByteString& bsURI)
{
    CFX_ByteString rURI;
    int nLength = bsURI.GetLength();
    for (int i = 0; i < nLength; i++) {
        if (i < nLength - 2 && bsURI[i] == '%' && FX_IsXDigit(bsURI[i + 1]) && FX_IsXDigit(bsURI[i + 2])) {
            rURI += (FX_HexToI(bsURI[i + 1]) << 4 | FX_HexToI(bsURI[i + 2]));
            i += 2;
        } else {
            rURI += bsURI[i];
        }
    }
    return CFX_WideString::FromUTF8(rURI, rURI.GetLength());
}
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
class CFindFileData 
{
public:
    virtual ~CFindFileData() {}
    HANDLE				m_Handle;
    FX_BOOL				m_bEnd;
};
class CFindFileDataA : public CFindFileData
{
public:
    virtual ~CFindFileDataA() {}
    WIN32_FIND_DATAA	m_FindData;
};
class CFindFileDataW : public CFindFileData
{
public:
    virtual ~CFindFileDataW() {}
    WIN32_FIND_DATAW	m_FindData;
};
#endif
void* FX_OpenFolder(FX_LPCSTR path)
{
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#ifndef _WIN32_WCE
    CFindFileDataA* pData = new CFindFileDataA;
#ifdef _FX_WINAPI_PARTITION_DESKTOP_
    pData->m_Handle = FindFirstFileA(CFX_ByteString(path) + "/*.*", &pData->m_FindData);
#else
    pData->m_Handle = FindFirstFileExA(CFX_ByteString(path) + "/*.*", FindExInfoStandard, &pData->m_FindData, FindExSearchNameMatch, NULL, 0);
#endif
#else
    CFindFileDataW* pData = new CFindFileDataW;
    pData->m_Handle = FindFirstFileW(CFX_WideString::FromLocal(path) + L"/*.*", &pData->m_FindData);
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
void* FX_OpenFolder(FX_LPCWSTR path)
{
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    CFindFileDataW* pData = new CFindFileDataW;
#ifdef _FX_WINAPI_PARTITION_DESKTOP_
    pData->m_Handle = FindFirstFileW((CFX_WideString(path) + L"/*.*").c_str(), &pData->m_FindData);
#else
    pData->m_Handle = FindFirstFileExW((CFX_WideString(path) + L"/*.*").c_str(), FindExInfoStandard, &pData->m_FindData, FindExSearchNameMatch, NULL, 0);
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
FX_BOOL FX_GetNextFile(void* handle, CFX_ByteString& filename, FX_BOOL& bFolder)
{
    if (handle == NULL) {
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
    struct dirent *de = readdir((DIR*)handle);
    if (de == NULL) {
        return FALSE;
    }
    filename = de->d_name;
    bFolder = de->d_type == DT_DIR;
    return TRUE;
#endif
}
FX_BOOL FX_GetNextFile(void* handle, CFX_WideString& filename, FX_BOOL& bFolder)
{
    if (handle == NULL) {
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
    struct dirent *de = readdir((DIR*)handle);
    if (de == NULL) {
        return FALSE;
    }
    filename = CFX_WideString::FromLocal(de->d_name);
    bFolder = de->d_type == DT_DIR;
    return TRUE;
#endif
}
void FX_CloseFolder(void* handle)
{
    if (handle == NULL) {
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
FX_WCHAR FX_GetFolderSeparator()
{
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    return '\\';
#else
    return '/';
#endif
}

CFX_Matrix_3by3 CFX_Matrix_3by3::Inverse()
{
    FX_FLOAT det = a*(e*i - f*h) - b*(i*d - f*g) + c*(d*h - e*g);
    if (FXSYS_fabs(det) < 0.0000001)
        return CFX_Matrix_3by3();
    else
        return CFX_Matrix_3by3(
            (e*i - f*h) / det,
            -(b*i - c*h) / det,
            (b*f - c*e) / det,
            -(d*i - f*g) / det,
            (a*i - c*g) / det,
            -(a*f - c*d) / det,
            (d*h - e*g) / det,
            -(a*h - b*g) / det,
            (a*e - b*d) / det
        );
}

CFX_Matrix_3by3 CFX_Matrix_3by3::Multiply(const CFX_Matrix_3by3 &m)
{
    return CFX_Matrix_3by3(
        a*m.a + b*m.d + c*m.g,
        a*m.b + b*m.e + c*m.h,
        a*m.c + b*m.f + c*m.i,
        d*m.a + e*m.d + f*m.g,
        d*m.b + e*m.e + f*m.h,
        d*m.c + e*m.f + f*m.i,
        g*m.a + h*m.d + i*m.g,
        g*m.b + h*m.e + i*m.h,
        g*m.c + h*m.f + i*m.i
      );
}

CFX_Vector_3by1 CFX_Matrix_3by3::TransformVector(const CFX_Vector_3by1 &v)
{
    return CFX_Vector_3by1(
        a * v.a + b * v.b + c * v.c,
        d * v.a + e * v.b + f * v.c,
        g * v.a + h * v.b + i * v.c
    );
}
