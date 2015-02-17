// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_



class PrintParamsObj : public CJS_EmbedObj
{
public:
	PrintParamsObj(CJS_Object* pJSObject);
	virtual ~PrintParamsObj(){}
	
public:
	FX_BOOL bUI;
	int nStart;
	int nEnd;
	FX_BOOL bSilent;
	FX_BOOL bShrinkToFit;
	FX_BOOL bPrintAsImage;
	FX_BOOL bReverse;
	FX_BOOL bAnnotations;
};

class CJS_PrintParamsObj : public CJS_Object
{
public:
	CJS_PrintParamsObj(JSFXObject pObject) : CJS_Object(pObject) {}
	virtual ~CJS_PrintParamsObj(){}
	
	DECLARE_JS_CLASS(CJS_PrintParamsObj);
};


class Icon;
class Field;

struct IconElement
{
	IconElement() : IconName(L""), NextIcon(NULL), IconStream(NULL) {}
	virtual ~IconElement()
	{
	}
	CFX_WideString	IconName;
	IconElement*	NextIcon;
	Icon*			IconStream;
};

class IconTree
{
public:
	IconTree():m_pHead(NULL), m_pEnd(NULL), m_iLength(0)
	{

	}

	virtual ~IconTree()
	{
	}

public:
	void			InsertIconElement(IconElement* pNewIcon);
	void			DeleteIconElement(CFX_WideString swIconName);
	void			DeleteIconTree();
	int				GetLength();
	IconElement*	operator[](int iIndex);

private:
	IconElement*	m_pHead;
	IconElement*	m_pEnd;
	int				m_iLength;
};

struct CJS_DelayData;
struct CJS_DelayAnnot;
struct CJS_AnnotObj;

