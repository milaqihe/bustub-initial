# Project 1 最终验收记录

整理日期：2026-11-23。P1 三个模块及独立质量测试合计 **48 项**，完整 ASan 复跑通过；每项用例重复 20 次，共 **960 次执行全部通过**。运行截图见[截图汇总](README.md#运行截图)。

## 被测版本与环境

| 记录 | 版本与环境 |
| --- | --- |
| 核心模块集成 | `dfa3c63`，三个模块实现已全部合并 |
| 质量测试与组员截图 | 测试源代码提交 `79b91b5`；截图随 `bd966e1` 提交 |
| 最终报告汇总基线 | `cc2dc2b`，含 2—4 号完整设计章节；本次整理不修改模块实现和 48 项用例逻辑 |
| 2026-11-23 组长复跑 | macOS 26.4 arm64，AppleClang 21.0.0，CMake 3.31.10，Debug + ASan |
| 统一 CI | Ubuntu 22.04 x86_64，Clang/format/tidy 14，CMake 3.31.10，Debug + ASan；开启 Linux 泄漏检测 |
| 最终发布版本 | 项目默认分支 `main`；精确提交号及运行结果由对应 [Actions 记录](https://github.com/milaqihe/bustub-initial/actions/workflows/cmake.yml?query=branch%3Amain)关联 |

## 测试套件与结果

| 套件 | 数量 | 结果 |
| --- | --- | --- |
| ExtendibleHashTableTest | 12 | 通过 |
| LRUKReplacerTest | 13 | 通过 |
| BufferPoolManagerInstanceTest | 13 | 通过 |
| P1QualityLRUKTest | 4 | 通过 |
| P1QualityBPMTest | 6 | 通过 |
| **合计** | **48** | **48/48 通过** |

## 独立用例与实际参数

| 用例 | 验证的预期与参数 |
| --- | --- |
| ThirdPreviousAccessBeatsRecentAccess | K=3；帧 0 的历史为 0/4/5，帧 1 为 1/2/3，先淘汰帧 0 |
| RemoveAndReuseClearsHistoryAndSize | 重复设为可淘汰不重复计数；Remove 后同 ID 重建历史，不足 K 次者优先 |
| ExactFrameBoundsAndUnknownState | 负帧号、等于容量的帧号报错；从未访问的合法帧不凭空加入 |
| ConcurrentDistinctFramesKeepExactCount | 4 线程，每线程 16 帧；最终 64 帧恰好各淘汰一次，Size 归零 |
| PoolOnePinsAndReloadsDirtyBinaryPage | 容量 1；全 pinned 时失败、命中增加 pin、dirty 不被 false 清除、二进制脏页淘汰后完整读回 |
| FlushAllSkipsUnusedFramesAndFlushPreservesPin | 空帧不写盘；有效页写回清 dirty；显式 Flush 干净页也写盘，pin 不变 |
| DeletePinnedPageAndReuseCleanFrame | pinned 页删除失败；Unpin 后删除成功且可重复调用；复用帧的数据与元数据重置 |
| PoolTwoFixedSeedPageChurn | 容量 2、100 个页面、随机种子 20261115；逐页写入后打乱顺序读回并比较 |
| ConcurrentIndependentPages | 4 线程各持独立页，每线程 100 次更新；所有线程结束后验证最终值 |
| ConcurrentSharedPageUsesPageLatch | 容量 1，共享页写入遵守 Page 写锁；4 线程各累加 100 次，最终结果 400 |

基础哈希用例另覆盖单次/多次拆分、深度、实际桶数、更新、删除及并发。每个 CTest 用例超时为 120 秒。

## 2026-11-23 最终复跑

组长在 `build/integration` 的 Debug + ASan 构建执行：

```bash
cmake --build build/integration --target check-p1 check-p1-format check-p1-lint -j 4
ctest --test-dir build/integration --output-on-failure --no-tests=error -L '^p1$' --repeat until-fail:20
```

| 项目 | 实际结果 |
| --- | --- |
| 完整构建与测试 | 48/48，通过，单轮耗时 4.77 秒 |
| 每项用例重复 20 次 | 960 次执行全部通过，总耗时 90.34 秒 |
| clang-format 14 与 cpplint | 通过 |

重复测试使用 CTest 的 `until-fail:20`，即每个用例独立启动并最多执行 20 次，任一次失败即报告失败。本次全部用例均完成 20 次。Linux clang-tidy 在 CI 统一执行。

## ASan / TSan 运行截图

5 号提交的 [ASan 截图](images/asan_check_p1.png)显示 48/48 通过，耗时 4.14 秒；[TSan 截图](images/tsan_check_p1.png)显示使用 `BUSTUB_SANITIZER=thread` 的独立构建，48/48 通过，耗时 10.17 秒。截图记录的是组员 2026-11-15 的运行，2026-11-23 的组长复跑另列于上表。

CI 固定使用 ASan，其泄漏检测结果与组员的 TSan 运行分别记录。可从 [2026-11-15 的 CI](https://github.com/milaqihe/bustub-initial/actions/runs/43697051038)和[报告汇总基线 CI](https://github.com/milaqihe/bustub-initial/actions/runs/44069233922)查看实际执行步骤；最终默认分支记录见页面顶部入口。

## Linux 复现命令

先按[构建指南](../../docs/BUILD.md)安装工具，在仓库根目录运行：

```bash
cmake -S . -B build/asan-clang14 -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=address -DCMAKE_C_COMPILER=clang-14 -DCMAKE_CXX_COMPILER=clang++-14 -DCLANG_FORMAT_BIN=clang-format-14 -DCLANG_TIDY_BIN=clang-tidy-14
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 cmake --build build/asan-clang14 --target check-p1 -j 4
cmake --build build/asan-clang14 --target check-p1-format check-p1-lint check-clang-tidy-p1

cmake -S . -B build/tsan-clang14 -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=thread -DCMAKE_C_COMPILER=clang-14 -DCMAKE_CXX_COMPILER=clang++-14
TSAN_OPTIONS=halt_on_error=1 cmake --build build/tsan-clang14 --target check-p1 -j 4
```

## 修复与结论

| 实际问题 | 修复 | 回归结果 |
| --- | --- | --- |
| 新 macOS SDK 触发第三方头文件告警 | PR #2：第三方头文件采用 SYSTEM 路径，保留项目自身严格告警 | 本机三个模块均能构建并通过测试 |
| 缓冲池注释缺少空格导致 lint 失败 | `ed7f342` 修正 24 处注释空格 | PR #4 修正后 CI 全绿 |
| 补充用例的格式检查失败 | `79b91b5` 规范 BPM/LRU-K 测试格式 | 48 项测试与静态检查通过 |

结论：已提交的三个模块、48 项基础及补充用例通过现有验收；完整代码、报告、流程图与真实运行截图在登记的项目链接中统一展示。
