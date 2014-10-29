// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_SAX_
#define _FX_SAX_
class IFX_SAXReaderHandler;
class IFX_SAXReader;
#define FX_SAXPARSEMODE_NotConvert_amp		0x0001
#define FX_SAXPARSEMODE_NotConvert_lt		0x0002
#define FX_SAXPARSEMODE_NotConvert_gt		0x0004
#define FX_SAXPARSEMODE_NotConvert_apos		0x0008
#define FX_SAXPARSEMODE_NotConvert_quot		0x0010
#define FX_SAXPARSEMODE_NotConvert_sharp	0x0020
#define FX_SAXPARSEMODE_NotSkipSpace		0x0100
enum FX_SAXNODE {
    FX_SAXNODE_Unknown = 0,
    FX_SAXNODE_Instruction,
    FX_SAXNODE_Declaration,
    FX_SAXNODE_Comment,
    FX_SAXNODE_Tag,
    FX_SAXNODE_Text,
    FX_SAXNODE_CharData,
};
class IFX_SAXReaderHandler
{
public:
    virtual FX_LPVOID	OnTagEnter(FX_BSTR bsTagName, FX_SAXNODE eType, FX_DWORD dwStartPos) = 0;
    virtual void		OnTagAttribute(FX_LPVOID pTag, FX_BSTR bsAttri, FX_BSTR bsValue) = 0;
    virtual void		OnTagBreak(FX_LPVOID pTag) = 0;
    virtual void		OnTagData(FX_LPVOID pTag, FX_SAXNODE eType, FX_BSTR bsData, FX_DWORD dwStartPos) = 0;
    virtual void		OnTagClose(FX_LPVOID pTag, FX_DWORD dwEndPos) = 0;
    virtual void		OnTagEnd(FX_LPVOID pTag, FX_BSTR bsTagName, FX_DWORD dwEndPos) = 0;
    virtual void		OnTargetData(FX_LPVOID pTag, FX_SAXNODE eType, FX_BSTR bsData, FX_DWORD dwStartPos) = 0;
};
class IFX_SAXReader
{
public:
    virtual void		Release() = 0;
    virtual FX_INT32	StartParse(IFX_FileRead *pFile, FX_DWORD dwStart = 0, FX_DWORD dwLen = -1, FX_DWORD dwParseMode = 0) = 0;
    virtual FX_INT32	ContinueParse(IFX_Pause *pPause = NULL) = 0;
    virtual void		SkipCurrentNode() = 0;
    virtual void		SetHandler(IFX_SAXReaderHandler *pHandler) = 0;
};
IFX_SAXReader*	FX_SAXReader_Create();
#endif