class Document : public CJS_EmbedObj
{
public:
	Document(CJS_Object* pJSObject);
	virtual ~Document();

public:
	FX_BOOL	ADBE(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	author(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	baseURL(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	bookmarkRoot(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	calculate(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	Collab(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	creationDate(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	creator(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	delay(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	dirty(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	documentFileName(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL external(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	filesize(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	icons(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	info(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	keywords(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	layout(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	media(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	modDate(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	mouseX(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	mouseY(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	numFields(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	numPages(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	pageNum(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	pageWindowRect(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	path(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	producer(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	subject(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	title(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	zoom(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL	zoomType(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);

	FX_BOOL addAnnot(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	addField(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	addLink(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	addIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	calculateNow(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	closeDoc(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	createDataObject(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL deletePages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	exportAsText(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	exportAsFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	exportAsXFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL extractPages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getAnnot(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getAnnots(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getAnnot3D(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getAnnots3D(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getField(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getLinks(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getNthFieldName(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getOCGs(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getPageBox(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getPageNthWord(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getPageNthWordQuads(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	getPageNumWords(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL getPrintParams(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL getURL(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	importAnFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	importAnXFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	importTextData(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL insertPages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	mailForm(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	print(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	removeField(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL replacePages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	resetForm(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	saveAs(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	submitForm(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	mailDoc(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL	removeIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	
public:
	void AttachDoc(CPDFSDK_Document* pDoc);
	CPDFSDK_Document* GetReaderDoc();

	static FX_BOOL				ExtractFileName(CPDFSDK_Document* pDoc, CFX_ByteString& strFileName);
	static FX_BOOL				ExtractFolderName(CPDFSDK_Document* pDoc, CFX_ByteString& strFolderName);

public:
	void						AddDelayData(CJS_DelayData* pData);
	void						DoFieldDelay(const CFX_WideString& sFieldName, int nControlIndex);

	void						AddDelayAnnotData(CJS_AnnotObj *pData);
	void						DoAnnotDelay();
	void						SetIsolate(v8::Isolate* isolate) {m_isolate = isolate;}

private:
	CFX_WideString				ReversalStr(CFX_WideString cbFrom);
	CFX_WideString				CutString(CFX_WideString cbFrom);
	bool						IsEnclosedInRect(CFX_FloatRect rect, CFX_FloatRect LinkRect);
	int							CountWords(CPDF_TextObject* pTextObj);
	CFX_WideString				GetObjWordStr(CPDF_TextObject* pTextObj, int nWordIndex);

	FX_BOOL						ParserParams(JSObject *pObj,CJS_AnnotObj& annotobj);

private:
	v8::Isolate*					m_isolate;
	IconTree*					m_pIconTree;
	CPDFSDK_Document*			m_pDocument;
	CFX_WideString				m_cwBaseURL;

	FX_BOOL								m_bDelay;
	CFX_ArrayTemplate<CJS_DelayData*>	m_DelayData;
	CFX_ArrayTemplate<CJS_AnnotObj*>	m_DelayAnnotData;
};

class CJS_Document : public CJS_Object
{
public:
	CJS_Document(JSFXObject pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Document(){};

	virtual FX_BOOL	InitInstance(IFXJS_Context* cc);	

	DECLARE_JS_CLASS(CJS_Document);

	JS_STATIC_PROP(ADBE, Document);
	JS_STATIC_PROP(author, Document);
	JS_STATIC_PROP(baseURL, Document);
	JS_STATIC_PROP(bookmarkRoot, Document);
	JS_STATIC_PROP(calculate, Document);
	JS_STATIC_PROP(Collab, Document);
	JS_STATIC_PROP(creationDate, Document);
	JS_STATIC_PROP(creator, Document);
	JS_STATIC_PROP(delay, Document);
	JS_STATIC_PROP(dirty, Document);
	JS_STATIC_PROP(documentFileName, Document);
	JS_STATIC_PROP(external, Document);
	JS_STATIC_PROP(filesize, Document);
	JS_STATIC_PROP(icons, Document);
	JS_STATIC_PROP(info, Document);
	JS_STATIC_PROP(keywords, Document);
	JS_STATIC_PROP(layout, Document);
	JS_STATIC_PROP(media, Document);
	JS_STATIC_PROP(modDate, Document);
	JS_STATIC_PROP(mouseX, Document);
	JS_STATIC_PROP(mouseY, Document);
	JS_STATIC_PROP(numFields, Document);
	JS_STATIC_PROP(numPages, Document);
	JS_STATIC_PROP(pageNum, Document);
	JS_STATIC_PROP(pageWindowRect, Document);
	JS_STATIC_PROP(path, Document);
	JS_STATIC_PROP(producer, Document);
	JS_STATIC_PROP(subject, Document);
	JS_STATIC_PROP(title, Document);
	JS_STATIC_PROP(zoom, Document);
	JS_STATIC_PROP(zoomType, Document);

	JS_STATIC_METHOD(addAnnot,Document);
	JS_STATIC_METHOD(addField, Document);
	JS_STATIC_METHOD(addLink, Document);
	JS_STATIC_METHOD(addIcon, Document);
	JS_STATIC_METHOD(calculateNow, Document);
	JS_STATIC_METHOD(closeDoc, Document);
	JS_STATIC_METHOD(createDataObject, Document);
	JS_STATIC_METHOD(deletePages, Document);
	JS_STATIC_METHOD(exportAsText, Document);
	JS_STATIC_METHOD(exportAsFDF, Document);
	JS_STATIC_METHOD(exportAsXFDF, Document);
	JS_STATIC_METHOD(extractPages, Document);
	JS_STATIC_METHOD(getAnnot, Document);
	JS_STATIC_METHOD(getAnnots, Document);
	JS_STATIC_METHOD(getAnnot3D, Document);
	JS_STATIC_METHOD(getAnnots3D, Document);
	JS_STATIC_METHOD(getField, Document);
	JS_STATIC_METHOD(getIcon, Document);
	JS_STATIC_METHOD(getLinks, Document);
	JS_STATIC_METHOD(getNthFieldName, Document);
	JS_STATIC_METHOD(getOCGs, Document);
	JS_STATIC_METHOD(getPageBox, Document);
	JS_STATIC_METHOD(getPageNthWord, Document);
	JS_STATIC_METHOD(getPageNthWordQuads, Document);
	JS_STATIC_METHOD(getPageNumWords, Document);
	JS_STATIC_METHOD(getPrintParams, Document);
	JS_STATIC_METHOD(getURL, Document);
	JS_STATIC_METHOD(importAnFDF, Document);
	JS_STATIC_METHOD(importAnXFDF, Document);
	JS_STATIC_METHOD(importTextData, Document);
	JS_STATIC_METHOD(insertPages, Document);
	JS_STATIC_METHOD(mailForm, Document);
	JS_STATIC_METHOD(print, Document);
	JS_STATIC_METHOD(removeField, Document);
	JS_STATIC_METHOD(replacePages, Document);
	JS_STATIC_METHOD(removeIcon, Document);
	JS_STATIC_METHOD(resetForm, Document);
	JS_STATIC_METHOD(saveAs, Document);
	JS_STATIC_METHOD(submitForm, Document);
	JS_STATIC_METHOD(mailDoc, Document);
};

#endif//_DOCUMENT_H_

