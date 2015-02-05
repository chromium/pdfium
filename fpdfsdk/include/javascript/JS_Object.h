// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JS_OBJECT_H_
#define _JS_OBJECT_H_

class CJS_Object;
class CJS_Timer;
class CJS_Context;

class CJS_EmbedObj : public CFX_Object
{
public:
	CJS_EmbedObj(CJS_Object* pJSObject);
	virtual ~CJS_EmbedObj();

	virtual void				TimerProc(CJS_Timer* pTimer){};

	CJS_Timer*					BeginTimer(CPDFDoc_Environment * pApp, FX_UINT nElapse);
	void						EndTimer(CJS_Timer* pTimer);

	CJS_Object*					GetJSObject(){return m_pJSObject;};
	operator					CJS_Object* (){return m_pJSObject;};

	CPDFSDK_PageView *			JSGetPageView(IFXJS_Context* cc);
	int							MsgBox(CPDFDoc_Environment* pApp, CPDFSDK_PageView* pPageView, FX_LPCWSTR swMsg, FX_LPCWSTR swTitle = NULL, FX_UINT nType = 0, FX_UINT nIcon = 0);
	void						Alert(CJS_Context* pContext, FX_LPCWSTR swMsg);

protected:
	CJS_Object*					m_pJSObject;
};

class CJS_Object : public CFX_Object
{
public:
	CJS_Object(JSFXObject pObject);
	virtual ~CJS_Object(void);

	void						MakeWeak();

	virtual FX_BOOL				IsType(FX_LPCSTR sClassName){return TRUE;};
	virtual CFX_ByteString		GetClassName(){return "";};

	virtual FX_BOOL				InitInstance(IFXJS_Context* cc){return TRUE;};
	virtual FX_BOOL				ExitInstance(){return TRUE;};

	operator					JSFXObject () {return v8::Local<v8::Object>::New(m_pIsolate, m_pObject);}
	operator					CJS_EmbedObj* (){return m_pEmbedObj;};

	void						SetEmbedObject(CJS_EmbedObj* pObj){m_pEmbedObj = pObj;};
	CJS_EmbedObj *				GetEmbedObject(){return m_pEmbedObj;};

	static CPDFSDK_PageView *	JSGetPageView(IFXJS_Context* cc);
	static int					MsgBox(CPDFDoc_Environment* pApp, CPDFSDK_PageView* pPageView, FX_LPCWSTR swMsg, FX_LPCWSTR swTitle = NULL, FX_UINT nType = 0,FX_UINT nIcon = 0);
	static void					Alert(CJS_Context* pContext, FX_LPCWSTR swMsg);

	v8::Isolate*					GetIsolate() {return m_pIsolate;}
protected:
	CJS_EmbedObj *				m_pEmbedObj;
	v8::Persistent<v8::Object>			m_pObject;
	v8::Isolate*					m_pIsolate;
};

struct JS_TIMER_MAP
{
	FX_UINT nID;
	CJS_Timer * pTimer;
};

typedef CFX_ArrayTemplate<JS_TIMER_MAP*>	CTimerMapArray;

struct JS_TIMER_MAPARRAY
{
public:
	JS_TIMER_MAPARRAY()
	{
	}

	~JS_TIMER_MAPARRAY()
	{
		Reset();
	}

	void Reset()
	{
		for (int i=0,sz=m_Array.GetSize(); i<sz; i++)
			delete m_Array.GetAt(i);

		m_Array.RemoveAll();
	}

	void SetAt(FX_UINT nIndex,CJS_Timer * pTimer)
	{
		int i = Find(nIndex);

		if (i>=0)
		{
			if (JS_TIMER_MAP * pMap = m_Array.GetAt(i))
				pMap->pTimer = pTimer;
		}
		else
		{
			if (JS_TIMER_MAP * pMap = new JS_TIMER_MAP)
			{
				pMap->nID = nIndex;
				pMap->pTimer = pTimer;
				m_Array.Add(pMap);
			}
		}
	}

	CJS_Timer * GetAt(FX_UINT nIndex)
	{
		int i = Find(nIndex);

		if (i>=0)
		{
			if (JS_TIMER_MAP * pMap = m_Array.GetAt(i))
				return pMap->pTimer;
		}
		return NULL;
	}

