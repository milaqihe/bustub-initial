# 1 号集成记录

更新日期：2026-11-16。三个核心模块已合入 `main`，当前模块集成版本为 `dfa3c63`。本次在 `team/05-quality` 汇总报告中的环境、集成过程与 1 号贡献；质量分支的已验证版本为 `bd966e1`，最终验收和课程提交在报告汇总合并后进行。

## 环境与构建

| 项目 | 1 号本机复跑环境 | 团队 CI 环境 |
| --- | --- | --- |
| 系统与架构 | macOS 26.4，arm64 | Ubuntu 22.04，x86_64 |
| 编译器 | AppleClang 21.0.0 | Clang / Clang++ 14 |
| CMake | 3.31.10 | 3.31.10 |
| clang-format | 14.0.6 | 14 |
| clang-tidy | 14.0.6；无法正常解析本机新 SDK | 14；实际检查通过 |
| 构建类型 | Debug + ASan | Debug + ASan，`detect_leaks=1:halt_on_error=1` |

本机复跑使用 `build/integration`。安装和 Sanitizer 分目录构建方法见[构建指南](../../docs/BUILD.md)，报告第 2 章提供 Linux 环境的完整复现命令。本机已配置目录使用以下入口：

```bash
cmake -S . -B build/integration -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=address
cmake --build build/integration --target check-p1 -j 4
cmake --build build/integration --target check-p1-format check-p1-lint
```

Linux CI 另外执行 `check-clang-tidy-p1`，统一采用 14 版工具。本机 SDK 不兼容的 clang-tidy 执行不计为通过。5 号的本地 ASan/TSan 环境与结果在 [acceptance.md](acceptance.md) 中记录，与 CI 环境分别列出。

## 公共集成配置

