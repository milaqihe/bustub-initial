# P2 构建、测试与打包

从仓库根目录执行。复用 P1 的 C++17、CMake 3.26 以上环境，团队推荐 Ubuntu 22.04 / Clang 14 / CMake 3.31.10。安装步骤见[原构建指南](../BUILD.md)，本页以 P2 的实际文件和目标为准。

## 配置独立构建目录

Linux / WSL：

```bash
cmake -S . -B build/p2-asan -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=address -DCMAKE_C_COMPILER=clang-14 -DCMAKE_CXX_COMPILER=clang++-14 -DCLANG_FORMAT_BIN=clang-format-14 -DCLANG_TIDY_BIN=clang-tidy-14
```

macOS 已安装工具的同学：

```bash
cmake -S . -B build/p2-asan -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=address
```

换编译器使用新目录，例如 `build/p2-asan-clang19`。TSan 另外使用 `build/p2-tsan` 并设置 `-DBUSTUB_SANITIZER=thread`；不能把 ASan 与 TSan 缓存混在一个目录中。

## 本仓库真实存在的目标

```bash
cmake --build build/p2-asan --target b_plus_tree_insert_test b_plus_tree_delete_test b_plus_tree_concurrent_test -j 4
./build/p2-asan/test/b_plus_tree_insert_test
./build/p2-asan/test/b_plus_tree_delete_test --gtest_also_run_disabled_tests
./build/p2-asan/test/b_plus_tree_concurrent_test
```

| 目标 | 当前源码中的用例 | 对应阶段 |
| --- | --- | --- |
| `b_plus_tree_insert_test` | InsertTest1、InsertTest2、ScaleTestC1，3 项 | 查询、插入和规模测试；部分用例同时使用 Begin/End，需要迭代器联调 |
| `b_plus_tree_delete_test` | DISABLED_DeleteTest1 / 2，2 项 | 默认禁用，验收时必须用上面的 `--gtest_also_run_disabled_tests` 显式运行，不能把 0 项运行算通过 |
| `b_plus_tree_concurrent_test` | 插入 2、删除 2、混合 1，共 5 项 | 完整单线程树与并发改造接入后运行 |
| `b_plus_tree_contention_test` | contention benchmark；另有默认禁用的 benchmark2 | 5 号单独测锁竞争；该目标主要计时，正确性仍由功能测试与独立断言验证 |

教程写到的 `b_plus_tree_sequential_scale_test` 在本仓库没有单独源码，规模测试是上表中的 `ScaleTestC1`，不要直接照抄不存在的目标。

当前 P2 是待实现骨架。先构建、再运行本人负责目标，PR 写出实际通过/失败/未具备依赖的情况。每个模块先做小页面容量测试，使分裂、借元素、合并和根变化可以在少量操作内出现。

## 并发、重复运行与工具

5 号在独立 TSan 构建中运行同样的并发目标；4 号汇总 ASan、TSan 和独立质量用例的命令、版本与结果。重复运行可使用：

```bash
./build/p2-asan/test/b_plus_tree_concurrent_test --gtest_repeat=20 --gtest_break_on_failure
```

超时由 1 号接入 P2 测试入口，4/5 号确定实际压力参数并记录。`b_plus_tree_contention_test` 当前会分配约 1GB 的内存磁盘并使用 32 线程，单独执行和记录，不能用它的耗时替代功能验收。

```bash
cmake --build build/p2-asan --target b_plus_tree_printer -j 4
./build/p2-asan/bin/b_plus_tree_printer
```

printer 可以输出 dot 树图；2/3/4 号据此提供插入、删除、扫描的具体示例和运行截图。

## 公共检查与 P1 回归

当前已有 `check-clang-tidy-p2`、`submit-p2`，尚未建立 P2 专用 `check-p2` 和 CI。1 号负责添加统一入口、P2 标签、独立测试发现与超时，并把 CI 接到 `project2` / `p2/**`。现有 P1 工作流仅监听 main、team 分支和目标为 main 的 PR，不会自动替代 P2 验收。

P2 最终集成时同时保留 P1 回归：

```bash
cmake --build build/p2-asan --target check-p1 -j 4
cmake --build build/p2-asan --target check-clang-tidy-p2
```

单独编译器版本或平台导致静态工具报错时，记录实际错误，由 1 号安排统一 Linux 环境处理；不能把未执行项目写成通过。

## 最终打包

根 CMake 的 `P2_FILES` 包含 **P2 十个源码文件 + P1 六个源码文件，共 16 个文件**。全部验收完成后，组长执行：

```bash
cmake --build build/p2-asan --target submit-p2
unzip -l project2-submission.zip
```

当前 `submit-p2` 依赖系统 `zip`，1 号可参考 P1 的 CMake 原生 zip 方式统一它。编译目录、数据库临时文件和生成的 zip 不提交到 Git；最终代码、报告及运行截图按老师通知交付。
