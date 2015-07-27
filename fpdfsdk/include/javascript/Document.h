// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_DOCUMENT_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_DOCUMENT_H_

#include "JS_Define.h"

class PrintParamsObj : public CJS_EmbedObj
{
public:
	PrintParamsObj(CJS_Object* pJSObject);
	virtual ~PrintParamsObj(){}

public:
	bool bUI;
	int nStart;
	int nEnd;
	bool bSilent;
	bool bShrinkToFit;
	bool bPrintAsImage;
	bool bReverse;
	bool bAnnotations;
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
	bool	ADBE(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	author(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	baseURL(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	bookmarkRoot(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	calculate(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	Collab(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	creationDate(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	creator(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	delay(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	dirty(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	documentFileName(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool external(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	filesize(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	icons(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	info(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	keywords(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	layout(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	media(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	modDate(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	mouseX(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	mouseY(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	numFields(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	numPages(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	pageNum(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	pageWindowRect(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	path(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	producer(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	subject(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	title(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	zoom(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool	zoomType(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);

	bool addAnnot(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	addField(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	addLink(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	addIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	calculateNow(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	closeDoc(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	createDataObject(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool deletePages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	exportAsText(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	exportAsFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	exportAsXFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool extractPages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getAnnot(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getAnnots(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getAnnot3D(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getAnnots3D(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getField(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getLinks(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getNthFieldName(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getOCGs(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getPageBox(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getPageNthWord(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getPageNthWordQuads(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	getPageNumWords(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool getPrintParams(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool getURL(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	importAnFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	importAnXFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	importTextData(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool insertPages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	mailForm(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	print(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	removeField(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool replacePages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	resetForm(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	saveAs(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	submitForm(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	mailDoc(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool	removeIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);

public:
	void                                    AttachDoc(CPDFSDK_Document* pDoc);
	CPDFSDK_Document*                       GetReaderDoc();
	static bool                          ExtractFileName(CPDFSDK_Document* pDoc, CFX_ByteString& strFileName);
	static bool                          ExtractFolderName(CPDFSDK_Document* pDoc, CFX_ByteString& strFolderName);
	void                                    AddDelayData(CJS_DelayData* pData);
	void                                    DoFieldDelay(const CFX_WideString& sFieldName, int nControlIndex);
	void                                    AddDelayAnnotData(CJS_AnnotObj *pData);
	void                                    DoAnnotDelay();
	void                                    SetIsolate(v8::Isolate* isolate) {m_isolate = isolate;}
	CJS_Document*                           GetCJSDoc() const;

private:
	CFX_WideString                          ReversalStr(CFX_WideString cbFrom);
	CFX_WideString                          CutString(CFX_WideString cbFrom);
	bool                                    IsEnclosedInRect(CFX_FloatRect rect, CFX_FloatRect LinkRect);
	int                                     CountWords(CPDF_TextObject* pTextObj);
	CFX_WideString                          GetObjWordStr(CPDF_TextObject* pTextObj, int nWordIndex);
	bool                                 ParserParams(JSObject *pObj,CJS_AnnotObj& annotobj);

	v8::Isolate*                            m_isolate;
	IconTree*                               m_pIconTree;
	CPDFSDK_Document*                       m_pDocument;
	CFX_WideString                          m_cwBaseURL;
	bool                                    m_bDelay;
	CFX_ArrayTemplate<CJS_DelayData*>       m_DelayData;
	CFX_ArrayTemplate<CJS_AnnotObj*>        m_DelayAnnotData;
};

class CJS_Document : public CJS_Object
{
public:
	CJS_Document(JSFXObject pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Document(){};

	virtual bool	InitInstance(IFXJS_Context* cc);

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

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_DOCUMENT_H_
