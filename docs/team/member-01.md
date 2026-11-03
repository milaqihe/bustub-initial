# 1 号 组长与集成负责人

你负责让五个人的成果能在同一环境构建、通过测试并形成最终提交。2、3、4 号分别完成模块实现，5 号独立验收；你负责公共配置、接口协调、审阅合并与最终联调。

## 代码与文档入口

- [根 CMakeLists.txt](../../CMakeLists.txt)：C++17、Sanitizer、静态检查、P1 六文件清单和 `submit-p1`。
- [测试 CMakeLists.txt](../../test/CMakeLists.txt)：按 `test/*/*test.cpp` 自动生成测试目标。
- [P1 测试入口](../../p1_test.sh)：构建并运行 P1，失败立即返回非零，不自动修改源码格式。
- [环境搭建](../BUILD.md)、[接口约定](../INTERFACES.md)、[集成记录](../../reports/p1/integration.md)。
- [P1 CI](../../.github/workflows/cmake.yml)：监听 main、团队分支与 main 的 PR，使用 Ubuntu 22.04、Clang 14、CMake 3.31.10 和 ASan。

## 具体任务

1. 确认 2—5 号的 GitHub 用户名，邀请协作者，并让每人接受邀请、克隆和推送自己的分支。
2. 选择团队统一构建环境，实际记录 OS、编译器、CMake、format/tidy 版本与构建命令。
3. 协助全组首次构建，区分编译依赖错误与初始空实现导致的测试失败。
4. 和 2、3、4 号确认 `Find/Insert/Remove`、LRU-K 五个方法、BPM 六个方法的固定签名与状态规则。
5. 将公共接口争议落实到 `docs/INTERFACES.md`，避免各人自行改名或改变返回类型。
6. 管理公共 CMake 修改。5 号新增测试如已匹配自动发现规则，无需手工增加重复目标。
7. 若启用自动检查，适配 main 和 P1 三组目标；更新原有过期环境配置时另开明确的配置 PR，先验证后采用。
8. 审阅 2、3 号模块 PR：范围、返回值、基础测试、线程保护和给 4 号的接口均符合约定。
9. 合并 2、3 号后，通知 4 号同步 main；审阅并合并缓冲池实现，特别检查 pin、dirty、旧映射与死锁。
10. 接收 5 号缺陷复现，分配到模块负责人修复，不通过削弱断言或跳过用例解决失败。
11. 在最终 main 执行三组测试、补充测试和约定检查；核对 5 号报告的被测提交号与最终版本。
12. 生成 `project1-submission.zip`，确认六个文件，汇总报告、组员分工和提交信息，按老师渠道交付。

## 审阅时逐项检查

- 2 号：满桶旧键更新不拆桶；目录翻倍与桶拆分条件正确；计数为实际桶数。
- 3 号：不足 K 次优先；成熟帧按倒数第 K 次时间排序；`Size` 只数可淘汰帧；Evict 清历史。
- 4 号：优先 free list；全部 pinned 时返回失败；dirty 写回后复用；FlushAll 不递归锁死。
- 5 号：独立预期、稳定复现、测试归因准确；ASan/TSan 受阻不能写通过。

## 集成命令

先按[构建指南](../BUILD.md)配置环境。在合并后的完整版本运行：

```bash
cmake --build build --target extendible_hash_table_test lru_k_replacer_test buffer_pool_manager_instance_test -j 4
./build/test/extendible_hash_table_test
./build/test/lru_k_replacer_test
./build/test/buffer_pool_manager_instance_test
cmake --build build --target check-p1 -j 4
cmake --build build --target check-p1-format check-p1-lint check-clang-tidy-p1
```

5 号新增目标按实际文件名另行构建运行。所有最终结果记录到[集成记录](../../reports/p1/integration.md)。

## 你的推送步骤

先完成[克隆和本人身份配置](../GIT_WORKFLOW.md)，首次开分支：

```bash
git switch main
git pull --ff-only origin main
git switch -c team/01-integration
```

完成环境、接口说明和集成记录后，仅暂存这次实际修改的文件：

```bash
git status
git diff
git add docs/BUILD.md docs/INTERFACES.md reports/p1/integration.md
git diff --cached
git commit -m "docs(p1): document build and integration decisions"
git push -u origin team/01-integration
```

如本次确实修改 CMake 或 CI，再逐个 `git add` 对应文件；不要为了凑提交去改配置。打开[新建集成 PR](https://github.com/milaqihe/bustub-initial/compare/main...team/01-integration?expand=1)，base 为 main，请相关成员复核。

最终验收后：

```bash
git switch main
git pull --ff-only origin main
cmake --build build --target submit-p1
unzip -l project1-submission.zip
```

## 完成标准

- 五人分支可协作、公开接口一致、模块 PR 已按依赖合并。
- 全组可照文档重现构建，最终 main 的测试与静态检查有真实结论。
- 5 号验收与最终提交号对应，待处理缺陷有明确结论。
- 最终课程压缩包只有要求的六个文件，报告和提交渠道已按老师通知落实。
