// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FSDK_RENDERCONTEXT_H_
#define FPDFSDK_INCLUDE_FSDK_RENDERCONTEXT_H_

#include "../../public/fpdf_progressive.h"
#include "fsdk_define.h"

// Everything about rendering is put here: for OOM recovery
class CRenderContext 
{
public:
	CRenderContext() { Clear(); }
	~CRenderContext();
	
	void Clear();
	
	CFX_RenderDevice*		m_pDevice;
	CPDF_RenderContext*		m_pContext;
	CPDF_ProgressiveRenderer*	m_pRenderer;
	CPDF_AnnotList*			m_pAnnots;
	CPDF_RenderOptions*		m_pOptions;
#ifdef _WIN32_WCE
	CFX_DIBitmap*	m_pBitmap;
	HBITMAP			m_hBitmap;
#endif
};

class IFSDK_PAUSE_Adapter : public IFX_Pause
{
public:
	IFSDK_PAUSE_Adapter(IFSDK_PAUSE* IPause );
	FX_BOOL NeedToPauseNow();
	
private:
	IFSDK_PAUSE* m_IPause;
};

#endif  // FPDFSDK_INCLUDE_FSDK_RENDERCONTEXT_H_
