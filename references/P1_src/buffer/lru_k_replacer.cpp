//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"

namespace bustub {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {}

/**
 * 尝试逐出一个页面。
 * @param frame_id 指向将要被逐出的帧ID的指针。
 * @return 如果成功逐出页面，返回true；否则返回false。
 */
auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);

  // 检查当前是否有可逐出的页面
  if (curr_size_ == 0) {
    return false;  // 没有可逐出的页面
  }

  // 优先从历史列表中逐出页面
  for (auto it = history_list_.rbegin(); it != history_list_.rend(); ++it) {
    auto frame = *it;
    // 检查页面是否可逐出
    if (is_evictable_[frame]) {
      // 重置访问次数并从列表中移除
      access_count_[frame] = 0;
      auto list_it = history_map_.find(frame);
      if (list_it != history_map_.end()) {
        history_list_.erase(list_it->second);
        history_map_.erase(list_it);
      }
      *frame_id = frame;
      curr_size_--;
      is_evictable_[frame] = false;
      return true;  // 页面被成功逐出
    }
  }

  // 如果历史列表中没有可逐出的页面，尝试从缓存列表中逐出
  for (auto it = cache_list_.rbegin(); it != cache_list_.rend(); ++it) {
    auto frame = *it;
    if (is_evictable_[frame]) {
      access_count_[frame] = 0;
      auto list_it = cache_map_.find(frame);
      if (list_it != cache_map_.end()) {
        cache_list_.erase(list_it->second);
        cache_map_.erase(list_it);
      }
      *frame_id = frame;
      curr_size_--;
      is_evictable_[frame] = false;
      return true;  // 页面被成功逐出
    }
  }

  return false;  // 没有页面被逐出
}

/**
 * 记录对指定页面的访问。
 * @param frame_id 被访问的帧ID。
 */
void LRUKReplacer::RecordAccess(frame_id_t frame_id) {
  std::scoped_lock<std::mutex> lock(latch_);

  // 检查frame_id的有效性
  if (frame_id > static_cast<int>(replacer_size_)) {
    throw std::exception();  // 抛出异常
  }

  // 增加访问次数
  access_count_[frame_id]++;

  // 根据访问次数将页面移动到适当的列表
  if (access_count_[frame_id] == k_) {
    // 达到k次访问，移动到缓存列表
    // 从历史列表中移除
    auto hist_it = history_map_.find(frame_id);
    if (hist_it != history_map_.end()) {
      history_list_.erase(hist_it->second);
      history_map_.erase(hist_it);
    }
    // 添加到缓存列表
    cache_list_.push_front(frame_id);
    cache_map_[frame_id] = cache_list_.begin();
  } else if (access_count_[frame_id] > k_) {
    // 超过k次访问，更新在缓存列表中的位置
    auto cache_it = cache_map_.find(frame_id);
    if (cache_it != cache_map_.end()) {
      cache_list_.erase(cache_it->second);
    }
    // 将frame移动到缓存列表的前端
    cache_list_.push_front(frame_id);
    cache_map_[frame_id] = cache_list_.begin();
  } else {
    // 未达到k次访问，加入或保留在历史列表
    // 如果访问次数未达到k，且frame不在历史列表中，将其添加到历史列表
    if (history_map_.count(frame_id) == 0U) {
      history_list_.push_front(frame_id);
      history_map_[frame_id] = history_list_.begin();
    }
  }
}

/**
 * 设置指定页面的逐出状态。
 * @param frame_id 要设置的帧ID。
 * @param set_evictable 是否可逐出。
 */
void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  std::scoped_lock<std::mutex> lock(latch_);

  // 检查frame_id的有效性
  if (frame_id > static_cast<int>(replacer_size_)) {
    throw std::exception();  // 抛出异常
  }

  // 如果访问次数为0，忽略操作
  if (access_count_[frame_id] == 0) {
    return;
  }

  // 更新逐出状态和当前大小
  if (set_evictable != is_evictable_[frame_id]) {
    curr_size_ += (set_evictable ? 1 : -1);
  }
  is_evictable_[frame_id] = set_evictable;
}

/**
 * @brief 从LRUK替换器中移除指定帧ID的函数。
 *
 * @param frame_id 要移除的帧ID。
 *
 * @throws std::exception 如果frame_id大于替换器的大小或其他错误情况。
 */
void LRUKReplacer::Remove(frame_id_t frame_id) {
  std::scoped_lock<std::mutex> lock(latch_);

  // 检查frame_id是否超出替换器的范围
  if (frame_id > static_cast<int>(replacer_size_)) {
    throw std::exception();
  }

  auto cnt = access_count_[frame_id];

  // 如果访问计数为0，则不执行任何操作，直接返回
  if (cnt == 0) {
    return;
  }

  // 如果帧不可驱逐，则抛出异常
  if (!is_evictable_[frame_id]) {
    throw std::exception();
  }

  // 根据访问计数决定移除帧的位置
  if (cnt < k_) {
    // 从历史列表中移除
    history_list_.erase(history_map_[frame_id]);
    history_map_.erase(frame_id);

  } else {
    // 从缓存列表中移除
    cache_list_.erase(cache_map_[frame_id]);
    cache_map_.erase(frame_id);
  }

  // 更新当前大小、访问计数和帧的可驱逐状态
  curr_size_--;
  access_count_[frame_id] = 0;
  is_evictable_[frame_id] = false;
}

auto LRUKReplacer::Size() -> size_t {
  std::scoped_lock<std::mutex> lock(latch_);
  return curr_size_;
}

}  // namespace bustub
