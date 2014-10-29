// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_ARABIC_IMP
#define _FX_ARABIC_IMP
typedef struct _FX_ARABICCHARRANGE {
    FX_WCHAR wStart;
    FX_WCHAR wEnd;
} FX_ARABICCHARRANGE;
class CFX_ArabicChar : public IFX_ArabicChar, public CFX_Object
{
public:
    virtual void		Release()
    {
        delete this;
    }
    virtual FX_BOOL		IsArabicChar(FX_WCHAR wch) const;
    virtual FX_BOOL		IsArabicFormChar(FX_WCHAR wch) const;

    virtual FX_WCHAR	GetFormChar(FX_WCHAR wch, FX_WCHAR prev = 0, FX_WCHAR next = 0) const;
    virtual FX_WCHAR	GetFormChar(const CFX_Char *cur, const CFX_Char *prev, const CFX_Char *next) const;
protected:
    FX_LPCARBFORMTABLE ParseChar(const CFX_Char *pTC, FX_WCHAR &wChar, FX_CHARTYPE &eType) const;
};
void FX_BidiReverseString(CFX_WideString &wsText, FX_INT32 iStart, FX_INT32 iCount);
void FX_BidiSetDeferredRun(CFX_Int32Array &values, FX_INT32 iStart, FX_INT32 iCount, FX_INT32 iValue);
#define FX_BCON		FX_BIDICLASS_ON
#define FX_BCL		FX_BIDICLASS_L
#define FX_BCR		FX_BIDICLASS_R
#define FX_BCAN		FX_BIDICLASS_AN
#define FX_BCEN		FX_BIDICLASS_EN
#define FX_BCAL		FX_BIDICLASS_AL
#define FX_BCNSM	FX_BIDICLASS_NSM
#define FX_BCCS		FX_BIDICLASS_CS
#define FX_BCES		FX_BIDICLASS_ES
#define FX_BCET		FX_BIDICLASS_ET
#define FX_BCBN		FX_BIDICLASS_BN
#define FX_BCS		FX_BIDICLASS_S
#define FX_BCWS		FX_BIDICLASS_WS
#define FX_BCB		FX_BIDICLASS_B
#define FX_BCRLO	FX_BIDICLASS_RLO
#define FX_BCRLE	FX_BIDICLASS_RLE
#define FX_BCLRO	FX_BIDICLASS_LRO
#define FX_BCLRE	FX_BIDICLASS_LRE
#define FX_BCPDF	FX_BIDICLASS_PDF
#define FX_BCN		FX_BIDICLASS_N
void FX_BidiClassify(const CFX_WideString &wsText, CFX_Int32Array &classes, FX_BOOL bWS = FALSE);
#define FX_BIDIMAXLEVEL			61
#define FX_BidiGreaterEven(a)	(FX_IsOdd(a) ? ((a) + 1) : ((a) + 2))
#define FX_BidiGreaterOdd(a)	(FX_IsOdd(a) ? ((a) + 2) : ((a) + 1))
FX_INT32 FX_BidiResolveExplicit(FX_INT32 iBaseLevel, FX_INT32 iDirection, CFX_Int32Array &classes, CFX_Int32Array &levels, FX_INT32 iStart, FX_INT32 iCount, FX_INT32 iNest = 0);
#define FX_BidiDirection(a)	(FX_IsOdd(a) ? FX_BIDICLASS_R : FX_BIDICLASS_L)
enum FX_BIDIWEAKSTATE {
    FX_BIDIWEAKSTATE_xa = 0,
    FX_BIDIWEAKSTATE_xr,
    FX_BIDIWEAKSTATE_xl,
    FX_BIDIWEAKSTATE_ao,
    FX_BIDIWEAKSTATE_ro,
    FX_BIDIWEAKSTATE_lo,
    FX_BIDIWEAKSTATE_rt,
    FX_BIDIWEAKSTATE_lt,
    FX_BIDIWEAKSTATE_cn,
    FX_BIDIWEAKSTATE_ra,
    FX_BIDIWEAKSTATE_re,
    FX_BIDIWEAKSTATE_la,
    FX_BIDIWEAKSTATE_le,
    FX_BIDIWEAKSTATE_ac,
    FX_BIDIWEAKSTATE_rc,
    FX_BIDIWEAKSTATE_rs,
    FX_BIDIWEAKSTATE_lc,
    FX_BIDIWEAKSTATE_ls,
    FX_BIDIWEAKSTATE_ret,
    FX_BIDIWEAKSTATE_let,
};
#define FX_BWSxa	FX_BIDIWEAKSTATE_xa
#define FX_BWSxr	FX_BIDIWEAKSTATE_xr
#define FX_BWSxl	FX_BIDIWEAKSTATE_xl
#define FX_BWSao	FX_BIDIWEAKSTATE_ao
#define FX_BWSro	FX_BIDIWEAKSTATE_ro
#define FX_BWSlo	FX_BIDIWEAKSTATE_lo
#define FX_BWSrt	FX_BIDIWEAKSTATE_rt
#define FX_BWSlt	FX_BIDIWEAKSTATE_lt
#define FX_BWScn	FX_BIDIWEAKSTATE_cn
#define FX_BWSra	FX_BIDIWEAKSTATE_ra
#define FX_BWSre	FX_BIDIWEAKSTATE_re
#define FX_BWSla	FX_BIDIWEAKSTATE_la
#define FX_BWSle	FX_BIDIWEAKSTATE_le
#define FX_BWSac	FX_BIDIWEAKSTATE_ac
#define FX_BWSrc	FX_BIDIWEAKSTATE_rc
#define FX_BWSrs	FX_BIDIWEAKSTATE_rs
#define FX_BWSlc	FX_BIDIWEAKSTATE_lc
#define FX_BWSls	FX_BIDIWEAKSTATE_ls
#define FX_BWSret	FX_BIDIWEAKSTATE_ret
#define FX_BWSlet	FX_BIDIWEAKSTATE_let
enum FX_BIDIWEAKACTION {
    FX_BIDIWEAKACTION_IX = 0x100,
    FX_BIDIWEAKACTION_XX = 0x0F,
    FX_BIDIWEAKACTION_xxx = (0x0F << 4) + 0x0F,
    FX_BIDIWEAKACTION_xIx = 0x100 + FX_BIDIWEAKACTION_xxx,
    FX_BIDIWEAKACTION_xxN = (0x0F << 4) + FX_BIDICLASS_ON,
    FX_BIDIWEAKACTION_xxE = (0x0F << 4) + FX_BIDICLASS_EN,
    FX_BIDIWEAKACTION_xxA = (0x0F << 4) + FX_BIDICLASS_AN,
    FX_BIDIWEAKACTION_xxR = (0x0F << 4) + FX_BIDICLASS_R,
    FX_BIDIWEAKACTION_xxL = (0x0F << 4) + FX_BIDICLASS_L,
    FX_BIDIWEAKACTION_Nxx = (FX_BIDICLASS_ON << 4) + 0x0F,
    FX_BIDIWEAKACTION_Axx = (FX_BIDICLASS_AN << 4) + 0x0F,
    FX_BIDIWEAKACTION_ExE = (FX_BIDICLASS_EN << 4) + FX_BIDICLASS_EN,
    FX_BIDIWEAKACTION_NIx = (FX_BIDICLASS_ON << 4) + 0x0F + 0x100,
    FX_BIDIWEAKACTION_NxN = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_ON,
    FX_BIDIWEAKACTION_NxR = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_R,
    FX_BIDIWEAKACTION_NxE = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_EN,
    FX_BIDIWEAKACTION_AxA = (FX_BIDICLASS_AN << 4) + FX_BIDICLASS_AN,
    FX_BIDIWEAKACTION_NxL = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_L,
    FX_BIDIWEAKACTION_LxL = (FX_BIDICLASS_L << 4) + FX_BIDICLASS_L,
    FX_BIDIWEAKACTION_xIL = (0x0F << 4) + FX_BIDICLASS_L + 0x100,
    FX_BIDIWEAKACTION_AxR = (FX_BIDICLASS_AN << 4) + FX_BIDICLASS_R,
    FX_BIDIWEAKACTION_Lxx = (FX_BIDICLASS_L << 4) + 0x0F,
};
#define FX_BWAIX	FX_BIDIWEAKACTION_IX
#define FX_BWAXX	FX_BIDIWEAKACTION_XX
#define FX_BWAxxx	FX_BIDIWEAKACTION_xxx
#define FX_BWAxIx	FX_BIDIWEAKACTION_xIx
#define FX_BWAxxN	FX_BIDIWEAKACTION_xxN
#define FX_BWAxxE	FX_BIDIWEAKACTION_xxE
#define FX_BWAxxA	FX_BIDIWEAKACTION_xxA
#define FX_BWAxxR	FX_BIDIWEAKACTION_xxR
#define FX_BWAxxL	FX_BIDIWEAKACTION_xxL
#define FX_BWANxx	FX_BIDIWEAKACTION_Nxx
#define FX_BWAAxx	FX_BIDIWEAKACTION_Axx
#define FX_BWAExE	FX_BIDIWEAKACTION_ExE
#define FX_BWANIx	FX_BIDIWEAKACTION_NIx
#define FX_BWANxN	FX_BIDIWEAKACTION_NxN
#define FX_BWANxR	FX_BIDIWEAKACTION_NxR
#define FX_BWANxE	FX_BIDIWEAKACTION_NxE
#define FX_BWAAxA	FX_BIDIWEAKACTION_AxA
#define FX_BWANxL	FX_BIDIWEAKACTION_NxL
#define FX_BWALxL	FX_BIDIWEAKACTION_LxL
#define FX_BWAxIL	FX_BIDIWEAKACTION_xIL
#define FX_BWAAxR	FX_BIDIWEAKACTION_AxR
#define FX_BWALxx	FX_BIDIWEAKACTION_Lxx
#define FX_BidiGetDeferredType(a)	(((a) >> 4) & 0x0F)
#define FX_BidiGetResolvedType(a)	((a) & 0x0F)
void FX_BidiResolveWeak(FX_INT32 iBaseLevel, CFX_Int32Array &classes, CFX_Int32Array &levels);
enum FX_BIDINEUTRALSTATE {
    FX_BIDINEUTRALSTATE_r = 0,
    FX_BIDINEUTRALSTATE_l,
    FX_BIDINEUTRALSTATE_rn,
    FX_BIDINEUTRALSTATE_ln,
    FX_BIDINEUTRALSTATE_a,
    FX_BIDINEUTRALSTATE_na,
};
#define FX_BNSr		FX_BIDINEUTRALSTATE_r
#define FX_BNSl		FX_BIDINEUTRALSTATE_l
#define FX_BNSrn	FX_BIDINEUTRALSTATE_rn
#define FX_BNSln	FX_BIDINEUTRALSTATE_ln
#define FX_BNSa		FX_BIDINEUTRALSTATE_a
#define FX_BNSna	FX_BIDINEUTRALSTATE_na
enum FX_BIDINEUTRALACTION {
    FX_BIDINEUTRALACTION_nL = FX_BIDICLASS_L,
    FX_BIDINEUTRALACTION_En = (FX_BIDICLASS_AN << 4),
    FX_BIDINEUTRALACTION_Rn = (FX_BIDICLASS_R << 4),
    FX_BIDINEUTRALACTION_Ln = (FX_BIDICLASS_L << 4),
    FX_BIDINEUTRALACTION_In = FX_BIDIWEAKACTION_IX,
    FX_BIDINEUTRALACTION_LnL = (FX_BIDICLASS_L << 4) + FX_BIDICLASS_L,
};
#define FX_BNAnL	FX_BIDINEUTRALACTION_nL
#define FX_BNAEn	FX_BIDINEUTRALACTION_En
#define FX_BNARn	FX_BIDINEUTRALACTION_Rn
#define FX_BNALn	FX_BIDINEUTRALACTION_Ln
#define FX_BNAIn	FX_BIDINEUTRALACTION_In
#define FX_BNALnL	FX_BIDINEUTRALACTION_LnL
FX_INT32 FX_BidiGetDeferredNeutrals(FX_INT32 iAction, FX_INT32 iLevel);
FX_INT32 FX_BidiGetResolvedNeutrals(FX_INT32 iAction);
void FX_BidiResolveNeutrals(FX_INT32 iBaseLevel, CFX_Int32Array &classes, const CFX_Int32Array &levels);
void FX_BidiResolveImplicit(const CFX_Int32Array &classes, CFX_Int32Array &levels);
void FX_BidiResolveWhitespace(FX_INT32 iBaseLevel, const CFX_Int32Array &classes, CFX_Int32Array &levels);
FX_INT32 FX_BidiReorderLevel(FX_INT32 iBaseLevel, CFX_WideString &wsText, const CFX_Int32Array &levels, FX_INT32 iStart, FX_BOOL bReverse = FALSE);
void FX_BidiReorder(FX_INT32 iBaseLevel, CFX_WideString &wsText, const CFX_Int32Array &levels);
class CFX_BidiChar : public IFX_BidiChar, public CFX_Object
{
public:
    CFX_BidiChar();
    virtual void		Release() FX_OVERRIDE
    {
        delete this;
    }
    virtual void		SetPolicy(FX_BOOL bSeparateNeutral = TRUE) FX_OVERRIDE
    {
        m_bSeparateNeutral = bSeparateNeutral;
    }
    virtual FX_BOOL		AppendChar(FX_WCHAR wch) FX_OVERRIDE;
    virtual FX_BOOL		EndChar() FX_OVERRIDE;
    virtual FX_INT32	GetBidiInfo(FX_INT32 &iStart, FX_INT32 &iCount) FX_OVERRIDE;
    virtual void		Reset() FX_OVERRIDE;
protected:
    FX_BOOL		m_bSeparateNeutral;
    FX_INT32	m_iCurStart;
    FX_INT32	m_iCurCount;
    FX_INT32	m_iCurBidi;
    FX_INT32	m_iLastBidi;
    FX_INT32	m_iLastStart;
    FX_INT32	m_iLastCount;
};
#endif