1 号在提交 `78b155e` 中完成公共集成配置，通过 [PR #2](https://github.com/milaqihe/bustub-initial/pull/2) 合入：

- 增加 `build-p1-tests` / `check-p1`，只运行 P1 标签用例，单个用例超时 120 秒；约定命名的质量测试自动接入。
- CI 监听 `main`、`team/**` 推送及目标为 `main` 的 PR，执行构建、测试、格式、lint、clang-tidy，并保留测试诊断；全部检查通过后生成提交包。
- 将第三方头文件标记为 SYSTEM，兼容本机新 SDK；Python 构建辅助脚本由解释器调用。
- 按 `P1_FILES` 统一六文件打包，`submit-p1` 通过 CMake 原生 zip 功能生成 `project1-submission.zip`。

## 接口与任务交接

以[接口约定](../../docs/INTERFACES.md)为开发基线：

| 对接成员 | 交接内容 | 合并前验证 |
| --- | --- | --- |
| 2 号 | 线程安全的页号到帧号映射；Find/Insert/Remove 与深度查询签名保持一致 | 哈希表完整测试、满桶更新、拆分和并发 |
| 3 号 | LRU-K 五个方法；Size 统计可淘汰帧，Evict 清理历史 | 淘汰顺序、K=1/2/3、状态计数和边界 |
| 4 号 | 缓冲池六个页面操作；pin/dirty、旧映射和帧复用规则 | 同步 2、3 号后运行完整缓冲池测试 |
| 5 号 | 独立测试目录 `test/p1_quality/`，约定命名的测试自动进入 P1 检查 | 边界、并发、ASan/TSan、缺陷复测与报告 |

各模块公开签名保持一致，按“哈希表/LRU-K → 缓冲池 → 补充测试与最终验收”顺序集成。BPM 持有自身元数据锁后调用哈希表和 LRU-K 的公开方法，下层不回调 BPM；页面数据并发写入由调用方遵守 Page 锁约定。

## 模块审阅与合并记录

| 日期 | PR 与负责人 | 审阅和验证 | 合并后的 main |
| --- | --- | --- | --- |
| 2026-11-03 | [#2 公共集成配置](https://github.com/milaqihe/bustub-initial/pull/2)，1 号 | 构建入口、接口、CI 和打包配置；当时功能模块尚未齐备，后续按模块验收 | `a9d9cf1` |
| 2026-11-03 | [#1 哈希表](https://github.com/milaqihe/bustub-initial/pull/1)，2 号 | 审阅初始化、重复键、拆分与线程保护；本地 Debug + ASan 复跑 12/12 通过 | `3eedb16` |
| 2026-11-10 | [#3 LRU-K](https://github.com/milaqihe/bustub-initial/pull/3)，3 号 | 审阅历史、淘汰顺序、计数与清理；本地 Debug + ASan 复跑 13/13 通过 | `1094b91` |
| 2026-11-13 | [#4 缓冲池](https://github.com/milaqihe/bustub-initial/pull/4)，4 号 | 审阅六操作与 AcquireFrame；缓冲池 13/13、三模块合计 38/38 通过；1 号修正 24 处注释空格 `ed7f342` 后 CI 全绿 | `dfa3c63` |

三个模块的 GitHub 批准与合并均使用组长 `milaqihe` 账号。质量与报告内容继续在 `team/05-quality` 汇总，由 1 号统一审阅合入 `main`。

## 实际验证结果

| 被测版本 | 执行环境与范围 | 实际结果与证据 |
| --- | --- | --- |
| 哈希表 PR 的 `e417fd4`、LRU-K PR 的 `eb97883`、缓冲池 PR 的 `ef48724` | 1 号本机 Debug + ASan，分别复跑模块测试；缓冲池接入后运行三模块 | 哈希表 12/12、LRU-K 13/13、缓冲池 13/13，完整三模块 38/38 通过 |
| `main` 的 `dfa3c63` | Ubuntu 22.04 / Clang 14 / Debug + ASan | [CI 全部通过](https://github.com/milaqihe/bustub-initial/actions/runs/43398140831)：构建、基础测试、格式/lint、clang-tidy 和打包 |
| `team/05-quality` 的 `79b91b5` | 同一 CI，加入补充测试 | [48/48 通过](https://github.com/milaqihe/bustub-initial/actions/runs/43697051038)，含 38 项基础测试、6 项 BPM 补充测试和 4 项 LRU-K 补充测试；全部检查与打包成功 |
| `team/05-quality` 的 `bd966e1` | 同一 CI，加入验收文档与截图 | [48/48 通过](https://github.com/milaqihe/bustub-initial/actions/runs/43722662432)，测试总耗时 4.33 秒；全部检查与打包成功 |
| 5 号本地 ASan / TSan | 由 5 号执行并提交截图 | [ASan 截图](images/asan_check_p1.png)与[TSan 截图](images/tsan_check_p1.png)均显示 48/48 通过；实际工具版本、被测提交号与重复/压力运行记录由验收文档补齐 |

CI 使用 `BUSTUB_SANITIZER=address`，其结果覆盖 ASan 及配置中的泄漏检测。TSan 为独立本地验收记录，不归入 CI 的 ASan 检查结果。当前补充测试位于质量分支，最终结果以报告汇总合并后的 `main` 复跑为准。本地原始输出位于 `build/integration/`，构建目录由 Git 忽略。

## 合并与最终验收

2—4 号分别补报告第 3—5 章的实际设计、示例和图；5 号汇总第 6 章测试结果并完善验收记录。各人补本人贡献与答辩内容，1 号统一环境与版本信息。

汇总 PR 合入 `main` 后，1 号记录最终提交号，执行 P1 完整测试、格式、lint 和 Linux clang-tidy，并与 5 号验收记录对应；通过后生成正式包：

```bash
git switch main
git pull --ff-only origin main
git rev-parse HEAD
cmake --build build/asan-clang14 --target check-p1 -j 4
cmake --build build/asan-clang14 --target check-p1-format check-p1-lint check-clang-tidy-p1
cmake --build build/asan-clang14 --target submit-p1
unzip -l project1-submission.zip
```

上述 Linux 构建目录先按报告第 2 章配置。包内保留仓库相对路径，内容为：

```text
src/include/container/hash/extendible_hash_table.h
src/container/hash/extendible_hash_table.cpp
src/include/buffer/lru_k_replacer.h
src/buffer/lru_k_replacer.cpp
src/include/buffer/buffer_pool_manager_instance.h
src/buffer/buffer_pool_manager_instance.cpp
```

截至本次更新，CI 已能生成六文件提交包。正式交付将在报告汇总与最终验收完成后执行，届时补记最终 main 提交号、验收结果、包名、课程提交时间与回执。
