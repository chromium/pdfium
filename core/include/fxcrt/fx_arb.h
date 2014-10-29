// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_ARABIC_
#define _FX_ARABIC_
class IFX_ArabicChar;
class IFX_BidiChar;
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct _FX_ARBFORMTABLE {
    FX_WCHAR	wIsolated;
    FX_WCHAR	wFinal;
    FX_WCHAR	wInitial;
    FX_WCHAR	wMedial;
} FX_ARBFORMTABLE, * FX_LPARBFORMTABLE;
typedef FX_ARBFORMTABLE const * FX_LPCARBFORMTABLE;
typedef struct _FX_ARAALEF {
    FX_WCHAR	wAlef;
    FX_WCHAR	wIsolated;
} FX_ARAALEF, * FX_LPARAALEF;
typedef FX_ARAALEF const * FX_LPCARAALEF;
typedef struct _FX_ARASHADDA {
    FX_WCHAR	wShadda;
    FX_WCHAR	wIsolated;
} FX_ARASHADDA, * FX_LPARASHADDA;
typedef FX_ARASHADDA const * FX_LPCARASHADDA;
FX_LPCARBFORMTABLE FX_GetArabicFormTable(FX_WCHAR unicode);
FX_WCHAR FX_GetArabicFromAlefTable(FX_WCHAR alef);
FX_WCHAR FX_GetArabicFromShaddaTable(FX_WCHAR shadda);
#ifdef __cplusplus
};
#endif
enum FX_ARBPOSITION {
    FX_ARBPOSITION_Isolated = 0,
    FX_ARBPOSITION_Final,
    FX_ARBPOSITION_Initial,
    FX_ARBPOSITION_Medial,
};
class IFX_ArabicChar
{
public:
    static IFX_ArabicChar*		Create();
    virtual void				Release() = 0;
    virtual FX_BOOL				IsArabicChar(FX_WCHAR wch) const = 0;
    virtual FX_BOOL				IsArabicFormChar(FX_WCHAR wch) const = 0;
    virtual FX_WCHAR			GetFormChar(FX_WCHAR wch, FX_WCHAR prev = 0, FX_WCHAR next = 0) const = 0;
    virtual FX_WCHAR			GetFormChar(const CFX_Char *cur, const CFX_Char *prev, const CFX_Char *next) const = 0;
};
void FX_BidiLine(CFX_WideString &wsText, FX_INT32 iBaseLevel = 0);
void FX_BidiLine(CFX_TxtCharArray &chars, FX_INT32 iCount, FX_INT32 iBaseLevel = 0);
void FX_BidiLine(CFX_RTFCharArray &chars, FX_INT32 iCount, FX_INT32 iBaseLevel = 0);
class IFX_BidiChar
{
public:
    static IFX_BidiChar*	Create();
    virtual void			Release() = 0;
    virtual void			SetPolicy(FX_BOOL bSeparateNeutral = TRUE) = 0;
    virtual FX_BOOL			AppendChar(FX_WCHAR wch) = 0;
    virtual FX_BOOL			EndChar() = 0;
    virtual FX_INT32		GetBidiInfo(FX_INT32 &iStart, FX_INT32 &iCount) = 0;
    virtual void			Reset() = 0;
};
#endif
