# Project 2 五人任务与分工

依据课程《Tutorial for Project2》整理：第 1 页给出两个 Checkpoint，第 22—23 页说明迭代器与并发任务，第 34—35 页列出限制和测试。本页是本组的协作分工；课程要求以老师发布的教程和通知为准。

## 分支与成员

公共集成分支：[`project2`](https://github.com/milaqihe/bustub-initial/tree/project2)。以下为小组已确认的 P2 分工，任务编号与 P1 的人员顺序不同。本轮组长为何佳绮（任务 5）；吴佳忆（任务 1）负责页面和公共构建，何佳绮统一安排审阅、合并与交付。

| 任务 | 姓名 / GitHub | 职责 | 分支与指南 |
| --- | --- | --- | --- |
| 1 | 吴佳忆 / `wjy676` | 三类页面、公共接口、构建与集成 | [`p2/01-pages`](https://github.com/milaqihe/bustub-initial/tree/p2/01-pages) · [指南](tasks/01-pages.md) |
| 2 | 梁怡莲 / `yawyhdhdn` | 查询、插入与分裂 | [`p2/02-search-insert`](https://github.com/milaqihe/bustub-initial/tree/p2/02-search-insert) · [指南](tasks/02-search-insert.md) |
| 3 | 肖子瑞 / `xxz00z` | 删除与再平衡 | [`p2/03-delete`](https://github.com/milaqihe/bustub-initial/tree/p2/03-delete) · [指南](tasks/03-delete.md) |
| 4 | 吴雅卓 / `WUCROW2006` | 迭代器、独立验收及报告汇总 | [`p2/04-iterator-quality`](https://github.com/milaqihe/bustub-initial/tree/p2/04-iterator-quality) · [指南](tasks/04-iterator-quality.md) |
| 5 | 何佳绮 / `milaqihe` | 并发查询、插入、删除与锁管理 | [`p2/05-concurrency`](https://github.com/milaqihe/bustub-initial/tree/p2/05-concurrency) · [指南](tasks/05-concurrency.md) |

每位成员同时提交本人基础测试、模块说明和必要的运行截图。4 号汇总独立测试和报告，组长负责安排审阅、公共分支合并与最终交付。

## 实际代码范围

| 用途 | 代码位置 | 主要负责人 |
| --- | --- | --- |
| 页面公共头与元数据 | `src/include/storage/page/b_plus_tree_page.h`、`src/storage/page/b_plus_tree_page.cpp` | 1 |
| 内部页 | 同目录 `b_plus_tree_internal_page.h/.cpp` | 1，和 2/3 确定辅助方法 |
| 叶子页及单向 next 链 | 同目录 `b_plus_tree_leaf_page.h/.cpp` | 1，和 2/3/4 交接 |
| 树的查询、插入、删除、迭代器入口 | `src/include/storage/index/b_plus_tree.h`、`src/storage/index/b_plus_tree.cpp` | 2/3/4 按函数划分；5 后续加入并发 |
| 索引迭代器 | 同目录 `index_iterator.h/.cpp` | 4 |
| Page 读写锁 | `src/include/storage/page/page.h`、`src/include/common/rwlatch.h` | 5 阅读使用 |
| 事务持锁页 / 删除页集合 | `src/include/concurrency/transaction.h` | 5 阅读使用 |

完整函数归属见[接口与交接](INTERFACES.md)。

## 开发与合并顺序

1. **先定接口。** 1、2、3 号约定页面容量、size 含义、定位与搬移方法；2、4、5 号约定公共找叶子页方法及返回的页面资源归属。1 号先交付页面基础的小 PR。
2. **单线程并行。** 2 号实现查询和插入，3 号实现删除。3 号可先使用自己构造的页面验证借元素/合并，完整删除测试需要插入路径齐备。
3. **迭代器与测试提前开工。** 4 号先实现比较、结束态和测试方案；插入能建立叶子链后联调跨页遍历，删除接入后验证合并后的扫描。
4. **并发先设计，后接入。** 5 号先定义安全节点条件、锁顺序与事务页集合用法并编写并发用例；单线程基线合入后同步 `project2`，再分批提交查询、插入、删除的锁改造。
5. **验收与交付。** 完整 P2 测试、独立边界、ASan/TSan 和 P1 回归通过后，汇总说明与运行截图，组长审阅后再创建 `project2 → main` 的最终 PR。

## 协作规则

- 任务 PR 的目标是 `project2`，不是 `main`，也不是其他人的任务分支。
- `b_plus_tree.cpp` / `.h` 是共享文件，按函数提交小改动；2/3 号不重写对方逻辑，4 号先只改 Begin/End，5 号基于单线程实现追加加锁。
- 先提交本人工作，再 `git fetch origin`、`git merge origin/project2`；出现冲突按函数归属沟通，不用整份文件覆盖。
- 保留原有公开接口、断言和模板显式实例化；辅助函数可以按本组约定新增。
- 复用 P1 的 BufferPoolManager 分配/获取树节点。P1 缺陷如实际出现，单独说明和修复，不在 P2 PR 中悄悄换掉 P1 模块。
- 教程要求页面级 latch crabbing，并使用传入的 transaction 管理相关页面；不能用一把整树锁替代并发索引实现。

## 本轮文档入口

[构建与测试](BUILD.md) · [推送与 PR](GIT_WORKFLOW.md) · [接口交接](INTERFACES.md) · [报告与验收](../../reports/p2/README.md)
