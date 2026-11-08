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

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {
  BUSTUB_ASSERT(k_ > 0, "LRU-K requires k >= 1");
}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);

  if (curr_size_ == 0) {
    return false;
  }

  bool found = false;
  frame_id_t victim = -1;
  std::pair<int, size_t> best{0, 0};

  for (const auto &[id, info] : frames_) {
    if (!info.evictable_) {
      continue;
    }
    const std::pair<int, size_t> cand{info.history_.size() < k_ ? 0 : 1, info.history_.front()};
    if (!found || cand < best || (cand == best && id < victim)) {
      found = true;
      best = cand;
      victim = id;
    }
  }

  if (!found) {
    return false;
  }

  frames_.erase(victim);
  curr_size_--;
  *frame_id = victim;
  return true;
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id) {
  std::scoped_lock<std::mutex> lock(latch_);

  if (frame_id < 0 || static_cast<size_t>(frame_id) >= replacer_size_) {
    throw std::invalid_argument("LRUKReplacer::RecordAccess: frame id out of range");
  }

  auto &info = frames_[frame_id];
  info.history_.push_back(current_timestamp_++);
  if (info.history_.size() > k_) {
    info.history_.pop_front();
  }
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  std::scoped_lock<std::mutex> lock(latch_);

  if (frame_id < 0 || static_cast<size_t>(frame_id) >= replacer_size_) {
    throw std::invalid_argument("LRUKReplacer::SetEvictable: frame id out of range");
  }

  auto it = frames_.find(frame_id);

  if (it == frames_.end() || it->second.evictable_ == set_evictable) {
    return;
  }

  it->second.evictable_ = set_evictable;
  curr_size_ += set_evictable ? 1 : -1;
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
  std::scoped_lock<std::mutex> lock(latch_);

  if (frame_id < 0 || static_cast<size_t>(frame_id) >= replacer_size_) {
    throw std::invalid_argument("LRUKReplacer::Remove: frame id out of range");
  }

  auto it = frames_.find(frame_id);
  if (it == frames_.end()) {
    return;
  }
  if (!it->second.evictable_) {
    throw std::invalid_argument("LRUKReplacer::Remove: frame is not evictable");
  }

  frames_.erase(it);
  curr_size_--;
}

auto LRUKReplacer::Size() -> size_t {
  std::scoped_lock<std::mutex> lock(latch_);
  return curr_size_;
}

}  // namespace bustub
