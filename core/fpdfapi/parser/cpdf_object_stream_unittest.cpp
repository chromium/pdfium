// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_object_stream.h"

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/cxx17_backports.h"

using testing::ElementsAre;
using testing::Pair;

namespace {

constexpr char kNormalStreamContent[] =
    "10 0 11 14 12 21<</Name /Foo>>[1 2 3]4";
constexpr int kNormalStreamContentOffset = 16;
static_assert(kNormalStreamContent[kNormalStreamContentOffset] == '<',
              "Wrong offset");
static_assert(kNormalStreamContent[kNormalStreamContentOffset + 1] == '<',
              "Wrong offset");

}  // namespace

TEST(CPDF_ObjectStreamTest, StreamDictNormal) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", kNormalStreamContentOffset);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  auto obj_stream = CPDF_ObjectStream::Create(stream.Get());
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->objects_offsets(),
              ElementsAre(Pair(10, 0), Pair(11, 14), Pair(12, 21)));

  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsDictionary());

  RetainPtr<CPDF_Object> obj11 = obj_stream->ParseObject(&holder, 11);
  ASSERT_TRUE(obj11);
  EXPECT_EQ(11u, obj11->GetObjNum());
  EXPECT_EQ(0u, obj11->GetGenNum());
  EXPECT_TRUE(obj11->IsArray());

  RetainPtr<CPDF_Object> obj12 = obj_stream->ParseObject(&holder, 12);
  ASSERT_TRUE(obj12);
  EXPECT_EQ(12u, obj12->GetObjNum());
  EXPECT_EQ(0u, obj12->GetGenNum());
  EXPECT_TRUE(obj12->IsNumber());
}

TEST(CPDF_ObjectStreamTest, StreamNoDict) {
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), /*pDict=*/nullptr);
  EXPECT_FALSE(CPDF_ObjectStream::Create(stream.Get()));
}

TEST(CPDF_ObjectStreamTest, StreamDictNoType) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 5);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(stream.Get()));
}

TEST(CPDF_ObjectStreamTest, StreamDictWrongType) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_String>("Type", "ObjStm", /*bHex=*/false);
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 5);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(stream.Get()));
}

TEST(CPDF_ObjectStreamTest, StreamDictWrongTypeValue) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStmmmm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 5);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(stream.Get()));
}

TEST(CPDF_ObjectStreamTest, StreamDictNoCount) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("First", 5);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(stream.Get()));
}

TEST(CPDF_ObjectStreamTest, StreamDictFloatCount) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 2.2f);
  dict->SetNewFor<CPDF_Number>("First", 5);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(stream.Get()));
}

TEST(CPDF_ObjectStreamTest, StreamDictNegativeCount) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", -1);
  dict->SetNewFor<CPDF_Number>("First", 5);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(stream.Get()));
}

TEST(CPDF_ObjectStreamTest, StreamDictCountTooBig) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 999999999);
  dict->SetNewFor<CPDF_Number>("First", 5);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(stream.Get()));
}

TEST(CPDF_ObjectStreamTest, StreamDictNoOffset) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(stream.Get()));
}

TEST(CPDF_ObjectStreamTest, StreamDictFloatOffset) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 5.5f);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(stream.Get()));
}

TEST(CPDF_ObjectStreamTest, StreamDictNegativeOffset) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", -5);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  EXPECT_FALSE(CPDF_ObjectStream::Create(stream.Get()));
}

TEST(CPDF_ObjectStreamTest, StreamDictOffsetTooBig) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  constexpr int kTooBigOffset = pdfium::size(kNormalStreamContent);
  dict->SetNewFor<CPDF_Number>("First", kTooBigOffset);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  auto obj_stream = CPDF_ObjectStream::Create(stream.Get());
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->objects_offsets(),
              ElementsAre(Pair(10, 0), Pair(11, 14), Pair(12, 21)));

  CPDF_IndirectObjectHolder holder;
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 10));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 11));
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 12));
}

