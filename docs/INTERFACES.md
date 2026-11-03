# Project 1 接口与状态约定

2、3、4 号按本仓库头文件协作，1 号负责协调公开接口变更。保持既有函数名、参数、返回类型和公开/保护访问级别；私有成员与辅助函数可由模块负责人调整。教学 PDF 中的泛称不能替代本仓库的真实签名。

## 1 号集成基线

2026-11-03 按仓库现有头文件建立接口基线，本轮集成准备不修改六个模块源码和公开签名。2、3、4 号据此开发；接口变更需在 PR 中明确列出，由 1 号与受影响模块共同审阅。当前仓库尚无模块 PR，跨模块联调与共同验收在实现合入后进行。

## 2 号向 4 号提供哈希表

类型为 `ExtendibleHashTable<page_id_t, frame_id_t>`，映射方向是 **页 ID → 帧 ID**。

| 接口 | 调用结果与责任 |
| --- | --- |
| `ExtendibleHashTable(size_t bucket_size)` | 以深度 0、一个桶开始，目录可用 |
| `Find(const K &key, V &value) -> bool` | 命中时把值写入引用 `value` 并返回 true；不存在返回 false |
| `Insert(const K &key, const V &value) -> void` | 插入新键或覆盖旧值；返回类型是 void |
| `Remove(const K &key) -> bool` | 删除成功 true，不存在 false |
| `GetGlobalDepth() const -> int` | 返回目录全局深度 |
| `GetLocalDepth(int dir_index) const -> int` | 返回对应桶的局部深度 |
| `GetNumBuckets() const -> int` | 返回实际桶数，不能用目录长度代替 |

满桶更新旧键不应拆桶。新键放不下时再拆桶；只有局部深度等于全局深度才扩大目录，拆分后可能还需继续拆分。当前 `IndexOf` 使用哈希值的**低位**。无需实现合桶和缩容；哈希表实现不能由另一内置哈希表代替。

`Bucket::Insert` 返回 bool，与外层 void 接口不同；同样应先检查是否更新已有键，再检查容量。目录里多个元素可以指向同一桶。

## 3 号向 4 号提供 LRU-K

| 接口 | 调用结果与责任 |
| --- | --- |
| `LRUKReplacer(size_t num_frames, size_t k)` | 容量对应缓冲池帧数，初始可淘汰数量为 0 |
| `RecordAccess(frame_id_t frame_id)` | 记录访问，首次访问建立历史；不自动增加可淘汰数量 |
| `SetEvictable(frame_id_t frame_id, bool)` | 状态实际改变时才调整数量；尚无历史的合法帧不凭空加入 |
| `Evict(frame_id_t *frame_id) -> bool` | 仅在可淘汰帧中选择，成功写出帧 ID，并清除其历史、减少数量；没有候选返回 false |
| `Remove(frame_id_t frame_id)` | 移除指定可淘汰帧及历史；不存在直接返回；不可淘汰则按头文件报错/断言 |
| `Size() -> size_t` | 可淘汰帧数量，不是记录过历史的帧数 |

合法帧编号为 `0 <= frame_id < num_frames`，所以 `frame_id == num_frames` 已越界。`RecordAccess`、`SetEvictable` 对非法 ID 按接口约定报错/断言。

淘汰顺序：先考虑访问不足 K 次的可淘汰帧，距离视为无穷大，在其中选最早历史时间；否则选倒数第 K 次访问时间最早的帧。最近一次访问时间不能代替倒数第 K 次时间。历史保存最近 K 个时间戳即可；K 是参数，不能只实现 K=2。

例如 K=2：A 的访问时间为 1、9，B 为 2、3，二者均可淘汰时选 A；因为倒数第二次时间 1 比 2 更早。若另有仅访问一次的可淘汰 C，则先淘汰 C。页面从第一次访问起就可能占用真实缓冲帧，不是达到 K 次才装入。

## 4 号提供缓冲池

外部调用 `BufferPoolManager` 包装层；内部在 `BufferPoolManagerInstance` 实现保护方法。测试从公开方法调用，不改访问级别来绕过接口。

| 对外公开调用 | 4 号真实实现方法 | 成功与失败行为 |
| --- | --- | --- |
| `NewPage(page_id_t *)` | `NewPgImp(page_id_t *)` | 先从空闲链表或置换器取得帧，再分配新页号；成功返回 Page*；全 pinned 返回 nullptr |
| `FetchPage(page_id_t)` | `FetchPgImp(page_id_t)` | 命中直接固定；缺失则读盘；无可用帧返回 nullptr |
| `UnpinPage(page_id_t, bool)` | `UnpinPgImp(page_id_t, bool)` | 缓存不存在或 pin 已为 0 返回 false；成功减少一次 pin |
| `FlushPage(page_id_t)` | `FlushPgImp(page_id_t)` | 不存在/无效页失败；存在则无论 dirty 与否均写盘，成功后清 dirty |
| `FlushAllPages()` | `FlushAllPgsImp()` | 写回所有有效缓存页；跳过未使用帧 |
| `DeletePage(page_id_t)` | `DeletePgImp(page_id_t)` | 不在池中直接 true；pinned 则 false；其余清映射/历史、释放帧并调用已有 DeallocatePage |

## 共同状态规则

1. `free_list_` 优先于 `replacer_->Evict`，空闲帧与被占用的可淘汰帧是两类不同状态。
2. New 成功的页 `pin_count=1`；Fetch 命中也必须增加 pin、记录访问并设不可淘汰。
3. 只有 pin 降到 0 时才设可淘汰。pin 大于 0 的页不能淘汰或删除。
4. Unpin 时 `dirty = dirty || is_dirty`，传入 false 不能抹掉先前尚未持久化的修改。
5. 淘汰旧页前，若 dirty 则按旧 page_id 写回；移除旧映射，再复用该帧并建立新映射。
6. `Evict` 已经清掉被淘汰帧历史，4 号不再对它额外 `Remove`；复用帧时重新 `RecordAccess`。
7. 创建新页和删除页时正确清理数据及元信息；Fetch 缺失则用磁盘数据覆盖帧内容，保持非脏初始状态。
8. `FlushPage` 不减少 pin、不删除映射，清 dirty；禁止“Flush 等同 Unpin”。
9. 三个组件分别保护自身共享状态。可先用组件级互斥锁实现正确性，再考虑优化。
10. BPM 可持自己的锁再调用哈希表/LRU-K 各自加锁的公开方法；下层不回调 BPM，保持锁顺序一致。
11. `FlushAllPgsImp` 不可在持有普通 `std::mutex` 时调用再次获取同一把锁的 `FlushPgImp`。共享逻辑可抽成要求调用者已持锁的私有方法。

`Page` 的内容读写与 BPM 的元数据同步是不同责任。并发测试若让多个线程写同一页数据，必须在调用方遵守页面锁约定；不能把测试自身的数据竞争归咎于置换器。

## 修改接口时怎样交接

提出具体签名、调用点和原因 → 1 号协调 → 对应模块负责人和 4 号共同确认 → 同一 PR 更新实现、调用方、测试与本文档。不要为了让个人分支临时编译而改别人的公共方法。
