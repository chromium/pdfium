// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDFXFA_APP_H_
#define _FPDFXFA_APP_H_

class CPDFXFA_App;
class IFXJS_Runtime;
class CJS_RuntimeFactory;

class CPDFXFA_App : public IXFA_AppProvider, public CFX_Object 
{
public:
	CPDFXFA_App();
	~CPDFXFA_App();

	FX_BOOL				Initialize();

	IXFA_App*			GetXFAApp() { return m_pXFAApp; }

						
	/*CPDFDoc_Environment*GetFormFillEnv(){ return m_pEnv; }*/
	FX_BOOL				AddFormFillEnv(CPDFDoc_Environment* pEnv);
	FX_BOOL				RemoveFormFillEnv(CPDFDoc_Environment* pEnv);

	FXJSE_HRUNTIME		GetJSERuntime() { return m_hJSERuntime; }

	void				ReleaseRuntime();

	FX_BOOL				InitRuntime(FX_BOOL bReset = FALSE) {
		if (bReset) {
			m_bInitRuntime = FALSE;
			return TRUE;
		}
		if (m_bInitRuntime)
			return TRUE;
		m_bInitRuntime = TRUE;
		return FALSE;
	}

	//IFXJS_Runtime*		GetJSRuntime();

	CJS_RuntimeFactory* GetRuntimeFactory() {return m_pJSRuntimeFactory;}

public:
	/**
	 *Specifies the name of the client application in which a form currently exists. Such as Exchange-Pro.
	 */
	virtual void		GetAppType(CFX_WideString &wsAppType);
	virtual void		SetAppType(FX_WSTR wsAppType) ;
	virtual void		SetFoxitAppType(FX_WSTR wsFoxitAppType) { return; }
	virtual void		GetFoxitAppType(CFX_WideString &wsFoxitAppType) { return; }
	virtual void		GetFoxitAppName(CFX_WideString& wsFoxitName) {wsFoxitName = L"Foxit";}
	virtual void		GetFoxitVersion(CFX_WideString &wsFoxitVersion) {wsFoxitVersion = L"7.0";}
	/**
	 *Returns the language of the running host application. Such as zh_CN
	 */
	virtual void		GetLanguage(CFX_WideString &wsLanguage);
	/** 
		*Returns the platform of the machine running the script. Such as WIN
	 */
	virtual void		GetPlatform(CFX_WideString &wsPlatform);
	/** 
	 * Indicates the packaging of the application that is running the script. Such as Full
	 */
	virtual void		GetVariation(CFX_WideString &wsVariation);
	/** 
	 * Indicates the version number of the current application. Such as 9
	 */
	virtual void		GetVersion(CFX_WideString &wsVersion);
	//Get application name, such as Phantom
	virtual void		GetAppName(CFX_WideString& wsName);
	/** 
	 *Causes the system to play a sound.	
	 * @param[in] dwType The system code for the appropriate sound.0 (Error)1 (Warning)2 (Question)3 (Status)4 (Default)
	 */
	virtual void		Beep(FX_DWORD dwType);
	/**
	 * Displays a message box.
	 * @param[in] dwIconType	Icon type, refer to XFA_MBICON.
	 * @param[in] dwButtonType	Button type, refer to XFA_MESSAGEBUTTON.
	 * @return A valid integer representing the value of the button pressed by the user, refer to XFA_ID.
	 */
	virtual FX_INT32	MsgBox(FX_WSTR wsMessage, FX_WSTR wsTitle = FX_WSTRC(L""), FX_DWORD dwIconType = 0, FX_DWORD dwButtonType = 0);
	//bMark True (default) Masks the user¡¯s answer with * (asterisks). False Does not mask the user¡¯s answer. 
	virtual void		Response(CFX_WideString &wsAnswer, FX_WSTR wsQuestion, FX_WSTR wsTitle = FX_WSTRC(L""), FX_WSTR wsDefaultAnswer = FX_WSTRC(L""), FX_BOOL bMark = TRUE);
	virtual FX_INT32	GetDocumentCountInBatch();
	virtual FX_INT32	GetCurDocumentInBatch();
	//wsURL http, ftp, such as "http://www.w3.org/TR/REC-xml-names/".
	virtual IFX_FileRead* DownloadURL(FX_WSTR wsURL);
	/*
	* Post data to the given url.
	* @param[in] wsURL			the URL being uploaded.
	* @param[in] wsData			the data being uploaded.
	* @param[in] wsContentType	the content type of data including text/html, text/xml, text/plain, multipart/form-data, 
	*							application/x-www-form-urlencoded, application/octet-stream, any valid MIME type.
	* @param[in] wsEncode		the encode of data including UTF-8, UTF-16, ISO8859-1, any recognized [IANA]character encoding
	* @param[in] wsHeader		any additional HTTP headers to be included in the post.
	* @param[out] wsResponse	decoded response from server.
	* @return TRUE Server permitted the post request, FALSE otherwise.
	*/
	virtual FX_BOOL		PostRequestURL(FX_WSTR wsURL, FX_WSTR wsData, FX_WSTR wsContentType, 
									    FX_WSTR wsEncode, FX_WSTR wsHeader, CFX_WideString &wsResponse);

	/*
	* Put data to the given url.
	* @param[in] wsURL			the URL being uploaded.
	* @param[in] wsData			the data being uploaded.
	* @param[in] wsEncode		the encode of data including UTF-8, UTF-16, ISO8859-1, any recognized [IANA]character encoding
	* @return TRUE Server permitted the post request, FALSE otherwise.
	*/
	virtual FX_BOOL		PutRequestURL(FX_WSTR wsURL, FX_WSTR wsData, FX_WSTR wsEncode);

	virtual void		LoadString(FX_INT32 iStringID, CFX_WideString &wsString);
	virtual	FX_BOOL		ShowFileDialog(FX_WSTR wsTitle, FX_WSTR wsFilter, CFX_WideStringArray &wsPathArr, FX_BOOL bOpen = TRUE);
	virtual IFWL_AdapterTimerMgr* GetTimerMgr();

	CFX_ArrayTemplate<CPDFDoc_Environment*> m_pEnvList;

public:
	static CPDFXFA_App* m_pApp;

private:
	IXFA_App*  m_pXFAApp;
	IXFA_FontMgr* m_pFontMgr;
	FXJSE_HRUNTIME m_hJSERuntime;
	IFXJS_Runtime* m_pJSRuntime;
	CJS_RuntimeFactory*  m_pJSRuntimeFactory;

	CFX_WideString	m_csAppType;
	FX_BOOL			m_bInitRuntime;
};

CPDFXFA_App* FPDFXFA_GetApp();
void FPDFXFA_ReleaseApp();

#endif 