	void RemoveAt(FX_UINT nIndex)
	{
		int i = Find(nIndex);

		if (i>=0)
		{
			delete m_Array.GetAt(i);
			m_Array.RemoveAt(i);
		}
		//To prevent potential fake memory leak reported by vc6.
		if(m_Array.GetSize() == 0)
			m_Array.RemoveAll();
	}

	int Find(FX_UINT nIndex)
	{
		for (int i=0,sz=m_Array.GetSize(); i<sz; i++)
		{
			if (JS_TIMER_MAP * pMap = m_Array.GetAt(i))
			{
				if (pMap->nID == nIndex)
					return i;
			}
		}

		return -1;
	}

	CTimerMapArray		m_Array;
};

JS_TIMER_MAPARRAY& GetTimeMap();

class CJS_Runtime;

class CJS_Timer
{
public:
	CJS_Timer(CJS_EmbedObj * pObj,CPDFDoc_Environment* pApp):
		m_nTimerID(0),
		m_pEmbedObj(pObj),
		m_bProcessing(FALSE),
		m_dwStartTime(0),
		m_dwTimeOut(0),
		m_dwElapse(0),
		m_pRuntime(NULL),
		m_nType(0),
		m_pApp(pApp)
	{
	}

	virtual ~CJS_Timer()
	{
		KillJSTimer();
	}

public:
	FX_UINT SetJSTimer(FX_UINT nElapse)
	{
		if (m_nTimerID)KillJSTimer();
		IFX_SystemHandler* pHandler = m_pApp->GetSysHandler();
		m_nTimerID = pHandler->SetTimer(nElapse,TimerProc);
		GetTimeMap().SetAt(m_nTimerID,this);
		m_dwElapse = nElapse;
		return m_nTimerID;
	};

	void KillJSTimer()
	{
		if (m_nTimerID)
		{
			IFX_SystemHandler* pHandler = m_pApp->GetSysHandler();
			pHandler->KillTimer(m_nTimerID);
			GetTimeMap().RemoveAt(m_nTimerID);
			m_nTimerID = 0;
		}
	};

	void SetType(int nType)
	{
		m_nType = nType;
	}

	int GetType() const
	{
		return m_nType;
	}

	void SetStartTime(FX_DWORD dwStartTime)
	{
		m_dwStartTime = dwStartTime;
	}

	FX_DWORD GetStartTime() const
	{
		return m_dwStartTime;
	}

	void SetTimeOut(FX_DWORD dwTimeOut)
	{
		m_dwTimeOut = dwTimeOut;
	}

	FX_DWORD GetTimeOut() const
	{
		return m_dwTimeOut;
	}

	void SetRuntime(CJS_Runtime* pRuntime)
	{
		m_pRuntime = pRuntime;
	}

	CJS_Runtime* GetRuntime() const
	{
		return m_pRuntime;
	}

	void SetJScript(const CFX_WideString& script)
	{
		m_swJScript = script;
	}

	CFX_WideString GetJScript() const
	{
		return m_swJScript;
	}

	static void TimerProc(int idEvent)
	{
		if (CJS_Timer * pTimer = GetTimeMap().GetAt(idEvent))
		{
			if (!pTimer->m_bProcessing)
			{
				pTimer->m_bProcessing = TRUE;
				if (pTimer->m_pEmbedObj) pTimer->m_pEmbedObj->TimerProc(pTimer);
				pTimer->m_bProcessing = FALSE;
			}
			else
			{
			//	TRACE(L"BUSY!\n");
			}
		}
	};

private:
	FX_UINT							m_nTimerID;
	CJS_EmbedObj*					m_pEmbedObj;
	FX_BOOL							m_bProcessing;

	//data
	FX_DWORD							m_dwStartTime;
	FX_DWORD							m_dwTimeOut;
	FX_DWORD						m_dwElapse;
	CJS_Runtime*					m_pRuntime;
	CFX_WideString					m_swJScript;
	int								m_nType; //0:Interval; 1:TimeOut

	CPDFDoc_Environment*			m_pApp;
};
#endif //_JS_OBJECT_H_
