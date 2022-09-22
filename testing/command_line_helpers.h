// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_COMMAND_LINE_HELPERS_H_
#define TESTING_COMMAND_LINE_HELPERS_H_

#include <string>

#include "public/fpdfview.h"

// Extract the value from a keyed command line argument.
// `arg` is expected to be "--key=value", and `key` is "--key=".
bool ParseSwitchKeyValue(const std::string& arg,
                         const std::string& key,
                         std::string* value);

// Identifies the compile-time default 2D graphics library to use for rendering
// to FPDF_BITMAPs. Used as part of support to override the renderer at runtime
// based upon command line options.
FPDF_RENDERER_TYPE GetDefaultRendererType();

#endif  // TESTING_COMMAND_LINE_HELPERS_H_
