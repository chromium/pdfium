// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_INCLUDE_FXFA_H_
#define XFA_FXFA_INCLUDE_FXFA_H_

#include <vector>

#include "xfa/fxfa/include/fxfa_basic.h"
#include "xfa/fxfa/include/fxfa_widget.h"

class CFX_Graphics;
class CPDF_Document;
class CXFA_FFPageView;
class CXFA_Node;
class CXFA_NodeList;
class CXFA_WidgetAcc;
class IFWL_AdapterTimerMgr;
class IFX_Font;
class IXFA_AppProvider;
class IXFA_DocProvider;
class IXFA_WidgetAccIterator;
class IXFA_WidgetIterator;

#define XFA_MBICON_Error 0
#define XFA_MBICON_Warning 1
#define XFA_MBICON_Question 2
#define XFA_MBICON_Status 3
#define XFA_MB_OK 0
#define XFA_MB_OKCancel 1
#define XFA_MB_YesNo 2
#define XFA_MB_YesNoCancel 3
#define XFA_IDOK 1
#define XFA_IDCancel 2
#define XFA_IDNo 3
#define XFA_IDYes 4
#define XFA_IDS_ValidateFailed 1
#define XFA_IDS_CalcOverride 2
#define XFA_IDS_ModifyField 3
#define XFA_IDS_NotModifyField 4
#define XFA_IDS_AppName 5
#define XFA_IDS_ImageFilter 6
#define XFA_IDS_UNKNOW_CATCHED 7
#define XFA_IDS_Unable_TO_SET 8
#define XFA_IDS_VALUE_EXCALMATORY 9
#define XFA_IDS_INVALID_ENUM_VALUE 10
#define XFA_IDS_UNSUPPORT_METHOD 11
#define XFA_IDS_UNSUPPORT_PROP 12
#define XFA_IDS_INVAlID_PROP_SET 13
#define XFA_IDS_NOT_DEFAUL_VALUE 14
#define XFA_IDS_UNABLE_SET_LANGUAGE 15
#define XFA_IDS_UNABLE_SET_NUMPAGES 16
#define XFA_IDS_UNABLE_SET_PLATFORM 17
#define XFA_IDS_UNABLE_SET_VALIDATIONENABLE 18
#define XFA_IDS_UNABLE_SET_VARIATION 19
#define XFA_IDS_UNABLE_SET_VERSION 20
#define XFA_IDS_UNABLE_SET_READY 21
#define XFA_IDS_NUMBER_OF_OCCUR 22
#define XFA_IDS_UNABLE_SET_CLASS_NAME 23
#define XFA_IDS_UNABLE_SET_LENGTH_VALUE 24
#define XFA_IDS_UNSUPPORT_CHAR 25
#define XFA_IDS_BAD_SUFFIX 26
#define XFA_IDS_EXPECTED_IDENT 27
#define XFA_IDS_EXPECTED_STRING 28
#define XFA_IDS_INVALIDATE_CHAR 29
#define XFA_IDS_REDEFINITION 30
#define XFA_IDS_INVALIDATE_TOKEN 31
#define XFA_IDS_INVALIDATE_EXPRESSION 32
#define XFA_IDS_UNDEFINE_IDENTIFIER 33
#define XFA_IDS_INVALIDATE_LEFTVALUE 34
#define XFA_IDS_COMPILER_ERROR 35
#define XFA_IDS_CANNOT_MODIFY_VALUE 36
#define XFA_IDS_ERROR_PARAMETERS 37
#define XFA_IDS_EXPECT_ENDIF 38
#define XFA_IDS_UNEXPECTED_EXPRESSION 39
#define XFA_IDS_CONDITION_IS_NULL 40
#define XFA_IDS_ILLEGALBREAK 41
#define XFA_IDS_ILLEGALCONTINUE 42
#define XFA_IDS_EXPECTED_OPERATOR 43
#define XFA_IDS_DIVIDE_ZERO 44
#define XFA_IDS_CANNOT_COVERT_OBJECT 45
#define XFA_IDS_NOT_FOUND_CONTAINER 46
#define XFA_IDS_NOT_FOUND_PROPERTY 47
#define XFA_IDS_NOT_FOUND_METHOD 48
#define XFA_IDS_NOT_FOUND_CONST 49
#define XFA_IDS_NOT_ASSIGN_OBJECT 50
#define XFA_IDS_IVALIDATE_INSTRUCTION 51
#define XFA_IDS_EXPECT_NUMBER 52
#define XFA_IDS_VALIDATE_OUT_ARRAY 53
#define XFA_IDS_CANNOT_ASSIGN_IDENT 54
#define XFA_IDS_NOT_FOUNT_FUNCTION 55
#define XFA_IDS_NOT_ARRAY 56
#define XFA_IDS_OUT_ARRAY 57
#define XFA_IDS_NOT_SUPPORT_CALC 58
#define XFA_IDS_ARGUMENT_NOT_ARRAY 59
#define XFA_IDS_ARGUMENT_EXPECT_CONTAINER 60
#define XFA_IDS_ACCESS_PROPERTY_IN_NOT_OBJECT 61
#define XFA_IDS_FUNCTION_IS_BUILDIN 62
#define XFA_IDS_ERROR_MSG 63
#define XFA_IDS_INDEX_OUT_OF_BOUNDS 64
#define XFA_IDS_INCORRECT_NUMBER_OF_METHOD 65
#define XFA_IDS_ARGUMENT_MISMATCH 66
#define XFA_IDS_INVALID_ENUMERATE 67
#define XFA_IDS_INVALID_APPEND 68
#define XFA_IDS_SOM_EXPECTED_LIST 69
#define XFA_IDS_NOT_HAVE_PROPERTY 70
#define XFA_IDS_INVALID_NODE_TYPE 71
#define XFA_IDS_VIOLATE_BOUNDARY 72
#define XFA_IDS_SERVER_DENY 73
#define XFA_IDS_StringWeekDay_Sun 74
#define XFA_IDS_StringWeekDay_Mon 75
#define XFA_IDS_StringWeekDay_Tue 76
#define XFA_IDS_StringWeekDay_Wed 77
#define XFA_IDS_StringWeekDay_Thu 78
#define XFA_IDS_StringWeekDay_Fri 79
#define XFA_IDS_StringWeekDay_Sat 80
#define XFA_IDS_StringMonth_Jan 81
#define XFA_IDS_StringMonth_Feb 82
#define XFA_IDS_StringMonth_March 83
#define XFA_IDS_StringMonth_April 84
#define XFA_IDS_StringMonth_May 85
#define XFA_IDS_StringMonth_June 86
#define XFA_IDS_StringMonth_July 87
#define XFA_IDS_StringMonth_Aug 88
#define XFA_IDS_StringMonth_Sept 89
#define XFA_IDS_StringMonth_Oct 90
#define XFA_IDS_StringMonth_Nov 91
#define XFA_IDS_StringMonth_Dec 92
#define XFA_IDS_String_Today 93
#define XFA_IDS_ValidateLimit 94
#define XFA_IDS_ValidateNullWarning 95
#define XFA_IDS_ValidateNullError 96
#define XFA_IDS_ValidateWarning 97
#define XFA_IDS_ValidateError 98
#define XFA_IDS_ValidateNumberError 99

