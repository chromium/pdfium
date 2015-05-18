// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../../../include/fxge/fx_ge.h"
//#define _SKIA_SUPPORT_
#if defined(_SKIA_SUPPORT_)
#include "../../../include/fxcodec/fx_codec.h"


//#define _FOXIT_DEBUG_
//#define _FOXIT_BENCHMARK_

extern "C" {
	extern void FX_OUTPUT_LOG_FUNC(const char*, ...);
	extern int FX_GET_TICK_FUNC();
};

#ifdef _FOXIT_DEBUG_
#define FOXIT_DEBUG1(msg) FX_OUTPUT_LOG_FUNC(msg)
#define FOXIT_DEBUG2(msg,para) FX_OUTPUT_LOG_FUNC(msg,para)
#define FOXIT_DEBUG3(msg,para1,para2) FX_OUTPUT_LOG_FUNC(msg,para1,para2)
#define FOXIT_DEBUG4(msg,para1,para2,para3) FX_OUTPUT_LOG_FUNC(msg,para1,para2,para3)
#define FOXIT_DEBUG5(msg,para1,para2,para3,param4) FX_OUTPUT_LOG_FUNC(msg,para1,para2,para3,param4)
#else
#define FOXIT_DEBUG1(msg)
#define FOXIT_DEBUG2(msg,para)
#define FOXIT_DEBUG3(msg,para1,para2)
#define FOXIT_DEBUG4(msg,para1,para2,para3)
#define FOXIT_DEBUG5(msg,para1,para2,para3,param4)
#endif

#include "SkDashPathEffect.h"
#include "SkTLazy.h"
#include "SkScan.h"
#include "SkRasterClip.h"
#include "SkStroke.h"


#include "fx_skia_blitter_new.h"
#include "../agg/fx_agg_driver.h"
#include "fx_skia_device.h"
/// Run-length-encoded supersampling antialiased blitter.
class SuperBlitter_skia
{
public:
	static void DrawPath(const SkPath& srcPath, SkBlitter* blitter,  const SkRasterClip& rect, const SkPaint& origPaint);
};
FX_BOOL FxSkDrawTreatAsHairline(const SkPaint& paint, SkScalar* coverage) {
	if (SkPaint::kStroke_Style != paint.getStyle())
		return FALSE;
	FXSYS_assert(coverage);
	SkScalar strokeWidth = paint.getStrokeWidth();
	if (0 == strokeWidth) {
		*coverage = SK_Scalar1;
		return TRUE;
	}
	// if we get here, we need to try to fake a thick-stroke with a modulated
	// hairline
	if (!paint.isAntiAlias())
		return FALSE;
	if (strokeWidth <= SK_Scalar1) {
		*coverage = strokeWidth;
		return TRUE;
	}
	return FALSE;
}

void SuperBlitter_skia::DrawPath(const SkPath& srcPath, SkBlitter* blitter, const SkRasterClip& rect, const SkPaint& origPaint) 
{
	SkPath*		pathPtr = (SkPath*)&srcPath;
	bool		doFill = true;
	SkPath		tmpPath;
	SkTCopyOnFirstWrite<SkPaint> paint(origPaint);
	{
		SkScalar coverage;
		if (FxSkDrawTreatAsHairline(origPaint, &coverage)) {
			if (SK_Scalar1 == coverage) {
				paint.writable()->setStrokeWidth(0);
			} else if (1) {//xfermodeSupportsCoverageAsAlpha(xfer), we not use blend mode here, xfer aways NULL.
				U8CPU newAlpha;
				// this is the old technique, which we preserve for now so
				// we don't change previous results (testing)
				// the new way seems fine, its just (a tiny bit) different
				int scale = (int)SkScalarMul(coverage, 256);
				newAlpha = origPaint.getAlpha() * scale >> 8;
				SkPaint* writablePaint = paint.writable();
				writablePaint->setStrokeWidth(0);
				writablePaint->setAlpha(newAlpha);
			}
		}
	}
	if (paint->getPathEffect() || paint->getStyle() != SkPaint::kFill_Style) {
		SkIRect devBounds = rect.getBounds();
		// outset to have slop for antialasing and hairlines
		devBounds.outset(1, 1);
		SkRect cullRect = SkRect::Make(devBounds);
		doFill = paint->getFillPath(*pathPtr, &tmpPath, &cullRect);
		pathPtr = &tmpPath;
	}
	// avoid possibly allocating a new path in transform if we can
	SkPath* devPathPtr = pathPtr;
	void (*proc)(const SkPath&, const SkRasterClip&, SkBlitter*);
	if (doFill) {
		if (paint->isAntiAlias()) {
			proc = SkScan::AntiFillPath;
		} else {
			proc = SkScan::FillPath;
		}
	} else {	// hairline
		if (paint->isAntiAlias()) {
			proc = SkScan::AntiHairPath;
		} else {
			proc = SkScan::HairPath;
		}
	}
	proc(*devPathPtr, rect, blitter);
}

