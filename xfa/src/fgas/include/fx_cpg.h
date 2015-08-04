// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_CODEPAGE
#define _FX_CODEPAGE
class IFX_CodePage;
#define FX_CODEPAGE_DefANSI 0
#define FX_CODEPAGE_DefOEM 1
#define FX_CODEPAGE_DefMAC 2
#define FX_CODEPAGE_Thread 3
#define FX_CODEPAGE_Symbol 42
#define FX_CODEPAGE_MSDOS_US 437
#define FX_CODEPAGE_Arabic_ASMO708 708
#define FX_CODEPAGE_Arabic_ASMO449Plus 709
#define FX_CODEPAGE_Arabic_Transparent 710
#define FX_CODEPAGE_Arabic_NafithaEnhanced 711
#define FX_CODEPAGE_Arabic_TransparentASMO 720
#define FX_CODEPAGE_MSDOS_Greek1 737
#define FX_CODEPAGE_MSDOS_Baltic 775
#define FX_CODEPAGE_MSWin31_WesternEuropean 819
#define FX_CODEPAGE_MSDOS_WesternEuropean 850
#define FX_CODEPAGE_MSDOS_EasternEuropean 852
#define FX_CODEPAGE_MSDOS_Latin3 853
#define FX_CODEPAGE_MSDOS_Cyrillic 855
#define FX_CODEPAGE_MSDOS_Turkish 857
#define FX_CODEPAGE_MSDOS_Latin1Euro 858
#define FX_CODEPAGE_MSDOS_Portuguese 860
#define FX_CODEPAGE_MSDOS_Icelandic 861
#define FX_CODEPAGE_MSDOS_Hebrew 862
#define FX_CODEPAGE_MSDOS_FrenchCanadian 863
#define FX_CODEPAGE_MSDOS_Arabic 864
#define FX_CODEPAGE_MSDOS_Norwegian 865
#define FX_CODEPAGE_MSDOS_Russian 866
#define FX_CODEPAGE_MSDOS_Greek2 869
#define FX_CODEPAGE_MSDOS_Thai 874
#define FX_CODEPAGE_MSDOS_KamenickyCS 895
#define FX_CODEPAGE_ShiftJIS 932
#define FX_CODEPAGE_ChineseSimplified 936
#define FX_CODEPAGE_Korean 949
#define FX_CODEPAGE_ChineseTraditional 950
#define FX_CODEPAGE_UTF16LE 1200
#define FX_CODEPAGE_UTF16BE 1201
#define FX_CODEPAGE_MSWin_EasternEuropean 1250
#define FX_CODEPAGE_MSWin_Cyrillic 1251
#define FX_CODEPAGE_MSWin_WesternEuropean 1252
#define FX_CODEPAGE_MSWin_Greek 1253
#define FX_CODEPAGE_MSWin_Turkish 1254
#define FX_CODEPAGE_MSWin_Hebrew 1255
#define FX_CODEPAGE_MSWin_Arabic 1256
#define FX_CODEPAGE_MSWin_Baltic 1257
#define FX_CODEPAGE_MSWin_Vietnamese 1258
#define FX_CODEPAGE_Johab 1361
#define FX_CODEPAGE_MAC_Roman 10000
#define FX_CODEPAGE_MAC_ShiftJIS 10001
#define FX_CODEPAGE_MAC_ChineseTraditional 10002
#define FX_CODEPAGE_MAC_Korean 10003
#define FX_CODEPAGE_MAC_Arabic 10004
#define FX_CODEPAGE_MAC_Hebrew 10005
#define FX_CODEPAGE_MAC_Greek 10006
#define FX_CODEPAGE_MAC_Cyrillic 10007
#define FX_CODEPAGE_MAC_ChineseSimplified 10008
#define FX_CODEPAGE_MAC_Thai 10021
#define FX_CODEPAGE_MAC_EasternEuropean 10029
#define FX_CODEPAGE_MAC_Turkish 10081
#define FX_CODEPAGE_UTF32LE 12000
#define FX_CODEPAGE_UTF32BE 12001
#define FX_CODEPAGE_ISO8859_1 28591
#define FX_CODEPAGE_ISO8859_2 28592
#define FX_CODEPAGE_ISO8859_3 28593
#define FX_CODEPAGE_ISO8859_4 28594
#define FX_CODEPAGE_ISO8859_5 28595
#define FX_CODEPAGE_ISO8859_6 28596
#define FX_CODEPAGE_ISO8859_7 28597
#define FX_CODEPAGE_ISO8859_8 28598
#define FX_CODEPAGE_ISO8859_9 28599
#define FX_CODEPAGE_ISO8859_10 28600
#define FX_CODEPAGE_ISO8859_11 28601
#define FX_CODEPAGE_ISO8859_12 28602
#define FX_CODEPAGE_ISO8859_13 28603
#define FX_CODEPAGE_ISO8859_14 28604
#define FX_CODEPAGE_ISO8859_15 28605
#define FX_CODEPAGE_ISO8859_16 28606
#define FX_CODEPAGE_ISCII_Devanagari 57002
#define FX_CODEPAGE_ISCII_Bengali 57003
#define FX_CODEPAGE_ISCII_Tamil 57004
#define FX_CODEPAGE_ISCII_Telugu 57005
#define FX_CODEPAGE_ISCII_Assamese 57006
#define FX_CODEPAGE_ISCII_Oriya 57007
#define FX_CODEPAGE_ISCII_Kannada 57008
#define FX_CODEPAGE_ISCII_Malayalam 57009
#define FX_CODEPAGE_ISCII_Gujarati 57010
#define FX_CODEPAGE_ISCII_Punjabi 57011
#define FX_CODEPAGE_UTF7 65000
#define FX_CODEPAGE_UTF8 65001
#define FX_CHARSET_ANSI 0
#define FX_CHARSET_Default 1
#define FX_CHARSET_Symbol 2
#define FX_CHARSET_MAC_Roman 77
#define FX_CHARSET_MAC_ShiftJIS 78
#define FX_CHARSET_MAC_Korean 79
#define FX_CHARSET_MAC_ChineseSimplified 80
#define FX_CHARSET_MAC_ChineseTriditional 81
#define FX_CHARSET_MAC_Johab 82
#define FX_CHARSET_MAC_Hebrew 83
#define FX_CHARSET_MAC_Arabic 84
#define FX_CHARSET_MAC_Greek 85
#define FX_CHARSET_MAC_Turkish 86
#define FX_CHARSET_MAC_Thai 87
#define FX_CHARSET_MAC_EasternEuropean 88
#define FX_CHARSET_MAC_Cyrillic 89
#define FX_CHARSET_ShiftJIS 128
#define FX_CHARSET_Korean 129
#define FX_CHARSET_Johab 130
#define FX_CHARSET_ChineseSimplified 134
#define FX_CHARSET_ChineseTriditional 136
#define FX_CHARSET_MSWin_Greek 161
#define FX_CHARSET_MSWin_Turkish 162
#define FX_CHARSET_MSWin_Vietnamese 163
#define FX_CHARSET_MSWin_Hebrew 177
#define FX_CHARSET_MSWin_Arabic 178
#define FX_CHARSET_ArabicTraditional 179
#define FX_CHARSET_ArabicUser 180
#define FX_CHARSET_HebrewUser 181
#define FX_CHARSET_MSWin_Baltic 186
#define FX_CHARSET_MSWin_Cyrillic 204
#define FX_CHARSET_Thai 222
#define FX_CHARSET_MSWin_EasterEuropean 238
#define FX_CHARSET_US 254
#define FX_CHARSET_OEM 255
FX_WORD FX_GetCodePageFromCharset(uint8_t charset);
FX_WORD FX_GetCharsetFromCodePage(FX_WORD codepage);
FX_WORD FX_GetCodePageFromStringA(const FX_CHAR* pStr, int32_t iLength);
FX_WORD FX_GetCodePageFormStringW(const FX_WCHAR* pStr, int32_t iLength);
FX_WORD FX_GetDefCodePageByLanguage(FX_WORD wLanguage);
void FX_SwapByteOrder(FX_WCHAR* pStr, int32_t iLength);
void FX_SwapByteOrderCopy(const FX_WCHAR* pSrc,
                          FX_WCHAR* pDst,
                          int32_t iLength);
