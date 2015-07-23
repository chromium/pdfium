// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_Object.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/JS_EventHandler.h"
//#include "../include/JS_ResMgr.h"
#include "../../include/javascript/JS_Context.h"
#include "../../include/javascript/event.h"
#include "../../include/javascript/Field.h"

/* -------------------------- event -------------------------- */

BEGIN_JS_STATIC_CONST(CJS_Event)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_Event)
	JS_STATIC_PROP_ENTRY(change)
	JS_STATIC_PROP_ENTRY(changeEx)
	JS_STATIC_PROP_ENTRY(commitKey)
	JS_STATIC_PROP_ENTRY(fieldFull)
	JS_STATIC_PROP_ENTRY(keyDown)
	JS_STATIC_PROP_ENTRY(modifier)
	JS_STATIC_PROP_ENTRY(name)
	JS_STATIC_PROP_ENTRY(rc)
	JS_STATIC_PROP_ENTRY(richChange)
	JS_STATIC_PROP_ENTRY(richChangeEx)
	JS_STATIC_PROP_ENTRY(richValue)
	JS_STATIC_PROP_ENTRY(selEnd)
	JS_STATIC_PROP_ENTRY(selStart)
	JS_STATIC_PROP_ENTRY(shift)
	JS_STATIC_PROP_ENTRY(source)
	JS_STATIC_PROP_ENTRY(target)
	JS_STATIC_PROP_ENTRY(targetName)
	JS_STATIC_PROP_ENTRY(type)
	JS_STATIC_PROP_ENTRY(value)
	JS_STATIC_PROP_ENTRY(willCommit)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_Event)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_Event,event)

event::event(CJS_Object * pJsObject) : CJS_EmbedObj(pJsObject)
{
}

event::~event(void)
{
}

bool event::change(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	CFX_WideString &wChange = pEvent->Change();
	if (vp.IsSetting())
	{
		if (vp.GetType() == VT_string)
			vp >> wChange;
	}
	else
	{
		vp << wChange;
	}
	return true;
}

bool event::changeEx(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (!vp.IsGetting())return false;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	vp << pEvent->ChangeEx();
	return true;
}

bool event::commitKey(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (!vp.IsGetting())return false;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	vp << pEvent->CommitKey();
	return true;
}

bool event::fieldFull(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (!vp.IsGetting() && wcscmp((const wchar_t*)pEvent->Name(),L"Keystroke") != 0)
		return false;

	if (pEvent->FieldFull())
		vp << true;
	else
		vp << false;
	return true;
}

bool event::keyDown(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (!vp.IsGetting())return false;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (pEvent->KeyDown())
		vp << true;
	else
		vp << false;
	return true;
}

bool event::modifier(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (!vp.IsGetting())return false;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (pEvent->Modifier())
		vp << true;
	else
		vp << false;
	return true;
}

bool event::name(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (!vp.IsGetting())return false;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	vp << pEvent->Name();
	return true;
}

bool event::rc(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

    bool &bRc = pEvent->Rc();
	if (vp.IsSetting())
	{
		vp>>bRc;
	}
	else
	{
		vp<<bRc;
	}
	return true;
}

bool event::richChange(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	return true;
	if (vp.IsSetting())
	{
	}
	else
	{
		;
	}
	return true;
}

bool event::richChangeEx(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	return true;
	if (vp.IsSetting())
	{
	}
	else
	{
		;
	}
	return true;
}


bool event::richValue(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	return true;
	if (vp.IsSetting())
	{
	}
	else
	{
		;
	}
	return true;
}

bool event::selEnd(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (wcscmp((const wchar_t*)pEvent->Name(),L"Keystroke") != 0)
	{
		return true;
	}

	int &iSelEnd = pEvent->SelEnd();
	if (vp.IsSetting())
	{
		vp >> iSelEnd;
	}
	else
	{
		vp << iSelEnd;
	}
	return true;
}

bool event::selStart(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (wcscmp((const wchar_t*)pEvent->Name(),L"Keystroke") != 0)
	{
		return true;
	}
	int &iSelStart = pEvent->SelStart();
	if (vp.IsSetting())
	{
		vp >> iSelStart;
	}
	else
	{
		vp << iSelStart;
	}
	return true;
}

bool event::shift(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (!vp.IsGetting())return false;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (pEvent->Shift())
		vp << true;
	else
		vp << false;
	return true;
}

bool event::source(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (!vp.IsGetting())return false;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	vp << pEvent->Source()->GetJSObject();
	return true;
}

bool event::target(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (!vp.IsGetting())return false;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	vp<<pEvent->Target_Field()->GetJSObject();
	return true;
}

bool event::targetName(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (!vp.IsGetting())return false;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	vp << pEvent->TargetName();
	return true;
}

bool event::type(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (!vp.IsGetting())return false;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	vp << pEvent->Type();
	return true;
}

bool event::value(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (wcscmp((const wchar_t*)pEvent->Type(),L"Field") != 0)
		return false;
	if(!pEvent->m_pValue)
		return false;
	CFX_WideString & val = pEvent->Value();
	if (vp.IsSetting())
	{
		vp >> val;
	}
	else
	{
		vp << val;
	}
	return true;
}

bool event::willCommit(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (!vp.IsGetting())return false;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_EventHandler* pEvent = pContext->GetEventHandler();
	ASSERT(pEvent != NULL);

	if (pEvent->WillCommit())
		vp << true;
	else
		vp << false;
	return true;
}