class CSkia_PathData 
{
public:
	CSkia_PathData() {}
	~CSkia_PathData() {}
	SkPath			m_PathData;

	void			BuildPath(const CFX_PathData* pPathData, const CFX_AffineMatrix* pObject2Device);
};

void CSkia_PathData::BuildPath(const CFX_PathData* pPathData, const CFX_AffineMatrix* pObject2Device)
{
	const CFX_PathData* pFPath = pPathData;
	int nPoints = pFPath->GetPointCount();
	FX_PATHPOINT* pPoints = pFPath->GetPoints();
	for (int i = 0; i < nPoints; i ++) {
		FX_FIXFLOAT x = pPoints[i].m_PointX, y = pPoints[i].m_PointY;
		if (pObject2Device) pObject2Device->Transform(x, y);
		int point_type = pPoints[i].m_Flag & FXPT_TYPE;
		if (point_type == FXPT_MOVETO) {
			m_PathData.moveTo(x, y);
		} else if (point_type == FXPT_LINETO) {
			if (pPoints[i-1].m_Flag == FXPT_MOVETO && (i == nPoints-1 || pPoints[i+1].m_Flag == FXPT_MOVETO) &&
					FXSYS_abs(pPoints[i].m_PointX - pPoints[i-1].m_PointX) < 0.4f && FXSYS_abs(pPoints[i].m_PointY - pPoints[i-1].m_PointY)< 0.4f)
				// PDF line includes the destination point, unlike Windows line.
				// We received some PDF which actually draws zero length lines. TESTDOC: summer cha show.pdf
				// Therefore, we have to extend the line by 0.4 pixel here.
				// But only for standalone segment. TESTDOC: bug #1434 - maze.pdf; TESTDOC: bug#1508 di704P_QIG_111.pdf 
				x += 0.4;
			// TODO: we should actually tell skia vertex generator to process zero length stroked line 
			// (only butts are drawn)
			m_PathData.lineTo(x, y);
		} else if (point_type == FXPT_BEZIERTO) {
			FX_FIXFLOAT x2 = pPoints[i+1].m_PointX, y2 = pPoints[i+1].m_PointY;
			FX_FIXFLOAT x3 = pPoints[i+2].m_PointX, y3 = pPoints[i+2].m_PointY;
			if (pObject2Device){
				pObject2Device->Transform(x2, y2);
				pObject2Device->Transform(x3, y3);
			}
			m_PathData.cubicTo(x, y, x2, y2, x3, y3);
			i += 2;
		}
		if (pPoints[i].m_Flag & FXPT_CLOSEFIGURE) m_PathData.close();
	}
}

