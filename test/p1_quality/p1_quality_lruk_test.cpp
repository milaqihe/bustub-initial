#include <stdexcept>
#include <thread>  // NOLINT
#include <vector>

#include "buffer/lru_k_replacer.h"
#include "gtest/gtest.h"

namespace bustub {

TEST(P1QualityLRUKTest, ThirdPreviousAccessBeatsRecentAccess) {
  LRUKReplacer replacer(3, 3);
  replacer.RecordAccess(0);  // 0: [0]
  replacer.RecordAccess(1);  // 1: [1]
  replacer.RecordAccess(1);  // 1: [1,2]
  replacer.RecordAccess(1);  // 1: [1,2,3]
  replacer.RecordAccess(0);  // 0: [0,4]
  replacer.RecordAccess(0);  // 0: [0,4,5]
  replacer.SetEvictable(0, true);
  replacer.SetEvictable(1, true);
  frame_id_t victim = -1;
  ASSERT_TRUE(replacer.Evict(&victim));
  EXPECT_EQ(victim, 0);  // Its third previous access is older, despite its recent access.
}

TEST(P1QualityLRUKTest, RemoveAndReuseClearsHistoryAndSize) {
  LRUKReplacer replacer(2, 2);
  replacer.RecordAccess(0);
  replacer.RecordAccess(0);
  replacer.RecordAccess(1);
  replacer.RecordAccess(1);
  replacer.SetEvictable(0, true);
  replacer.SetEvictable(0, true);
  replacer.SetEvictable(1, true);
  ASSERT_EQ(replacer.Size(), 2);

  replacer.Remove(0);
  EXPECT_EQ(replacer.Size(), 1);
  replacer.Remove(0);        // Missing frame is a no-op.
  replacer.RecordAccess(0);  // Reused frame now has just one access.
  replacer.SetEvictable(0, true);
  EXPECT_EQ(replacer.Size(), 2);

  frame_id_t victim = -1;
  ASSERT_TRUE(replacer.Evict(&victim));
  EXPECT_EQ(victim, 0);  // Incomplete history wins over mature frame 1.
  EXPECT_EQ(replacer.Size(), 1);
}

TEST(P1QualityLRUKTest, ExactFrameBoundsAndUnknownState) {
  LRUKReplacer replacer(2, 2);
  replacer.SetEvictable(0, true);  // No history: no evictable frame is created.
  EXPECT_EQ(replacer.Size(), 0);
  EXPECT_THROW(replacer.RecordAccess(-1), std::invalid_argument);
  EXPECT_THROW(replacer.RecordAccess(2), std::invalid_argument);
  EXPECT_THROW(replacer.SetEvictable(2, true), std::invalid_argument);
  replacer.RecordAccess(1);
  replacer.SetEvictable(1, true);
  EXPECT_EQ(replacer.Size(), 1);
}

TEST(P1QualityLRUKTest, ConcurrentDistinctFramesKeepExactCount) {
  constexpr int kThreads = 4;
  constexpr int kFramesPerThread = 16;
  LRUKReplacer replacer(kThreads * kFramesPerThread, 2);
  std::vector<std::thread> workers;
  for (int thread_id = 0; thread_id < kThreads; ++thread_id) {
    workers.emplace_back([&, thread_id] {
      for (int i = 0; i < kFramesPerThread; ++i) {
        const int id = thread_id * kFramesPerThread + i;
        replacer.RecordAccess(id);
        replacer.SetEvictable(id, true);
      }
    });
  }
  for (auto &worker : workers) {
    worker.join();
  }
  ASSERT_EQ(replacer.Size(), kThreads * kFramesPerThread);
  std::vector<bool> seen(kThreads * kFramesPerThread, false);
  for (int i = 0; i < kThreads * kFramesPerThread; ++i) {
    frame_id_t victim = -1;
    ASSERT_TRUE(replacer.Evict(&victim));
    ASSERT_GE(victim, 0);
    ASSERT_LT(victim, static_cast<frame_id_t>(seen.size()));
    EXPECT_FALSE(seen[victim]);
    seen[victim] = true;
  }
  EXPECT_EQ(replacer.Size(), 0);
}

}  // namespace bustub
