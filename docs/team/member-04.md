# 4 号 Buffer Pool Manager 负责人

你完整负责缓冲池管理器的实现和基础测试，调用 2 号哈希表与 3 号 LRU-K 的固定接口，实现页面在磁盘与内存之间的管理。

## 直接打开这些文件

| 用途 | 文件 |
| --- | --- |
| 你的实现类与六个保护方法 | [buffer_pool_manager_instance.h](../../src/include/buffer/buffer_pool_manager_instance.h) |
| 当前构造异常与空方法 | [buffer_pool_manager_instance.cpp](../../src/buffer/buffer_pool_manager_instance.cpp) |
| 对外公开调用包装层，主要阅读 | [buffer_pool_manager.h](../../src/include/buffer/buffer_pool_manager.h) |
| 页数据、pin、dirty 与页面锁 | [page.h](../../src/include/storage/page/page.h) |
| 已提供的磁盘读写接口 | [disk_manager.h](../../src/include/storage/disk/disk_manager.h) |
| 你的基础与集成测试 | [buffer_pool_manager_instance_test.cpp](../../test/buffer/buffer_pool_manager_instance_test.cpp) |

本版本方法名是 `NewPgImp`、`FetchPgImp`、`UnpinPgImp`、`FlushPgImp`、`FlushAllPgsImp`、`DeletePgImp`。不要按教程中另一版本的拼写新建重复文件或方法。

## 先准备公共逻辑

1. 阅读[接口与状态约定](../INTERFACES.md)，和 2、3 号确认调用行为。
2. 梳理五个状态来源：`pages_`、`page_table_`、`replacer_`、`free_list_`、`disk_manager_`。
3. 画出“优先取空闲帧→否则淘汰→必要时写回→清旧映射→建立新页”的流程。
4. 可抽取私有辅助方法复用选帧/清理逻辑，明确每个辅助方法要求持锁还是自行加锁。
5. 保留构造函数已有资源初始化和析构资源释放；实现完成后移除构造函数中的 `NotImplementedException`。只删异常而不实现方法不能算完成。

## 六个方法逐项完成

### NewPgImp

1. 加锁保护 BPM 元数据，优先取 `free_list_`。
2. 没有空闲帧时调用 `Evict`；全 pinned/无候选时返回 nullptr，不破坏已有页。
3. 对旧脏页按旧 page_id 写回，删除旧页表映射。Evict 已清历史，不重复 Remove。
4. 取得帧后再 `AllocatePage` 分配新页号，重置帧数据与元数据。
5. 设置新 page_id、pin_count=1、dirty=false，插入映射，记录访问并设不可淘汰，返回页指针。

### FetchPgImp

1. 查页表；命中时 pin 加 1、记录访问、设不可淘汰，返回同一缓存页。
2. 未命中时按 New 的规则取帧；没有候选返回 nullptr。
3. 必要时写回旧脏页并清旧映射，再用 `DiskManager::ReadPage` 读入目标页。
4. 设置目标页元数据、建立新映射，记录访问并固定；读入页初始非脏。

### UnpinPgImp

1. 页不在缓存或当前 pin_count 已为 0，返回 false。
2. 合并 dirty 状态：已有 true 不能被这次 false 清掉。
3. pin 减 1，只有降到 0 才 `SetEvictable(true)`，成功返回 true。

### FlushPgImp

1. 拒绝 `INVALID_PAGE_ID` 和不在缓存的页。
2. 对存在页调用 `WritePage`，不以 dirty=false 为理由跳过显式 Flush。
3. 成功后清 dirty；保持 pin_count、页表和可淘汰状态不变。

### FlushAllPgsImp

1. 遍历所有有效缓存页，逐页写回并清 dirty，跳过未使用帧。
2. 不在持有普通 BPM mutex 时调用再次获取该锁的 `FlushPgImp`；直接复用不重复加锁的内部逻辑。
3. 不改变引用计数，不把页移出缓冲池。

### DeletePgImp

1. 不在缓存返回 true；在缓存但 pin_count>0 返回 false。
2. 删除页表项和 replacer 中的记录，清理数据并设无效 page_id、pin=0、dirty=false。
3. 把帧放回 free list，调用已有 `DeallocatePage`。删除与为换页而写回是不同操作。

## 基础测试与重点风险

原测试已经包括 `NewPage`、`UnpinPage`、`FetchPage`、`DeletePage`、`IsDirty`、`BinaryDataTest`、`ConcurrencyTest`、`IntegratedTest` 和 `HardTest_1` 至 `HardTest_4`。保留这些测试，追加以下容易漏的场景：

- 容量为 1，全 pinned 时 New/缺失 Fetch 失败；解除固定后可继续使用。
- 同一页 Fetch 多次，必须相应多次 Unpin 后才能被淘汰。
- dirty=true 后再次以 false Unpin，dirty 仍为 true。
- 脏页被淘汰后重新 Fetch，内容保持一致；复用帧不留下旧页映射与访问历史。
- 显式 Flush 后 dirty=false，pin 不变；FlushAll 有空闲帧时不写无效页且不死锁。
- 删除 pinned 页失败；删除未缓存页成功；删除后帧可再次使用。
- 读写包含零字节的二进制内容，比较固定长度内存而非仅用字符串。

## 你的测试命令与依赖

2、3 号尚未完成时可以读代码、写流程与实现、编译；完整行为验收需要它们的真实实现。不要把依赖失败包装成 BPM 通过。

2、3 号合入 main 后，在自己的干净工作区同步，按[构建指南](../BUILD.md)配置并运行：

```bash
git fetch origin
git merge origin/main
cmake --build build --target buffer_pool_manager_instance_test -j 4
./build/test/buffer_pool_manager_instance_test
```

定位可使用 `--gtest_filter=BufferPoolManagerInstanceTest.FetchPage`；提交前运行完整测试。跨模块问题附最小复现给 2 或 3 号，别在自己的 PR 中悄悄替换他们的文件。

## 你的推送步骤

先完成[克隆和本人身份配置](../GIT_WORKFLOW.md)，首次创建分支：

```bash
git switch main
git pull --ff-only origin main
git switch -c team/04-buffer-pool
```

完成实现与测试后：

```bash
git status
git diff
git add src/include/buffer/buffer_pool_manager_instance.h src/buffer/buffer_pool_manager_instance.cpp test/buffer/buffer_pool_manager_instance_test.cpp
git diff --cached
git commit -m "feat(p1): implement buffer pool manager"
git push -u origin team/04-buffer-pool
```

打开[新建缓冲池 PR](https://github.com/milaqihe/bustub-initial/compare/main...team/04-buffer-pool?expand=1)，base 为 main。未合入依赖时标 Draft 并列明依赖；依赖合并并同步、完整测试通过后转为 Ready，请 1 号审阅、5 号复核边界。

## 完成标准

六个页面操作和资源生命周期完整；全部现有缓冲池测试及新增基础用例通过；pin/dirty/映射/空闲帧与置换器状态一致；完整版本可让 5 号运行内存和并发验收；PR 中注明所依赖的 main 版本和真实结果。
