// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__2BC4C1DE_4165_4967_AC3E_AE2EA2A4D2FD__INCLUDED_)
#define AFX_STDAFX_H__2BC4C1DE_4165_4967_AC3E_AE2EA2A4D2FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#include <afxinet.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#include "../../include/fpdfview.h"
#include "../../include/fpdfdoc.h"
#include "../../include/fpdftext.h"
#include "../../include/fpdfformfill.h"
#include "../../include/fpdf_sysfontinfo.h"
#include "../../include/fpdfsave.h"
#include <afxcview.h>
#include "afxtempl.h"

#include "../../include/fpdfedit.h"

#ifdef _DEBUG
	#pragma comment(lib, "../../lib/dbg_w32_vc10/formfiller[dbg,w32,vc10].lib")
	#pragma comment(lib, "../../lib/dbg_w32_vc10/fpdfsdk[dbg,w32,vc10].lib")
	#pragma comment(lib, "../../lib/dbg_w32_vc10/fxedit[dbg,w32,vc10].lib")
	#pragma comment(lib, "../../lib/dbg_w32_vc10/javascript[dbg,w32,vc10].lib")
	#pragma comment(lib, "../../lib/dbg_w32_vc10/jsapi[dbg,w32,vc10].lib")
	#pragma comment(lib, "../../lib/dbg_w32_vc10/pdfwindow[dbg,w32,vc10].lib")
//	#pragma comment(lib, "../../lib/dbg_w32_vc10/foxitopenpdf.lib")

// 	#pragma comment(lib, "../../../../../../v8/build/Debug/lib/icui18n.lib")
// 	#pragma comment(lib, "../../../../../../v8/build/Debug/lib/icuuc.lib")
// 	#pragma comment(lib, "../../../../../../v8/build/Debug/lib/v8_base.ia32.lib")
// 	#pragma comment(lib, "../../../../../../v8/build/Debug/lib/v8_nosnapshot.ia32.lib")
// 	#pragma comment(lib, "../../../../../../v8/build/Debug/lib/v8_snapshot.lib")
	#pragma comment(lib, "../../../v8/build/Debug/lib/icui18n.lib")
	#pragma comment(lib, "../../../v8/build/Debug/lib/icuuc.lib")
    #pragma comment(lib, "../../../v8/build/Debug/lib/v8_base.ia32.lib")
	#pragma comment(lib, "../../../v8/build/Debug/lib/v8_nosnapshot.ia32.lib")
	#pragma comment(lib, "../../../v8/build/Debug/lib/v8_snapshot.lib")

	#pragma comment(lib, "../../../fxlib/fxcore/lib/dbg/x86_vc10/fxcrt[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fxcore/lib/dbg/x86_vc10/fxge[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fxcore/lib/dbg/x86_vc10/fpdfdoc[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fxcore/lib/dbg/x86_vc10/fpdfapi[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fxcore/lib/dbg/x86_vc10/fxcodec[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fxcore/lib/dbg/x86_vc10/fpdftext[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fxcore/lib/dbg/x86_vc10/fdrm[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fxcore/lib/dbg/x86_vc10/fxmath[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fxcore/lib/dbg/x86_vc10/fxbarcode[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fxcore/lib/dbg/x86_vc10/fxhal[dbg_x86_vc10].lib")
	
	#pragma comment(lib, "../../../fxlib/fxjse/lib/dbg/x86_vc10/fxjse_bare[dbg_x86_vc10].lib")
	
	#pragma comment(lib, "../../../fxlib/fwl/lib/dbg/x86_vc10/fwlbasewidget[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fwl/lib/dbg/x86_vc10/fwlcore[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fwl/lib/dbg/x86_vc10/fwltheme[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fwl/lib/dbg/x86_vc10/fwllightwidget[dbg_x86_vc10].lib")

	#pragma comment(lib, "../../../fxlib/fxgraphics/lib/dbg/x86_vc10/fxgraphics[dbg_x86_vc10].lib")
	
	#pragma comment(lib, "../../../fxlib/fgas/lib/dbg/x86_vc10/fgas[dbg_x86_vc10].lib")

	#pragma comment(lib, "../../../fxlib/fee/lib/dbg/x86_vc10/fees[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fee/lib/dbg/x86_vc10/fx_wordbreaks[dbg_x86_vc10].lib")

	#pragma comment(lib, "../../../fxlib/fdp/lib/dbg/x86_vc10/fde[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fdp/lib/dbg/x86_vc10/fdetto[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fdp/lib/dbg/x86_vc10/fdexml[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fdp/lib/dbg/x86_vc10/fdecss[dbg_x86_vc10].lib")

	#pragma comment(lib, "../../../fxlib/fxfa/lib/dbg/x86_vc10/fxfa_app[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fxfa/lib/dbg/x86_vc10/fxfa_fm2js[dbg_x86_vc10].lib")
	#pragma comment(lib, "../../../fxlib/fxfa/lib/dbg/x86_vc10/fxfa_parser[dbg_x86_vc10].lib")

#else
	#pragma comment(lib, "../../lib/rel_w32_vc6/fpdfsdk.lib")
#endif
//#pragma comment(lib, "../../lib/fpdfsdk.dll")

//#include "fpdfsdk_ext_wh.h"
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__2BC4C1DE_4165_4967_AC3E_AE2EA2A4D2FD__INCLUDED_)