#define XFA_DOCVIEW_View 0x00000000
#define XFA_DOCVIEW_MasterPage 0x00000001
#define XFA_DOCVIEW_Design 0x00000002
#define XFA_DOCTYPE_Dynamic 0
#define XFA_DOCTYPE_Static 1
#define XFA_DOCTYPE_XDP 2
#define XFA_PARSESTATUS_StatusErr -3
#define XFA_PARSESTATUS_StreamErr -2
#define XFA_PARSESTATUS_SyntaxErr -1
#define XFA_PARSESTATUS_Ready 0
#define XFA_PARSESTATUS_Done 100
#define XFA_VALIDATE_preSubmit 1
#define XFA_VALIDATE_prePrint 2
#define XFA_VALIDATE_preExecute 3
#define XFA_VALIDATE_preSave 4

#define XFA_INVALIDATE_AllPages 0x00000000
#define XFA_INVALIDATE_CurrentPage 0x00000001
#define XFA_PRINTOPT_ShowDialog 0x00000001
#define XFA_PRINTOPT_CanCancel 0x00000002
#define XFA_PRINTOPT_ShrinkPage 0x00000004
#define XFA_PRINTOPT_AsImage 0x00000008
#define XFA_PRINTOPT_ReverseOrder 0x00000010
#define XFA_PRINTOPT_PrintAnnot 0x00000020
#define XFA_PAGEVIEWEVENT_PostAdded 1
#define XFA_PAGEVIEWEVENT_PostRemoved 3
#define XFA_PAGEVIEWEVENT_StopLayout 4

