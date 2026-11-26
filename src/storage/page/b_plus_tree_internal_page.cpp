//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_internal_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <iostream>
#include <sstream>

#include "common/exception.h"
#include "storage/page/b_plus_tree_internal_page.h"

namespace bustub {
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/*
 * Init method after creating a new internal page
 * Including set page type, set current size, set page id, set parent id and set
 * max page size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
  SetPageType(IndexPageType::INTERNAL_PAGE);
  SetSize(0);
  SetMaxSize(max_size);
  SetParentPageId(parent_id);
  SetPageId(page_id);
  SetLSN(INVALID_LSN);
  Array()[0].first = KeyType{};
  Array()[0].second = ValueType{};
}
/*
 * Helper method to get/set the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyAt(int index) const -> KeyType { return Array()[index].first; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetKeyAt(int index, const KeyType &key) {
  if (index == 0) {
    Array()[0].first = KeyType{};
    return;
  }
  Array()[index].first = key;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueIndex(const ValueType &value) const -> int {
  for (int i = 0; i < GetSize(); i++) {
    if (ValueAt(i) == value) {
      return i;
    }
  }
  return -1;
}

/*
 * Helper method to get the value associated with input "index"(a.k.a array
 * offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueAt(int index) const -> ValueType { return Array()[index].second; }

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Lookup(const KeyType &key, const KeyComparator &comparator) const -> ValueType {
  // KeyAt(0) is intentionally invalid. The key at index i separates child
  // i-1 from child i, so return the rightmost child whose separator is <= key.
  int child_index = 0;
  for (int i = 1; i < GetSize(); i++) {
    if (comparator(key, KeyAt(i)) < 0) {
      break;
    }
    child_index = i;
  }
  return ValueAt(child_index);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::PopulateNewRoot(const ValueType &old_value, const KeyType &new_key,
                                                     const ValueType &new_value) {
  Array()[0] = {KeyType{}, old_value};
  Array()[1] = {new_key, new_value};
  SetSize(2);
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::InsertNodeAfter(const ValueType &old_value, const KeyType &new_key,
                                                     const ValueType &new_value) -> int {
  const int old_index = ValueIndex(old_value);
  if (old_index < 0 || GetSize() >= GetMaxSize()) {
    return GetSize();
  }
  const int insert_index = old_index + 1;
  std::copy_backward(Array() + insert_index, Array() + GetSize(), Array() + GetSize() + 1);
  Array()[insert_index] = {new_key, new_value};
  IncreaseSize(1);
  Array()[0].first = KeyType{};
  return GetSize();
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Remove(int index) {
  if (index < 0 || index >= GetSize()) {
    return;
  }
  std::copy(Array() + index + 1, Array() + GetSize(), Array() + index);
  IncreaseSize(-1);
  // The first child pointer remains valid, but its separator key is always
  // the page sentinel.
  Array()[0].first = KeyType{};
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::RemoveAndReturnOnlyChild() -> ValueType {
  const ValueType only_child = ValueAt(0);
  SetSize(0);
  Array()[0].first = KeyType{};
  return only_child;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyNFrom(MappingType *items, int size, BufferPoolManager *buffer_pool_manager) {
  if (items == nullptr || size <= 0 || size > GetMaxSize() - GetSize()) {
    return;
  }
  const int old_size = GetSize();
  std::copy(items, items + size, Array() + old_size);
  IncreaseSize(size);
  if (old_size == 0) {
    Array()[0].first = KeyType{};
  }

  // The page operation does not acquire latches. The tree operation owns the
  // source and destination latches and passes the BPM only so moved children
  // can have their parent metadata repaired.
  if (buffer_pool_manager == nullptr) {
    return;
  }
  for (int i = old_size; i < GetSize(); i++) {
    auto *child_page = buffer_pool_manager->FetchPage(ValueAt(i));
    if (child_page == nullptr) {
      continue;
    }
    auto *child = reinterpret_cast<BPlusTreePage *>(child_page->GetData());
    child->SetParentPageId(GetPageId());
    buffer_pool_manager->UnpinPage(ValueAt(i), true);
  }
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyLastFrom(const MappingType &pair, BufferPoolManager *buffer_pool_manager) {
  if (GetSize() >= GetMaxSize()) {
    return;
  }
  Array()[GetSize()] = pair;
  Array()[0].first = KeyType{};
  IncreaseSize(1);
  if (buffer_pool_manager != nullptr) {
    auto *child_page = buffer_pool_manager->FetchPage(pair.second);
    if (child_page != nullptr) {
      reinterpret_cast<BPlusTreePage *>(child_page->GetData())->SetParentPageId(GetPageId());
      buffer_pool_manager->UnpinPage(pair.second, true);
    }
  }
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyFirstFrom(const MappingType &pair, BufferPoolManager *buffer_pool_manager) {
  if (GetSize() >= GetMaxSize()) {
    return;
  }
  std::copy_backward(Array(), Array() + GetSize(), Array() + GetSize() + 1);
  Array()[0] = pair;
  Array()[0].first = KeyType{};
  IncreaseSize(1);
  if (buffer_pool_manager != nullptr) {
    auto *child_page = buffer_pool_manager->FetchPage(pair.second);
    if (child_page != nullptr) {
      reinterpret_cast<BPlusTreePage *>(child_page->GetData())->SetParentPageId(GetPageId());
      buffer_pool_manager->UnpinPage(pair.second, true);
    }
  }
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveHalfTo(BPlusTreeInternalPage *recipient,
                                                BufferPoolManager *buffer_pool_manager) {
  const int start_index = GetMinSize();
  if (recipient == nullptr || GetSize() <= start_index) {
    return;
  }
  const int moved = GetSize() - start_index;
  if (moved > recipient->GetMaxSize() - recipient->GetSize()) {
    return;
  }
  recipient->CopyNFrom(Array() + start_index, moved, buffer_pool_manager);
  IncreaseSize(-moved);
  Array()[0].first = KeyType{};
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveAllTo(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                                               BufferPoolManager *buffer_pool_manager) {
  if (recipient == nullptr || GetSize() == 0) {
    return;
  }
  if (GetSize() > recipient->GetMaxSize() - recipient->GetSize()) {
    return;
  }
  Array()[0].first = middle_key;
  recipient->CopyNFrom(Array(), GetSize(), buffer_pool_manager);
  SetSize(0);
  Array()[0].first = KeyType{};
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveFirstToEndOf(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                                                      BufferPoolManager *buffer_pool_manager) {
  if (recipient == nullptr || GetSize() <= 1 || recipient->GetSize() >= recipient->GetMaxSize()) {
    return;
  }
  recipient->CopyLastFrom(Array()[0], buffer_pool_manager);
  if (recipient->GetSize() > 1) {
    recipient->SetKeyAt(recipient->GetSize() - 1, middle_key);
  }
  Remove(0);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveLastToFrontOf(BPlusTreeInternalPage *recipient, const KeyType &middle_key,
                                                       BufferPoolManager *buffer_pool_manager) {
  if (recipient == nullptr || GetSize() <= 1 || recipient->GetSize() >= recipient->GetMaxSize()) {
    return;
  }
  const int last_index = GetSize() - 1;
  const MappingType pair = Array()[last_index];
  Remove(GetSize() - 1);
  recipient->CopyFirstFrom(pair, buffer_pool_manager);
  if (recipient->GetSize() > 1) {
    recipient->SetKeyAt(1, middle_key);
  }
}

// valuetype for internalNode should be page id_t
template class BPlusTreeInternalPage<GenericKey<4>, page_id_t, GenericComparator<4>>;
template class BPlusTreeInternalPage<GenericKey<8>, page_id_t, GenericComparator<8>>;
template class BPlusTreeInternalPage<GenericKey<16>, page_id_t, GenericComparator<16>>;
template class BPlusTreeInternalPage<GenericKey<32>, page_id_t, GenericComparator<32>>;
template class BPlusTreeInternalPage<GenericKey<64>, page_id_t, GenericComparator<64>>;
}  // namespace bustub
