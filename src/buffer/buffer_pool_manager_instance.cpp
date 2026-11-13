//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager_instance.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager_instance.h"

#include "common/exception.h"
#include "common/macros.h"

namespace bustub {

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, DiskManager *disk_manager, size_t replacer_k,
                                                     LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // we allocate a consecutive memory space for the buffer pool
  pages_ = new Page[pool_size_];
  page_table_ = new ExtendibleHashTable<page_id_t, frame_id_t>(bucket_size_);
  replacer_ = new LRUKReplacer(pool_size, replacer_k);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManagerInstance::~BufferPoolManagerInstance() {
  delete[] pages_;
  delete page_table_;
  delete replacer_;
}

// 腾出一个可用帧
auto BufferPoolManagerInstance::AcquireFrame(frame_id_t *frame_id) -> bool {
  // 空闲优先
  if (!free_list_.empty()) {
    *frame_id = free_list_.front();  // 取链表头（最老的空闲帧）
    free_list_.pop_front();
    return true;
  }

  // 没有空闲，淘汰
  if (!replacer_->Evict(frame_id)) {
    return false;  // 池已满且所有页都被pin住了
  }
  // 脏页写回
  Page *victim = &pages_[*frame_id];
  if (victim->IsDirty()) {
    disk_manager_->WritePage(victim->page_id_, victim->GetData());  // 写回磁盘
  }
  page_table_->Remove(victim->page_id_);  // 从页表中删除旧的映射

  // 清除帧
  victim->ResetMemory();               // 数据区清零
  victim->page_id_ = INVALID_PAGE_ID;  // 标注为空帧
  victim->pin_count_ = 0;
  victim->is_dirty_ = false;
  return true;
}

auto BufferPoolManagerInstance::NewPgImp(page_id_t *page_id) -> Page * {
  std::scoped_lock<std::mutex> lock(latch_);

  frame_id_t frame_id;  // 出参容器，接收选中的帧号
  if (!AcquireFrame(&frame_id)) {
    return nullptr;
  }

  page_id_t new_page_id = AllocatePage();
  Page *page = &pages_[frame_id];  // 获得帧地址

  page->ResetMemory();
  page->page_id_ = new_page_id;
  page->is_dirty_ = false;
  page->pin_count_ = 1;

  page_table_->Insert(new_page_id, frame_id);
  replacer_->RecordAccess(frame_id);         // 记录这个帧被访问了一次
  replacer_->SetEvictable(frame_id, false);  // 不可淘汰

  *page_id = new_page_id;
  return page;
}

auto BufferPoolManagerInstance::FetchPgImp(page_id_t page_id) -> Page * {
  std::scoped_lock<std::mutex> lock(latch_);

  frame_id_t frame_id;
  if (page_table_->Find(page_id, frame_id)) {  // 查页表，将页号对应的帧号存到frame_id中
    Page *page = &pages_[frame_id];
    page->pin_count_++;
    replacer_->RecordAccess(frame_id);
    replacer_->SetEvictable(frame_id, false);
    return page;
  }

  if (!AcquireFrame(&frame_id)) {
    return nullptr;
  }

  Page *page = &pages_[frame_id];
  page->ResetMemory();
  page->page_id_ = page_id;
  page->is_dirty_ = false;
  page->pin_count_ = 1;

  disk_manager_->ReadPage(page_id, page->GetData());  // 读磁盘

  page_table_->Insert(page_id, frame_id);
  replacer_->RecordAccess(frame_id);
  replacer_->SetEvictable(frame_id, false);
  return page;
}

// 归还当前pin住的页
auto BufferPoolManagerInstance::UnpinPgImp(page_id_t page_id, bool is_dirty) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);

  frame_id_t frame_id;
  if (!page_table_->Find(page_id, frame_id)) {
    return false;
  }

  Page *page = &pages_[frame_id];
  if (page->pin_count_ <= 0) {  // 已经无人在用
    return false;
  }

  page->pin_count_--;
  if (is_dirty) {
    page->is_dirty_ = true;
  }

  if (page->pin_count_ == 0) {
    replacer_->SetEvictable(frame_id, true);
  }  // 已经完全没人使用，就允许淘汰
  return true;
}

// 写回磁盘，标记为干净（保证磁盘为最新内容）
auto BufferPoolManagerInstance::FlushPgImp(page_id_t page_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);

  if (page_id == INVALID_PAGE_ID) {
    return false;
  }

  frame_id_t frame_id;
  if (!page_table_->Find(page_id, frame_id)) {
    return false;
  }

  Page *page = &pages_[frame_id];
  disk_manager_->WritePage(page_id, page->GetData());  // 写回磁盘无论是否被修改过
  page->is_dirty_ = false;
  return true;
}

// 全部更新
void BufferPoolManagerInstance::FlushAllPgsImp() {
  std::scoped_lock<std::mutex> lock(latch_);

  for (size_t i = 0; i < pool_size_; ++i) {
    Page *page = &pages_[i];
    if (page->page_id_ == INVALID_PAGE_ID) {
      continue;
    }
    disk_manager_->WritePage(page->page_id_, page->GetData());
    page->is_dirty_ = false;
  }
}

// 从缓冲池里面h删掉
auto BufferPoolManagerInstance::DeletePgImp(page_id_t page_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);

  frame_id_t frame_id;
  if (!page_table_->Find(page_id, frame_id)) {
    return true;
  }

  Page *page = &pages_[frame_id];
  if (page->pin_count_ > 0) {
    return false;
  }

  page_table_->Remove(page_id);
  replacer_->Remove(frame_id);
  free_list_.push_back(frame_id);

  page->ResetMemory();
  page->page_id_ = INVALID_PAGE_ID;
  page->pin_count_ = 0;
  page->is_dirty_ = false;

  DeallocatePage(page_id);
  return true;
}

auto BufferPoolManagerInstance::AllocatePage() -> page_id_t { return next_page_id_++; }

}  // namespace bustub
