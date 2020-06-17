// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXGE_CFX_TEXTRENDEROPTIONS_H_
#define CORE_FXGE_CFX_TEXTRENDEROPTIONS_H_

struct CFX_TextRenderOptions {
  // AliasingType defines the options for drawing pixels on the edges of the
  // text. The values are defined in an incrementing order due to the latter
  // aliasing type's dependency on the previous one.
  enum AliasingType {
    // No transparent pixels on glyph edges.
    kAliasing,

    // May have transparent pixels on glyph edges.
    kAntiAliasing,

    // LCD optimization, can be enabled when anti-aliasing is allowed.
    kLcd,

    // BGR stripe optimization, can be enabled when LCD optimazation is enabled.
    kBgrStripe,
  };

  static const CFX_TextRenderOptions& LcdOptions();

  CFX_TextRenderOptions();
  explicit CFX_TextRenderOptions(AliasingType type);
  CFX_TextRenderOptions(const CFX_TextRenderOptions& other);

  // Indicates whether LCD optimazation is enabled.
  bool IsLcd() const { return aliasing_type >= kLcd; }

  // Indicates whether anti aliasing is enabled.
  bool IsSmooth() const { return aliasing_type >= kAntiAliasing; }

  // Aliasing option for fonts.
  AliasingType aliasing_type = kAntiAliasing;

  // Font is CID font.
  bool font_is_cid = false;

  // Using the native text output available on some platforms.
  bool native_text = true;
};

#endif  // CORE_FXGE_CFX_TEXTRENDEROPTIONS_H_
