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
  // initially, every page is not in the buffer pool and all the page slots are in the free list
  replacer_ = new LRUKReplacer(pool_size, replacer_k);
  // create locks and condition variables

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

auto BufferPoolManagerInstance::NewPgImp(page_id_t *page_id) -> Page * {
  // 通过互斥锁确保多线程安全
  std::scoped_lock<std::mutex> lock(latch_);

  // 检查是否有空闲的页面
  bool is_free_page = false;
  for (size_t i = 0; i < pool_size_; i++) {
    if (pages_[i].GetPinCount() == 0) {
      is_free_page = true;
      break;
    }
  }

  // 如果没有空闲页面，返回nullptr
  if (!is_free_page) {
    return nullptr;
  }

  // 分配一个新的页面ID
  *page_id = AllocatePage();

  frame_id_t frame_id;

  // 检查空闲列表是否为空
  if (!free_list_.empty()) {
    // 从空闲列表中获取一个frame_id
    frame_id = free_list_.front();
    free_list_.pop_front();
  } else {
    // 如果空闲列表为空，则尝试逐出一个页面
    if (!replacer_->Evict(&frame_id)) {
      // 如果逐出失败，返回nullptr
      return nullptr;
    }

    // 获取被逐出页面的ID
    page_id_t evicted_page_id = pages_[frame_id].GetPageId();

    // 如果被逐出的页面是脏的，则将其写回磁盘
    if (pages_[frame_id].IsDirty()) {
      disk_manager_->WritePage(evicted_page_id, pages_[frame_id].GetData());
      pages_[frame_id].is_dirty_ = false;
    }

    // 重置被逐出的页面内存
    pages_[frame_id].ResetMemory();

    // 从页表中移除被逐出的页面
    page_table_->Remove(evicted_page_id);
  }

  // 在页表中插入新页面
  page_table_->Insert(*page_id, frame_id);

  // 初始化新页面的元数据
  pages_[frame_id].page_id_ = *page_id;
  pages_[frame_id].pin_count_ = 1;

  // 记录页面访问和设置页面为不可逐出
  replacer_->RecordAccess(frame_id);
  replacer_->SetEvictable(frame_id, false);

  return &pages_[frame_id];
}

auto BufferPoolManagerInstance::FetchPgImp(page_id_t page_id) -> Page * {
  // 通过互斥锁确保多线程安全
  std::scoped_lock<std::mutex> lock(latch_);

  frame_id_t frame_id;

  // 尝试从页表中查找页面
  if (page_table_->Find(page_id, frame_id)) {
    // 如果找到了页面，增加其引用计数并返回该页面
    pages_[frame_id].pin_count_++;
    replacer_->RecordAccess(frame_id);
    replacer_->SetEvictable(frame_id, false);
    return &pages_[frame_id];
  }

  // 检查是否有未被引用的页面
  bool is_free_page = false;
  for (size_t i = 0; i < pool_size_; i++) {
    if (pages_[i].GetPinCount() == 0) {
      is_free_page = true;
      break;
    }
  }

  // 如果没有未被引用的页面，则返回空指针
  if (!is_free_page) {
    return nullptr;
  }

  // 从空闲列表或通过逐出操作获取一个空闲帧
  if (!free_list_.empty()) {
    frame_id = free_list_.front();
    free_list_.pop_front();
  } else {
    if (!replacer_->Evict(&frame_id)) {
      // 如果无法逐出页面，则返回空指针
      return nullptr;
    }
    // 处理被逐出的页面
    auto evicted_page_id = pages_[frame_id].GetPageId();
    if (pages_[frame_id].IsDirty()) {
      // 如果页面是脏的，则写回磁盘
      disk_manager_->WritePage(evicted_page_id, pages_[frame_id].GetData());
      pages_[frame_id].is_dirty_ = false;
    }
    // 重置页面内容
    pages_[frame_id].ResetMemory();
    page_table_->Remove(evicted_page_id);
  }

  // 将新页面加载到帧中
  disk_manager_->ReadPage(page_id, pages_[frame_id].GetData());
  page_table_->Insert(page_id, frame_id);
  pages_[frame_id].page_id_ = page_id;
  pages_[frame_id].pin_count_ = 1;
  replacer_->RecordAccess(frame_id);
  replacer_->SetEvictable(frame_id, false);

  return &pages_[frame_id];
}

auto BufferPoolManagerInstance::UnpinPgImp(page_id_t page_id, bool is_dirty) -> bool {
  // 通过互斥锁确保多线程安全
  std::scoped_lock<std::mutex> lock(latch_);

  frame_id_t frame_id;
  // 如果在页表中找不到该页面，返回false
  if (!page_table_->Find(page_id, frame_id)) {
    return false;
  }

  // 如果页面的引用计数已经为0或更小，返回false
  if (pages_[frame_id].GetPinCount() <= 0) {
    return false;
  }

  // 根据is_dirty标志设置页面的脏状态
  if (is_dirty) {
    pages_[frame_id].is_dirty_ = true;
  }

  // 减少页面的引用计数
  pages_[frame_id].pin_count_--;

  // 如果页面的引用计数降至0，则设置为可逐出
  if (pages_[frame_id].pin_count_ == 0) {
    replacer_->SetEvictable(frame_id, true);
  }

  return true;
}

auto BufferPoolManagerInstance::FlushPgImp(page_id_t page_id) -> bool {
  if (page_id == INVALID_PAGE_ID) {
    return false;  // 无效的页面ID，返回false
  }

  std::scoped_lock<std::mutex> lock(latch_);
  frame_id_t frame_id;

  // 如果在页表中找不到该页面，返回false
  if (!page_table_->Find(page_id, frame_id)) {
    return false;
  }

  // 将页面内容写回磁盘
  disk_manager_->WritePage(page_id, pages_[frame_id].data_);
  return true;
}

void BufferPoolManagerInstance::FlushAllPgsImp() {
  std::scoped_lock<std::mutex> lock(latch_);
  // 遍历所有帧并刷新每个页面
  for (size_t frame_id = 0; frame_id < pool_size_; frame_id++) {
    FlushPgImp(pages_[frame_id].GetPageId());
  }
}

auto BufferPoolManagerInstance::DeletePgImp(page_id_t page_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);

  frame_id_t frame_id;
  // 如果在页表中找不到该页面，返回true
  if (!page_table_->Find(page_id, frame_id)) {
    return true;
  }

  // 如果页面仍被引用，返回false
  if (pages_[frame_id].GetPinCount() > 0) {
    return false;
  }

  // 从替换策略中移除该页面
  replacer_->Remove(frame_id);

  // 重置页面内容并更新状态
  pages_[frame_id].ResetMemory();
  pages_[frame_id].page_id_ = INVALID_PAGE_ID;
  pages_[frame_id].pin_count_ = 0;
  pages_[frame_id].is_dirty_ = false;

  // 从页表中移除该页面，并将其添加到空闲列表
  page_table_->Remove(page_id);
  free_list_.push_back(frame_id);
  DeallocatePage(page_id);

  return true;
}

auto BufferPoolManagerInstance::AllocatePage() -> page_id_t { return next_page_id_++; }

}  // namespace bustub