// convert a stroking path to scanlines
static void SkRasterizeStroke(SkPaint& spaint, SkPath* dstPathData, SkPath& path_data, 
					 const CFX_AffineMatrix* pObject2Device,
					 const CFX_GraphStateData* pGraphState, FX_FIXFLOAT scale = FIX8_ONE, 
					 FX_BOOL bStrokeAdjust = FALSE, FX_BOOL bTextMode = FALSE)
{
	SkPaint::Cap cap;
	switch (pGraphState->m_LineCap) {
		case CFX_GraphStateData::LineCapRound:
			cap = SkPaint::kRound_Cap;
			break;
		case CFX_GraphStateData::LineCapSquare:
			cap = SkPaint::kSquare_Cap;
			break;
		default:
			cap = SkPaint::kButt_Cap;
			break;
	}
	SkPaint::Join join;
	switch (pGraphState->m_LineJoin) {
		case CFX_GraphStateData::LineJoinRound:
			join = SkPaint::kRound_Join;
			break;
		case CFX_GraphStateData::LineJoinBevel:
			join = SkPaint::kBevel_Join;
			break;
		default:
			join = SkPaint::kMiter_Join;
			break;
	}
	FX_FIXFLOAT width = pGraphState->m_LineWidth*scale;
	FX_FIXFLOAT unit = fix32_to_8(fixdiv_8_8_to_32(FIX8_ONE, (pObject2Device->GetXUnit() + pObject2Device->GetYUnit()) / 2));
	if (width <= unit) width = unit;

	if (pGraphState->m_DashArray == NULL) {
		SkStroke stroker;
		stroker.setCap(cap);
		stroker.setJoin(join);
		stroker.setMiterLimit(pGraphState->m_MiterLimit);
		stroker.setWidth(width);
		stroker.setDoFill(FALSE);
		stroker.strokePath(path_data, dstPathData);
		SkMatrix smatrix; 
		smatrix.setAll(pObject2Device->a, pObject2Device->c, pObject2Device->e, pObject2Device->b, pObject2Device->d, pObject2Device->f, 0, 0, 1);
		dstPathData->transform(smatrix);
	} else {
		int count = (pGraphState->m_DashCount+1)/2;
		SkScalar* intervals = FX_Alloc2D(SkScalar, count, sizeof(SkScalar));
		// Set dash pattern
		for (int i = 0; i < count; i ++) {
			FX_FIXFLOAT on = pGraphState->m_DashArray[i*2];
			if (on <= 0.000001f) on = FIX8_ONE/10;
			FX_FIXFLOAT off = i*2+1 == pGraphState->m_DashCount ? on :
					pGraphState->m_DashArray[i*2+1];
			if (off < 0) off = 0;		
			intervals[i*2]=on*scale;
			intervals[i*2+1]=off*scale;
		}
		SkDashPathEffect* pEffect = new SkDashPathEffect(intervals,count*2, pGraphState->m_DashPhase*scale);
		spaint.setPathEffect(pEffect)->unref();
		spaint.setStrokeWidth(width);
		spaint.setStrokeMiter(pGraphState->m_MiterLimit);
		spaint.setStrokeCap(cap);
		spaint.setStrokeJoin(join);
		spaint.getFillPath(path_data, dstPathData);
		SkMatrix smatrix; 
		smatrix.setAll(pObject2Device->a, pObject2Device->c, pObject2Device->e, pObject2Device->b, pObject2Device->d, pObject2Device->f, 0, 0, 1);
		dstPathData->transform(smatrix);
		FX_Free(intervals);
	}
}

