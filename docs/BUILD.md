# 环境搭建与测试

所有命令均从仓库根目录执行。1 号负责把全组实际使用的环境记录进 `reports/p1/integration.md`。本工程使用 C++17，CMake 至少 3.26；团队 CI 固定 Ubuntu 22.04、Clang/format/tidy 14 和 CMake 3.31.10。Windows 同学可在 WSL2 Ubuntu 中执行 Linux 命令。

## 安装与配置

Ubuntu 环境按需安装构建依赖：

```bash
sudo apt-get update
sudo apt-get install build-essential clang-14 clang-format-14 clang-tidy-14 git pkg-config zlib1g-dev python3 python3-pip
python3 -m pip install --user "cmake==3.31.10"
export PATH="$HOME/.local/bin:$PATH"
cmake --version
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang-14 -DCMAKE_CXX_COMPILER=clang++-14 -DCLANG_FORMAT_BIN=clang-format-14 -DCLANG_TIDY_BIN=clang-tidy-14
```

macOS 安装 Xcode Command Line Tools、CMake 3.26 以上和所需工具后，可使用：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

首次安装开发工具或缺少具体依赖时，由 1 号统一解决。若已配置的编译器或平台改变，使用新的构建目录，不把不同工具链的缓存混用。构建依赖已包含在 `third_party/` 中，按本仓库配置排查问题，不用另一年份 BusTub 覆盖工程。

1 号已在 macOS 26.4/AppleClang 21 上验证 Debug+ASan 编译。第三方头文件使用 SYSTEM 包含以兼容新的 SDK，项目自身保留 `-Werror`。较老的 clang-tidy 14 不能正常解析该新 SDK；本机静态检查未通过，最终静态检查统一在 CI 的 Linux/Clang 14 环境复核，不把工具不兼容当成通过。

## 先编译，再运行目标

| 负责人 | 构建目标 | 运行路径 |
| --- | --- | --- |
| 2 号 | `extendible_hash_table_test` | `./build/test/extendible_hash_table_test` |
| 3 号 | `lru_k_replacer_test` | `./build/test/lru_k_replacer_test` |
| 4 号 | `buffer_pool_manager_instance_test` | `./build/test/buffer_pool_manager_instance_test` |
| 1、5 号 | 三个目标及 5 号实际新增目标 | 按完整集成版本运行 |

```bash
cmake --build build --target extendible_hash_table_test lru_k_replacer_test buffer_pool_manager_instance_test -j 4
./build/test/extendible_hash_table_test
./build/test/lru_k_replacer_test
./build/test/buffer_pool_manager_instance_test
```

初始工程的业务方法尚未实现，缓冲池构造函数有显式异常。此阶段的失败不等同环境故障。模块完成后，应直接运行整个测试可执行文件；临时使用 `--gtest_filter` 定位问题，不能只通过筛选后的几个用例就验收。

## 1 号提供的 P1 集成入口

```bash
cmake --build build --target build-p1-tests -j 4
cmake --build build --target check-p1 -j 4
```

`build-p1-tests` 构建上述三个目标及约定命名的质量测试；`check-p1` 构建后通过 CTest 的 `p1` 标签运行它们，零用例或任意失败返回非零。P2/P3/P4 的测试不进入该入口。单个用例的超时为 120 秒。

从任意工作目录也可执行 `bash /仓库路径/p1_test.sh /构建目录路径`。在仓库根目录执行 `bash p1_test.sh` 使用默认 `build/`。脚本失败立即退出，不自动格式化。若 CMake 不在 PATH，设置 `P1_CMAKE` 为你本机 CMake 可执行文件的绝对路径；`P1_JOBS` 控制构建并发数，默认 4。

## 格式与静态检查

```bash
cmake --build build --target check-p1-format
cmake --build build --target check-p1-lint
cmake --build build --target check-clang-tidy-p1
```

新的 `check-p1-format`、`check-p1-lint` 检查 P1 六文件和已接入的 P1 测试。原有 `check-format`、`check-lint` 仍可用于全工程检查。`format` 会修改文件，个人开发先对自己负责的文件格式化，再查看 `git diff`；全工程格式化由 1 号协调。格式和静态检查失败时，区分自己改动、工具版本和初始工程已有问题，修复结论写进 PR。

## 5 号的独立 Sanitizer 构建

根 `CMakeLists.txt` 中的真实变量为 `BUSTUB_SANITIZER`，Debug 默认值为 `address`。分别建子目录，避免 ASan 和 TSan 混在同一构建中。以下命令在统一 Linux 环境执行：

```bash
cmake -S . -B build/asan -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=address -DCMAKE_C_COMPILER=clang-14 -DCMAKE_CXX_COMPILER=clang++-14
cmake --build build/asan --target extendible_hash_table_test lru_k_replacer_test buffer_pool_manager_instance_test -j 4
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./build/asan/test/extendible_hash_table_test
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./build/asan/test/lru_k_replacer_test
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./build/asan/test/buffer_pool_manager_instance_test

cmake -S . -B build/tsan -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=thread -DCMAKE_C_COMPILER=clang-14 -DCMAKE_CXX_COMPILER=clang++-14
cmake --build build/tsan --target extendible_hash_table_test lru_k_replacer_test buffer_pool_manager_instance_test -j 4
TSAN_OPTIONS=halt_on_error=1 ./build/tsan/test/extendible_hash_table_test
TSAN_OPTIONS=halt_on_error=1 ./build/tsan/test/lru_k_replacer_test
TSAN_OPTIONS=halt_on_error=1 ./build/tsan/test/buffer_pool_manager_instance_test
```

新增质量测试后，在相同构建目录重新执行 `cmake -S . -B ...`，再构建和运行新增目标。平台不支持某 Sanitizer 或运行时无法启动时，记录“环境受阻”和报错，交 1 号安排统一环境复测；不能填写“通过”。

## 新增测试如何接入

`test/CMakeLists.txt` 递归匹配 `test/*/*test.cpp`，再按文件 basename 创建目标。5 号将测试放在 `test/p1_quality/`，使用全仓库唯一文件名，见[质量测试入口](../test/p1_quality/README.md)。

`p1_quality_*_test.cpp` 和 `p1_integration_*_test.cpp` 会自动接入 P1 构建/标签；文件增删由 `CONFIGURE_DEPENDS` 跟踪。其他名称的测试仍由全工程入口管理。

## 团队 CI

工作流响应 main 和 `team/**` 推送、面向 main 的 PR，也可手动触发。它依次构建 P1、运行测试及静态检查，保留测试诊断；全部通过后生成课程提交包。执行结果在仓库 Actions 页面查看。

## 最终提交包

在最终集成和验收完成后，由 1 号执行：

```bash
cmake --build build --target submit-p1
unzip -l project1-submission.zip
```

压缩包生成在仓库根目录，由 CMake 原生 zip 功能重建，包含 `CMakeLists.txt` 中 `P1_FILES` 指定的六个 `.h/.cpp` 文件，不沿用旧 zip 的多余条目，也不再依赖系统 `zip` 命令。报告、测试和团队文档留在 GitHub；老师如另外要求报告，再按通知提交。构建目录与提交 zip 已被仓库 `.gitignore` 忽略，无需提交编译产物。
