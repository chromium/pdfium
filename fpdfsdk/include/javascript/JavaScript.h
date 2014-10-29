// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JAVASCRIPT_H_
#define _JAVASCRIPT_H_


#ifndef _INC_PDFAPI
	#define _INC_PDFAPI

	#include "../../../core/include/fpdfapi/fpdf_module.h" 
	#include "../../../core/include/fpdfdoc/fpdf_doc.h" 
	#include "../../../core/include/fpdfdoc/fpdf_vt.h" 
	#include "../../../core/include/fxcrt/fx_xml.h" 
	#include "../../../core/include/fdrm/fx_crypt.h" 
	#include "../../../core/include/fpdfapi/fpdf_pageobj.h" 
	#include "../../../core/include/fpdfapi/fpdf_serial.h"


	#include "../../include/fx_systemhandler.h"	
#endif


#include "../jsapi/fxjs_v8.h"
#include "../fxedit/fx_edit.h"
#include "../pdfwindow/IPDFWindow.h"
#include "../fsdk_mgr.h"
#include "../fpdfxfa/fpdfxfa_app.h"


#include <string>
//#pragma warning( disable : 4786) 
#include <vector>


#endif //_JAVASCRIPT_H_

