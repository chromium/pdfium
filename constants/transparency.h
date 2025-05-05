// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONSTANTS_TRANSPARENCY_H_
#define CONSTANTS_TRANSPARENCY_H_

namespace pdfium {
namespace transparency {

// PDF 1.7 spec, table 7.2.
// Standard separable blend modes.
inline constexpr char kNormal[] = "Normal";
inline constexpr char kMultiply[] = "Multiply";
inline constexpr char kScreen[] = "Screen";
inline constexpr char kOverlay[] = "Overlay";
inline constexpr char kDarken[] = "Darken";
inline constexpr char kLighten[] = "Lighten";
inline constexpr char kColorDodge[] = "ColorDodge";
inline constexpr char kColorBurn[] = "ColorBurn";
inline constexpr char kHardLight[] = "HardLight";
inline constexpr char kSoftLight[] = "SoftLight";
inline constexpr char kDifference[] = "Difference";
inline constexpr char kExclusion[] = "Exclusion";

// PDF 1.7 spec, table 7.3.
// Standard nonseparable blend modes.
inline constexpr char kHue[] = "Hue";
inline constexpr char kSaturation[] = "Saturation";
inline constexpr char kColor[] = "Color";
inline constexpr char kLuminosity[] = "Luminosity";

// PDF 1.7 spec, table 7.10.
// Entries in a soft-mask dictionary.
inline constexpr char kSoftMaskSubType[] = "S";
inline constexpr char kAlpha[] = "Alpha";
inline constexpr char kG[] = "G";
inline constexpr char kBC[] = "BC";
inline constexpr char kTR[] = "TR";

// PDF 1.7 spec, table 7.13.
// Additional entries specific to a transparency group attributes dictionary.
inline constexpr char kGroupSubType[] = "S";
inline constexpr char kTransparency[] = "Transparency";
inline constexpr char kCS[] = "CS";
inline constexpr char kI[] = "I";

}  // namespace transparency
}  // namespace pdfium

#endif  // CONSTANTS_TRANSPARENCY_H_
