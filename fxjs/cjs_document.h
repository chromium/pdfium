// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_DOCUMENT_H_
#define FXJS_CJS_DOCUMENT_H_

#include <list>
#include <memory>
#include <vector>

#include "fxjs/JS_Define.h"

class CJS_Document;
class CPDF_TextObject;

struct CJS_DelayData;

class Document : public CJS_EmbedObj {
 public:
  explicit Document(CJS_Object* pJSObject);
  ~Document() override;

  CJS_Return get_ADBE(CJS_Runtime* pRuntime);
  CJS_Return set_ADBE(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_author(CJS_Runtime* pRuntime);
  CJS_Return set_author(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_base_URL(CJS_Runtime* pRuntime);
  CJS_Return set_base_URL(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_bookmark_root(CJS_Runtime* pRuntime);
  CJS_Return set_bookmark_root(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_calculate(CJS_Runtime* pRuntime);
  CJS_Return set_calculate(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_collab(CJS_Runtime* pRuntime);
  CJS_Return set_collab(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_creation_date(CJS_Runtime* pRuntime);
  CJS_Return set_creation_date(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_creator(CJS_Runtime* pRuntime);
  CJS_Return set_creator(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_delay(CJS_Runtime* pRuntime);
  CJS_Return set_delay(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_dirty(CJS_Runtime* pRuntime);
  CJS_Return set_dirty(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_document_file_name(CJS_Runtime* pRuntime);
  CJS_Return set_document_file_name(CJS_Runtime* pRuntime,
                                    v8::Local<v8::Value> vp);

  CJS_Return get_external(CJS_Runtime* pRuntime);
  CJS_Return set_external(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_filesize(CJS_Runtime* pRuntime);
  CJS_Return set_filesize(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_icons(CJS_Runtime* pRuntime);
  CJS_Return set_icons(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_info(CJS_Runtime* pRuntime);
  CJS_Return set_info(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_keywords(CJS_Runtime* pRuntime);
  CJS_Return set_keywords(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_layout(CJS_Runtime* pRuntime);
  CJS_Return set_layout(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_media(CJS_Runtime* pRuntime);
  CJS_Return set_media(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_mod_date(CJS_Runtime* pRuntime);
  CJS_Return set_mod_date(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_mouse_x(CJS_Runtime* pRuntime);
  CJS_Return set_mouse_x(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_mouse_y(CJS_Runtime* pRuntime);
  CJS_Return set_mouse_y(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_num_fields(CJS_Runtime* pRuntime);
  CJS_Return set_num_fields(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_num_pages(CJS_Runtime* pRuntime);
  CJS_Return set_num_pages(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_page_num(CJS_Runtime* pRuntime);
  CJS_Return set_page_num(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_page_window_rect(CJS_Runtime* pRuntime);
  CJS_Return set_page_window_rect(CJS_Runtime* pRuntime,
                                  v8::Local<v8::Value> vp);

  CJS_Return get_path(CJS_Runtime* pRuntime);
  CJS_Return set_path(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_producer(CJS_Runtime* pRuntime);
  CJS_Return set_producer(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_subject(CJS_Runtime* pRuntime);
  CJS_Return set_subject(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_title(CJS_Runtime* pRuntime);
  CJS_Return set_title(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_zoom(CJS_Runtime* pRuntime);
  CJS_Return set_zoom(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_zoom_type(CJS_Runtime* pRuntime);
  CJS_Return set_zoom_type(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_URL(CJS_Runtime* pRuntime);
  CJS_Return set_URL(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return addAnnot(CJS_Runtime* pRuntime,
                      const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return addField(CJS_Runtime* pRuntime,
                      const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return addLink(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return addIcon(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return calculateNow(CJS_Runtime* pRuntime,
                          const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return closeDoc(CJS_Runtime* pRuntime,
                      const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return createDataObject(CJS_Runtime* pRuntime,
                              const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return deletePages(CJS_Runtime* pRuntime,
                         const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return exportAsText(CJS_Runtime* pRuntime,
                          const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return exportAsFDF(CJS_Runtime* pRuntime,
                         const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return exportAsXFDF(CJS_Runtime* pRuntime,
                          const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return extractPages(CJS_Runtime* pRuntime,
                          const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getAnnot(CJS_Runtime* pRuntime,
                      const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getAnnots(CJS_Runtime* pRuntime,
                       const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getAnnot3D(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getAnnots3D(CJS_Runtime* pRuntime,
                         const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getField(CJS_Runtime* pRuntime,
                      const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getIcon(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getLinks(CJS_Runtime* pRuntime,
                      const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getNthFieldName(CJS_Runtime* pRuntime,
                             const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getOCGs(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getPageBox(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getPageNthWord(CJS_Runtime* pRuntime,
                            const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getPageNthWordQuads(
      CJS_Runtime* pRuntime,
      const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getPageNumWords(CJS_Runtime* pRuntime,
                             const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getPrintParams(CJS_Runtime* pRuntime,
                            const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return getURL(CJS_Runtime* pRuntime,
                    const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return gotoNamedDest(CJS_Runtime* pRuntime,
                           const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return importAnFDF(CJS_Runtime* pRuntime,
                         const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return importAnXFDF(CJS_Runtime* pRuntime,
                          const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return importTextData(CJS_Runtime* pRuntime,
                            const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return insertPages(CJS_Runtime* pRuntime,
                         const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return mailForm(CJS_Runtime* pRuntime,
                      const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return print(CJS_Runtime* pRuntime,
                   const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return removeField(CJS_Runtime* pRuntime,
                         const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return replacePages(CJS_Runtime* pRuntime,
                          const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return resetForm(CJS_Runtime* pRuntime,
                       const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return saveAs(CJS_Runtime* pRuntime,
                    const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return submitForm(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return syncAnnotScan(CJS_Runtime* pRuntime,
                           const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return mailDoc(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return removeIcon(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params);

  void SetFormFillEnv(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  CPDFSDK_FormFillEnvironment* GetFormFillEnv() const {
    return m_pFormFillEnv.Get();
  }
  void AddDelayData(CJS_DelayData* pData);
  void DoFieldDelay(const WideString& sFieldName, int nControlIndex);
  CJS_Document* GetCJSDoc() const;

 private:
  bool IsEnclosedInRect(CFX_FloatRect rect, CFX_FloatRect LinkRect);
  int CountWords(CPDF_TextObject* pTextObj);
  WideString GetObjWordStr(CPDF_TextObject* pTextObj, int nWordIndex);

  CJS_Return getPropertyInternal(CJS_Runtime* pRuntime,
                                 const ByteString& propName);
  CJS_Return setPropertyInternal(CJS_Runtime* pRuntime,
                                 v8::Local<v8::Value> vp,
                                 const ByteString& propName);

  CPDFSDK_FormFillEnvironment::ObservedPtr m_pFormFillEnv;
  WideString m_cwBaseURL;
  std::list<std::unique_ptr<CJS_DelayData>> m_DelayData;
  // Needs to be a std::list for iterator stability.
  std::list<WideString> m_IconNames;
  bool m_bDelay;
};

class CJS_Document : public CJS_Object {
 public:
  static int GetObjDefnID();
  static void DefineJSObjects(CFXJS_Engine* pEngine);

  explicit CJS_Document(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Document() override {}

  // CJS_Object
  void InitInstance(IJS_Runtime* pIRuntime) override;

  JS_STATIC_PROP(ADBE, ADBE, Document);
  JS_STATIC_PROP(author, author, Document);
  JS_STATIC_PROP(baseURL, base_URL, Document);
  JS_STATIC_PROP(bookmarkRoot, bookmark_root, Document);
  JS_STATIC_PROP(calculate, calculate, Document);
  JS_STATIC_PROP(Collab, collab, Document);
  JS_STATIC_PROP(creationDate, creation_date, Document);
  JS_STATIC_PROP(creator, creator, Document);
  JS_STATIC_PROP(delay, delay, Document);
  JS_STATIC_PROP(dirty, dirty, Document);
  JS_STATIC_PROP(documentFileName, document_file_name, Document);
  JS_STATIC_PROP(external, external, Document);
  JS_STATIC_PROP(filesize, filesize, Document);
  JS_STATIC_PROP(icons, icons, Document);
  JS_STATIC_PROP(info, info, Document);
  JS_STATIC_PROP(keywords, keywords, Document);
  JS_STATIC_PROP(layout, layout, Document);
  JS_STATIC_PROP(media, media, Document);
  JS_STATIC_PROP(modDate, mod_date, Document);
  JS_STATIC_PROP(mouseX, mouse_x, Document);
  JS_STATIC_PROP(mouseY, mouse_y, Document);
  JS_STATIC_PROP(numFields, num_fields, Document);
  JS_STATIC_PROP(numPages, num_pages, Document);
  JS_STATIC_PROP(pageNum, page_num, Document);
  JS_STATIC_PROP(pageWindowRect, page_window_rect, Document);
  JS_STATIC_PROP(path, path, Document);
  JS_STATIC_PROP(producer, producer, Document);
  JS_STATIC_PROP(subject, subject, Document);
  JS_STATIC_PROP(title, title, Document);
  JS_STATIC_PROP(URL, URL, Document);
  JS_STATIC_PROP(zoom, zoom, Document);
  JS_STATIC_PROP(zoomType, zoom_type, Document);

  JS_STATIC_METHOD(addAnnot, Document);
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
  JS_STATIC_METHOD(gotoNamedDest, Document);
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
  JS_STATIC_METHOD(syncAnnotScan, Document);
  JS_STATIC_METHOD(mailDoc, Document);

 private:
  static int ObjDefnID;
  static const JSPropertySpec PropertySpecs[];
  static const JSMethodSpec MethodSpecs[];
};

#endif  // FXJS_CJS_DOCUMENT_H_