void FX_UTF16ToWChar(void* pBuffer, int32_t iLength);
void FX_UTF16ToWCharCopy(const FX_WORD* pUTF16,
                         FX_WCHAR* pWChar,
                         int32_t iLength);
void FX_WCharToUTF16(void* pBuffer, int32_t iLength);
void FX_WCharToUTF16Copy(const FX_WCHAR* pWChar,
                         FX_WORD* pUTF16,
                         int32_t iLength);
int32_t FX_DecodeString(FX_WORD wCodePage,
                        const FX_CHAR* pSrc,
                        int32_t* pSrcLen,
                        FX_WCHAR* pDst,
                        int32_t* pDstLen,
                        FX_BOOL bErrBreak = FALSE);
int32_t FX_UTF8Decode(const FX_CHAR* pSrc,
                      int32_t* pSrcLen,
                      FX_WCHAR* pDst,
                      int32_t* pDstLen);
enum FX_CODESYSTEM {
  FX_MBCS = 0,
  FX_SBCS,
  FX_DBCS,
};
typedef struct _FX_CODEPAGE_HEADER {
  uint16_t uCPID;
  uint8_t uMinCharBytes;
  uint8_t uMaxCharBytes;
  FX_CODESYSTEM eCPType;
  FX_BOOL bHasLeadByte;
  FX_WCHAR wMinChar;
  FX_WCHAR wMaxChar;
  FX_WCHAR wDefChar;
  FX_WCHAR wMinUnicode;
  FX_WCHAR wMaxUnicode;
  FX_WCHAR wDefUnicode;
} FX_CODEPAGE_HEADER;
#define FX_CPMAPTYPE_Consecution 1
#define FX_CPMAPTYPE_Strict 2
#define FX_CPMAPTYPE_NoMapping 3
#define FX_CPMAPTYPE_Delta 4
typedef struct _FX_CPCU_MAPTABLE1 {
  uint16_t uMapType;
  uint16_t uUniocde;
} FX_CPCU_MAPTABLE1;
typedef struct _FX_CPCU_MAPTABLE2 {
  uint8_t uTrailByte;
  uint8_t uMapType;
  uint16_t uOffset;
} FX_CPCU_MAPTABLE2;
typedef struct _FX_CPCU_MAPINFO {
  FX_CPCU_MAPTABLE1* pMapTable1;
  FX_CPCU_MAPTABLE2* pMapTable2;
  const uint8_t* pMapData;
} FX_CPCU_MAPINFO;
typedef struct _FX_CPUC_MAPTABLE {
  uint16_t uStartUnicode;
  uint16_t uEndUnicode;
  uint16_t uMapType;
  uint16_t uOffset;
} FX_CPUC_MAPTABLE;
typedef struct _FX_CPUC_MAPINFO {
  uint32_t uMapCount;
  FX_CPUC_MAPTABLE* pMapTable;
  const uint8_t* pMapData;
} FX_CPUC_MAPINFO;
typedef struct _FX_CODEPAGE {
  FX_CODEPAGE_HEADER const* pCPHeader;
  FX_CPCU_MAPINFO const* pCPCUMapInfo;
  FX_CPUC_MAPINFO const* pCPUCMapInfo;
} FX_CODEPAGE, *FX_LPCODEPAGE;
typedef FX_CODEPAGE const* FX_LPCCODEPAGE;
typedef struct _FX_STR2CPHASH {
  uint32_t uHash;
  uint32_t uCodePage;
} FX_STR2CPHASH;
typedef struct _FX_CHARSET_MAP {
  uint16_t charset;
  uint16_t codepage;
} FX_CHARSET_MAP;
typedef struct _FX_LANG2CPMAP {
  FX_WORD wLanguage;
  FX_WORD wCodepage;
} FX_LANG2CPMAP;