CFX_SkiaDeviceDriver::CFX_SkiaDeviceDriver(CFX_DIBitmap* pBitmap, int dither_bits, FX_BOOL bRgbByteOrder, CFX_DIBitmap* pOriDevice, FX_BOOL bGroupKnockout)
{
	m_pAggDriver = new CFX_AggDeviceDriver(pBitmap, dither_bits, bRgbByteOrder, pOriDevice, bGroupKnockout);
}
CFX_SkiaDeviceDriver::~CFX_SkiaDeviceDriver()
{
	if (m_pAggDriver) delete m_pAggDriver;
}
FX_BOOL CFX_SkiaDeviceDriver::DrawDeviceText(int nChars, const FXTEXT_CHARPOS* pCharPos, CFX_Font* pFont,
		CFX_FontCache* pCache, const CFX_AffineMatrix* pObject2Device, FX_FIXFLOAT font_size, FX_DWORD color, 
		int alpha_flag, void* pIccTransform)
{
	return m_pAggDriver->DrawDeviceText(nChars, pCharPos, pFont,pCache, pObject2Device, font_size, color, 
		alpha_flag, pIccTransform);
}
int CFX_SkiaDeviceDriver::GetDeviceCaps(int caps_id)
{
	return m_pAggDriver->GetDeviceCaps(caps_id);
}
void CFX_SkiaDeviceDriver::SaveState()
{
	m_pAggDriver->SaveState();	
}

