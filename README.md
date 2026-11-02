# BusTub Project 1 五人小组实验

本仓库用于完成可扩展哈希表、LRU-K 置换器和缓冲池管理器。仓库所有者为 `milaqihe`，默认分支为 `main`。当前数据库源码保留初始工程的待实现状态，按以下分工开发。

## 每位同学从这里开始

先接受仓库协作者邀请，再克隆仓库。编号沿用小组分工图，GitHub 用户名由各位在自己的提交中使用。

| 编号 | 职责与个人指南 | 主要源码或交付位置 | 开发分支 |
| --- | --- | --- | --- |
| 1 号 | [组长与集成负责人](docs/team/member-01.md) | 根目录 `CMakeLists.txt`、构建说明、接口约定、合并与最终提交 | `team/01-integration` |
| 2 号 | [可扩展哈希表负责人](docs/team/member-02.md) | [头文件](src/include/container/hash/extendible_hash_table.h)、[实现](src/container/hash/extendible_hash_table.cpp)、[单元测试](test/container/hash/extendible_hash_table_test.cpp) | `team/02-hash` |
| 3 号 | [LRU-K 负责人](docs/team/member-03.md) | [头文件](src/include/buffer/lru_k_replacer.h)、[实现](src/buffer/lru_k_replacer.cpp)、[单元测试](test/buffer/lru_k_replacer_test.cpp) | `team/03-lruk` |
| 4 号 | [缓冲池负责人](docs/team/member-04.md) | [头文件](src/include/buffer/buffer_pool_manager_instance.h)、[实现](src/buffer/buffer_pool_manager_instance.cpp)、[单元测试](test/buffer/buffer_pool_manager_instance_test.cpp) | `team/04-buffer-pool` |
| 5 号 | [质量与验收负责人](docs/team/member-05.md) | [补充测试入口](test/p1_quality/README.md)、[验收与报告入口](reports/p1/README.md) | `team/05-quality` |

各模块负责人同时负责实现与基础测试。1 号管理公共配置与合并，5 号独立补充边界、内存、并发和压力测试。4 号对接 2、3 号的固定接口。

## 公共指南

- [环境搭建与测试命令](docs/BUILD.md)：统一构建环境、三个测试目标、格式检查和打包。
- [接口与状态约定](docs/INTERFACES.md)：准确函数名、返回值、LRU-K 决策、pin/dirty 与锁的约定。
- [Git 协作与推送指南](docs/GIT_WORKFLOW.md)：首次克隆、设置自己的身份、同步、推送、开 PR 与冲突处理。
- [五人任务与集成顺序](docs/team/README.md)：文件归属、阶段产物、依赖与合并条件。

## 代码目录怎么找

```text
src/include/container/hash/extendible_hash_table.h  # 2 号：接口、Bucket 和成员变量
src/container/hash/extendible_hash_table.cpp        # 2 号：哈希表实现
src/include/buffer/lru_k_replacer.h                 # 3 号：接口和访问历史结构
src/buffer/lru_k_replacer.cpp                       # 3 号：LRU-K 实现
src/include/buffer/buffer_pool_manager.h            # 4 号阅读：公开调用包装层
src/include/buffer/buffer_pool_manager_instance.h   # 4 号：具体实现类
src/buffer/buffer_pool_manager_instance.cpp         # 4 号：六个页面操作
src/include/storage/page/page.h                    # 4、5 号阅读：页与元数据
src/include/storage/disk/disk_manager.h             # 4、5 号阅读：磁盘接口
test/container/hash/extendible_hash_table_test.cpp  # 2 号基础测试
test/buffer/lru_k_replacer_test.cpp                 # 3 号基础测试
test/buffer/buffer_pool_manager_instance_test.cpp   # 4 号基础与集成测试
test/p1_quality/                                   # 5 号补充测试位置
reports/p1/                                       # 5 号报告、1 号最终提交记录
```

## 第一次运行

安装环境见[构建指南](docs/BUILD.md)，所有命令从仓库根目录执行：

```bash
git clone https://github.com/milaqihe/bustub-initial.git
cd bustub-initial
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target extendible_hash_table_test lru_k_replacer_test buffer_pool_manager_instance_test -j 4
```

初始代码中，哈希表和 LRU-K 多个方法尚为空实现；缓冲池构造函数会抛 `NotImplementedException`。首次运行测试失败属于当前开发起点，不能据此宣称模块已经完成。4 号在实现完整后移除该异常。具体通过条件见每人的指南。

本次实现范围为 Project 1。`clock_replacer`、普通 `lru_replacer`、磁盘哈希和 B+ 树等文件不属于这次三个模块。`DiskManager` 已提供。

## 课程附带的 P1_src 代码

[P1_src 使用说明](references/README.md)中附有[原始压缩包](references/P1_src.zip)和[可直接浏览的六个源码文件](references/P1_src/)。它包含哈希表、LRU-K、缓冲池的已有实现，可用来阅读调用关系、对照思路和练习定位问题。

这份代码与上面的 `src/` 初始工程分开放置，未覆盖开发源码。当前 CMake 构建和 `submit-p1` 仍使用 `src/` 中的六个文件。附带实现存在 LRU-K 排序、满桶更新和 FlushAll 重复加锁等问题，具体位置及各成员如何使用见[参考说明](references/README.md)。各人仍需按接口约定完成实现与测试。
