# 2 号 可扩展哈希表负责人

你完整负责 Extendible Hash Table 的实现和基础测试，为 4 号提供线程安全的 `page_id → frame_id` 映射。

## 直接打开这些文件

| 用途 | 文件 |
| --- | --- |
| 类接口、Bucket、目录与深度成员 | [extendible_hash_table.h](../../src/include/container/hash/extendible_hash_table.h) |
| 需要补全的实现 | [extendible_hash_table.cpp](../../src/container/hash/extendible_hash_table.cpp) |
| 你的基础测试 | [extendible_hash_table_test.cpp](../../test/container/hash/extendible_hash_table_test.cpp) |
| 4 号如何使用映射 | [buffer_pool_manager_instance.cpp](../../src/buffer/buffer_pool_manager_instance.cpp) |

本次实现的是内存哈希表，不是 `src/container/disk/hash/` 中的磁盘哈希表。先读[接口约定](../INTERFACES.md)。

## 按顺序完成

1. 构造函数创建一个局部深度为 0 的桶，目录大小为 1；全局深度 0、桶数 1，与类中初值一致。
2. `Bucket::Find` 遍历桶，命中把值写入引用，未命中 false。
3. `Bucket::Insert` 先查重复键并覆盖，再判断容量；只有新键放不下才 false。
4. `Bucket::Remove` 删除唯一键，正确返回存在与否；不影响其他键。
5. 外层 `Find`、`Remove` 通过现有 `IndexOf` 找目录项再操作桶。
6. 外层 `Insert` 先处理已有键更新；新键溢出时，比较桶局部深度与全局深度。
7. 两者相等才把全局深度增加 1、目录翻倍；新目录高半部分继承低半部分指针。
8. 拆目标桶，局部深度加 1，按新增低位 bit 分配原键值；更新所有指向旧桶的兄弟指针。
9. 实际桶数每次拆分增加 1；不把目录项数量当作桶数。
10. 重试待插入键；一次拆分后仍可能全部落在同一桶，需要继续拆分。
11. 保持 `GetGlobalDepth`、`GetLocalDepth`、`GetNumBuckets` 返回正确值；无需合桶、缩容。
12. 用类的 mutex 保护共享状态。现有公开 getter 已加锁，持锁内部使用内部 getter，避免重复加同一锁。
13. 保留实现文件末尾的模板显式实例化；不要破坏 BPM 使用的 `<page_id_t, frame_id_t>` 和测试中的类型。

不允许用 `std::unordered_map` 等另一内置哈希表代替本模块。允许的容器与当前类结构以头文件和课程要求为准。

## 必须覆盖的基础用例

- 空表查找、删除返回 false，初始深度和桶数正确。
- 插入后查找返回原值；重复插入同键更新值。
- 桶恰好满时更新旧键，目录深度和桶数不变。
- 插入新键触发一次拆桶、多次连续拆桶。
- 局部深度小于全局深度时只拆桶，不扩大目录。
- 扩容后所有旧键仍可查，兄弟目录项指向关系正确。
- 删除存在/不存在键、删除后再次插入，其他键值保留。
- 并发插入/查找/删除后状态一致；基础测试保留已有并发用例。

原测试已含 `InsertSplit`、`InsertMultipleSplit`、`InitiallyEmpty`、`InsertAndReplace`、深度/桶数和并发测试。追加针对实现风险的用例，保留已有断言。

## 你的测试命令

环境配置见[构建指南](../BUILD.md)。每次改动后：

```bash
cmake --build build --target extendible_hash_table_test -j 4
./build/test/extendible_hash_table_test
```

定位问题时可筛选：

```bash
./build/test/extendible_hash_table_test --gtest_filter=ExtendibleHashTableTest.InsertSplit
```

提交前回到完整测试，PR 写出实际用例数量和结果。

## 交给 4 号什么

确认 `Find(K, V&)`、void `Insert(K,V)`、bool `Remove(K)` 签名不变，说明哈希表已经内部加锁。提供一次“插入映射→查找帧号→删除旧映射”的调用例子和完整测试结果。发现需要改公共接口时先找 1、4 号确认。

## 你的推送步骤

先完成[克隆和本人身份配置](../GIT_WORKFLOW.md)，首次创建分支：

```bash
git switch main
git pull --ff-only origin main
git switch -c team/02-hash
```

完成实现和基础测试后：

```bash
git fetch origin
git merge origin/main
cmake --build build --target extendible_hash_table_test -j 4
./build/test/extendible_hash_table_test
git status
git diff
git add src/include/container/hash/extendible_hash_table.h src/container/hash/extendible_hash_table.cpp test/container/hash/extendible_hash_table_test.cpp
git diff --cached
git commit -m "feat(p1): implement extendible hash table"
git push -u origin team/02-hash
```

打开[新建哈希表 PR](https://github.com/milaqihe/bustub-initial/compare/main...team/02-hash?expand=1)，base 为 main，提交给 1 号审阅，4 号复核接口。后续修改在同分支 commit 后 `git push`，原 PR 自动更新。

## 完成标准

你的三个负责文件完成实现与测试；完整哈希表测试通过，重复键、连续拆桶、深度/桶数和并发均有覆盖；4 号能直接使用接口；PR 已提交并根据审阅修复。5 号后续独立验收不替代你的基础测试。
