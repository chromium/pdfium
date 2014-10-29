// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_PEN
#define _FDE_PEN
class IFDE_Pen;
#define FDE_PENTYPE_Unknown				FDE_BRUSHTYPE_Unknown
#define FDE_PENTYPE_SolidColor			FDE_BRUSHTYPE_Solid
#define FDE_PENTYPE_HatchBrush			FDE_BRUSHTYPE_Hatch
#define FDE_PENTYPE_TextureBrush		FDE_BRUSHTYPE_Texture
#define FDE_PENTYPE_LinearGradient		FDE_BRUSHTYPE_LinearGradient
#define FDE_PENTYPE_MAX					FDE_BRUSHTYPE_MAX
#define FDE_DASHSTYLE_Solid			0
#define FDE_DASHSTYLE_Dash			1
#define FDE_DASHSTYLE_Dot			2
#define FDE_DASHSTYLE_DashDot		3
#define FDE_DASHSTYLE_DashDotDot	4
#define FDE_DASHSTYLE_Customized	5
#define FDE_LINEJOIN_Miter			0
#define FDE_LINEJOIN_Round			1
#define FDE_LINEJOIN_Bevel			2
#define FDE_LINECAP_Flat			0
#define FDE_LINECAP_Round			1
#define FDE_LINECAP_Square			2
typedef struct _FDE_COMPOUNDPATTERN {
    FX_FLOAT	pos;
    FX_FLOAT	width;
} FDE_COMPOUNDPATTERN, * FDE_LPCOMPOUNDPATTERN;
typedef FDE_COMPOUNDPATTERN const *				FDE_LPCCOMPOUNDPATTERN;
typedef CFX_ArrayTemplate<FDE_COMPOUNDPATTERN>	CFDE_CompoundPatterns;
class IFDE_Pen
{
public:
    static IFDE_Pen*			Create();
    virtual void				Release() = 0;
    virtual FX_INT32			GetType() const = 0;
    virtual FX_ARGB				GetColor() const = 0;
    virtual void				SetColor(FX_ARGB color) = 0;
    virtual IFDE_Brush*			GetBrush() const = 0;
    virtual void				SetBrush(IFDE_Brush *pBrush, FX_BOOL bAutoRelease) = 0;
    virtual FX_INT32			GetLineCap() const = 0;
    virtual void				SetLineCap(FX_INT32 iLineCap) = 0;
    virtual FX_INT32			GetDashStyle() const = 0;
    virtual void				SetDashStyle(FX_INT32 iDashStyle) = 0;
    virtual FX_FLOAT			GetDashPhase() const = 0;
    virtual void				SetDashPhase(FX_FLOAT fPhase) = 0;
    virtual FX_INT32			CountDashArray() const = 0;
    virtual FX_INT32			GetDashArray(CFX_FloatArray &dashArray) const = 0;
    virtual void				SetDashArray(const CFX_FloatArray &dashArray) = 0;
    virtual FX_INT32			GetLineJoin() const = 0;
    virtual void				SetLineJoin(FX_INT32 iLineJoin) = 0;
    virtual FX_FLOAT			GetMiterLimit() const = 0;
    virtual void				SetMiterLimit(FX_FLOAT fMiterLimit) = 0;
    virtual FX_INT32			CountCompoundPatterns() const = 0;
    virtual FX_BOOL				GetCompoundPatterns(CFDE_CompoundPatterns &compoundPatterns) const = 0;
    virtual FX_BOOL				SetCompoundPatterns(const CFDE_CompoundPatterns &compoundPatterns) = 0;
};
#endif