void CFX_SkiaDeviceDriver::RestoreState(FX_BOOL bKeepSaved)
{
	m_pAggDriver->RestoreState(bKeepSaved);
}
void CFX_SkiaDeviceDriver::SetClipMask(rasterizer_scanline_aa& rasterizer)
{
	m_pAggDriver->SetClipMask(rasterizer);
}
void CFX_SkiaDeviceDriver::SetClipMask(SkPath& skPath, SkPaint* spaint)
{	
	SkIRect clip_box;
	clip_box.set(0, 0, fix0_to_8(GetDeviceCaps(FXDC_PIXEL_WIDTH)), fix0_to_8(GetDeviceCaps(FXDC_PIXEL_HEIGHT)));
	clip_box.intersect(m_pAggDriver->m_pClipRgn->GetBox().left, m_pAggDriver->m_pClipRgn->GetBox().top,
		m_pAggDriver->m_pClipRgn->GetBox().right, m_pAggDriver->m_pClipRgn->GetBox().bottom);

	SkPath* pathPtr = &skPath;

	SkRect path_rect = skPath.getBounds();

	clip_box.intersect(FXSYS_floor(path_rect.fLeft), FXSYS_floor(path_rect.fTop), FXSYS_floor(path_rect.fRight)+1, FXSYS_floor(path_rect.fBottom)+1);
	CFX_DIBitmapRef mask;
	CFX_DIBitmap* pThisLayer = mask.New();
	pThisLayer->Create(clip_box.width(), clip_box.height(), FXDIB_8bppMask);
	pThisLayer->Clear(0);

	CFX_SkiaA8Renderer render;
	render.Init(pThisLayer, clip_box.fLeft, clip_box.fTop);

	SkRasterClip rasterClip(clip_box);
	SuperBlitter_skia::DrawPath(skPath, (SkBlitter*)&render, rasterClip, *spaint);
	
	// Finally, we have got the mask that we need, intersect with current clip region
	m_pAggDriver->m_pClipRgn->IntersectMaskF(clip_box.fLeft, clip_box.fTop, mask);

}
FX_BOOL CFX_SkiaDeviceDriver::SetClip_PathFill(const CFX_PathData* pPathData,	// path info
						const CFX_AffineMatrix* pObject2Device,	// optional transformation
						int fill_mode	// fill mode, WINDING or ALTERNATE
						)
{
	if (m_pAggDriver->m_pClipRgn == NULL)
		m_pAggDriver->m_pClipRgn = new CFX_ClipRgn(GetDeviceCaps(FXDC_PIXEL_WIDTH), GetDeviceCaps(FXDC_PIXEL_HEIGHT));

	if (pPathData->GetPointCount() == 5 || pPathData->GetPointCount() == 4) {
		CFX_FloatRect rectf;
		if (pPathData->IsRect(pObject2Device, &rectf)) {
			rectf.Intersect(CFX_FloatRect(0, 0, (FX_FIXFLOAT)GetDeviceCaps(FXDC_PIXEL_WIDTH), (FX_FIXFLOAT)GetDeviceCaps(FXDC_PIXEL_HEIGHT)));
			FX_RECT rect = rectf.GetOutterRect();
			m_pAggDriver->m_pClipRgn->IntersectRect(rect);
			return TRUE;
		}
	}
	CSkia_PathData path_data;
	path_data.BuildPath(pPathData, pObject2Device);
	path_data.m_PathData.close();
	path_data.m_PathData.setFillType((fill_mode&3) == FXFILL_WINDING? SkPath::kWinding_FillType:SkPath::kEvenOdd_FillType);

	SkPaint spaint;
	spaint.setColor(0xffffffff);
	spaint.setAntiAlias(TRUE);
	spaint.setStyle(SkPaint::kFill_Style);

	SetClipMask(path_data.m_PathData, &spaint);

	return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::SetClip_PathStroke(const CFX_PathData* pPathData,	// path info
						const CFX_AffineMatrix* pObject2Device,	// optional transformation
						const CFX_GraphStateData* pGraphState	// graphic state, for pen attributes
					)
{
	if (m_pAggDriver->m_pClipRgn == NULL)
		m_pAggDriver->m_pClipRgn = new CFX_ClipRgn(GetDeviceCaps(FXDC_PIXEL_WIDTH), GetDeviceCaps(FXDC_PIXEL_HEIGHT));

	// build path data
	CSkia_PathData path_data;
	path_data.BuildPath(pPathData, NULL);
	path_data.m_PathData.setFillType(SkPath::kWinding_FillType);

	SkPaint spaint;
	spaint.setColor(0xffffffff);
	spaint.setStyle(SkPaint::kStroke_Style);
	spaint.setAntiAlias(TRUE);

	SkPath dst_path;
	SkRasterizeStroke(spaint, &dst_path, path_data.m_PathData, pObject2Device, pGraphState, 1, FALSE, 0);
	spaint.setStyle(SkPaint::kFill_Style);
	SetClipMask(dst_path, &spaint);
	
	return TRUE;
}
FX_BOOL CFX_SkiaDeviceDriver::RenderRasterizer(rasterizer_scanline_aa& rasterizer, FX_DWORD color, FX_BOOL bFullCover, FX_BOOL bGroupKnockout,
											  int alpha_flag, void* pIccTransform)
{
	return m_pAggDriver->RenderRasterizer(rasterizer, color, bFullCover, bGroupKnockout,alpha_flag, pIccTransform);
}
FX_BOOL	CFX_SkiaDeviceDriver::RenderRasterizerSkia(SkPath& skPath, const SkPaint& origPaint, SkIRect& rect, FX_DWORD color, FX_BOOL bFullCover, FX_BOOL bGroupKnockout, 
							int alpha_flag, void* pIccTransform, FX_BOOL bFill)
{
	CFX_DIBitmap* pt = bGroupKnockout?m_pAggDriver->GetBackDrop():NULL;
	CFX_SkiaRenderer render;
	if (!render.Init(m_pAggDriver->m_pBitmap, pt, m_pAggDriver->m_pClipRgn, color, bFullCover, m_pAggDriver->m_bRgbByteOrder, alpha_flag, pIccTransform))
		return FALSE;
	
	SkRasterClip rasterClip(rect);
	SuperBlitter_skia::DrawPath(skPath, (SkBlitter*)&render,  rasterClip, origPaint);

	return TRUE;
	
}

FX_BOOL	CFX_SkiaDeviceDriver::DrawPath(const CFX_PathData* pPathData,	// path info
						const CFX_AffineMatrix* pObject2Device,	// optional transformation
						const CFX_GraphStateData* pGraphState,	// graphic state, for pen attributes
						FX_DWORD fill_color,			// fill color
						FX_DWORD stroke_color,			// stroke color
						int fill_mode,					// fill mode, WINDING or ALTERNATE. 0 for not filled
						int alpha_flag, 
						void* pIccTransform
						)
{
	if (GetBuffer() == NULL) return TRUE;
	FOXIT_DEBUG1("CFX_SkiaDeviceDriver::DrawPath: entering");
	SkIRect rect;
	rect.set(0, 0, GetDeviceCaps(FXDC_PIXEL_WIDTH), GetDeviceCaps(FXDC_PIXEL_HEIGHT));
	if ((fill_mode & 3) && fill_color) {
		// We have to transform before building path data, otherwise we'll have flatting problem
		// when we enlarge a small path (flatten before transformed)
		// TESTDOC: Bug #5115 - DS_S1Dimpact_lr.pdf 
		// build path data
		CSkia_PathData path_data;
		path_data.BuildPath(pPathData, pObject2Device);
		//path_data.m_PathData.close();
		path_data.m_PathData.setFillType((fill_mode&3) == FXFILL_WINDING? SkPath::kWinding_FillType:SkPath::kEvenOdd_FillType);

		SkPaint spaint;
		spaint.setAntiAlias(TRUE);
		spaint.setStyle(SkPaint::kFill_Style);
		spaint.setColor(fill_color);
		if (!RenderRasterizerSkia(path_data.m_PathData, spaint, rect, fill_color, fill_mode & FXFILL_FULLCOVER, FALSE, alpha_flag, pIccTransform))
			return FALSE;	
	}

	int stroke_alpha = FXGETFLAG_COLORTYPE(alpha_flag) ? FXGETFLAG_ALPHA_STROKE(alpha_flag) : FXARGB_A(stroke_color);

	if (pGraphState && stroke_alpha) {
		// We split the matrix into two parts: first part doing the scaling, so we won't have the
		// flatness problem, second part doing the transformation, so we don't have stroking geo problem.
		// TESTDOC: Bug #5253 - test[1].pdf 
		CFX_AffineMatrix matrix1, matrix2;
		if (pObject2Device) {
			matrix1.a = FXSYS_fabs(pObject2Device->a) > FXSYS_fabs(pObject2Device->b) ?
					FXSYS_fabs(pObject2Device->a) : FXSYS_fabs(pObject2Device->b);
			matrix1.d = matrix1.a;//FXSYS_fabs(pObject2Device->c) > FXSYS_fabs(pObject2Device->d) ?
					//pObject2Device->c : pObject2Device->d;
			matrix2.Set(pObject2Device->a/matrix1.a, pObject2Device->b/matrix1.a, 
					pObject2Device->c/matrix1.d, pObject2Device->d/matrix1.d, 
					pObject2Device->e, pObject2Device->f);
		}
		// build path data
		CSkia_PathData path_data;
		path_data.BuildPath(pPathData, &matrix1);
		path_data.m_PathData.setFillType(SkPath::kWinding_FillType);

		SkPaint spaint;
		spaint.setColor(stroke_color);
		spaint.setStyle(SkPaint::kStroke_Style);
		spaint.setAntiAlias(TRUE);
		SkPath dst_path;
		SkRasterizeStroke(spaint, &dst_path, path_data.m_PathData, &matrix2, pGraphState, matrix1.a, FALSE, 0);
		spaint.setStyle(SkPaint::kFill_Style);
		int fill_flag = FXGETFLAG_COLORTYPE(alpha_flag)<<8 | FXGETFLAG_ALPHA_STROKE(alpha_flag);
		
		if (!RenderRasterizerSkia(dst_path, spaint, rect, stroke_color, fill_mode & FXFILL_FULLCOVER, FALSE, fill_flag, pIccTransform, FALSE))
			return FALSE;
			
	}
	
	return TRUE;
}

FX_BOOL CFX_SkiaDeviceDriver::SetPixel(int x, int y, FX_DWORD color,
						int alpha_flag, void* pIccTransform)
{
	return m_pAggDriver->SetPixel(x, y, color, alpha_flag, pIccTransform);
}

FX_BOOL CFX_SkiaDeviceDriver::FillRect(const FX_RECT* pRect, FX_DWORD fill_color, int alpha_flag, void* pIccTransform)
{
	return m_pAggDriver->FillRect(pRect, fill_color, alpha_flag, pIccTransform);
}

FX_BOOL CFX_SkiaDeviceDriver::GetClipBox(FX_RECT* pRect)
{
	return m_pAggDriver->GetClipBox(pRect);
}

FX_BOOL	CFX_SkiaDeviceDriver::GetDIBits(CFX_DIBitmap* pBitmap, int left, int top, void* pIccTransform, FX_BOOL bDEdge)
{
	return m_pAggDriver->GetDIBits(pBitmap, left, top, pIccTransform, bDEdge);
}

FX_BOOL	CFX_SkiaDeviceDriver::SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD argb, const FX_RECT* pSrcRect, int left, int top, int blend_type, 
									   int alpha_flag, void* pIccTransform)
{
	return m_pAggDriver->SetDIBits(pBitmap, argb, pSrcRect, left, top, blend_type, alpha_flag, pIccTransform);
}

