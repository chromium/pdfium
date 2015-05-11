// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../public/fpdf_progressive.h"
#include "../../public/fpdfview.h"
#include "../include/fsdk_define.h"
#include "../include/fsdk_rendercontext.h"

extern void (*Func_RenderPage)( CRenderContext*, FPDF_PAGE page, int start_x, int start_y, int size_x, int size_y,
						int rotate, int flags,FX_BOOL bNeedToRestore, IFSDK_PAUSE_Adapter * pause );

extern void DropContext(void* data);

DLLEXPORT int STDCALL FPDF_RenderPageBitmap_Start( FPDF_BITMAP bitmap, FPDF_PAGE page, 
													int start_x, int start_y, int size_x,
												    int size_y, int rotate, int flags,
													IFSDK_PAUSE * pause )
{
	if (bitmap == NULL || page == NULL)
		return FPDF_RENDER_FAILED;

 	if (!pause)
 		return FPDF_RENDER_FAILED;

	if (pause->version !=1)
		return FPDF_RENDER_FAILED;

	CPDF_Page* pPage = (CPDF_Page*)page;
	CRenderContext* pContext = new CRenderContext;
	pPage->SetPrivateData((void*)1, pContext, DropContext);
#ifdef _SKIA_SUPPORT_
	pContext->m_pDevice = new CFX_SkiaDevice;
	if (flags & FPDF_REVERSE_BYTE_ORDER)
		((CFX_SkiaDevice*)pContext->m_pDevice)->Attach((CFX_DIBitmap*)bitmap,0,TRUE);
	else
		((CFX_SkiaDevice*)pContext->m_pDevice)->Attach((CFX_DIBitmap*)bitmap);
#else
	pContext->m_pDevice = new CFX_FxgeDevice;
	if (flags & FPDF_REVERSE_BYTE_ORDER)
		((CFX_FxgeDevice*)pContext->m_pDevice)->Attach((CFX_DIBitmap*)bitmap,0,TRUE);
	else
		((CFX_FxgeDevice*)pContext->m_pDevice)->Attach((CFX_DIBitmap*)bitmap);
#endif
	IFSDK_PAUSE_Adapter IPauseAdapter(pause);
	
	Func_RenderPage(pContext, page, start_x, start_y, size_x, size_y, rotate, flags,FALSE, &IPauseAdapter);

	if ( pContext->m_pRenderer )
	{
		CPDF_ProgressiveRenderer::RenderStatus status = CPDF_ProgressiveRenderer::Failed;
		status = pContext->m_pRenderer->GetStatus();
		return status;
	}
	return FPDF_RENDER_FAILED;
}

DLLEXPORT int STDCALL FPDF_RenderPage_Continue(FPDF_PAGE page,IFSDK_PAUSE * pause)
{
	if (page == NULL)
		return FPDF_RENDER_FAILED;

 	if (!pause)
		return FPDF_RENDER_FAILED;
	
	if (pause->version !=1)
		return FPDF_RENDER_FAILED;

	CPDF_Page* pPage = (CPDF_Page*)page;

//	FXMT_CSLOCK_OBJ(&pPage->m_PageLock);

	CRenderContext * pContext = (CRenderContext*)pPage->GetPrivateData((void*)1);
	if (pContext && pContext->m_pRenderer)
	{
		IFSDK_PAUSE_Adapter IPauseAdapter(pause);
		pContext->m_pRenderer->Continue(&IPauseAdapter);

		CPDF_ProgressiveRenderer::RenderStatus status = CPDF_ProgressiveRenderer::Failed;
		status = pContext->m_pRenderer->GetStatus();
		return status;
	}
	return FPDF_RENDER_FAILED;
}


DLLEXPORT void STDCALL FPDF_RenderPage_Close(FPDF_PAGE page)
{
	if (page == NULL) return;
	CPDF_Page* pPage = (CPDF_Page*)page;

//	FXMT_CSLOCK_OBJ(&pPage->m_PageLock);

	CRenderContext * pContext = (CRenderContext*)pPage->GetPrivateData((void*)1);
	if (pContext)
	{
		pContext->m_pDevice->RestoreState();
		delete pContext;
		pPage->RemovePrivateData((void*)1);
	}
}