#define XFA_EVENTERROR_Success 1
#define XFA_EVENTERROR_Error -1
#define XFA_EVENTERROR_NotExist 0
#define XFA_EVENTERROR_Disabled 2

#define XFA_RENDERSTATUS_Ready 1
#define XFA_RENDERSTATUS_ToBeContinued 2
#define XFA_RENDERSTATUS_Done 3
#define XFA_RENDERSTATUS_Failed -1

#define XFA_TRAVERSEWAY_Tranvalse 0x0001
#define XFA_TRAVERSEWAY_Form 0x0002

enum XFA_WidgetStatus {
  XFA_WidgetStatus_None = 0,

  XFA_WidgetStatus_Access = 1 << 0,
  XFA_WidgetStatus_ButtonDown = 1 << 1,
  XFA_WidgetStatus_Disabled = 1 << 2,
  XFA_WidgetStatus_Focused = 1 << 3,
  XFA_WidgetStatus_Highlight = 1 << 4,
  XFA_WidgetStatus_Printable = 1 << 5,
  XFA_WidgetStatus_RectCached = 1 << 6,
  XFA_WidgetStatus_TextEditValueChanged = 1 << 7,
  XFA_WidgetStatus_Viewable = 1 << 8,
  XFA_WidgetStatus_Visible = 1 << 9
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

enum XFA_WIDGETORDER {
  XFA_WIDGETORDER_PreOrder,
};

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

// Probably should be called IXFA_AppDelegate.
class IXFA_AppProvider {
 public:
  virtual ~IXFA_AppProvider() {}

  /**
   * Specifies the name of the client application in which a form currently
   * exists. Such as Exchange-Pro.
   */
  virtual void SetAppType(const CFX_WideStringC& wsAppType) = 0;
  virtual void GetAppType(CFX_WideString& wsAppType) = 0;
  virtual void SetFoxitAppType(const CFX_WideStringC& wsFoxitAppType) {}
  virtual void GetFoxitAppType(CFX_WideString& wsFoxitAppType) {
    wsFoxitAppType.clear();
  }

  /**
   * Returns the language of the running host application. Such as zh_CN
   */
  virtual void GetLanguage(CFX_WideString& wsLanguage) = 0;

  /**
   * Returns the platform of the machine running the script. Such as WIN
   */
  virtual void GetPlatform(CFX_WideString& wsPlatform) = 0;

  /**
   * Indicates the packaging of the application that is running the script. Such
   * as Full
   */
  virtual void GetVariation(CFX_WideString& wsVariation) = 0;

  /**
   * Indicates the version number of the current application. Such as 9
   */
  virtual void GetVersion(CFX_WideString& wsVersion) = 0;
  virtual void GetFoxitVersion(CFX_WideString& wsFoxitVersion) {
    wsFoxitVersion.clear();
  }

  /**
   * Get application name, such as Phantom.
   */
  virtual void GetAppName(CFX_WideString& wsName) = 0;
  virtual void GetFoxitAppName(CFX_WideString& wsFoxitName) {
    wsFoxitName.clear();
  }

  /**
   * Causes the system to play a sound.
   * @param[in] dwType The system code for the appropriate sound.0 (Error)1
   * (Warning)2 (Question)3 (Status)4 (Default)
   */
  virtual void Beep(uint32_t dwType) = 0;

