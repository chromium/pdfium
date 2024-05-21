// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_object_stream.h"

#include <iterator>
#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/data_vector.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::ElementsAre;

namespace {

constexpr char kNormalStreamContent[] =
    "10 0 11 14 12 21<</Name /Foo>>[1 2 3]4";
constexpr int kNormalStreamContentOffset = 16;
static_assert(kNormalStreamContent[kNormalStreamContentOffset] == '<',
              "Wrong offset");
static_assert(kNormalStreamContent[kNormalStreamContentOffset + 1] == '<',
              "Wrong offset");

}  // namespace

TEST(ObjectStreamTest, StreamDictNormal) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", kNormalStreamContentOffset);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  auto obj_stream = CPDF_ObjectStream::Create(std::move(stream));
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->object_info(),
              ElementsAre(CPDF_ObjectStream::ObjectInfo(10, 0),
                          CPDF_ObjectStream::ObjectInfo(11, 14),
                          CPDF_ObjectStream::ObjectInfo(12, 21)));

  // Check expected indices.
  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10, 0);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsDictionary());

  RetainPtr<CPDF_Object> obj11 = obj_stream->ParseObject(&holder, 11, 1);
  ASSERT_TRUE(obj11);
  EXPECT_EQ(11u, obj11->GetObjNum());
  EXPECT_EQ(0u, obj11->GetGenNum());
  EXPECT_TRUE(obj11->IsArray());

  RetainPtr<CPDF_Object> obj12 = obj_stream->ParseObject(&holder, 12, 2);
  ASSERT_TRUE(obj12);
  EXPECT_EQ(12u, obj12->GetObjNum());
  EXPECT_EQ(0u, obj12->GetGenNum());
  EXPECT_TRUE(obj12->IsNumber());

  // Check bad indices.
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 10, 1));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 10, 2));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 10, 3));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 11, 0));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 11, 2));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 11, 3));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 12, 0));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 12, 1));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 12, 3));
}

TEST(ObjectStreamTest, StreamEmptyDict) {
  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()),
      pdfium::MakeRetain<CPDF_Dictionary>());
  EXPECT_FALSE(CPDF_ObjectStream::Create(std::move(stream)));
}

TEST(ObjectStreamTest, StreamDictNoType) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 5);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(std::move(stream)));
}

TEST(ObjectStreamTest, StreamDictWrongType) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_String>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 5);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(std::move(stream)));
}

TEST(ObjectStreamTest, StreamDictWrongTypeValue) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStmmmm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 5);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(std::move(stream)));
}

TEST(ObjectStreamTest, StreamDictNoCount) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("First", 5);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(std::move(stream)));
}

TEST(ObjectStreamTest, StreamDictFloatCount) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 2.2f);
  dict->SetNewFor<CPDF_Number>("First", 5);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(std::move(stream)));
}

TEST(ObjectStreamTest, StreamDictNegativeCount) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", -1);
  dict->SetNewFor<CPDF_Number>("First", 5);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(std::move(stream)));
}

TEST(ObjectStreamTest, StreamDictCountTooBig) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 999999999);
  dict->SetNewFor<CPDF_Number>("First", 5);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(std::move(stream)));
}

TEST(ObjectStreamTest, StreamDictNoOffset) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(std::move(stream)));
}

TEST(ObjectStreamTest, StreamDictFloatOffset) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 5.5f);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(std::move(stream)));
}

TEST(ObjectStreamTest, StreamDictNegativeOffset) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", -5);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(std::move(stream)));
}

TEST(ObjectStreamTest, StreamDictOffsetTooBig) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  constexpr int kTooBigOffset = std::size(kNormalStreamContent);
  dict->SetNewFor<CPDF_Number>("First", kTooBigOffset);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  auto obj_stream = CPDF_ObjectStream::Create(std::move(stream));
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->object_info(),
              ElementsAre(CPDF_ObjectStream::ObjectInfo(10, 0),
                          CPDF_ObjectStream::ObjectInfo(11, 14),
                          CPDF_ObjectStream::ObjectInfo(12, 21)));

  CPDF_IndirectObjectHolder holder;
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 10, 0));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 11, 1));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 12, 2));
}