TEST(CPDF_ObjectStreamTest, StreamDictTooFewCount) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 2);
  dict->SetNewFor<CPDF_Number>("First", kNormalStreamContentOffset);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  auto obj_stream = CPDF_ObjectStream::Create(stream.Get());
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->objects_offsets(),
              ElementsAre(Pair(10, 0), Pair(11, 14)));

  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsDictionary());

  RetainPtr<CPDF_Object> obj11 = obj_stream->ParseObject(&holder, 11);
  ASSERT_TRUE(obj11);
  EXPECT_EQ(11u, obj11->GetObjNum());
  EXPECT_EQ(0u, obj11->GetGenNum());
  EXPECT_TRUE(obj11->IsArray());

  EXPECT_FALSE(obj_stream->ParseObject(&holder, 12));
}

TEST(CPDF_ObjectStreamTest, StreamDictTooManyObject) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 9);
  dict->SetNewFor<CPDF_Number>("First", kNormalStreamContentOffset);

  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kNormalStreamContent).raw_span(), dict);
  auto obj_stream = CPDF_ObjectStream::Create(stream.Get());
  ASSERT_TRUE(obj_stream);

  // TODO(thestig): Can this avoid finding object 2?
  EXPECT_THAT(obj_stream->objects_offsets(),
              ElementsAre(Pair(2, 3), Pair(10, 0), Pair(11, 14), Pair(12, 21)));

  CPDF_IndirectObjectHolder holder;
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 2));
}

TEST(CPDF_ObjectStreamTest, StreamDictGarbageObjNum) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 19);

  const char kStreamContent[] = "10 0 hi 14 12 21<</Name /Foo>>[1 2 3]4";
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kStreamContent).raw_span(), dict);
  auto obj_stream = CPDF_ObjectStream::Create(stream.Get());
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->objects_offsets(),
              ElementsAre(Pair(10, 0), Pair(12, 21)));
}

TEST(CPDF_ObjectStreamTest, StreamDictGarbageObjectOffset) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 16);

  const char kStreamContent[] = "10 0 11 hi 12 21<</Name /Foo>>[1 2 3]4";
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kStreamContent).raw_span(), dict);
  auto obj_stream = CPDF_ObjectStream::Create(stream.Get());
  ASSERT_TRUE(obj_stream);

  // TODO(thestig): Should object 11 be rejected?
  EXPECT_THAT(obj_stream->objects_offsets(),
              ElementsAre(Pair(10, 0), Pair(11, 0), Pair(12, 21)));

  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsDictionary());

  RetainPtr<CPDF_Object> obj11 = obj_stream->ParseObject(&holder, 11);
  ASSERT_TRUE(obj11);
  EXPECT_EQ(11u, obj11->GetObjNum());
  EXPECT_EQ(0u, obj11->GetGenNum());
  EXPECT_TRUE(obj11->IsDictionary());
}

TEST(CPDF_ObjectStreamTest, StreamDictNegativeObjectOffset) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 16);

  const char kStreamContent[] = "10 0 11 -1 12 21<</Name /Foo>>[1 2 3]4";
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kStreamContent).raw_span(), dict);
  auto obj_stream = CPDF_ObjectStream::Create(stream.Get());
  ASSERT_TRUE(obj_stream);

  // TODO(thestig): Should object 11 be rejected?
  EXPECT_THAT(obj_stream->objects_offsets(),
              ElementsAre(Pair(10, 0), Pair(11, 4294967295), Pair(12, 21)));

  CPDF_IndirectObjectHolder holder;
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 11));
}

TEST(CPDF_ObjectStreamTest, StreamDictObjectOffsetTooBig) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 17);

  const char kStreamContent[] = "10 0 11 999 12 21<</Name /Foo>>[1 2 3]4";
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kStreamContent).raw_span(), dict);
  auto obj_stream = CPDF_ObjectStream::Create(stream.Get());
  ASSERT_TRUE(obj_stream);

  // TODO(thestig): Should object 11 be rejected?
  EXPECT_THAT(obj_stream->objects_offsets(),
              ElementsAre(Pair(10, 0), Pair(11, 999), Pair(12, 21)));

  CPDF_IndirectObjectHolder holder;
  EXPECT_FALSE(obj_stream->ParseObject(&holder, 11));
}

