// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SAMPLES_HELPERS_EVENT_H_
#define SAMPLES_HELPERS_EVENT_H_

#include <functional>
#include <string>

#include "public/fpdf_formfill.h"
#include "public/fpdfview.h"

void SendPageEvents(FPDF_FORMHANDLE form,
                    FPDF_PAGE page,
                    const std::string& events,
                    const std::function<void()>& idler);

#endif  // SAMPLES_HELPERS_EVENT_H_
