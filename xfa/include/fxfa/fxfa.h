// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA
#define _FXFA
class IFDE_XMLElement;
class CXFA_Node;
class CXFA_NodeList;
class CXFA_WidgetAcc;
class IFWL_AdapterTimerMgr;
class IFX_Font;
class CFX_Graphics;
class IXFA_AppProvider;
class IXFA_App;
class IXFA_FontMgr;
class IXFA_DocProvider;
class IXFA_DocHandler;
class IXFA_DocView;
class IXFA_PageViewRender;
class IXFA_PageView;
class IXFA_WidgetHandler;
class IXFA_WidgetIterator;
class IXFA_MenuHandler;
class IXFA_ChecksumContext;
class IXFA_WidgetAccIterator;
typedef struct _XFA_HDOC {
    FX_LPVOID pData;
}* XFA_HDOC;
typedef struct _XFA_HWIDGET {
    FX_LPVOID pData;
}* XFA_HWIDGET;
#include "fxfa_basic.h"
#include "fxfa_widget.h"
#define XFA_MBICON_Error	0
#define	XFA_MBICON_Warning	1
#define	XFA_MBICON_Question	2
#define	XFA_MBICON_Status	3
#define XFA_MB_OK			0
#define	XFA_MB_OKCancel		1
#define	XFA_MB_YesNo		2
#define XFA_MB_YesNoCancel	3
#define XFA_IDOK			1
#define XFA_IDCancel		2
#define XFA_IDNo			3
#define XFA_IDYes			4
#define XFA_IDS_ValidateFailed					1
#define XFA_IDS_CalcOverride					2
#define XFA_IDS_ModifyField						3
#define XFA_IDS_NotModifyField					4
#define XFA_IDS_AppName							5
#define XFA_IDS_ImageFilter						6
#define XFA_IDS_UNKNOW_CATCHED					7
#define XFA_IDS_Unable_TO_SET					8
#define XFA_IDS_VALUE_EXCALMATORY				9
#define XFA_IDS_INVALID_ENUM_VALUE				10
#define XFA_IDS_UNSUPPORT_METHOD				11
#define XFA_IDS_UNSUPPORT_PROP					12
#define XFA_IDS_INVAlID_PROP_SET				13
#define XFA_IDS_NOT_DEFAUL_VALUE				14
#define XFA_IDS_UNABLE_SET_LANGUAGE				15
#define XFA_IDS_UNABLE_SET_NUMPAGES				16
#define XFA_IDS_UNABLE_SET_PLATFORM				17
#define XFA_IDS_UNABLE_SET_VALIDATIONENABLE		18
#define XFA_IDS_UNABLE_SET_VARIATION			19
#define XFA_IDS_UNABLE_SET_VERSION				20
#define	XFA_IDS_UNABLE_SET_READY				21
#define XFA_IDS_NUMBER_OF_OCCUR					22
#define XFA_IDS_UNABLE_SET_CLASS_NAME			23
#define XFA_IDS_UNABLE_SET_LENGTH_VALUE			24
#define XFA_IDS_UNSUPPORT_CHAR				    25
#define XFA_IDS_BAD_SUFFIX						26
#define XFA_IDS_EXPECTED_IDENT					27
#define XFA_IDS_EXPECTED_STRING					28
#define XFA_IDS_INVALIDATE_CHAR					29
#define XFA_IDS_REDEFINITION					30
#define XFA_IDS_INVALIDATE_TOKEN				31
#define XFA_IDS_INVALIDATE_EXPRESSION			32
#define XFA_IDS_UNDEFINE_IDENTIFIER				33
#define XFA_IDS_INVALIDATE_LEFTVALUE			34
#define XFA_IDS_COMPILER_ERROR					35
#define XFA_IDS_CANNOT_MODIFY_VALUE				36
#define XFA_IDS_ERROR_PARAMETERS				37
#define XFA_IDS_EXPECT_ENDIF					38
#define XFA_IDS_UNEXPECTED_EXPRESSION			39
#define XFA_IDS_CONDITION_IS_NULL				40
#define XFA_IDS_ILLEGALBREAK					41
#define XFA_IDS_ILLEGALCONTINUE					42
#define XFA_IDS_EXPECTED_OPERATOR				43
#define XFA_IDS_DIVIDE_ZERO						44
#define XFA_IDS_CANNOT_COVERT_OBJECT			45
#define XFA_IDS_NOT_FOUND_CONTAINER				46
#define XFA_IDS_NOT_FOUND_PROPERTY				47
#define XFA_IDS_NOT_FOUND_METHOD				48
#define XFA_IDS_NOT_FOUND_CONST					49
#define XFA_IDS_NOT_ASSIGN_OBJECT				50
#define XFA_IDS_IVALIDATE_INSTRUCTION			51
#define XFA_IDS_EXPECT_NUMBER					52
#define XFA_IDS_VALIDATE_OUT_ARRAY				53
#define XFA_IDS_CANNOT_ASSIGN_IDENT				54
#define XFA_IDS_NOT_FOUNT_FUNCTION				55
#define XFA_IDS_NOT_ARRAY						56
#define XFA_IDS_OUT_ARRAY						57
#define XFA_IDS_NOT_SUPPORT_CALC				58
#define XFA_IDS_ARGUMENT_NOT_ARRAY				59
#define XFA_IDS_ARGUMENT_EXPECT_CONTAINER		60
#define XFA_IDS_ACCESS_PROPERTY_IN_NOT_OBJECT	61
#define XFA_IDS_FUNCTION_IS_BUILDIN				62
#define XFA_IDS_ERROR_MSG						63
#define XFA_IDS_INDEX_OUT_OF_BOUNDS				64
#define XFA_IDS_INCORRECT_NUMBER_OF_METHOD		65
#define XFA_IDS_ARGUMENT_MISMATCH				66
#define XFA_IDS_INVALID_ENUMERATE				67
#define XFA_IDS_INVALID_APPEND					68
#define XFA_IDS_SOM_EXPECTED_LIST				69
#define XFA_IDS_NOT_HAVE_PROPERTY				70
#define XFA_IDS_INVALID_NODE_TYPE				71
#define XFA_IDS_VIOLATE_BOUNDARY				72
#define XFA_IDS_SERVER_DENY						73
#define XFA_IDS_StringWeekDay_Sun				74
#define XFA_IDS_StringWeekDay_Mon				75
#define XFA_IDS_StringWeekDay_Tue				76
#define XFA_IDS_StringWeekDay_Wed				77
#define XFA_IDS_StringWeekDay_Thu				78
#define XFA_IDS_StringWeekDay_Fri				79
#define XFA_IDS_StringWeekDay_Sat				80
#define XFA_IDS_StringMonth_Jan					81
#define XFA_IDS_StringMonth_Feb					82
#define XFA_IDS_StringMonth_March				83
#define XFA_IDS_StringMonth_April				84
#define XFA_IDS_StringMonth_May					85
#define XFA_IDS_StringMonth_June				86
#define XFA_IDS_StringMonth_July				87
#define XFA_IDS_StringMonth_Aug					88
#define XFA_IDS_StringMonth_Sept				89
#define XFA_IDS_StringMonth_Oct					90
#define XFA_IDS_StringMonth_Nov					91
#define XFA_IDS_StringMonth_Dec					92
#define XFA_IDS_String_Today					93
#define XFA_IDS_ValidateLimit					94
#define XFA_IDS_ValidateNullWarning				95
#define XFA_IDS_ValidateNullError				96
#define XFA_IDS_ValidateWarning					97
#define XFA_IDS_ValidateError					98
class IXFA_AppProvider
{
public:

    virtual void		SetAppType(FX_WSTR wsAppType) = 0;
    virtual void		GetAppType(CFX_WideString &wsAppType) = 0;

    virtual void		SetFoxitAppType(FX_WSTR wsFoxitAppType)
    {
        return;
    }
    virtual void		GetFoxitAppType(CFX_WideString &wsFoxitAppType)
    {
        return;
    }

    virtual void		GetLanguage(CFX_WideString &wsLanguage) = 0;

    virtual void		GetPlatform(CFX_WideString &wsPlatform) = 0;

    virtual void		GetVariation(CFX_WideString &wsVariation) = 0;

    virtual void		GetVersion(CFX_WideString &wsVersion) = 0;

    virtual void		GetFoxitVersion(CFX_WideString &wsFoxitVersion)
    {
        return;
    }

    virtual void		GetAppName(CFX_WideString& wsName) = 0;

    virtual void		GetFoxitAppName(CFX_WideString& wsFoxitName)
    {
        return;
    }

    virtual void		Beep(FX_DWORD dwType) = 0;

    virtual FX_INT32	MsgBox(FX_WSTR wsMessage, FX_WSTR wsTitle = FX_WSTRC(L""), FX_DWORD dwIconType = 0, FX_DWORD dwButtonType = 0) = 0;
    virtual void		Response(CFX_WideString &wsAnswer, FX_WSTR wsQuestion, FX_WSTR wsTitle = FX_WSTRC(L""), FX_WSTR wsDefaultAnswer = FX_WSTRC(L""), FX_BOOL bMark = TRUE) = 0;
    virtual FX_INT32	GetDocumentCountInBatch() = 0;
    virtual FX_INT32	GetCurDocumentInBatch() = 0;
    virtual IFX_FileRead* DownloadURL(FX_WSTR wsURL) = 0;

