// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_XFA_BASIC_DATA_H_
#define XFA_FXFA_PARSER_XFA_BASIC_DATA_H_

#include <stddef.h>

#include <optional>

#include "core/fxcrt/widestring.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/fxfa_basic.h"

using XFA_ATTRIBUTE_CALLBACK = void (*)(v8::Isolate* pIsolate,
                                        CJX_Object* pNode,
                                        v8::Local<v8::Value>* pValue,
                                        bool bSetting,
                                        XFA_Attribute eAttribute);

enum class XFA_PacketMatch : uint8_t {
  kCompleteMatch = 1,
  kPrefixMatch,
  kNoMatch,
};

enum class XFA_PacketSupport : uint8_t {
  kSupportOne = 1,
  kSupportMany,
};

struct XFA_PACKETINFO {
  XFA_PacketType packet_type;
  XFA_PacketMatch match;
  XFA_PacketSupport support;
  const char* name;
  const char* uri;
};

struct XFA_ATTRIBUTEINFO {
  XFA_Attribute attribute;
  XFA_ScriptType eValueType;
};

struct XFA_SCRIPTATTRIBUTEINFO {
  XFA_Attribute attribute;
  XFA_ScriptType eValueType;
  XFA_ATTRIBUTE_CALLBACK callback = nullptr;
};

XFA_PACKETINFO XFA_GetPacketByIndex(XFA_PacketType ePacket);
std::optional<XFA_PACKETINFO> XFA_GetPacketByName(WideStringView wsName);

ByteStringView XFA_ElementToName(XFA_Element elem);
XFA_Element XFA_GetElementByName(WideStringView name);

ByteStringView XFA_AttributeToName(XFA_Attribute attr);
std::optional<XFA_ATTRIBUTEINFO> XFA_GetAttributeByName(WideStringView name);

ByteStringView XFA_AttributeValueToName(XFA_AttributeValue item);
std::optional<XFA_AttributeValue> XFA_GetAttributeValueByName(
    WideStringView name);

std::optional<XFA_SCRIPTATTRIBUTEINFO> XFA_GetScriptAttributeByName(
    XFA_Element eElement,
    WideStringView wsAttributeName);

#endif  // XFA_FXFA_PARSER_XFA_BASIC_DATA_H_
