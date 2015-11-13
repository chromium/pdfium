// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_DOCUMENT_H
#define _XFA_DOCUMENT_H

class CXFA_Document;
class CXFA_LayoutItem;
class CXFA_LayoutProcessor;
class CXFA_Node;
class IXFA_DocLayout;
class IXFA_DocParser;
class IXFA_LayoutPage;
class IXFA_Notify;
class IXFA_ObjFactory;
class IXFA_PageContent;
class IXFA_ScriptContext;

enum XFA_VERSION {
  XFA_VERSION_UNKNOWN = 0,
  XFA_VERSION_200 = 200,
  XFA_VERSION_202 = 202,
  XFA_VERSION_204 = 204,
  XFA_VERSION_205 = 205,
  XFA_VERSION_206 = 206,
  XFA_VERSION_207 = 207,
  XFA_VERSION_208 = 208,
  XFA_VERSION_300 = 300,
  XFA_VERSION_301 = 301,
  XFA_VERSION_303 = 303,
  XFA_VERSION_306 = 306,
  XFA_VERSION_DEFAULT = XFA_VERSION_303,
  XFA_VERSION_MIN = 200,
  XFA_VERSION_MAX = 400,
};

#define XFA_LAYOUTSTATUS_Visible 0x0001
#define XFA_LAYOUTSTATUS_Viewable 0x0010
#define XFA_LAYOUTSTATUS_Printable 0x0020
enum XFA_NODEEVENT {
  XFA_NODEEVENT_Ready,
  XFA_NODEEVENT_ValueChanging,
  XFA_NODEEVENT_ValueChanged,
  XFA_NODEEVENT_ChildAdded,
  XFA_NODEEVENT_ChildRemoved,
};
enum XFA_PAGEEVENT {
  XFA_PAGEEVENT_PageAdded,
  XFA_PAGEEVENT_PageRemoved,
};
enum XFA_LAYOUTEVENT {
  XFA_LAYOUTEVENT_ItemAdded,
  XFA_LAYOUTEVENT_ItemRemoving,
  XFA_LAYOUTEVENT_RectChanged,
  XFA_LAYOUTEVENT_StatusChanged,
};
enum XFA_LAYOUTRESULT {
  XFA_LAYOUTRESULT_Continue,
  XFA_LAYOUTRESULT_Done,
  XFA_LAYOUTRESULT_NextContent,
};
#define XFA_LAYOUTNOTIFY_StrictHeight 0x0001
#define XFA_LAYOUTNOTIFY_NoParentBreak 0x0002
class IXFA_Notify {
 public:
  virtual ~IXFA_Notify() {}
  virtual void OnPageEvent(IXFA_LayoutPage* pSender,
                           XFA_PAGEEVENT eEvent,
                           void* pParam = NULL) = 0;

  virtual void OnNodeEvent(CXFA_Node* pSender,
                           XFA_NODEEVENT eEvent,
                           void* pParam = NULL,
                           void* pParam2 = NULL,
                           void* pParam3 = NULL,
                           void* pParam4 = NULL) = 0;
  virtual void OnWidgetDataEvent(CXFA_WidgetData* pSender,
                                 FX_DWORD dwEvent,
                                 void* pParam = NULL,
                                 void* pAdditional = NULL,
                                 void* pAdditional2 = NULL) = 0;

