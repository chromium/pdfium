// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FXFA_BASIC_H_
#define XFA_FXFA_FXFA_BASIC_H_

#include <stdint.h>

enum XFA_HashCode : uint32_t {
  XFA_HASHCODE_None = 0,

  XFA_HASHCODE_Config = 0x4e1e39b6,
  XFA_HASHCODE_ConnectionSet = 0xe14c801c,
  XFA_HASHCODE_Data = 0xbde9abda,
  XFA_HASHCODE_DataDescription = 0x2b5df51e,
  XFA_HASHCODE_Datasets = 0x99b95079,
  XFA_HASHCODE_DataWindow = 0x83a550d2,
  XFA_HASHCODE_Event = 0x185e41e2,
  XFA_HASHCODE_Form = 0xcd309ff4,
  XFA_HASHCODE_Group = 0xf7f75fcd,
  XFA_HASHCODE_Host = 0xdb075bde,
  XFA_HASHCODE_Layout = 0x7e7e845e,
  XFA_HASHCODE_LocaleSet = 0x5473b6dc,
  XFA_HASHCODE_Log = 0x0b1b3d22,
  XFA_HASHCODE_Name = 0x31b19c1,
  XFA_HASHCODE_Occur = 0xf7eebe1c,
  XFA_HASHCODE_Pdf = 0xb843dba,
  XFA_HASHCODE_Record = 0x5779d65f,
  XFA_HASHCODE_Signature = 0x8b036f32,
  XFA_HASHCODE_SourceSet = 0x811929d,
  XFA_HASHCODE_Stylesheet = 0x6038580a,
  XFA_HASHCODE_Template = 0x803550fc,
  XFA_HASHCODE_This = 0x2d574d58,
  XFA_HASHCODE_Xdc = 0xc56afbf,
  XFA_HASHCODE_XDP = 0xc56afcc,
  XFA_HASHCODE_Xfa = 0xc56b9ff,
  XFA_HASHCODE_Xfdf = 0x48d004a8,
  XFA_HASHCODE_Xmpmeta = 0x132a8fbc
};

enum class XFA_PacketType : uint8_t {
#undef PCKT____
#define PCKT____(a, b, c, d, e, f) c,
#include "xfa/fxfa/parser/packets.inc"
#undef PCKT____
};

enum class XFA_XDPPACKET {
  kUNKNOWN = 0,
#undef PCKT____
#define PCKT____(a, b, c, d, e, f) \
  k##c = 1 << static_cast<uint8_t>(XFA_PacketType::c),
#include "xfa/fxfa/parser/packets.inc"
#undef PCKT____
};

enum class XFA_AttributeValue : uint16_t {
#undef VALUE____
#define VALUE____(a, b, c) c,
#include "xfa/fxfa/parser/attribute_values.inc"
#undef VALUE____
};

enum class XFA_Attribute : int16_t {
  Unknown = -1,
#undef ATTR____
#define ATTR____(a, b, c, d) c,
#include "xfa/fxfa/parser/attributes.inc"
#undef ATTR____
};

enum class XFA_Element : int16_t {
  Unknown = -1,
#undef ELEM____
#define ELEM____(a, b, c, d) c,
#include "xfa/fxfa/parser/elements.inc"
#undef ELEM____
};

enum class XFA_AttributeType : uint8_t {
  Enum,
  CData,
  Boolean,
  Integer,
  Measure,
};

enum class XFA_Unit : uint8_t {
  Percent = 0,
  Em,
  Pt,
  In,
  Pc,
  Cm,
  Mm,
  Mp,

  Unknown = 255,
};

enum class XFA_ScriptType : uint8_t {
  Basic,
  Object,
};

#endif  // XFA_FXFA_FXFA_BASIC_H_
