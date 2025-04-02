// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/observed_ptr.h"

#include "core/fxcrt/check.h"
#include "core/fxcrt/containers/contains.h"

namespace fxcrt {

Observable::Observable() = default;

Observable::~Observable() {
  NotifyObservers();
}

void Observable::AddObserver(ObserverIface* pObserver) {
  DCHECK(!pdfium::Contains(observers_, pObserver));
  observers_.insert(pObserver);
}

void Observable::RemoveObserver(ObserverIface* pObserver) {
  DCHECK(pdfium::Contains(observers_, pObserver));
  observers_.erase(pObserver);
}

void Observable::NotifyObservers() {
  for (auto* pObserver : observers_) {
    pObserver->OnObservableDestroyed();
  }
  observers_.clear();
}

}  // namespace fxcrt