  virtual CXFA_LayoutItem* OnCreateLayoutItem(CXFA_Node* pNode) = 0;
  virtual void OnLayoutEvent(IXFA_DocLayout* pLayout,
                             CXFA_LayoutItem* pSender,
                             XFA_LAYOUTEVENT eEvent,
                             void* pParam = NULL,
                             void* pParam2 = NULL) = 0;
  virtual void StartFieldDrawLayout(CXFA_Node* pItem,
                                    FX_FLOAT& fCalcWidth,
                                    FX_FLOAT& fCalcHeight) = 0;
  virtual FX_BOOL FindSplitPos(CXFA_Node* pItem,
                               int32_t iBlockIndex,
                               FX_FLOAT& fCalcHeightPos) = 0;
  virtual FX_BOOL RunScript(CXFA_Node* pScript, CXFA_Node* pFormItem) = 0;
  virtual int32_t ExecEventByDeepFirst(CXFA_Node* pFormNode,
                                       XFA_EVENTTYPE eEventType,
                                       FX_BOOL bIsFormReady = FALSE,
                                       FX_BOOL bRecursive = TRUE,
                                       CXFA_WidgetAcc* pExclude = NULL) = 0;
  virtual void AddCalcValidate(CXFA_Node* pNode) = 0;
  virtual IXFA_Doc* GetHDOC() = 0;
  virtual IXFA_DocProvider* GetDocProvider() = 0;
  virtual IXFA_AppProvider* GetAppProvider() = 0;
  virtual IXFA_WidgetHandler* GetWidgetHandler() = 0;
  virtual IXFA_Widget* GetHWidget(CXFA_LayoutItem* pLayoutItem) = 0;
  virtual void OpenDropDownList(IXFA_Widget* hWidget) = 0;
  virtual CFX_WideString GetCurrentDateTime() = 0;
  virtual void ResetData(CXFA_WidgetData* pWidgetData = NULL) = 0;
  virtual int32_t GetLayoutStatus() = 0;
  virtual void RunNodeInitialize(CXFA_Node* pNode) = 0;
  virtual void RunSubformIndexChange(CXFA_Node* pSubformNode) = 0;
  virtual CXFA_Node* GetFocusWidgetNode() = 0;
  virtual void SetFocusWidgetNode(CXFA_Node* pNode) = 0;
};
class IXFA_ObjFactory {
 public:
  virtual ~IXFA_ObjFactory() {}
  virtual CXFA_Node* CreateNode(FX_DWORD dwPacket, XFA_ELEMENT eElement) = 0;
  virtual CXFA_Node* CreateNode(XFA_LPCPACKETINFO pPacket,
                                XFA_ELEMENT eElement) = 0;
};
#define XFA_DOCFLAG_StrictScoping 0x0001
#define XFA_DOCFLAG_HasInteractive 0x0002
#define XFA_DOCFLAG_Interactive 0x0004
#define XFA_DOCFLAG_Scripting 0x0008
class CScript_DataWindow;
class CScript_EventPseudoModel;
class CScript_HostPseudoModel;
class CScript_LogPseudoModel;
class CScript_LayoutPseudoModel;
class CScript_SignaturePseudoModel;
class CXFA_Document : public IXFA_ObjFactory {
 public:
  CXFA_Document(IXFA_DocParser* pParser);
  ~CXFA_Document();
  CXFA_Node* GetRoot() const { return m_pRootNode; }
  IXFA_DocParser* GetParser() const { return m_pParser; }
  IXFA_Notify* GetNotify() const;
  void SetRoot(CXFA_Node* pNewRoot);
  CXFA_Object* GetXFANode(const CFX_WideStringC& wsNodeName);
  CXFA_Object* GetXFANode(FX_DWORD wsNodeNameHash);
  void AddPurgeNode(CXFA_Node* pNode);
  FX_BOOL RemovePurgeNode(CXFA_Node* pNode);
  void PurgeNodes();
  FX_BOOL HasFlag(FX_DWORD dwFlag) { return (m_dwDocFlags & dwFlag) == dwFlag; }
  void SetFlag(FX_DWORD dwFlag, FX_BOOL bOn = TRUE);
  FX_BOOL IsInteractive();
  XFA_VERSION GetCurVersionMode() { return m_eCurVersionMode; }
  XFA_VERSION RecognizeXFAVersionNumber(CFX_WideString& wsTemplateNS);
  CXFA_LocaleMgr* GetLocalMgr();
  virtual CXFA_Node* CreateNode(FX_DWORD dwPacket, XFA_ELEMENT eElement);
  virtual CXFA_Node* CreateNode(XFA_LPCPACKETINFO pPacket,
                                XFA_ELEMENT eElement);
  void DoProtoMerge();
  CXFA_Node* GetNodeByID(CXFA_Node* pRoot, const CFX_WideStringC& wsID);
  void DoDataMerge();
  void DoDataRemerge(FX_BOOL bDoDataMerge);
  CXFA_Node* DataMerge_CopyContainer(CXFA_Node* pTemplateNode,
                                     CXFA_Node* pFormNode,
                                     CXFA_Node* pDataScope,
                                     FX_BOOL bOneInstance = FALSE,
                                     FX_BOOL bDataMerge = TRUE,
                                     FX_BOOL bUpLevel = TRUE);
  void DataMerge_UpdateBindingRelations(CXFA_Node* pFormUpdateRoot);
  CXFA_Node* GetNotBindNode(CXFA_ObjArray& arrayNodes);
  CXFA_LayoutProcessor* GetLayoutProcessor();
  IXFA_DocLayout* GetDocLayout();
  IXFA_ScriptContext* InitScriptContext(FXJSE_HRUNTIME hRuntime);
  IXFA_ScriptContext* GetScriptContext();
  void ClearLayoutData();

  CFX_MapPtrTemplate<FX_DWORD, CXFA_Node*> m_rgGlobalBinding;
  CXFA_NodeArray m_pPendingPageSet;

 protected:
  IXFA_DocParser* m_pParser;
  IXFA_ScriptContext* m_pScriptContext;
  CXFA_LayoutProcessor* m_pLayoutProcessor;
  CXFA_Node* m_pRootNode;
  CXFA_LocaleMgr* m_pLocalMgr;
  CScript_DataWindow* m_pScriptDataWindow;
  CScript_EventPseudoModel* m_pScriptEvent;
  CScript_HostPseudoModel* m_pScriptHost;
  CScript_LogPseudoModel* m_pScriptLog;
  CScript_LayoutPseudoModel* m_pScriptLayout;
  CScript_SignaturePseudoModel* m_pScriptSignature;
  CXFA_NodeSet m_rgPurgeNodes;
  XFA_VERSION m_eCurVersionMode;
  FX_DWORD m_dwDocFlags;
  friend class CXFA_SimpleParser;
};
#endif