  /**
   * Displays a message box.
   * @param[in] wsMessage    - Message string to display in box.
   * @param[in] wsTitle      - Title string for box.
   * @param[in] dwIconType   - Icon type, refer to XFA_MBICON.
   * @param[in] dwButtonType - Button type, refer to XFA_MESSAGEBUTTON.
   * @return A valid integer representing the value of the button pressed by the
   * user, refer to XFA_ID.
   */
  virtual int32_t MsgBox(const CFX_WideString& wsMessage,
                         const CFX_WideString& wsTitle = L"",
                         uint32_t dwIconType = 0,
                         uint32_t dwButtonType = 0) = 0;

  /**
   * Get a response from the user.
   * @param[in] wsQuestion      - Message string to display in box.
   * @param[in] wsTitle         - Title string for box.
   * @param[in] wsDefaultAnswer - Initial contents for answer.
   * @param[in] bMask           - Mask the user input with asterisks when true,
   * @return A string containing the user's response.
   */
  virtual CFX_WideString Response(const CFX_WideString& wsQuestion,
                                  const CFX_WideString& wsTitle = L"",
                                  const CFX_WideString& wsDefaultAnswer = L"",
                                  FX_BOOL bMask = TRUE) = 0;

  virtual int32_t GetDocumentCountInBatch() = 0;
  virtual int32_t GetCurDocumentInBatch() = 0;

  /**
   * Download something from somewhere.
   * @param[in] wsURL - http, ftp, such as
   * "http://www.w3.org/TR/REC-xml-names/".
   */
  virtual IFX_FileRead* DownloadURL(const CFX_WideString& wsURL) = 0;

  /**
   * POST data to the given url.
   * @param[in] wsURL         the URL being uploaded.
   * @param[in] wsData        the data being uploaded.
   * @param[in] wsContentType the content type of data including text/html,
   * text/xml, text/plain, multipart/form-data,
   *                          application/x-www-form-urlencoded,
   * application/octet-stream, any valid MIME type.
   * @param[in] wsEncode      the encode of data including UTF-8, UTF-16,
   * ISO8859-1, any recognized [IANA]character encoding
   * @param[in] wsHeader      any additional HTTP headers to be included in the
   * post.
   * @param[out] wsResponse   decoded response from server.
   * @return TRUE Server permitted the post request, FALSE otherwise.
   */
  virtual FX_BOOL PostRequestURL(const CFX_WideString& wsURL,
                                 const CFX_WideString& wsData,
                                 const CFX_WideString& wsContentType,
                                 const CFX_WideString& wsEncode,
                                 const CFX_WideString& wsHeader,
                                 CFX_WideString& wsResponse) = 0;

  /**
   * PUT data to the given url.
   * @param[in] wsURL         the URL being uploaded.
   * @param[in] wsData            the data being uploaded.
   * @param[in] wsEncode      the encode of data including UTF-8, UTF-16,
   * ISO8859-1, any recognized [IANA]character encoding
   * @return TRUE Server permitted the post request, FALSE otherwise.
   */
  virtual FX_BOOL PutRequestURL(const CFX_WideString& wsURL,
                                const CFX_WideString& wsData,
                                const CFX_WideString& wsEncode) = 0;

  virtual void LoadString(int32_t iStringID, CFX_WideString& wsString) = 0;
  virtual IFWL_AdapterTimerMgr* GetTimerMgr() = 0;
};

class IXFA_DocProvider {
 public:
  virtual ~IXFA_DocProvider() {}

