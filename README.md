# BusTub Project 2：B+ 树索引

本分支用于第二次实验，基于已完成的 Project 1（`fb905e4`），继续使用其 Buffer Pool Manager。第一轮提交保留在 [main](https://github.com/milaqihe/bustub-initial/tree/main)。

## 本轮分工与开发分支

本轮分工已确认，编号对应第二次实验任务。组长为 **何佳绮（任务 5）**，公共页面与构建由任务 1 负责，组长统一审阅合并。

| 任务 | 负责人 / GitHub | 主要职责 | 开发分支 | 详细任务 |
| --- | --- | --- | --- | --- |
| 1 | 吴佳忆 / `wjy676` | 公共页、内部页、叶子页及公共集成 | `p2/01-pages` | [任务 1](docs/p2/tasks/01-pages.md) |
| 2 | 梁怡莲 / `yawyhdhdn` | 单点查询、插入、分裂与新根 | `p2/02-search-insert` | [任务 2](docs/p2/tasks/02-search-insert.md) |
| 3 | 肖子瑞 / `xxz00z` | 删除、借元素、合并与根收缩 | `p2/03-delete` | [任务 3](docs/p2/tasks/03-delete.md) |
| 4 | 吴雅卓 / `WUCROW2006` | 迭代器、独立测试与报告汇总 | `p2/04-iterator-quality` | [任务 4](docs/p2/tasks/04-iterator-quality.md) |
| 5 | 何佳绮 / `milaqihe` | 页面级并发控制与并发验证 | `p2/05-concurrency` | [任务 5](docs/p2/tasks/05-concurrency.md) |

**公共集成分支为 `project2`，五个任务的 PR 均以它为目标分支。** 每个任务分支已经包含本轮指南及负责人信息，按自己的任务页面开始即可。

- [任务总览、成员与开发顺序](docs/p2/README.md)
- [文件归属与接口交接](docs/p2/INTERFACES.md)
- [P2 构建、实际测试目标与打包](docs/p2/BUILD.md)
- [同步、推送和 PR 操作](docs/p2/GIT_WORKFLOW.md)
- [P2 报告与验收入口](reports/p2/README.md)

## 开发起点

当前已建立分支和协作说明，P2 功能仍是课程初始骨架：查询、插入、删除及迭代器尚待实现。本次准备没有代替各位完成算法，也没有声明 P2 测试通过。

课程教程将任务分为：Checkpoint 1 的页面类与单线程增删查，Checkpoint 2 的迭代器与并发索引。本组在此基础上拆成五个任务包，先完成单线程树，再集成迭代器和页面级 latch crabbing。

```bash
git clone https://github.com/milaqihe/bustub-initial.git
cd bustub-initial
git fetch origin
git switch --track origin/project2
```

已有克隆的同学不用重复 clone，阅读[推送指南](docs/p2/GIT_WORKFLOW.md)后切到自己认领的任务分支。P1 文档和已完成源码继续保留在仓库中，P2 的任务、报告和独立测试分别使用 `docs/p2/`、`reports/p2/`、`test/p2_quality/`。
