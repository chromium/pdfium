// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_ICON_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_ICON_H_

#include "JS_Define.h"

class Icon : public CJS_EmbedObj
{
public:
	Icon(CJS_Object* pJSObject);
	virtual ~Icon();
	FX_BOOL name(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	void				SetStream(CPDF_Stream* pIconStream);
	CPDF_Stream*		GetStream();
	void				SetIconName(CFX_WideString name);
	CFX_WideString		GetIconName();

private:
	CPDF_Stream*		m_pIconStream;
	CFX_WideString		m_swIconName;
};

class CJS_Icon : public CJS_Object
{
public:
	CJS_Icon(JSFXObject pObject) : CJS_Object(pObject){};
	virtual ~CJS_Icon(){};

public:
	DECLARE_JS_CLASS(CJS_Icon);

	JS_STATIC_PROP(name, Icon);
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_ICON_H_