  virtual void SetChangeMark(CXFA_FFDoc* hDoc) = 0;
  virtual void InvalidateRect(CXFA_FFPageView* pPageView,
                              const CFX_RectF& rt,
                              uint32_t dwFlags = 0) = 0;
  virtual void DisplayCaret(CXFA_FFWidget* hWidget,
                            FX_BOOL bVisible,
                            const CFX_RectF* pRtAnchor) = 0;
  virtual FX_BOOL GetPopupPos(CXFA_FFWidget* hWidget,
                              FX_FLOAT fMinPopup,
                              FX_FLOAT fMaxPopup,
                              const CFX_RectF& rtAnchor,
                              CFX_RectF& rtPopup) = 0;
  virtual FX_BOOL PopupMenu(CXFA_FFWidget* hWidget,
                            CFX_PointF ptPopup,
                            const CFX_RectF* pRectExclude = NULL) = 0;
  virtual void PageViewEvent(CXFA_FFPageView* pPageView, uint32_t dwFlags) = 0;
  virtual void WidgetPostAdd(CXFA_FFWidget* hWidget,
                             CXFA_WidgetAcc* pWidgetData) = 0;
  virtual void WidgetPreRemove(CXFA_FFWidget* hWidget,
                               CXFA_WidgetAcc* pWidgetData) = 0;
  virtual FX_BOOL RenderCustomWidget(CXFA_FFWidget* hWidget,
                                     CFX_Graphics* pGS,
                                     CFX_Matrix* pMatrix,
                                     const CFX_RectF& rtUI) {
    return FALSE;
  }
  virtual int32_t CountPages(CXFA_FFDoc* hDoc) = 0;
  virtual int32_t GetCurrentPage(CXFA_FFDoc* hDoc) = 0;
  virtual void SetCurrentPage(CXFA_FFDoc* hDoc, int32_t iCurPage) = 0;
  virtual FX_BOOL IsCalculationsEnabled(CXFA_FFDoc* hDoc) = 0;
  virtual void SetCalculationsEnabled(CXFA_FFDoc* hDoc, FX_BOOL bEnabled) = 0;
  virtual void GetTitle(CXFA_FFDoc* hDoc, CFX_WideString& wsTitle) = 0;
  virtual void SetTitle(CXFA_FFDoc* hDoc, const CFX_WideString& wsTitle) = 0;
  virtual void ExportData(CXFA_FFDoc* hDoc,
                          const CFX_WideString& wsFilePath,
                          FX_BOOL bXDP = TRUE) = 0;
  virtual void ImportData(CXFA_FFDoc* hDoc,
                          const CFX_WideString& wsFilePath) = 0;
  virtual void GotoURL(CXFA_FFDoc* hDoc,
                       const CFX_WideString& bsURL,
                       FX_BOOL bAppend = TRUE) = 0;
  virtual FX_BOOL IsValidationsEnabled(CXFA_FFDoc* hDoc) = 0;
  virtual void SetValidationsEnabled(CXFA_FFDoc* hDoc, FX_BOOL bEnabled) = 0;
  virtual void SetFocusWidget(CXFA_FFDoc* hDoc, CXFA_FFWidget* hWidget) = 0;
  virtual void Print(CXFA_FFDoc* hDoc,
                     int32_t nStartPage,
                     int32_t nEndPage,
                     uint32_t dwOptions) = 0;
  virtual int32_t AbsPageCountInBatch(CXFA_FFDoc* hDoc) = 0;
  virtual int32_t AbsPageInBatch(CXFA_FFDoc* hDoc, CXFA_FFWidget* hWidget) = 0;
  virtual int32_t SheetCountInBatch(CXFA_FFDoc* hDoc) = 0;
  virtual int32_t SheetInBatch(CXFA_FFDoc* hDoc, CXFA_FFWidget* hWidget) = 0;
  virtual int32_t Verify(CXFA_FFDoc* hDoc,
                         CXFA_Node* pSigNode,
                         FX_BOOL bUsed = TRUE) {
    return 0;
  }
  virtual FX_BOOL Sign(CXFA_FFDoc* hDoc,
                       CXFA_NodeList* pNodeList,
                       const CFX_WideStringC& wsExpression,
                       const CFX_WideStringC& wsXMLIdent,
                       const CFX_WideStringC& wsValue = FX_WSTRC(L"open"),
                       FX_BOOL bUsed = TRUE) {
    return 0;
  }
  virtual CXFA_NodeList* Enumerate(CXFA_FFDoc* hDoc) { return 0; }
  virtual FX_BOOL Clear(CXFA_FFDoc* hDoc,
                        CXFA_Node* pSigNode,
                        FX_BOOL bCleared = TRUE) {
    return 0;
  }
  virtual void GetURL(CXFA_FFDoc* hDoc, CFX_WideString& wsDocURL) = 0;
  virtual FX_ARGB GetHighlightColor(CXFA_FFDoc* hDoc) = 0;