TEST(ObjectStreamTest, StreamDictTooFewCount) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 2);
  dict->SetNewFor<CPDF_Number>("First", kNormalStreamContentOffset);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  auto obj_stream = CPDF_ObjectStream::Create(std::move(stream));
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->object_info(),
              ElementsAre(CPDF_ObjectStream::ObjectInfo(10, 0),
                          CPDF_ObjectStream::ObjectInfo(11, 14)));

  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10, 0);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsDictionary());

  RetainPtr<CPDF_Object> obj11 = obj_stream->ParseObject(&holder, 11, 1);
  ASSERT_TRUE(obj11);
  EXPECT_EQ(11u, obj11->GetObjNum());
  EXPECT_EQ(0u, obj11->GetGenNum());
  EXPECT_TRUE(obj11->IsArray());

  EXPECT_FALSE(obj_stream->ParseObject(&holder, 12, 2));
}

TEST(ObjectStreamTest, StreamDictTooManyObject) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 9);
  dict->SetNewFor<CPDF_Number>("First", kNormalStreamContentOffset);

  ByteStringView contents_view(kNormalStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  auto obj_stream = CPDF_ObjectStream::Create(std::move(stream));
  ASSERT_TRUE(obj_stream);

  // TODO(thestig): Can this avoid finding object 2?
  EXPECT_THAT(obj_stream->object_info(),
              ElementsAre(CPDF_ObjectStream::ObjectInfo(10, 0),
                          CPDF_ObjectStream::ObjectInfo(11, 14),
                          CPDF_ObjectStream::ObjectInfo(12, 21),
                          CPDF_ObjectStream::ObjectInfo(2, 3)));

  CPDF_IndirectObjectHolder holder;
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 2, 0));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 2, 1));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 2, 2));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 2, 3));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 2, 4));
}

TEST(ObjectStreamTest, StreamDictGarbageObjNum) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 19);

  const char kStreamContent[] = "10 0 hi 14 12 21<</Name /Foo>>[1 2 3]4";
  ByteStringView contents_view(kStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  auto obj_stream = CPDF_ObjectStream::Create(std::move(stream));
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->object_info(),
              ElementsAre(CPDF_ObjectStream::ObjectInfo(10, 0),
                          CPDF_ObjectStream::ObjectInfo(12, 21)));
}

TEST(ObjectStreamTest, StreamDictGarbageObjectOffset) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 16);

  const char kStreamContent[] = "10 0 11 hi 12 21<</Name /Foo>>[1 2 3]4";
  ByteStringView contents_view(kStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  auto obj_stream = CPDF_ObjectStream::Create(std::move(stream));
  ASSERT_TRUE(obj_stream);

  // TODO(thestig): Should object 11 be rejected?
  EXPECT_THAT(obj_stream->object_info(),
              ElementsAre(CPDF_ObjectStream::ObjectInfo(10, 0),
                          CPDF_ObjectStream::ObjectInfo(11, 0),
                          CPDF_ObjectStream::ObjectInfo(12, 21)));

  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10, 0);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsDictionary());

  RetainPtr<CPDF_Object> obj11 = obj_stream->ParseObject(&holder, 11, 1);
  ASSERT_TRUE(obj11);
  EXPECT_EQ(11u, obj11->GetObjNum());
  EXPECT_EQ(0u, obj11->GetGenNum());
  EXPECT_TRUE(obj11->IsDictionary());
}

TEST(ObjectStreamTest, StreamDictNegativeObjectOffset) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 16);

  const char kStreamContent[] = "10 0 11 -1 12 21<</Name /Foo>>[1 2 3]4";
  ByteStringView contents_view(kStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  auto obj_stream = CPDF_ObjectStream::Create(std::move(stream));
  ASSERT_TRUE(obj_stream);

  // TODO(thestig): Should object 11 be rejected?
  EXPECT_THAT(obj_stream->object_info(),
              ElementsAre(CPDF_ObjectStream::ObjectInfo(10, 0),
                          CPDF_ObjectStream::ObjectInfo(11, 4294967295),
                          CPDF_ObjectStream::ObjectInfo(12, 21)));

  CPDF_IndirectObjectHolder holder;
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 11, 1));
}

TEST(ObjectStreamTest, StreamDictObjectOffsetTooBig) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 17);

  const char kStreamContent[] = "10 0 11 999 12 21<</Name /Foo>>[1 2 3]4";
  ByteStringView contents_view(kStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  auto obj_stream = CPDF_ObjectStream::Create(std::move(stream));
  ASSERT_TRUE(obj_stream);

  // TODO(thestig): Should object 11 be rejected?
  EXPECT_THAT(obj_stream->object_info(),
              ElementsAre(CPDF_ObjectStream::ObjectInfo(10, 0),
                          CPDF_ObjectStream::ObjectInfo(11, 999),
                          CPDF_ObjectStream::ObjectInfo(12, 21)));

  CPDF_IndirectObjectHolder holder;
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 11, 1));
}

