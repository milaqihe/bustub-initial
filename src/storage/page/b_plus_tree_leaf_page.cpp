//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_leaf_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <sstream>

#include "common/exception.h"
#include "common/rid.h"
#include "storage/page/b_plus_tree_leaf_page.h"

namespace bustub {

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * Init method after creating a new leaf page
 * Including set page type, set current size to zero, set page id/parent id, set
 * next page id and set max size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
  SetPageType(IndexPageType::LEAF_PAGE);
  SetSize(0);
  SetMaxSize(max_size);
  SetParentPageId(parent_id);
  SetPageId(page_id);
  SetLSN(INVALID_LSN);
  SetNextPageId(INVALID_PAGE_ID);
}

/**
 * Helper methods to set/get next page id
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::GetNextPageId() const -> page_id_t { return next_page_id_; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::SetNextPageId(page_id_t next_page_id) { next_page_id_ = next_page_id; }

/*
 * Helper method to find and return the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::KeyAt(int index) const -> KeyType { return Array()[index].first; }

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::KeyIndex(const KeyType &key, const KeyComparator &comparator) const -> int {
  int left = 0;
  int right = GetSize();
  while (left < right) {
    const int middle = left + (right - left) / 2;
    if (comparator(KeyAt(middle), key) < 0) {
      left = middle + 1;
    } else {
      right = middle;
    }
  }
  return left;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::GetItem(int index) const -> const MappingType & { return Array()[index]; }

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::Insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator)
    -> int {
  const int index = KeyIndex(key, comparator);
  if ((index < GetSize() && comparator(KeyAt(index), key) == 0) || GetSize() >= GetMaxSize()) {
    return GetSize();
  }
  std::copy_backward(Array() + index, Array() + GetSize(), Array() + GetSize() + 1);
  Array()[index] = {key, value};
  IncreaseSize(1);
  return GetSize();
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::Lookup(const KeyType &key, ValueType *value, const KeyComparator &comparator) const
    -> bool {
  const int index = KeyIndex(key, comparator);
  if (index >= GetSize() || comparator(KeyAt(index), key) != 0) {
    return false;
  }
  *value = Array()[index].second;
  return true;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::RemoveAndDeleteRecord(const KeyType &key, const KeyComparator &comparator) -> int {
  const int index = KeyIndex(key, comparator);
  if (index < GetSize() && comparator(KeyAt(index), key) == 0) {
    Remove(index);
  }
  return GetSize();
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Remove(int index) {
  if (index < 0 || index >= GetSize()) {
    return;
  }
  std::copy(Array() + index + 1, Array() + GetSize(), Array() + index);
  IncreaseSize(-1);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyNFrom(MappingType *items, int size) {
  if (items == nullptr || size <= 0 || size > GetMaxSize() - GetSize()) {
    return;
  }
  std::copy(items, items + size, Array() + GetSize());
  IncreaseSize(size);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyLastFrom(const MappingType &item) {
  if (GetSize() >= GetMaxSize()) {
    return;
  }
  Array()[GetSize()] = item;
  IncreaseSize(1);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyFirstFrom(const MappingType &item) {
  if (GetSize() >= GetMaxSize()) {
    return;
  }
  std::copy_backward(Array(), Array() + GetSize(), Array() + GetSize() + 1);
  Array()[0] = item;
  IncreaseSize(1);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveHalfTo(BPlusTreeLeafPage *recipient,
                                            __attribute__((unused)) BufferPoolManager *buffer_pool_manager) {
  const int start_index = IsRootPage() ? GetMaxSize() / 2 : GetMinSize();
  if (recipient == nullptr || GetSize() <= start_index) {
    return;
  }
  const int moved = GetSize() - start_index;
  if (moved > recipient->GetMaxSize() - recipient->GetSize()) {
    return;
  }
  recipient->CopyNFrom(Array() + start_index, moved);
  recipient->SetNextPageId(GetNextPageId());
  SetNextPageId(recipient->GetPageId());
  IncreaseSize(-moved);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveAllTo(BPlusTreeLeafPage *recipient) {
  if (recipient == nullptr || GetSize() > recipient->GetMaxSize() - recipient->GetSize()) {
    return;
  }
  recipient->CopyNFrom(Array(), GetSize());
  recipient->SetNextPageId(GetNextPageId());
  SetSize(0);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveAllTo(BPlusTreeLeafPage *recipient,
                                           __attribute__((unused)) const KeyType &middle_key,
                                           __attribute__((unused)) BufferPoolManager *buffer_pool_manager) {
  MoveAllTo(recipient);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveFirstToEndOf(BPlusTreeLeafPage *recipient) {
  if (recipient == nullptr || GetSize() == 0 || recipient->GetSize() >= recipient->GetMaxSize()) {
    return;
  }
  recipient->CopyLastFrom(Array()[0]);
  Remove(0);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveFirstToEndOf(BPlusTreeLeafPage *recipient,
                                                  __attribute__((unused)) const KeyType &middle_key,
                                                  __attribute__((unused)) BufferPoolManager *buffer_pool_manager) {
  MoveFirstToEndOf(recipient);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveLastToFrontOf(BPlusTreeLeafPage *recipient) {
  if (recipient == nullptr || GetSize() == 0 || recipient->GetSize() >= recipient->GetMaxSize()) {
    return;
  }
  recipient->CopyFirstFrom(Array()[GetSize() - 1]);
  IncreaseSize(-1);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveLastToFrontOf(BPlusTreeLeafPage *recipient,
                                                   __attribute__((unused)) const KeyType &middle_key,
                                                   __attribute__((unused)) BufferPoolManager *buffer_pool_manager) {
  MoveLastToFrontOf(recipient);
}

template class BPlusTreeLeafPage<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTreeLeafPage<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTreeLeafPage<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTreeLeafPage<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTreeLeafPage<GenericKey<64>, RID, GenericComparator<64>>;
}  // namespace bustub
