# 任务 5：页面级并发控制

负责人：**何佳绮**；GitHub：`milaqihe`。开发分支：`p2/05-concurrency`，PR 目标：`project2`。

你把单线程树改造成并发 B+ 树。先设计锁规则和编写并发用例，单线程基线合入 project2 后再分批加入锁，保留 2/3 号已有算法。

## 你改哪些地方

- `src/include/storage/index/b_plus_tree.h`、`src/storage/index/b_plus_tree.cpp` 中根页保护、遍历加锁、安全判断及事务页面集合。
- `test/storage/b_plus_tree_concurrent_test.cpp` 和必要的 contention 测试。
- 迭代器如需锁协作，和 4 号在同一个 PR 中明确相关文件与修改。
- 本人说明：`reports/p2/parts/05-concurrency.md`。
- 阅读 `src/include/common/rwlatch.h`、`src/include/storage/page/page.h`、`src/include/concurrency/transaction.h`。

## 按顺序完成

1. 明确查询、插入、删除的安全节点条件、根页例外和获取锁的顺序。
2. 查询先获取子页读锁，再释放父页读锁。
3. 插入/删除使用写锁 latch crabbing，子页安全后释放不再需要的祖先锁。
4. 协调根页入口和 root_page_id 变化；根保护与页面锁配合，不能用一把整树锁串行替代课程要求的并发控制。
5. 使用 transaction 的页面集合跟踪持锁路径，删除页集合跟踪待回收页面。
6. 在实际返回路径上释放 latch 和 pin，避免对已经持有的页重复加同一锁；先 unlatch 再 unpin。
7. 借元素/合并时获取兄弟页锁，和 4 号共同确定叶链扫描策略并验证无循环等待。
8. 运行并发插入、删除、混合和独立正确性用例，再分别进行 TSan、重复执行与锁竞争测试。
9. 基础悲观锁抓取正确后再考虑教程的乐观优化，优化后复跑相同正确性测试。

## 和谁交接

先同步已经通过单线程测试的 project2，再和 2/3/4 对齐接口，按查询/插入/删除分小 PR 改造。你负责并发实现与基础并发测试，4 号负责独立验收与结果汇总。

## 测试与验证

先按[构建指南](../BUILD.md)配置本轮独立目录。

```bash
cmake --build build/p2-asan --target b_plus_tree_concurrent_test -j 4
./build/p2-asan/test/b_plus_tree_concurrent_test
./build/p2-asan/test/b_plus_tree_concurrent_test --gtest_repeat=20 --gtest_break_on_failure
```

TSan 单独构建：

```bash
cmake -S . -B build/p2-tsan -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=thread -DCMAKE_C_COMPILER=clang-14 -DCMAKE_CXX_COMPILER=clang++-14
cmake --build build/p2-tsan --target b_plus_tree_concurrent_test -j 4
TSAN_OPTIONS=halt_on_error=1 ./build/p2-tsan/test/b_plus_tree_concurrent_test
```

原并发目标目前有 5 项。contention/锁竞争目标主要计时，数据集合正确性由功能测试断言验证。测试参数和实际数量写入本人说明。

## 进入分支与推送

先配置[本人 Git 身份](../GIT_WORKFLOW.md)，首次进入已经创建的分支：

```bash
git fetch origin
git switch --track origin/p2/05-concurrency
```

提交本人已完成的小改动，只暂存以下路径中本次实际修改的文件；新增用例或其他协商后的文件再逐项添加：

```bash
git status
git diff
git add src/include/storage/index/b_plus_tree.h src/storage/index/b_plus_tree.cpp test/storage/b_plus_tree_concurrent_test.cpp reports/p2/parts/05-concurrency.md
git diff --cached
git commit -m "P2: implement task 5"
git fetch origin
git merge origin/project2
# 依赖更新后重跑本任务相关测试
git push
```

打开[创建本任务 PR](https://github.com/milaqihe/bustub-initial/compare/project2...p2/05-concurrency?expand=1)，确认 base 为 project2。后续原分支 commit / push 会更新同一个 PR。模块报告写本人 parts 文件，4 号最后统一汇总。

## 完成标准

页面级锁抓取、事务持锁页与删除页管理完整；并发和单线程回归通过；TSan、重复运行有实际结果；能解释锁方案和根页处理。
