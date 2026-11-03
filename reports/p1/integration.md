# 1 号集成记录

更新日期：2026-11-03。工作分支：`team/01-integration`；集成基线：main 的 `041dfed`。本轮完成公共构建、P1 检查入口、CI、接口基线及打包配置。

## 环境与构建

| 项目 | 本机验证环境 | 团队 CI 环境 |
| --- | --- | --- |
| 系统 | macOS 26.4，arm64 | Ubuntu 22.04 |
| 编译器 | AppleClang 21.0.0 | Clang 14 |
| CMake | 3.31.10 | 3.31.10 |
| clang-format | 14.0.6 | 14 |
| clang-tidy | 14.0.6 | 14 |
| 构建类型 | Debug + ASan | Debug + ASan |

构建目录为 `build/integration`。通用安装和配置命令见[构建指南](../../docs/BUILD.md)。已有配置可执行：

```bash
cmake -S . -B build/integration
cmake --build build/integration --target build-p1-tests -j 4
cmake --build build/integration --target check-p1 -j 4
cmake --build build/integration --target check-p1-format
cmake --build build/integration --target check-p1-lint
cmake --build build/integration --target check-clang-tidy-p1
```

公共 CMake 将第三方头文件标记为 SYSTEM，解决 bundled fmt 在新 SDK 下的弃用告警编译错误。项目自身保留严格告警；Python 辅助脚本由解释器调用，兼容压缩包解压后的文件权限。

## 接口与任务交接

以[接口约定](../../docs/INTERFACES.md)为开发基线：

| 对接成员 | 交接内容 | 合并前验证 |
| --- | --- | --- |
| 2 号 | 线程安全的页号到帧号映射；Find/Insert/Remove 与深度查询签名保持一致 | 哈希表完整测试、满桶更新、拆分和并发 |
| 3 号 | LRU-K 五个方法；Size 统计可淘汰帧，Evict 清理历史 | 淘汰顺序、K=1/2/3、状态计数和边界 |
| 4 号 | 缓冲池六个页面操作；pin/dirty、旧映射和帧复用规则 | 同步 2、3 号后运行完整缓冲池测试 |
| 5 号 | 独立测试目录 `test/p1_quality/`，约定命名的测试自动进入 P1 检查 | 边界、并发、ASan/TSan、缺陷复测与报告 |

本轮保持六个模块源码和公开签名不变。模块 PR 按“哈希表/LRU-K → 缓冲池 → 补充测试与最终验收”顺序集成。

## 实际验证结果

| 项目 | 结果 |
| --- | --- |
| CMake 配置 | 通过 |
| build-p1-tests | 三组 P1 测试程序编译、链接成功 |
| check-p1-format | 通过 |
| check-p1-lint | 通过 |
| p1_test.sh | Bash 语法通过，功能测试失败时返回非零 |
| CI YAML | 语法及触发分支配置检查通过，云端运行结果见 Actions |
| check-p1 | 29 个功能用例失败：哈希表 12、LRU-K 4、缓冲池 13；当前模块仍为初始实现 |
| check-clang-tidy-p1 | 本机 clang-tidy 14 无法正常解析 macOS 26.4 SDK，交 Linux/Clang 14 环境复核 |
| 提交包结构 | 恰好六个 P1 文件；重建能够清除旧 zip 的多余条目 |

本地原始输出位于 `build/integration/`，构建目录由 Git 忽略。打包结构验证产物为该目录下的 `project1-layout-preview.zip`。功能通过后，由 1 号在最终 main 上生成正式课程提交包。

## 合并与最终验收

本轮远端核实时只有 main，没有开放的模块 PR。后续按以下顺序推进：

1. 2、3 号提交各自实现和测试，1 号审阅合并。
2. 4 号同步已合并依赖，完成缓冲池联调。
3. 5 号提交独立测试及验收结果，模块负责人修复并复测。
4. 1 号在最终 main 重跑全部 P1 检查，生成六文件提交包，记录课程提交时间与回执。