TEST(CPDF_ObjectStreamTest, StreamDictDuplicateObjNum) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 16);

  const char kStreamContent[] = "10 0 10 14 12 21<</Name /Foo>>[1 2 3]4";
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kStreamContent).raw_span(), dict);
  auto obj_stream = CPDF_ObjectStream::Create(stream.Get());
  ASSERT_TRUE(obj_stream);

  // TODO(thestig): Should object 10 be at offset 0 instead?
  EXPECT_THAT(obj_stream->objects_offsets(),
              ElementsAre(Pair(10, 14), Pair(12, 21)));

  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsArray());

  RetainPtr<CPDF_Object> obj12 = obj_stream->ParseObject(&holder, 12);
  ASSERT_TRUE(obj12);
  EXPECT_EQ(12u, obj12->GetObjNum());
  EXPECT_EQ(0u, obj12->GetGenNum());
  EXPECT_TRUE(obj12->IsNumber());
}

TEST(CPDF_ObjectStreamTest, StreamDictUnorderedObjectNumbers) {
  // ISO 32000-1:2008 spec. section 7.5.7, note 6 says there is no restriction
  // on object number ordering.
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 16);

  const char kStreamContent[] = "11 0 12 14 10 21<</Name /Foo>>[1 2 3]4";
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kStreamContent).raw_span(), dict);
  auto obj_stream = CPDF_ObjectStream::Create(stream.Get());
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->objects_offsets(),
              ElementsAre(Pair(10, 21), Pair(11, 0), Pair(12, 14)));

  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsNumber());

  RetainPtr<CPDF_Object> obj11 = obj_stream->ParseObject(&holder, 11);
  ASSERT_TRUE(obj11);
  EXPECT_EQ(11u, obj11->GetObjNum());
  EXPECT_EQ(0u, obj11->GetGenNum());
  EXPECT_TRUE(obj11->IsDictionary());

  RetainPtr<CPDF_Object> obj12 = obj_stream->ParseObject(&holder, 12);
  ASSERT_TRUE(obj12);
  EXPECT_EQ(12u, obj12->GetObjNum());
  EXPECT_EQ(0u, obj12->GetGenNum());
  EXPECT_TRUE(obj12->IsArray());
}

TEST(CPDF_ObjectStreamTest, StreamDictUnorderedObjectOffsets) {
  // ISO 32000-1:2008 spec. section 7.5.7, says offsets shall be in increasing
  // order.
  // TODO(thestig): Should CPDF_ObjectStream check for this and reject this
  // object stream?
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  dict->SetNewFor<CPDF_Name>("Type", "ObjStm");
  dict->SetNewFor<CPDF_Number>("N", 3);
  dict->SetNewFor<CPDF_Number>("First", 16);

  const char kStreamContent[] = "10 21 11 0 12 14<</Name /Foo>>[1 2 3]4";
  auto stream = pdfium::MakeRetain<CPDF_Stream>(
      ByteStringView(kStreamContent).raw_span(), dict);
  auto obj_stream = CPDF_ObjectStream::Create(stream.Get());
  ASSERT_TRUE(obj_stream);

  EXPECT_THAT(obj_stream->objects_offsets(),
              ElementsAre(Pair(10, 21), Pair(11, 0), Pair(12, 14)));

  CPDF_IndirectObjectHolder holder;
  RetainPtr<CPDF_Object> obj10 = obj_stream->ParseObject(&holder, 10);
  ASSERT_TRUE(obj10);
  EXPECT_EQ(10u, obj10->GetObjNum());
  EXPECT_EQ(0u, obj10->GetGenNum());
  EXPECT_TRUE(obj10->IsNumber());

  RetainPtr<CPDF_Object> obj11 = obj_stream->ParseObject(&holder, 11);
  ASSERT_TRUE(obj11);
  EXPECT_EQ(11u, obj11->GetObjNum());
  EXPECT_EQ(0u, obj11->GetGenNum());
  EXPECT_TRUE(obj11->IsDictionary());

  RetainPtr<CPDF_Object> obj12 = obj_stream->ParseObject(&holder, 12);
  ASSERT_TRUE(obj12);
  EXPECT_EQ(12u, obj12->GetObjNum());
  EXPECT_EQ(0u, obj12->GetGenNum());
  EXPECT_TRUE(obj12->IsArray());
}
