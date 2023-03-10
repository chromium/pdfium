// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FXJS_GC_CONTAINER_TRACE_H_
#define FXJS_GC_CONTAINER_TRACE_H_

#include <list>
#include <map>
#include <set>
#include <vector>

#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"

namespace fxgc {

template <typename T, typename V = cppgc::Visitor>
void ContainerTrace(V* visitor, const std::list<cppgc::Member<T>>& container) {
  for (const auto& item : container)
    visitor->Trace(item);
}

template <typename T, typename U, typename V = cppgc::Visitor>
void ContainerTrace(V* visitor,
                    const std::map<cppgc::Member<T>, U>& container) {
  for (const auto& item : container) {
    visitor->Trace(item.first);
  }
}

template <typename T, typename U, typename V = cppgc::Visitor>
void ContainerTrace(V* visitor,
                    const std::map<U, cppgc::Member<T>>& container) {
  for (const auto& item : container)
    visitor->Trace(item.second);
}

template <typename T, typename U, typename V = cppgc::Visitor>
void ContainerTrace(
    V* visitor,
    const std::map<cppgc::Member<U>, cppgc::Member<T>>& container) {
  for (const auto& item : container) {
    visitor->Trace(item.first);
    visitor->Trace(item.second);
  }
}

template <typename T, typename V = cppgc::Visitor>
void ContainerTrace(V* visitor, const std::set<cppgc::Member<T>>& container) {
  for (const auto& item : container)
    visitor->Trace(item);
}

template <typename T, typename V = cppgc::Visitor>
void ContainerTrace(V* visitor,
                    const std::vector<cppgc::Member<T>>& container) {
  for (const auto& item : container)
    visitor->Trace(item);
}

}  // namespace fxgc

using fxgc::ContainerTrace;

#endif  // FXJS_GC_CONTAINER_TRACE_H_