TEST(ObjectStreamTest, StreamDictDuplicateObjNum) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 16);

  const char kStreamContent[] = "10 0 10 14 12 21<</Name /Foo>>[1 2 3]4";
  ByteStringView contents_view(kStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  auto obj_stream = CPDF_ObjectStream::Create(std::move(stream));
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->object_info(),
              ElementsAre(CPDF_ObjectStream::ObjectInfo(10, 0),
                          CPDF_ObjectStream::ObjectInfo(10, 14),
                          CPDF_ObjectStream::ObjectInfo(12, 21)));

  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10, 0);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsDictionary());

  obj10 = obj_stream->ParseObject(&holder, 10, 1);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsArray());

  EXPECT_FALSE(obj_stream->ParseObject(&holder, 10, 2));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 10, 3));

  RetainPtr<CPDF_Object> obj12 = obj_stream->ParseObject(&holder, 12, 2);
  ASSERT_TRUE(obj12);
  EXPECT_EQ(12u, obj12->GetObjNum());
  EXPECT_EQ(0u, obj12->GetGenNum());
  EXPECT_TRUE(obj12->IsNumber());
}

TEST(ObjectStreamTest, StreamDictUnorderedObjectNumbers) {
  // ISO 32000-1:2008 spec. section 7.5.7, note 6 says there is no restriction
  // on object number ordering.
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 16);

  const char kStreamContent[] = "11 0 12 14 10 21<</Name /Foo>>[1 2 3]4";
  ByteStringView contents_view(kStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  auto obj_stream = CPDF_ObjectStream::Create(std::move(stream));
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->object_info(),
              ElementsAre(CPDF_ObjectStream::ObjectInfo(11, 0),
                          CPDF_ObjectStream::ObjectInfo(12, 14),
                          CPDF_ObjectStream::ObjectInfo(10, 21)));

  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10, 2);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsNumber());

  RetainPtr<CPDF_Object> obj11 = obj_stream->ParseObject(&holder, 11, 0);
  ASSERT_TRUE(obj11);
  EXPECT_EQ(11u, obj11->GetObjNum());
  EXPECT_EQ(0u, obj11->GetGenNum());
  EXPECT_TRUE(obj11->IsDictionary());

  RetainPtr<CPDF_Object> obj12 = obj_stream->ParseObject(&holder, 12, 1);
  ASSERT_TRUE(obj12);
  EXPECT_EQ(12u, obj12->GetObjNum());
  EXPECT_EQ(0u, obj12->GetGenNum());
  EXPECT_TRUE(obj12->IsArray());
}

TEST(ObjectStreamTest, StreamDictUnorderedObjectOffsets) {
  // ISO 32000-1:2008 spec. section 7.5.7, says offsets shall be in increasing
  // order.
  // TODO(thestig): Should CPDF_ObjectStream check for this and reject this
  // object stream?
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 16);

  const char kStreamContent[] = "10 21 11 0 12 14<</Name /Foo>>[1 2 3]4";
  ByteStringView contents_view(kStreamContent);
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      DataVector<uint8_t>(contents_view.begin(), contents_view.end()), dict);
  auto obj_stream = CPDF_ObjectStream::Create(std::move(stream));
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->object_info(),
              ElementsAre(CPDF_ObjectStream::ObjectInfo(10, 21),
                          CPDF_ObjectStream::ObjectInfo(11, 0),
                          CPDF_ObjectStream::ObjectInfo(12, 14)));

  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10, 0);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsNumber());

  RetainPtr<CPDF_Object> obj11 = obj_stream->ParseObject(&holder, 11, 1);
  ASSERT_TRUE(obj11);
  EXPECT_EQ(11u, obj11->GetObjNum());
  EXPECT_EQ(0u, obj11->GetGenNum());
  EXPECT_TRUE(obj11->IsDictionary());

  RetainPtr<CPDF_Object> obj12 = obj_stream->ParseObject(&holder, 12, 2);
  ASSERT_TRUE(obj12);
  EXPECT_EQ(12u, obj12->GetObjNum());
  EXPECT_EQ(0u, obj12->GetGenNum());
  EXPECT_TRUE(obj12->IsArray());
}
