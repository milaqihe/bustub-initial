//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// b_plus_tree_page_test.cpp
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"

#include "common/rid.h"
#include "storage/page/b_plus_tree_internal_page.h"
#include "storage/page/b_plus_tree_leaf_page.h"
#include "storage/page/page.h"
#include "test_util.h"  // NOLINT

namespace bustub {

using Key = GenericKey<8>;
using Comparator = GenericComparator<8>;
using LeafPage = BPlusTreeLeafPage<Key, RID, Comparator>;
using InternalPage = BPlusTreeInternalPage<Key, page_id_t, Comparator>;

namespace {

auto MakeKey(int64_t value) -> Key {
  Key key;
  key.SetFromInteger(value);
  return key;
}

}  // namespace

TEST(BPlusTreePageTest, MetadataAndRootMinimums) {
  Page leaf_storage;
  auto *leaf = reinterpret_cast<LeafPage *>(leaf_storage.GetData());
  leaf->Init(7, INVALID_PAGE_ID, 5);

  EXPECT_TRUE(leaf->IsLeafPage());
  EXPECT_TRUE(leaf->IsRootPage());
  EXPECT_EQ(leaf->GetPageId(), 7);
  EXPECT_EQ(leaf->GetParentPageId(), INVALID_PAGE_ID);
  EXPECT_EQ(leaf->GetSize(), 0);
  EXPECT_EQ(leaf->GetMaxSize(), 5);
  EXPECT_EQ(leaf->GetMinSize(), 1);
  EXPECT_EQ(leaf->GetNextPageId(), INVALID_PAGE_ID);
  EXPECT_EQ(leaf_storage.GetLSN(), INVALID_LSN);
  leaf->SetLSN(123);
  EXPECT_EQ(leaf_storage.GetLSN(), 123);

  leaf->SetSize(3);
  leaf->IncreaseSize(-1);
  EXPECT_EQ(leaf->GetSize(), 2);
  leaf->SetParentPageId(11);
  EXPECT_FALSE(leaf->IsRootPage());
  EXPECT_EQ(leaf->GetMinSize(), 2);

  Page internal_storage;
  auto *internal = reinterpret_cast<InternalPage *>(internal_storage.GetData());
  internal->Init(12, INVALID_PAGE_ID, 5);
  EXPECT_FALSE(internal->IsLeafPage());
  EXPECT_TRUE(internal->IsRootPage());
  EXPECT_EQ(internal->GetMinSize(), 2);
  EXPECT_EQ(internal_storage.GetLSN(), INVALID_LSN);
}

TEST(BPlusTreePageTest, LeafOrderingLookupAndNextChain) {
  Page first_storage;
  Page second_storage;
  auto *first = reinterpret_cast<LeafPage *>(first_storage.GetData());
  auto *second = reinterpret_cast<LeafPage *>(second_storage.GetData());
  first->Init(20, INVALID_PAGE_ID, 4);
  second->Init(21, INVALID_PAGE_ID, 4);
  first->SetNextPageId(second->GetPageId());

  auto key_schema = ParseCreateStatement("a bigint");
  Comparator comparator(key_schema.get());
  EXPECT_EQ(first->Insert(MakeKey(30), RID(30), comparator), 1);
  EXPECT_EQ(first->Insert(MakeKey(10), RID(10), comparator), 2);
  EXPECT_EQ(first->Insert(MakeKey(20), RID(20), comparator), 3);
  EXPECT_EQ(first->Insert(MakeKey(20), RID(99), comparator), 3);
  EXPECT_EQ(first->Insert(MakeKey(40), RID(40), comparator), 4);
  EXPECT_EQ(first->Insert(MakeKey(50), RID(50), comparator), 4);
  EXPECT_EQ(first->GetNextPageId(), 21);
  EXPECT_EQ(first->KeyAt(0).ToString(), 10);
  EXPECT_EQ(first->KeyAt(1).ToString(), 20);
  EXPECT_EQ(first->KeyAt(2).ToString(), 30);
  EXPECT_EQ(first->KeyAt(3).ToString(), 40);
  EXPECT_EQ(first->GetItem(1).second, RID(20));

  RID value;
  EXPECT_TRUE(first->Lookup(MakeKey(20), &value, comparator));
  EXPECT_EQ(value, RID(20));
  EXPECT_FALSE(first->Lookup(MakeKey(25), &value, comparator));
  EXPECT_EQ(first->RemoveAndDeleteRecord(MakeKey(20), comparator), 3);
  EXPECT_EQ(first->KeyAt(1).ToString(), 30);

  EXPECT_EQ(second->GetNextPageId(), INVALID_PAGE_ID);
}

TEST(BPlusTreePageTest, LeafSplitRedistributionAndMerge) {
  Page source_storage;
  Page split_storage;
  Page merged_storage;
  auto *source = reinterpret_cast<LeafPage *>(source_storage.GetData());
  auto *split = reinterpret_cast<LeafPage *>(split_storage.GetData());
  auto *merged = reinterpret_cast<LeafPage *>(merged_storage.GetData());
  source->Init(30, INVALID_PAGE_ID, 4);
  split->Init(31, INVALID_PAGE_ID, 4);
  merged->Init(32, INVALID_PAGE_ID, 8);
  auto key_schema = ParseCreateStatement("a bigint");
  Comparator comparator(key_schema.get());

  for (int64_t key = 1; key <= 4; key++) {
    source->Insert(MakeKey(key), RID(key), comparator);
  }
  source->MoveHalfTo(split);
  EXPECT_EQ(source->GetSize(), 2);
  EXPECT_EQ(split->GetSize(), 2);
  EXPECT_EQ(source->KeyAt(1).ToString(), 2);
  EXPECT_EQ(split->KeyAt(0).ToString(), 3);

  source->MoveAllTo(merged);
  EXPECT_EQ(source->GetSize(), 0);
  EXPECT_EQ(merged->GetSize(), 2);
  split->MoveAllTo(merged);
  EXPECT_EQ(merged->GetSize(), 4);
  EXPECT_EQ(merged->KeyAt(0).ToString(), 1);
  EXPECT_EQ(merged->KeyAt(3).ToString(), 4);
}

TEST(BPlusTreePageTest, InternalLookupAndChildArrayOperations) {
  Page storage;
  auto *internal = reinterpret_cast<InternalPage *>(storage.GetData());
  internal->Init(40, INVALID_PAGE_ID, 6);
  internal->PopulateNewRoot(100, MakeKey(20), 101);
  internal->InsertNodeAfter(101, MakeKey(40), 102);
  internal->InsertNodeAfter(102, MakeKey(60), 103);
  internal->InsertNodeAfter(103, MakeKey(80), 104);
  internal->InsertNodeAfter(104, MakeKey(100), 105);

  auto key_schema = ParseCreateStatement("a bigint");
  Comparator comparator(key_schema.get());
  EXPECT_EQ(internal->ValueIndex(101), 1);
  EXPECT_EQ(internal->Lookup(MakeKey(10), comparator), 100);
  EXPECT_EQ(internal->Lookup(MakeKey(20), comparator), 101);
  EXPECT_EQ(internal->Lookup(MakeKey(79), comparator), 103);

  // max_size counts child pointers. A full page must reject the insertion and
  // preserve both the size and the existing key/value pairs.
  EXPECT_EQ(internal->InsertNodeAfter(102, MakeKey(50), 106), 6);
  EXPECT_EQ(internal->KeyAt(0).ToString(), 0);
  EXPECT_EQ(internal->ValueAt(3), 103);

  internal->Remove(4);
  EXPECT_EQ(internal->GetSize(), 5);

  // Keep removing the second child until only the first child remains.
  while (internal->GetSize() > 1) {
    internal->Remove(1);
  }

  EXPECT_EQ(internal->GetSize(), 1);
  EXPECT_EQ(internal->ValueAt(0), 100);
  EXPECT_EQ(internal->RemoveAndReturnOnlyChild(), 100);
  EXPECT_EQ(internal->GetSize(), 0);
}

TEST(BPlusTreePageTest, InternalSplitAndRedistribution) {
  Page source_storage;
  Page split_storage;
  auto *source = reinterpret_cast<InternalPage *>(source_storage.GetData());
  auto *split = reinterpret_cast<InternalPage *>(split_storage.GetData());
  source->Init(50, 90, 5);
  split->Init(51, 90, 5);
  source->PopulateNewRoot(200, MakeKey(20), 201);
  source->InsertNodeAfter(201, MakeKey(40), 202);
  source->InsertNodeAfter(202, MakeKey(60), 203);
  source->InsertNodeAfter(203, MakeKey(80), 204);

  source->MoveHalfTo(split, nullptr);
  EXPECT_EQ(source->GetSize(), 2);
  EXPECT_EQ(split->GetSize(), 3);
  EXPECT_EQ(source->KeyAt(0).ToString(), 0);
  EXPECT_EQ(source->ValueAt(0), 200);
  EXPECT_EQ(split->KeyAt(0).ToString(), 0);
  EXPECT_EQ(split->ValueAt(0), 202);

  Page recipient_storage;
  auto *recipient = reinterpret_cast<InternalPage *>(recipient_storage.GetData());
  recipient->Init(52, 90, 6);
  recipient->PopulateNewRoot(300, MakeKey(30), 301);
  source->MoveFirstToEndOf(recipient, MakeKey(25), nullptr);
  EXPECT_EQ(recipient->GetSize(), 3);
  EXPECT_EQ(source->GetSize(), 1);
  EXPECT_EQ(source->KeyAt(0).ToString(), 0);
  EXPECT_EQ(source->ValueAt(0), 201);
  EXPECT_EQ(recipient->KeyAt(0).ToString(), 0);
  EXPECT_EQ(recipient->ValueAt(0), 300);

  Page donor_storage;
  Page right_storage;
  auto *donor = reinterpret_cast<InternalPage *>(donor_storage.GetData());
  auto *right = reinterpret_cast<InternalPage *>(right_storage.GetData());
  donor->Init(53, 90, 5);
  right->Init(54, 90, 5);
  donor->PopulateNewRoot(400, MakeKey(40), 401);
  donor->InsertNodeAfter(401, MakeKey(60), 402);
  right->PopulateNewRoot(500, MakeKey(70), 501);

  donor->MoveLastToFrontOf(right, MakeKey(65), nullptr);
  EXPECT_EQ(donor->GetSize(), 2);
  EXPECT_EQ(donor->KeyAt(0).ToString(), 0);
  EXPECT_EQ(donor->ValueAt(0), 400);
  EXPECT_EQ(right->GetSize(), 3);
  EXPECT_EQ(right->KeyAt(0).ToString(), 0);
  EXPECT_EQ(right->ValueAt(0), 402);
  EXPECT_EQ(right->KeyAt(1).ToString(), 65);
  EXPECT_EQ(right->ValueAt(1), 500);

  donor->MoveAllTo(right, MakeKey(75), nullptr);
  EXPECT_EQ(donor->GetSize(), 0);
  EXPECT_EQ(donor->KeyAt(0).ToString(), 0);
  EXPECT_EQ(right->KeyAt(0).ToString(), 0);
  EXPECT_EQ(right->ValueAt(0), 402);
  EXPECT_EQ(right->KeyAt(3).ToString(), 75);
  EXPECT_EQ(right->ValueAt(3), 400);
  EXPECT_EQ(right->ValueAt(4), 401);
}

}  // namespace bustub
