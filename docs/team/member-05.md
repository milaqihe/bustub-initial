# 5 号 质量与验收负责人

你负责独立补充边界、内存、并发和压力测试，整理测试记录、流程图、实验报告与答辩材料。模块负责人负责自己的基础测试，你负责发现它们没有覆盖的问题，并针对集成版本给出验收结论。

## 你实际写在哪里

- [test/p1_quality/](../../test/p1_quality/README.md)：已创建的独立质量测试入口；新增 `.cpp` 放在这里。
- [验收计划与记录](../../reports/p1/acceptance.md)：已有用例清单和执行记录模板。
- [实验报告模板](../../reports/p1/report.md)：设计、流程、实验和分工章节。
- [报告目录说明](../../reports/p1/README.md)：产物及其负责人。
- [构建与 Sanitizer 命令](../BUILD.md)、[接口预期](../INTERFACES.md)。

1 号维护 `reports/p1/integration.md`。你维护其他验收/报告文件，避免同时改这份集成记录。原有三个模块测试文件由 2、3、4 号维护，你的新增测试放独立目录。

## 按顺序完成

1. 阅读六个模块头文件、Page/DiskManager 接口和现有三组测试，标出已有覆盖及缺口。
2. 在 `acceptance.md` 补充每个用例的输入、操作顺序、预期结果、严重程度和对应负责人。
3. 先手算哈希拆分、LRU-K 淘汰结果与 BPM 状态；预期不能通过复制被测实现计算。
4. 在独立目录添加有意义的边界/回归测试，文件名使用本指南约定，测试名也全局可辨。
5. 接入自动测试发现，重新运行 CMake，再分别构建和运行新增目标。
6. 集成实现齐备后先跑现有三组完整测试，再跑补充用例，记录被测提交号与环境。
7. 分开执行 ASan 和 TSan；内存错误、竞态、挂起、断言失败分别记录。
8. 加入容量 1/2、小容量反复换页、固定种子随机访问与多线程重复运行；明确操作数量、线程数、超时和正确性预期。
9. 用超时包装器或 CTest 的测试超时识别死锁；Linux 可用 `timeout 120s` 包装单次运行。单纯运行很久不能作为“压力通过”的证据。
10. 发现问题时，把最小复现、预期/实际、提交号、环境、日志摘要交 2/3/4 号修复，1 号跟踪处理。
11. 修复合并后重跑对应回归，并在最终 main 版本重跑约定验收；旧提交通过不能自动代表最终版本通过。
12. 汇总报告、页面操作流程图和每人的答辩提纲；用实际数据填表，不提前写“全部通过”。

## 新增测试文件与目标

按需创建以下文件；它们是待你编写的实现任务，当前目录只提供说明：

| 新文件 | 自动生成的目标 | 重点 |
| --- | --- | --- |
| `test/p1_quality/p1_quality_hash_test.cpp` | `p1_quality_hash_test` | 满桶更新、连续拆分、并发状态 |
| `test/p1_quality/p1_quality_lruk_test.cpp` | `p1_quality_lruk_test` | 真正的第 K 次访问排序、边界 ID、计数与历史清理 |
| `test/p1_quality/p1_quality_bpm_test.cpp` | `p1_quality_bpm_test` | 全 pinned、脏页重载、帧复用、FlushAll、死锁与并发 |

`test/CMakeLists.txt` 使用文件名作为目标名，不要把文件命名成已有的 `lru_k_replacer_test.cpp` 等，避免重名。建议测试套件分别命名 `P1QualityHashTest`、`P1QualityLRUKTest`、`P1QualityBPMTest`。

三份文件都写好后：

```bash
cmake -S . -B build
cmake --build build --target p1_quality_hash_test p1_quality_lruk_test p1_quality_bpm_test -j 4
./build/test/p1_quality_hash_test
./build/test/p1_quality_lruk_test
./build/test/p1_quality_bpm_test
```

只创建其中一份时，仅构建对应目标。ASan/TSan 目录也需要重新 configure 后构建这些目标。尚未创建文件前执行上述新目标会报不存在，这是正常的任务起点。

## 并发与压力用例怎样写

- 对独立页或独立 key 的操作可用分段所有权验证最终集合，重复运行使用固定且记录的随机种子。
- 验证同一页并发写时，测试自身先遵守 `Page` 页面锁约定；BPM 元数据锁不自动保护调用方的数据写入。
- 多线程中避免失败后提前退出导致其他线程未 join；需要把线程内结果汇总回主线程检查。
- 测试文件使用独立磁盘文件名并在结束时清理，避免不同用例互相覆盖文件；异常路径用 RAII 清理资源。
- 内存/竞态检测通过与吞吐性能是不同结论。性能数据必须记录硬件、构建类型和负载，Debug+Sanitizer 耗时不能直接当作正常 Release 性能。
- 重复执行可用 `--gtest_repeat=20 --gtest_break_on_failure`，但必须说明重复的是哪些目标/用例，且每轮独立恢复初始状态。

## 你的推送步骤

先完成[克隆和本人身份配置](../GIT_WORKFLOW.md)，首次创建分支：

```bash
git switch main
git pull --ff-only origin main
git switch -c team/05-quality
```

补完测试与记录后，先同步最新集成代码并运行相关用例，再提交本人目录：

```bash
git fetch origin
git merge origin/main
git status
git diff
git add test/p1_quality/ reports/p1/README.md reports/p1/acceptance.md reports/p1/report.md
git diff --cached
git commit -m "test(p1): add quality coverage and acceptance records"
git push -u origin team/05-quality
```

打开[新建质量验收 PR](https://github.com/milaqihe/bustub-initial/compare/main...team/05-quality?expand=1)，base 为 main。报告框架和测试可以分成小 PR；有可复现失败时明确注明，交 1 号协调修复，不把失败状态写成验收通过。

## 完成标准

约定边界、并发、压力和 ASan/TSan 项目有实际执行结论；发现的缺陷有回归记录；报告对应最终被测提交；三名模块负责人提供的算法说明和 1 号集成记录已汇总；流程图与答辩材料能解释真实代码的运行过程。
