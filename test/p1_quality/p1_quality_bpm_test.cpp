#include <algorithm>
#include <atomic>
#include <cstring>
#include <random>
#include <thread>  // NOLINT
#include <vector>

#include "buffer/buffer_pool_manager_instance.h"
#include "gtest/gtest.h"
#include "storage/disk/disk_manager_memory.h"

namespace bustub {

TEST(P1QualityBPMTest, PoolOnePinsAndReloadsDirtyBinaryPage) {
  DiskManagerMemory disk(8);
  BufferPoolManagerInstance bpm(1, &disk, 2);
  page_id_t first = INVALID_PAGE_ID;
  Page *page = bpm.NewPage(&first);
  ASSERT_NE(page, nullptr);
  ASSERT_EQ(page->GetPinCount(), 1);
  std::vector<char> expected(BUSTUB_PAGE_SIZE);
  for (int i = 0; i < BUSTUB_PAGE_SIZE; ++i) {
    expected[i] = static_cast<char>((i * 17) % 256);
  }
  expected[23] = 0;
  std::memcpy(page->GetData(), expected.data(), expected.size());

  page_id_t unused = INVALID_PAGE_ID;
  EXPECT_EQ(bpm.NewPage(&unused), nullptr);  // The only frame is pinned.
  EXPECT_EQ(bpm.FetchPage(first), page);     // A cache hit adds a second pin.
  EXPECT_EQ(page->GetPinCount(), 2);
  EXPECT_TRUE(bpm.UnpinPage(first, true));
  EXPECT_TRUE(bpm.UnpinPage(first, false));
  EXPECT_TRUE(page->IsDirty());  // false must not erase an earlier dirty mark.
  EXPECT_FALSE(bpm.UnpinPage(first, false));

  page_id_t second = INVALID_PAGE_ID;
  ASSERT_NE(bpm.NewPage(&second), nullptr);  // Evicts and writes first.
  ASSERT_NE(second, first);
  EXPECT_EQ(bpm.FetchPage(first), nullptr);  // Second is now pinned.
  ASSERT_TRUE(bpm.UnpinPage(second, false));
  Page *reloaded = bpm.FetchPage(first);
  ASSERT_NE(reloaded, nullptr);
  EXPECT_EQ(std::memcmp(reloaded->GetData(), expected.data(), expected.size()), 0);
  EXPECT_FALSE(reloaded->IsDirty());
  EXPECT_TRUE(bpm.UnpinPage(first, false));
}

TEST(P1QualityBPMTest, FlushAllSkipsUnusedFramesAndFlushPreservesPin) {
  DiskManagerMemory disk(8);
  BufferPoolManagerInstance bpm(2, &disk, 2);
  bpm.FlushAllPages();
  EXPECT_EQ(disk.GetNumWrites(), 0);

  page_id_t id = INVALID_PAGE_ID;
  Page *page = bpm.NewPage(&id);
  ASSERT_NE(page, nullptr);
  page->GetData()[0] = 'Q';
  EXPECT_TRUE(bpm.UnpinPage(id, true));
  ASSERT_TRUE(page->IsDirty());
  bpm.FlushAllPages();
  EXPECT_EQ(disk.GetNumWrites(), 1);  // Only one valid page exists.
  EXPECT_FALSE(page->IsDirty());

  Page *again = bpm.FetchPage(id);
  ASSERT_EQ(again, page);
  ASSERT_EQ(again->GetPinCount(), 1);
  EXPECT_TRUE(bpm.FlushPage(id));      // Flush also writes a clean page.
  EXPECT_EQ(disk.GetNumWrites(), 2);
  EXPECT_EQ(again->GetPinCount(), 1);  // Flush does not unpin.
  EXPECT_EQ(again->GetData()[0], 'Q');
  EXPECT_TRUE(bpm.UnpinPage(id, false));
  EXPECT_FALSE(bpm.FlushPage(INVALID_PAGE_ID));
}

TEST(P1QualityBPMTest, DeletePinnedPageAndReuseCleanFrame) {
  DiskManagerMemory disk(8);
  BufferPoolManagerInstance bpm(1, &disk, 2);
  page_id_t old_id = INVALID_PAGE_ID;
  Page *old_page = bpm.NewPage(&old_id);
  ASSERT_NE(old_page, nullptr);
  old_page->GetData()[0] = 'X';
  EXPECT_FALSE(bpm.DeletePage(old_id));
  EXPECT_TRUE(bpm.UnpinPage(old_id, false));
  EXPECT_TRUE(bpm.DeletePage(old_id));
  EXPECT_TRUE(bpm.DeletePage(old_id));

  page_id_t new_id = INVALID_PAGE_ID;
  Page *new_page = bpm.NewPage(&new_id);
  ASSERT_NE(new_page, nullptr);
  EXPECT_NE(new_id, old_id);
  EXPECT_EQ(new_page->GetData()[0], 0);
  EXPECT_EQ(new_page->GetPinCount(), 1);
  EXPECT_FALSE(new_page->IsDirty());
  EXPECT_TRUE(bpm.UnpinPage(new_id, false));
}

TEST(P1QualityBPMTest, PoolTwoFixedSeedPageChurn) {
  constexpr int kPages = 100;
  constexpr unsigned int kSeed = 20261115;
  DiskManagerMemory disk(kPages + 2);
  BufferPoolManagerInstance bpm(2, &disk, 2);
  std::vector<page_id_t> ids;
  ids.reserve(kPages);
  for (int i = 0; i < kPages; ++i) {
    page_id_t id = INVALID_PAGE_ID;
    Page *page = bpm.NewPage(&id);
    ASSERT_NE(page, nullptr) << "new page iteration=" << i;
    std::memcpy(page->GetData(), &i, sizeof(i));
    ids.push_back(id);
    ASSERT_TRUE(bpm.UnpinPage(id, true));
  }

  std::mt19937 rng(kSeed);
  std::shuffle(ids.begin(), ids.end(), rng);
  for (page_id_t id : ids) {
    Page *page = bpm.FetchPage(id);
    ASSERT_NE(page, nullptr) << "page_id=" << id;
    int actual = -1;
    std::memcpy(&actual, page->GetData(), sizeof(actual));
    EXPECT_EQ(actual, id);
    ASSERT_TRUE(bpm.UnpinPage(id, false));
  }
}

TEST(P1QualityBPMTest, ConcurrentIndependentPages) {
  constexpr int kThreads = 4;
  constexpr int kIterations = 100;
  DiskManagerMemory disk(kThreads + 1);
  BufferPoolManagerInstance bpm(kThreads, &disk, 2);
  std::vector<page_id_t> ids;
  for (int i = 0; i < kThreads; ++i) {
    page_id_t id = INVALID_PAGE_ID;
    ASSERT_NE(bpm.NewPage(&id), nullptr);
    ids.push_back(id);
    ASSERT_TRUE(bpm.UnpinPage(id, false));
  }

  std::atomic<bool> worker_ok{true};
  std::vector<std::thread> workers;
  for (int thread_id = 0; thread_id < kThreads; ++thread_id) {
    workers.emplace_back([&, thread_id] {
      for (int iteration = 0; iteration < kIterations; ++iteration) {
        Page *page = bpm.FetchPage(ids[thread_id]);
        if (page == nullptr) {
          worker_ok.store(false);
          return;
        }
        // Each worker owns a different resident page; no two threads write the same data.
        std::memcpy(page->GetData(), &iteration, sizeof(iteration));
        if (!bpm.UnpinPage(ids[thread_id], true)) {
          worker_ok.store(false);
          return;
        }
      }
    });
  }
  for (auto &worker : workers) {
    worker.join();
  }
  ASSERT_TRUE(worker_ok.load());
  bpm.FlushAllPages();
  for (page_id_t id : ids) {
    Page *page = bpm.FetchPage(id);
    ASSERT_NE(page, nullptr);
    int actual = -1;
    std::memcpy(&actual, page->GetData(), sizeof(actual));
    EXPECT_EQ(actual, kIterations - 1);
    EXPECT_TRUE(bpm.UnpinPage(id, false));
  }
}

TEST(P1QualityBPMTest, ConcurrentSharedPageUsesPageLatch) {
  constexpr int kThreads = 4;
  constexpr int kIterations = 100;
  DiskManagerMemory disk(2);
  BufferPoolManagerInstance bpm(1, &disk, 2);
  page_id_t id = INVALID_PAGE_ID;
  Page *page = bpm.NewPage(&id);
  ASSERT_NE(page, nullptr);
  int zero = 0;
  std::memcpy(page->GetData(), &zero, sizeof(zero));
  ASSERT_TRUE(bpm.UnpinPage(id, true));

  std::atomic<bool> worker_ok{true};
  std::vector<std::thread> workers;
  for (int thread_id = 0; thread_id < kThreads; ++thread_id) {
    workers.emplace_back([&] {
      for (int iteration = 0; iteration < kIterations; ++iteration) {
        Page *shared = bpm.FetchPage(id);
        if (shared == nullptr) {
          worker_ok.store(false);
          return;
        }
        shared->WLatch();
        int value = 0;
        std::memcpy(&value, shared->GetData(), sizeof(value));
        ++value;
        std::memcpy(shared->GetData(), &value, sizeof(value));
        shared->WUnlatch();
        if (!bpm.UnpinPage(id, true)) {
          worker_ok.store(false);
          return;
        }
      }
    });
  }
  for (auto &worker : workers) {
    worker.join();
  }
  ASSERT_TRUE(worker_ok.load());
  Page *final_page = bpm.FetchPage(id);
  ASSERT_NE(final_page, nullptr);
  int actual = -1;
  std::memcpy(&actual, final_page->GetData(), sizeof(actual));
  EXPECT_EQ(actual, kThreads * kIterations);
  EXPECT_TRUE(bpm.UnpinPage(id, false));
}

}  // namespace bustub
