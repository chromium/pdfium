// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FSDK_DEFINE_H_
#define FPDFSDK_INCLUDE_FSDK_DEFINE_H_

#ifdef _WIN32
#include <tchar.h>
#include <math.h>
#endif

#include "../../core/include/fpdfapi/fpdf_module.h"
#include "../../core/include/fpdfapi/fpdf_pageobj.h"
#include "../../core/include/fpdfapi/fpdf_parser.h"
#include "../../core/include/fpdfapi/fpdf_parser.h"
#include "../../core/include/fpdfapi/fpdf_render.h"
#include "../../core/include/fpdfapi/fpdf_serial.h"
#include "../../core/include/fpdfapi/fpdfapi.h"
#include "../../core/include/fpdfdoc/fpdf_doc.h"
#include "../../core/include/fpdfdoc/fpdf_vt.h"
#include "../../xfa/include/fxbarcode/BC_BarCode.h"
#include "../../xfa/include/fxjse/fxjse.h"
#include "../../xfa/include/fxgraphics/fx_graphics.h"
#include "../../xfa/include/fxfa/fxfa.h"
#include "../../xfa/include/fwl/core/fwl_error.h"
#include "../../xfa/include/fwl/core/fwl_timer.h"
#include "../../xfa/include/fwl/adapter/fwl_adaptertimermgr.h"


#ifndef FX_GetAValue
/** @brief It retrieves an intensity value for the alpha component of a #FX_ARGB value. */
#define FX_GetAValue(argb)			((argb & 0xFF000000) >> 24)
#endif

#ifndef FX_GetRValue
/** @brief It retrieves an intensity value for the red component of a #FX_ARGB value. */
#define FX_GetRValue(argb)			((argb & 0x00FF0000) >> 16)
#endif

#ifndef FX_GetGValue
/** @brief It retrieves an intensity value for the green component of a #FX_ARGB value. */
#define FX_GetGValue(argb)			((argb & 0x0000FF00) >> 8)
#endif

#ifndef FX_GetBValue
/** @brief It retrieves an intensity value for the blue component of a #FX_ARGB value. */
#define FX_GetBValue(argb)			(argb & 0x000000FF)
#endif

#ifndef FX_ARGBTOCOLORREF
/** @brief Convert a #FX_ARGB to a #FX_COLORREF. */
#define FX_ARGBTOCOLORREF(argb)		((((FX_DWORD)argb & 0x00FF0000) >> 16)|((FX_DWORD)argb & 0x0000FF00)|(((FX_DWORD)argb & 0x000000FF) << 16))
#endif

#ifndef FX_COLORREFTOARGB
/** @brief Convert a #FX_COLORREF to a #FX_ARGB. */
#define FX_COLORREFTOARGB(rgb)		((FX_DWORD)0xFF000000|(((FX_DWORD)rgb & 0x000000FF) << 16)|((FX_DWORD)rgb & 0x0000FF00)|(((FX_DWORD)rgb & 0x00FF0000) >> 16))
#endif

typedef unsigned int FX_UINT;

#include "../../public/fpdfview.h"

class CPDF_CustomAccess final : public IFX_FileRead
{
public:
	CPDF_CustomAccess(FPDF_FILEACCESS* pFileAccess);
	~CPDF_CustomAccess() {}


	virtual CFX_ByteString GetFullPath() { return ""; }
	virtual FX_FILESIZE	GetSize() override { return m_FileAccess.m_FileLen; }
	virtual FX_BOOL		GetByte(FX_DWORD pos, uint8_t& ch);
	virtual FX_BOOL		GetBlock(FX_DWORD pos, uint8_t* pBuf, FX_DWORD size);
	virtual void		Release() override { delete this; }
	virtual FX_BOOL		ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override;

	FPDF_FILEACCESS		m_FileAccess;
	uint8_t				m_Buffer[512];
	FX_DWORD			m_BufferOffset;
};

class CFPDF_FileStream : public IFX_FileStream
{
public:
	CFPDF_FileStream(FPDF_FILEHANDLER* pFS);
	virtual ~CFPDF_FileStream() {}

	virtual IFX_FileStream*		Retain();
	virtual void				Release();

	virtual FX_FILESIZE			GetSize();
	virtual FX_BOOL				IsEOF();
	virtual FX_FILESIZE			GetPosition() {return m_nCurPos;}
	virtual void				SetPosition(FX_FILESIZE pos) {m_nCurPos = pos; }
	virtual FX_BOOL				ReadBlock(void* buffer, FX_FILESIZE offset, size_t size);
	virtual size_t				ReadBlock(void* buffer, size_t size);
	virtual	FX_BOOL				WriteBlock(const void* buffer, FX_FILESIZE offset, size_t size);
	virtual FX_BOOL				Flush();

protected:
	FPDF_FILEHANDLER*	m_pFS;
	FX_FILESIZE		m_nCurPos;
};

void		FSDK_SetSandBoxPolicy(FPDF_DWORD policy, FPDF_BOOL enable);
FPDF_BOOL	FSDK_IsSandBoxPolicyEnabled(FPDF_DWORD policy);


#endif  // FPDFSDK_INCLUDE_FSDK_DEFINE_H_