    virtual FX_BOOL		PostRequestURL(FX_WSTR wsURL, FX_WSTR wsData, FX_WSTR wsContentType,
                                       FX_WSTR wsEncode, FX_WSTR wsHeader, CFX_WideString &wsResponse) = 0;

    virtual FX_BOOL		PutRequestURL(FX_WSTR wsURL, FX_WSTR wsData, FX_WSTR wsEncode) = 0;
    virtual void		LoadString(FX_INT32 iStringID, CFX_WideString &wsString) = 0;
    virtual	FX_BOOL		ShowFileDialog(FX_WSTR wsTitle, FX_WSTR wsFilter, CFX_WideStringArray &wsPathArr, FX_BOOL bOpen = TRUE) = 0;
    virtual IFWL_AdapterTimerMgr* GetTimerMgr() = 0;;
};
class IXFA_FontMgr
{
public:
    virtual void		Release() = 0;
    virtual IFX_Font*	GetFont(XFA_HDOC hDoc, FX_WSTR wsFontFamily, FX_DWORD dwFontStyles, FX_WORD wCodePage = 0xFFFF) = 0;
    virtual IFX_Font*	GetDefaultFont(XFA_HDOC hDoc, FX_WSTR wsFontFamily, FX_DWORD dwFontStyles, FX_WORD wCodePage = 0xFFFF) = 0;
};
IXFA_FontMgr*	XFA_GetDefaultFontMgr();
class IXFA_App
{
public:
    static IXFA_App*	Create(IXFA_AppProvider* pProvider);
    virtual void		Release() = 0;
    virtual IXFA_DocHandler*	GetDocHandler() = 0;
    virtual XFA_HDOC			CreateDoc(IXFA_DocProvider* pProvider, IFX_FileRead* pStream, FX_BOOL bTakeOverFile = TRUE) = 0;
    virtual XFA_HDOC			CreateDoc(IXFA_DocProvider* pProvider, CPDF_Document* pPDFDoc) = 0;
    virtual	IXFA_AppProvider*	GetAppProvider() = 0;
    virtual void				SetDefaultFontMgr(IXFA_FontMgr* pFontMgr) = 0;
    virtual IXFA_MenuHandler*	GetMenuHandler() = 0;
};
class IXFA_MenuHandler
{
public:
    virtual FX_BOOL		CanCopy(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		CanCut(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		CanPaste(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		CanSelectAll(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		CanDelete(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		CanDeSelect(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		Copy(XFA_HWIDGET hWidget, CFX_WideString &wsText) = 0;
    virtual FX_BOOL		Cut(XFA_HWIDGET hWidget, CFX_WideString &wsText) = 0;
    virtual FX_BOOL		Paste(XFA_HWIDGET hWidget, const CFX_WideString &wsText) = 0;
    virtual FX_BOOL		SelectAll(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		Delete(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		DeSelect(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		CanUndo(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		CanRedo(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		Undo(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		Redo(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL		GetSuggestWords(XFA_HWIDGET hWidget, CFX_PointF pointf, CFX_ByteStringArray &sSuggest) = 0;
    virtual FX_BOOL		ReplaceSpellCheckWord(XFA_HWIDGET hWidget, CFX_PointF pointf, FX_BSTR bsReplace) = 0;
};
#define XFA_INVALIDATE_AllPages		0x00000000
#define XFA_INVALIDATE_CurrentPage	0x00000001
#define XFA_PRINTOPT_ShowDialog		0x00000001
#define XFA_PRINTOPT_CanCancel		0x00000002
#define XFA_PRINTOPT_ShrinkPage		0x00000004
#define XFA_PRINTOPT_AsImage		0x00000008
#define XFA_PRINTOPT_ReverseOrder	0x00000010
#define XFA_PRINTOPT_PrintAnnot		0x00000020
#define  XFA_PAGEVIEWEVENT_PostAdded		1
#define  XFA_PAGEVIEWEVENT_PostRemoved		3
#define  XFA_WIDGETEVENT_PostAdded			2
#define  XFA_WIDGETEVENT_PreRemoved			3
#define  XFA_WIDGETEVENT_PostContentChanged	6
#define  XFA_WIDGETEVENT_ListItemRemoved	7
#define  XFA_WIDGETEVENT_ListItemAdded  	8
#define  XFA_WIDGETEVENT_AccessChanged  	9
class IXFA_DocProvider
{
public:
    virtual void		SetChangeMark(XFA_HDOC hDoc) = 0;
    virtual void		InvalidateRect(IXFA_PageView* pPageView, const CFX_RectF& rt, FX_DWORD dwFlags = 0) = 0;
    virtual void		DisplayCaret(XFA_HWIDGET hWidget, FX_BOOL bVisible, const CFX_RectF* pRtAnchor) = 0;
    virtual FX_BOOL		GetPopupPos(XFA_HWIDGET hWidget, FX_FLOAT fMinPopup, FX_FLOAT fMaxPopup,
                                    const CFX_RectF &rtAnchor, CFX_RectF &rtPopup) = 0;
    virtual FX_BOOL		PopupMenu(XFA_HWIDGET hWidget, CFX_PointF ptPopup, const CFX_RectF* pRectExclude = NULL) = 0;
    virtual void		PageViewEvent(IXFA_PageView* pPageView, FX_DWORD dwFlags) = 0;
    virtual void		WidgetEvent(XFA_HWIDGET hWidget, CXFA_WidgetAcc* pWidgetData, FX_DWORD dwEvent, FX_LPVOID pParam = NULL, FX_LPVOID pAdditional = NULL) = 0;
    virtual FX_BOOL		RenderCustomWidget(XFA_HWIDGET hWidget, CFX_Graphics* pGS, CFX_Matrix* pMatrix, const CFX_RectF& rtUI)
    {
        return FALSE;
    }
    virtual	FX_INT32	CountPages(XFA_HDOC hDoc) = 0;
    virtual	FX_INT32	GetCurrentPage(XFA_HDOC hDoc) = 0;
    virtual void		SetCurrentPage(XFA_HDOC hDoc, FX_INT32 iCurPage) = 0;
    virtual FX_BOOL		IsCalculationsEnabled(XFA_HDOC hDoc) = 0;
    virtual void		SetCalculationsEnabled(XFA_HDOC hDoc, FX_BOOL bEnabled) = 0;
    virtual void		GetTitle(XFA_HDOC hDoc, CFX_WideString &wsTitle) = 0;
    virtual void		SetTitle(XFA_HDOC hDoc, FX_WSTR wsTitle) = 0;
    virtual void		ExportData(XFA_HDOC hDoc, FX_WSTR wsFilePath, FX_BOOL bXDP = TRUE) = 0;
    virtual void		ImportData(XFA_HDOC hDoc, FX_WSTR wsFilePath) = 0;
    virtual void		GotoURL(XFA_HDOC hDoc, FX_WSTR bsURL, FX_BOOL bAppend = TRUE) = 0;
    virtual FX_BOOL		IsValidationsEnabled(XFA_HDOC hDoc) = 0;
    virtual void		SetValidationsEnabled(XFA_HDOC hDoc, FX_BOOL bEnabled) = 0;
    virtual void		SetFocusWidget(XFA_HDOC hDoc, XFA_HWIDGET hWidget) = 0;
    virtual void		Print(XFA_HDOC hDoc, FX_INT32 nStartPage, FX_INT32 nEndPage, FX_DWORD dwOptions) = 0;
    virtual FX_INT32			AbsPageCountInBatch(XFA_HDOC hDoc) = 0;
    virtual FX_INT32			AbsPageInBatch(XFA_HDOC hDoc, XFA_HWIDGET hWidget) = 0;
    virtual FX_INT32			SheetCountInBatch(XFA_HDOC hDoc) = 0;
    virtual FX_INT32			SheetInBatch(XFA_HDOC hDoc, XFA_HWIDGET hWidget) = 0;
    virtual FX_INT32			Verify(XFA_HDOC hDoc, CXFA_Node* pSigNode, FX_BOOL bUsed = TRUE)
    {
        return 0;
    }
    virtual FX_BOOL				Sign(XFA_HDOC hDoc, CXFA_NodeList* pNodeList, FX_WSTR wsExpression, FX_WSTR wsXMLIdent, FX_WSTR wsValue = FX_WSTRC(L"open"), FX_BOOL bUsed = TRUE)
    {
        return 0;
    }
    virtual CXFA_NodeList*		Enumerate(XFA_HDOC hDoc)
    {
        return 0;
    }
    virtual FX_BOOL				Clear(XFA_HDOC hDoc, CXFA_Node* pSigNode, FX_BOOL bCleared = TRUE)
    {
        return 0;
    }
    virtual void		GetURL(XFA_HDOC hDoc, CFX_WideString &wsDocURL) = 0;
    virtual FX_ARGB		GetHighlightColor(XFA_HDOC hDoc) = 0;
    virtual void		AddDoRecord(XFA_HWIDGET hWidget) = 0;

    virtual FX_BOOL		SubmitData(XFA_HDOC hDoc, CXFA_Submit submit) = 0;
    virtual FX_BOOL		CheckWord(XFA_HDOC hDoc, FX_BSTR sWord) = 0;
    virtual FX_BOOL		GetSuggestWords(XFA_HDOC hDoc, FX_BSTR sWord, CFX_ByteStringArray& sSuggest) = 0;
    virtual FX_BOOL		GetPDFScriptObject(XFA_HDOC hDoc, FX_BSTR utf8Name, FXJSE_HVALUE hValue) = 0;
    virtual FX_BOOL		GetGlobalProperty(XFA_HDOC hDoc, FX_BSTR szPropName, FXJSE_HVALUE hValue) = 0;
    virtual FX_BOOL		SetGlobalProperty(XFA_HDOC hDoc, FX_BSTR szPropName, FXJSE_HVALUE hValue) = 0;
    virtual CPDF_Document*  OpenPDF(XFA_HDOC hDoc, IFX_FileRead* pFile, FX_BOOL bTakeOverFile) = 0;
    virtual IFX_FileRead*	OpenLinkedFile(XFA_HDOC hDoc, const CFX_WideString& wsLink) = 0;
};
#define XFA_DOCVIEW_View		0x00000000
#define XFA_DOCVIEW_MasterPage	0x00000001
#define XFA_DOCVIEW_Design		0x00000002
#define XFA_DOCTYPE_Dynamic		0
#define XFA_DOCTYPE_Static		1
#define XFA_DOCTYPE_XDP			2
#define XFA_PARSESTATUS_StatusErr	-3
#define XFA_PARSESTATUS_StreamErr	-2
#define XFA_PARSESTATUS_SyntaxErr	-1
#define XFA_PARSESTATUS_Ready		0
#define XFA_PARSESTATUS_Done		100
class IXFA_DocHandler
{
public:
    virtual void				ReleaseDoc(XFA_HDOC hDoc) = 0;
    virtual IXFA_DocProvider*	GetDocProvider(XFA_HDOC hDoc) = 0;

    virtual FX_DWORD			GetDocType(XFA_HDOC hDoc) = 0;
    virtual	FX_INT32			StartLoad(XFA_HDOC hDoc) = 0;
    virtual FX_INT32			DoLoad(XFA_HDOC hDoc, IFX_Pause *pPause = NULL) = 0;
    virtual void				StopLoad(XFA_HDOC hDoc) = 0;

    virtual IXFA_DocView*		CreateDocView(XFA_HDOC hDoc, FX_DWORD dwView = 0) = 0;

    virtual FX_INT32			CountPackages(XFA_HDOC hDoc) = 0;
    virtual	void				GetPackageName(XFA_HDOC hDoc, FX_INT32 iPackage, CFX_WideStringC &wsPackage) = 0;

    virtual FX_BOOL				SavePackage(XFA_HDOC hDoc, FX_WSTR wsPackage, IFX_FileWrite* pFile, IXFA_ChecksumContext *pCSContext = NULL) = 0;
    virtual FX_BOOL				CloseDoc(XFA_HDOC hDoc) = 0;

    virtual FX_BOOL				ImportData(XFA_HDOC hDoc, IFX_FileRead* pStream, FX_BOOL bXDP = TRUE) = 0;
    virtual	void				SetJSERuntime(XFA_HDOC hDoc, FXJSE_HRUNTIME hRuntime) = 0;
    virtual FXJSE_HVALUE		GetXFAScriptObject(XFA_HDOC hDoc) = 0;
    virtual XFA_ATTRIBUTEENUM	GetRestoreState(XFA_HDOC hDoc) = 0;
    virtual FX_BOOL				RunDocScript(XFA_HDOC hDoc, XFA_SCRIPTTYPE eScriptType, FX_WSTR wsScript, FXJSE_HVALUE hRetValue, FXJSE_HVALUE hThisObject) = 0;
};
enum XFA_EVENTTYPE {
    XFA_EVENT_Click,
    XFA_EVENT_Change,
    XFA_EVENT_DocClose,
    XFA_EVENT_DocReady,
    XFA_EVENT_Enter,
    XFA_EVENT_Exit,
    XFA_EVENT_Full,
    XFA_EVENT_IndexChange,
    XFA_EVENT_Initialize,
    XFA_EVENT_MouseDown,
    XFA_EVENT_MouseEnter,
    XFA_EVENT_MouseExit,
    XFA_EVENT_MouseUp,
    XFA_EVENT_PostExecute,
    XFA_EVENT_PostOpen,
    XFA_EVENT_PostPrint,
    XFA_EVENT_PostSave,
    XFA_EVENT_PostSign,
    XFA_EVENT_PostSubmit,
    XFA_EVENT_PreExecute,
    XFA_EVENT_PreOpen,
    XFA_EVENT_PrePrint,
    XFA_EVENT_PreSave,
    XFA_EVENT_PreSign,
    XFA_EVENT_PreSubmit,
    XFA_EVENT_Ready,
    XFA_EVENT_InitCalculate,
    XFA_EVENT_InitVariables,
    XFA_EVENT_Calculate,
    XFA_EVENT_Validate,
    XFA_EVENT_Unknown,
};
#define XFA_VALIDATE_preSubmit		1
#define XFA_VALIDATE_prePrint		2
#define XFA_VALIDATE_preExecute		3
#define XFA_VALIDATE_preSave		4
class CXFA_EventParam : public CFX_Object
{
public:
    CXFA_EventParam()
    {
        m_pTarget = NULL;
        m_eType = XFA_EVENT_Unknown;
        m_wsResult.Empty();
        Reset();
    }
    void Reset()
    {
        m_wsChange.Empty();
        m_iCommitKey = 0;
        m_wsFullText.Empty();
        m_bKeyDown = FALSE;
        m_bModifier = FALSE;
        m_wsNewContentType.Empty();
        m_wsNewText.Empty();
        m_wsPrevContentType.Empty();
        m_wsPrevText.Empty();
        m_bReenter = FALSE;
        m_iSelEnd = 0;
        m_iSelStart = 0;
        m_bShift = FALSE;
        m_wsSoapFaultCode.Empty();
        m_wsSoapFaultString.Empty();
        m_bIsFormReady = FALSE;
        m_iValidateActivities = XFA_VALIDATE_preSubmit;
    }
    CXFA_WidgetAcc*	m_pTarget;
    XFA_EVENTTYPE	m_eType;
    CFX_WideString	m_wsResult;
    FX_BOOL			m_bCancelAction;
    FX_INT32		m_iCommitKey;
    FX_BOOL			m_bKeyDown;
    FX_BOOL			m_bModifier;
    FX_BOOL			m_bReenter;
    FX_INT32		m_iSelEnd;
    FX_INT32		m_iSelStart;
    FX_BOOL			m_bShift;
    CFX_WideString	m_wsChange;
    CFX_WideString	m_wsFullText;
    CFX_WideString	m_wsNewContentType;
    CFX_WideString	m_wsNewText;
    CFX_WideString	m_wsPrevContentType;
    CFX_WideString	m_wsPrevText;
    CFX_WideString	m_wsSoapFaultCode;
    CFX_WideString	m_wsSoapFaultString;
    FX_BOOL			m_bIsFormReady;
    FX_INT32		m_iValidateActivities;
};
#define XFA_EVENTERROR_Sucess		1
#define XFA_EVENTERROR_Error		-1
#define XFA_EVENTERROR_NotExist		0
#define XFA_EVENTERROR_Disabled		2
enum XFA_WIDGETORDER {
    XFA_WIDGETORDER_PreOrder,
};
class IXFA_DocView
{
public:
    virtual XFA_HDOC			GetDoc() = 0;
    virtual	FX_INT32			StartLayout(FX_INT32 iStartPage = 0) = 0;
    virtual FX_INT32			DoLayout(IFX_Pause *pPause = NULL) = 0;
    virtual void				StopLayout() = 0;

    virtual FX_INT32			GetLayoutStatus() = 0;
    virtual	void				UpdateDocView() = 0;
    virtual FX_INT32			CountPageViews() = 0;
    virtual IXFA_PageView*		GetPageView(FX_INT32 nIndex) = 0;
    virtual XFA_HWIDGET			GetWidgetByName(FX_WSTR wsName) = 0;
    virtual CXFA_WidgetAcc*		GetWidgetAccByName(FX_WSTR wsName) = 0;
    virtual void				ResetWidgetData(CXFA_WidgetAcc* pWidgetAcc = NULL) = 0;
    virtual FX_INT32			ProcessWidgetEvent(CXFA_EventParam* pParam, CXFA_WidgetAcc* pWidgetAcc = NULL) = 0;
    virtual IXFA_WidgetHandler*	GetWidgetHandler() = 0;
    virtual IXFA_WidgetIterator*	CreateWidgetIterator() = 0;
    virtual IXFA_WidgetAccIterator* CreateWidgetAccIterator(XFA_WIDGETORDER eOrder = XFA_WIDGETORDER_PreOrder) = 0;
    virtual XFA_HWIDGET			GetFocusWidget() = 0;
    virtual void				KillFocus() = 0;
    virtual FX_BOOL				SetFocus(XFA_HWIDGET hWidget) = 0;
};
#define XFA_TRAVERSEWAY_Tranvalse		0x0001
#define XFA_TRAVERSEWAY_Form			0x0002
#define XFA_WIDGETFILTER_Visible		0x0001
#define XFA_WIDGETFILTER_Viewable		0x0010
#define XFA_WIDGETFILTER_Printable		0x0020
#define XFA_WIDGETFILTER_Field			0x0100
#define XFA_WIDGETFILTER_AllType		0x0F00
class IXFA_PageView
{
public:
    virtual IXFA_DocView*	GetDocView() = 0;
    virtual FX_INT32		GetPageViewIndex() = 0;
    virtual void			GetPageViewRect(CFX_RectF &rtPage) = 0;

    virtual void			GetDisplayMatrix(CFX_Matrix &mt, const CFX_Rect &rtDisp, FX_INT32 iRotate) = 0;

    virtual FX_INT32		LoadPageView(IFX_Pause *pPause = NULL) = 0;
    virtual void			UnloadPageView() = 0;
    virtual XFA_HWIDGET		GetWidgetByPos(FX_FLOAT fx, FX_FLOAT fy) = 0;
    virtual IXFA_WidgetIterator* CreateWidgetIterator(FX_DWORD dwTraverseWay = XFA_TRAVERSEWAY_Form, FX_DWORD dwWidgetFilter = XFA_WIDGETFILTER_Visible | XFA_WIDGETFILTER_Viewable | XFA_WIDGETFILTER_AllType) = 0;
};
class CXFA_RenderOptions : public CFX_Object
{
public:
    CXFA_RenderOptions()
        : m_bPrint(FALSE)
        , m_bHighlight(TRUE)
    {
    }
    FX_BOOL			m_bPrint;
    FX_BOOL			m_bHighlight;
};
#define XFA_RENDERSTATUS_Ready			1
#define XFA_RENDERSTATUS_ToBeContinued	2
#define XFA_RENDERSTATUS_Done			3
#define XFA_RENDERSTATUS_Failed			-1
class IXFA_RenderContext
{
public:
    virtual void		Release() = 0;
    virtual FX_INT32	StartRender(IXFA_PageView* pPageView, CFX_Graphics* pGS, const CFX_Matrix& pMatrix, const CXFA_RenderOptions& options) = 0;
    virtual FX_INT32	DoRender(IFX_Pause* pPause = NULL) = 0;
    virtual	void		StopRender() = 0;
};
IXFA_RenderContext*	XFA_RenderContext_Create();
enum XFA_WIDGETTYPE {
    XFA_WIDGETTYPE_Barcode,
    XFA_WIDGETTYPE_PushButton,
    XFA_WIDGETTYPE_CheckButton,
    XFA_WIDGETTYPE_RadioButton,
    XFA_WIDGETTYPE_DatetimeEdit,
    XFA_WIDGETTYPE_DecimalField,
    XFA_WIDGETTYPE_NumericField,
    XFA_WIDGETTYPE_Signature,
    XFA_WIDGETTYPE_TextEdit,
    XFA_WIDGETTYPE_DropdownList,
    XFA_WIDGETTYPE_ListBox,
    XFA_WIDGETTYPE_ImageField,
    XFA_WIDGETTYPE_PasswordEdit,
    XFA_WIDGETTYPE_Arc,
    XFA_WIDGETTYPE_Rectangle,
    XFA_WIDGETTYPE_Image,
    XFA_WIDGETTYPE_Line,
    XFA_WIDGETTYPE_Text,
    XFA_WIDGETTYPE_ExcludeGroup,
    XFA_WIDGETTYPE_Subform,
    XFA_WIDGETTYPE_Unknown,
};
#define XFA_WIDGETSTATUS_Visible	0x00000001
#define XFA_WIDGETSTATUS_Invisible	0x00000002
#define XFA_WIDGETSTATUS_Hidden		0x00000004
#define XFA_WIDGETSTATUS_Viewable	0x00000010
#define XFA_WIDGETSTATUS_Printable	0x00000020
#define XFA_WIDGETSTATUS_Focused	0x00000100
class IXFA_WidgetHandler
{
public:

    virtual XFA_HWIDGET		CreateWidget(XFA_HWIDGET hParent, XFA_WIDGETTYPE eType, XFA_HWIDGET hBefore = NULL) = 0;
    virtual IXFA_PageView*	GetPageView(XFA_HWIDGET hWidget) = 0;
    virtual void			GetRect(XFA_HWIDGET hWidget, CFX_RectF &rt) = 0;
    virtual FX_DWORD		GetStatus(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL			GetBBox(XFA_HWIDGET hWidget, CFX_RectF &rtBox, FX_DWORD dwStatus, FX_BOOL bDrawFocus = FALSE) = 0;
    virtual CXFA_WidgetAcc*	GetDataAcc(XFA_HWIDGET hWidget) = 0;

    virtual void			GetName(XFA_HWIDGET hWidget, CFX_WideString &wsName, FX_INT32 iNameType = 0) = 0;
    virtual	FX_BOOL			GetToolTip(XFA_HWIDGET hWidget, CFX_WideString &wsToolTip) = 0;
    virtual	void			SetPrivateData(XFA_HWIDGET hWidget, FX_LPVOID module_id, FX_LPVOID pData, PD_CALLBACK_FREEDATA callback) = 0;
    virtual	FX_LPVOID		GetPrivateData(XFA_HWIDGET hWidget, FX_LPVOID module_id) = 0;
    virtual FX_BOOL			OnMouseEnter(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL			OnMouseExit(XFA_HWIDGET hWidget) = 0;
    virtual FX_BOOL			OnLButtonDown(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy) = 0;
    virtual FX_BOOL			OnLButtonUp(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy) = 0;
    virtual FX_BOOL			OnLButtonDblClk(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy) = 0;
    virtual FX_BOOL			OnMouseMove(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy) = 0;
    virtual FX_BOOL			OnMouseWheel(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_SHORT zDelta, FX_FLOAT fx, FX_FLOAT fy) = 0;
    virtual FX_BOOL			OnRButtonDown(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy) = 0;
    virtual FX_BOOL			OnRButtonUp(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy) = 0;
    virtual FX_BOOL			OnRButtonDblClk(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy) = 0;

    virtual FX_BOOL			OnKeyDown(XFA_HWIDGET hWidget, FX_DWORD dwKeyCode, FX_DWORD dwFlags) = 0;
    virtual FX_BOOL			OnKeyUp(XFA_HWIDGET hWidget, FX_DWORD dwKeyCode, FX_DWORD dwFlags) = 0;
    virtual FX_BOOL			OnChar(XFA_HWIDGET hWidget, FX_DWORD dwChar, FX_DWORD dwFlags) = 0;
    virtual	FX_DWORD		OnHitTest(XFA_HWIDGET hWidget, FX_FLOAT fx, FX_FLOAT fy) = 0;
    virtual FX_BOOL			OnSetCursor(XFA_HWIDGET hWidget, FX_FLOAT fx, FX_FLOAT fy) = 0;

    virtual void			RenderWidget(XFA_HWIDGET hWidget, CFX_Graphics* pGS, CFX_Matrix* pMatrix = NULL, FX_BOOL bHighlight = FALSE) = 0;
    virtual FX_BOOL			HasEvent(CXFA_WidgetAcc* pWidgetAcc, XFA_EVENTTYPE eEventType) = 0;
    virtual FX_INT32		ProcessEvent(CXFA_WidgetAcc* pWidgetAcc, CXFA_EventParam* pParam) = 0;
};
class IXFA_WidgetIterator
{
public:
    virtual void				Release() = 0;
    virtual void				Reset() = 0;
    virtual XFA_HWIDGET			MoveToFirst() = 0;
    virtual XFA_HWIDGET			MoveToLast() = 0;
    virtual	XFA_HWIDGET			MoveToNext() = 0;
    virtual XFA_HWIDGET			MoveToPrevious() = 0;
    virtual	XFA_HWIDGET			GetCurrentWidget() = 0;
    virtual	FX_BOOL				SetCurrentWidget(XFA_HWIDGET hWidget)  = 0;
};
class IXFA_WidgetAccIterator
{
public:
    virtual void				Release() = 0;

    virtual void				Reset() = 0;
    virtual CXFA_WidgetAcc*		MoveToFirst() = 0;
    virtual CXFA_WidgetAcc*		MoveToLast() = 0;
    virtual CXFA_WidgetAcc*		MoveToNext() = 0;
    virtual CXFA_WidgetAcc*		MoveToPrevious() = 0;
    virtual CXFA_WidgetAcc*		GetCurrentWidgetAcc() = 0;
    virtual FX_BOOL				SetCurrentWidgetAcc(CXFA_WidgetAcc* hWidget) = 0;
    virtual void				SkipTree() = 0;
};
IXFA_WidgetAccIterator* XFA_WidgetAccIterator_Create(CXFA_WidgetAcc* pTravelRoot, XFA_WIDGETORDER eOrder = XFA_WIDGETORDER_PreOrder);
class IXFA_ChecksumContext
{
public:
    virtual void				Release() = 0;

    virtual FX_BOOL				StartChecksum() = 0;
    virtual FX_BOOL				UpdateChecksum(IFX_FileRead* pSrcFile, FX_FILESIZE offset = 0, size_t size = 0) = 0;
    virtual void				FinishChecksum() = 0;
    virtual void				GetChecksum(CFX_ByteString &bsChecksum) = 0;
};
IXFA_ChecksumContext*	XFA_Checksum_Create();
#endif