  virtual FX_BOOL SubmitData(CXFA_FFDoc* hDoc, CXFA_Submit submit) = 0;
  virtual FX_BOOL CheckWord(CXFA_FFDoc* hDoc, const CFX_ByteStringC& sWord) = 0;
  virtual FX_BOOL GetSuggestWords(CXFA_FFDoc* hDoc,
                                  const CFX_ByteStringC& sWord,
                                  std::vector<CFX_ByteString>& sSuggest) = 0;
  virtual FX_BOOL GetPDFScriptObject(CXFA_FFDoc* hDoc,
                                     const CFX_ByteStringC& utf8Name,
                                     FXJSE_HVALUE hValue) = 0;
  virtual FX_BOOL GetGlobalProperty(CXFA_FFDoc* hDoc,
                                    const CFX_ByteStringC& szPropName,
                                    FXJSE_HVALUE hValue) = 0;
  virtual FX_BOOL SetGlobalProperty(CXFA_FFDoc* hDoc,
                                    const CFX_ByteStringC& szPropName,
                                    FXJSE_HVALUE hValue) = 0;
  virtual CPDF_Document* OpenPDF(CXFA_FFDoc* hDoc,
                                 IFX_FileRead* pFile,
                                 FX_BOOL bTakeOverFile) = 0;
  virtual IFX_FileRead* OpenLinkedFile(CXFA_FFDoc* hDoc,
                                       const CFX_WideString& wsLink) = 0;
};

class CXFA_EventParam {
 public:
  CXFA_EventParam() {
    m_pTarget = NULL;
    m_eType = XFA_EVENT_Unknown;
    m_wsResult.clear();
    Reset();
  }
  void Reset() {
    m_wsChange.clear();
    m_iCommitKey = 0;
    m_wsFullText.clear();
    m_bKeyDown = FALSE;
    m_bModifier = FALSE;
    m_wsNewContentType.clear();
    m_wsNewText.clear();
    m_wsPrevContentType.clear();
    m_wsPrevText.clear();
    m_bReenter = FALSE;
    m_iSelEnd = 0;
    m_iSelStart = 0;
    m_bShift = FALSE;
    m_wsSoapFaultCode.clear();
    m_wsSoapFaultString.clear();
    m_bIsFormReady = FALSE;
    m_iValidateActivities = XFA_VALIDATE_preSubmit;
  }
  CXFA_WidgetAcc* m_pTarget;
  XFA_EVENTTYPE m_eType;
  CFX_WideString m_wsResult;
  FX_BOOL m_bCancelAction;
  int32_t m_iCommitKey;
  FX_BOOL m_bKeyDown;
  FX_BOOL m_bModifier;
  FX_BOOL m_bReenter;
  int32_t m_iSelEnd;
  int32_t m_iSelStart;
  FX_BOOL m_bShift;
  CFX_WideString m_wsChange;
  CFX_WideString m_wsFullText;
  CFX_WideString m_wsNewContentType;
  CFX_WideString m_wsNewText;
  CFX_WideString m_wsPrevContentType;
  CFX_WideString m_wsPrevText;
  CFX_WideString m_wsSoapFaultCode;
  CFX_WideString m_wsSoapFaultString;
  FX_BOOL m_bIsFormReady;
  int32_t m_iValidateActivities;
};

class CXFA_RenderOptions {
 public:
  CXFA_RenderOptions() : m_bPrint(FALSE), m_bHighlight(TRUE) {}
  FX_BOOL m_bPrint;
  FX_BOOL m_bHighlight;
};

class IXFA_WidgetIterator {
 public:
  virtual ~IXFA_WidgetIterator() {}

  virtual void Reset() = 0;
  virtual CXFA_FFWidget* MoveToFirst() = 0;
  virtual CXFA_FFWidget* MoveToLast() = 0;
  virtual CXFA_FFWidget* MoveToNext() = 0;
  virtual CXFA_FFWidget* MoveToPrevious() = 0;
  virtual CXFA_FFWidget* GetCurrentWidget() = 0;
  virtual FX_BOOL SetCurrentWidget(CXFA_FFWidget* hWidget) = 0;
};

#endif  // XFA_FXFA_INCLUDE_FXFA_H_
