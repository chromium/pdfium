// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_cross_ref_table.h"

#include <algorithm>
#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_parser.h"

namespace {

constexpr char kXRefStm[] = "XRefStm";
constexpr char kPrev[] = "Prev";
constexpr char kSize[] = "Size";

}  // namespace

// static
std::unique_ptr<CPDF_CrossRefTable> CPDF_CrossRefTable::MergeUp(
    std::unique_ptr<CPDF_CrossRefTable> current,
    std::unique_ptr<CPDF_CrossRefTable> top) {
  if (!current)
    return top;

  if (!top)
    return current;

  current->Update(std::move(top));
  return current;
}

CPDF_CrossRefTable::CPDF_CrossRefTable() = default;

CPDF_CrossRefTable::CPDF_CrossRefTable(std::unique_ptr<CPDF_Dictionary> trailer)
    : trailer_(std::move(trailer)) {}

CPDF_CrossRefTable::~CPDF_CrossRefTable() = default;

void CPDF_CrossRefTable::AddCompressed(uint32_t obj_num,
                                       uint32_t archive_obj_num) {
  if (obj_num >= CPDF_Parser::kMaxObjectNumber ||
      archive_obj_num >= CPDF_Parser::kMaxObjectNumber) {
    NOTREACHED();
    return;
  }

  auto& info = objects_info_[obj_num];
  if (info.gennum > 0)
    return;

  if (info.type == ObjectType::kObjStream)
    return;

  info.type = ObjectType::kCompressed;
  info.archive_obj_num = archive_obj_num;
  info.gennum = 0;

  objects_info_[archive_obj_num].type = ObjectType::kObjStream;
}

void CPDF_CrossRefTable::AddNormal(uint32_t obj_num,
                                   uint16_t gen_num,
                                   FX_FILESIZE pos) {
  if (obj_num >= CPDF_Parser::kMaxObjectNumber) {
    NOTREACHED();
    return;
  }

  auto& info = objects_info_[obj_num];
  if (info.gennum > gen_num)
    return;

  if (info.type == ObjectType::kCompressed && gen_num == 0)
    return;

  if (info.type != ObjectType::kObjStream)
    info.type = ObjectType::kNormal;

  info.gennum = gen_num;
  info.pos = pos;
}

void CPDF_CrossRefTable::SetFree(uint32_t obj_num) {
  if (obj_num >= CPDF_Parser::kMaxObjectNumber) {
    NOTREACHED();
    return;
  }

  auto& info = objects_info_[obj_num];
  info.type = ObjectType::kFree;
  info.gennum = 0xFFFF;
  info.pos = 0;
}

void CPDF_CrossRefTable::SetTrailer(std::unique_ptr<CPDF_Dictionary> trailer) {
  trailer_ = std::move(trailer);
}

const CPDF_CrossRefTable::ObjectInfo* CPDF_CrossRefTable::GetObjectInfo(
    uint32_t obj_num) const {
  const auto it = objects_info_.find(obj_num);
  return it != objects_info_.end() ? &it->second : nullptr;
}

void CPDF_CrossRefTable::Update(
    std::unique_ptr<CPDF_CrossRefTable> new_cross_ref) {
  UpdateInfo(std::move(new_cross_ref->objects_info_));
  UpdateTrailer(std::move(new_cross_ref->trailer_));
}

void CPDF_CrossRefTable::ShrinkObjectMap(uint32_t max_size) {
  if (max_size == 0) {
    objects_info_.clear();
    return;
  }

  objects_info_.erase(objects_info_.lower_bound(max_size), objects_info_.end());
}

uint32_t CPDF_CrossRefTable::GetSize() const {
  const uint32_t size_from_objects_num =
      objects_info_.empty() ? 0 : (objects_info_.rbegin()->first + 1);
  const int size_from_trailer = trailer() ? trailer()->GetIntegerFor(kSize) : 0;
  if (size_from_trailer <= 0)
    return size_from_objects_num;

  return std::max(static_cast<uint32_t>(size_from_trailer),
                  size_from_objects_num);
}

void CPDF_CrossRefTable::UpdateInfo(
    std::map<uint32_t, ObjectInfo>&& new_objects_info) {
  auto cur_it = objects_info_.begin();
  auto new_it = new_objects_info.begin();
  while (cur_it != objects_info_.end() && new_it != new_objects_info.end()) {
    if (cur_it->first == new_it->first) {
      if (cur_it->second.type == ObjectType::kObjStream &&
          new_it->second.type == ObjectType::kNormal) {
        new_it->second.type = ObjectType::kObjStream;
      }
      ++cur_it;
      ++new_it;
    } else if (cur_it->first < new_it->first) {
      new_objects_info.insert(new_it, *cur_it);
      ++cur_it;
    } else {
      new_it = new_objects_info.lower_bound(cur_it->first);
    }
  }
  for (; cur_it != objects_info_.end(); ++cur_it) {
    new_objects_info.insert(new_objects_info.end(), *cur_it);
  }
  objects_info_ = std::move(new_objects_info);
}

void CPDF_CrossRefTable::UpdateTrailer(
    std::unique_ptr<CPDF_Dictionary> new_trailer) {
  if (!new_trailer)
    return;

  if (!trailer_) {
    trailer_ = std::move(new_trailer);
    return;
  }

  new_trailer->RemoveFor(kXRefStm);
  new_trailer->RemoveFor(kPrev);
  new_trailer->SetNewFor<CPDF_Number>(
      kSize, std::max(trailer_->GetIntegerFor(kSize),
                      new_trailer->GetIntegerFor(kSize)));

  for (auto it = new_trailer->begin(); it != new_trailer->end();) {
    const ByteString key = it->first;
    ++it;
    trailer_->SetFor(key, new_trailer->RemoveFor(key));
  }
}