FX_BOOL	CFX_SkiaDeviceDriver::StretchDIBits(const CFX_DIBSource* pSource, FX_DWORD argb, int dest_left, int dest_top, 
							int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags, 
							int alpha_flag, void* pIccTransform)
{
	return m_pAggDriver->StretchDIBits(pSource, argb, dest_left, dest_top, 
							dest_width, dest_height, pClipRect, flags, 
							alpha_flag, pIccTransform);
}

FX_BOOL	CFX_SkiaDeviceDriver::StartDIBits(const CFX_DIBSource* pSource, int bitmap_alpha, FX_DWORD argb, 
						const CFX_AffineMatrix* pMatrix, FX_DWORD render_flags, FX_LPVOID& handle, 
						int alpha_flag, void* pIccTransform)
{
	return m_pAggDriver->StartDIBits(pSource, bitmap_alpha, argb, 
						pMatrix, render_flags, handle, alpha_flag, pIccTransform);
}

FX_BOOL	CFX_SkiaDeviceDriver::ContinueDIBits(FX_LPVOID pHandle, IFX_Pause* pPause)
{
	return m_pAggDriver->ContinueDIBits(pHandle, pPause);
}

void CFX_SkiaDeviceDriver::CancelDIBits(FX_LPVOID pHandle)
{
	m_pAggDriver->CancelDIBits(pHandle);
}