class IFX_CodePage {
 public:
  static IFX_CodePage* Create(FX_WORD wCodePage);
  virtual ~IFX_CodePage() {}
  virtual void Release() = 0;
  virtual FX_WORD GetCodePageNumber() const = 0;
  virtual FX_CODESYSTEM GetCodeSystemType() const = 0;
  virtual FX_BOOL HasLeadByte() const = 0;
  virtual FX_BOOL IsLeadByte(uint8_t byte) const = 0;
  virtual int32_t GetMinBytesPerChar() const = 0;
  virtual int32_t GetMaxBytesPerChar() const = 0;
  virtual FX_WCHAR GetMinCharcode() const = 0;
  virtual FX_WCHAR GetMaxCharcode() const = 0;
  virtual FX_WCHAR GetDefCharcode() const = 0;
  virtual FX_WCHAR GetMinUnicode() const = 0;
  virtual FX_WCHAR GetMaxUnicode() const = 0;
  virtual FX_WCHAR GetDefUnicode() const = 0;
  virtual FX_BOOL IsValidCharcode(FX_WORD wCharcode) const = 0;
  virtual FX_WCHAR GetUnicode(FX_WORD wCharcode) const = 0;
  virtual FX_BOOL IsValidUnicode(FX_WCHAR wUnicode) const = 0;
  virtual FX_WORD GetCharcode(FX_WCHAR wUnicode) const = 0;
};
#endif
