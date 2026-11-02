# 3 号 LRU-K 负责人

你完整负责 LRUKReplacer 的实现和基础测试，为 4 号提供“从可淘汰帧中选择牺牲帧”的策略。

## 直接打开这些文件

- [lru_k_replacer.h](../../src/include/buffer/lru_k_replacer.h)：五个方法的契约、容量与时间戳成员。
- [lru_k_replacer.cpp](../../src/buffer/lru_k_replacer.cpp)：当前方法为空实现，需要补全。
- [lru_k_replacer_test.cpp](../../test/buffer/lru_k_replacer_test.cpp)：你的基础测试，含 `SampleTest`、`Evict`、`Size`、`ConcurrencyTest`。
- [接口约定](../INTERFACES.md)：淘汰语义、合法帧编号及与 4 号的调用关系。

课程附件提供[LRU-K 头文件](../../references/P1_src/include/buffer/lru_k_replacer.h)和[参考实现](../../references/P1_src/buffer/lru_k_replacer.cpp)。其成熟帧排序与通用 LRU-K 定义有出入，先读[参考说明](../../references/README.md)，再在正式 `src/` 中完成实现。

## 按顺序完成

1. 确定每帧状态：最近 K 次访问时间、是否可淘汰、是否存在记录；选用合适的内部容器。
2. 维护单调递增的逻辑时间戳；记录一次访问就更新一次，不依赖真实时钟。
3. `RecordAccess` 检查 `0 <= frame_id < num_frames`，首次访问创建记录，后续仅保留所需历史。
4. 首次记录不自动变成可淘汰，`Size` 不因单纯访问而增加。
5. `SetEvictable` 只在状态实际变化时增减 `curr_size_`；重复设同一状态不重复计数。
6. 合法但尚无访问记录的帧，`SetEvictable` 不创建条目；非法 ID 报错/断言。
7. `Evict` 先排除不可淘汰帧，再优先选不足 K 次的帧；其中按最早历史时间选择。
8. 候选都达到 K 次时，比较倒数第 K 次时间，选最早者；不用最近一次时间代替。
9. 淘汰成功把帧 ID 写到输出指针，删除该帧历史和状态，减少可淘汰数量；没有候选 false。
10. `Remove` 删除指定可淘汰帧；不存在无操作，不可淘汰则按头文件报错/断言。
11. `Size` 返回可淘汰数量；所有相关读写由同一同步方案保护，避免并发下计数漂移。
12. 覆盖 K=1、K=2、K=3；不要硬编码两个队列就视为完成通用 K。

不足 K 次的页面也可能已经驻留在缓冲池，不存在“访问达到 K 次才允许装入内存”的要求。本模块跟踪的是帧，不读写磁盘，不修改 Page 的 pin_count。

## 必须覆盖的基础用例

| 场景 | 预期 |
| --- | --- |
| 没有帧或全不可淘汰 | Evict false，Size 为 0 |
| 不足 K 次与达到 K 次混合 | 优先淘汰不足 K 次的可淘汰帧 |
| K=2，A 历史 1、9；B 历史 2、3 | 淘汰 A；这是区分最近访问与第 K 次访问的用例 |
| 多个不足 K 次候选 | 选择最早历史时间，不因最近一次访问覆盖而误排 |
| 重复设 true/false | Size 只在状态变化时增减 |
| 淘汰后相同 frame_id 再次访问 | 新记录重新开始，旧历史不残留 |
| Remove 不存在/不可淘汰帧 | 分别无操作/按契约报错 |
| frame_id 为 -1 或 num_frames | 非法，不能访问越界内存 |
| K=1 和 K=3 | 与参数对应的正确顺序 |
| 多线程访问、切换状态、删除与淘汰 | 无数据竞争，最终数量与成员状态一致 |

错误输入用例依据实现采用异常断言或 death test；先和 1 号约定，不削弱头文件要求。

## 你的测试命令

先按[构建指南](../BUILD.md)配置环境：

```bash
cmake --build build --target lru_k_replacer_test -j 4
./build/test/lru_k_replacer_test
```

定位用例可用 `--gtest_filter=LRUKReplacerTest.Evict`；提交前必须运行整个可执行文件。

## 交给 4 号什么

给出五个方法的实际行为与完整测试结果。明确：New/Fetch 后先记录访问并设不可淘汰；最后一次 Unpin 才可淘汰；Evict 已删除历史，BPM 不需要重复 Remove。交一份 K=2 的手算序列供 4、5 号核对。

## 你的推送步骤

先完成[克隆和本人身份配置](../GIT_WORKFLOW.md)，首次创建分支：

```bash
git switch main
git pull --ff-only origin main
git switch -c team/03-lruk
```

完成后同步、测试、提交：

```bash
git fetch origin
git merge origin/main
cmake --build build --target lru_k_replacer_test -j 4
./build/test/lru_k_replacer_test
git status
git diff
git add src/include/buffer/lru_k_replacer.h src/buffer/lru_k_replacer.cpp test/buffer/lru_k_replacer_test.cpp
git diff --cached
git commit -m "feat(p1): implement LRU-K replacer"
git push -u origin team/03-lruk
```

打开[新建 LRU-K PR](https://github.com/milaqihe/bustub-initial/compare/main...team/03-lruk?expand=1)，base 为 main，1 号审阅、4 号复核调用约定。后续在同分支提交并 `git push`。

## 完成标准

完整现有测试与新增基础用例通过；淘汰顺序符合第 K 次访问定义；Size、历史清理和边界输入正确；方法线程安全；4 号可按固定接口使用；PR 中有真实测试结果和算法说明。