CFX_SkiaDevice::CFX_SkiaDevice()
{
	m_bOwnedBitmap = FALSE;
}

FX_BOOL CFX_SkiaDevice::Attach(CFX_DIBitmap* pBitmap, int dither_bits, FX_BOOL bRgbByteOrder, CFX_DIBitmap* pOriDevice, FX_BOOL bGroupKnockout)
{
	if (pBitmap == NULL) 
		return FALSE;
	SetBitmap(pBitmap);
	CFX_SkiaDeviceDriver* pDriver = new CFX_SkiaDeviceDriver(pBitmap, dither_bits, bRgbByteOrder, pOriDevice, bGroupKnockout);
	SetDeviceDriver(pDriver);
	return TRUE;
}

FX_BOOL CFX_SkiaDevice::Create(int width, int height, FXDIB_Format format, int dither_bits, CFX_DIBitmap* pOriDevice)
{
	m_bOwnedBitmap = TRUE;
	CFX_DIBitmap* pBitmap = new CFX_DIBitmap;
	if (!pBitmap->Create(width, height, format)) {
		delete pBitmap;
		return FALSE;
	} 
	SetBitmap(pBitmap);
	CFX_SkiaDeviceDriver* pDriver =  new CFX_SkiaDeviceDriver(pBitmap, dither_bits, FALSE, pOriDevice, FALSE);
	SetDeviceDriver(pDriver);
	return TRUE;
}
CFX_SkiaDevice::~CFX_SkiaDevice()
{
	if (m_bOwnedBitmap && GetBitmap()) delete GetBitmap();
}

#endif
