# P2 模块说明、报告与验收

本轮尚未开始功能实现和验收，以下为协作入口。每人维护自己的模块说明，4 号汇总最终报告和运行截图，组长确认最终版本并提交。

| 材料 | 负责人 | 内容 |
| --- | --- | --- |
| [01-pages](parts/01-pages.md) | 任务 1 | 页面布局、容量与原语、公共构建集成 |
| [02-search-insert](parts/02-search-insert.md) | 任务 2 | 查询、插入、多层分裂和新根示例 |
| [03-delete](parts/03-delete.md) | 任务 3 | 借元素、合并、根收缩及回收 |
| [04-iterator-quality](parts/04-iterator-quality.md) | 任务 4 | 迭代器、独立测试与验收组织 |
| [05-concurrency](parts/05-concurrency.md) | 任务 5 | 页面锁、事务页集合、并发测试与锁流程 |
| [report.md](report.md) | 4 号汇总，全组提供内容 | 设计、图、测试、分工与答辩 |
| [acceptance.md](acceptance.md) | 4 号汇总，各负责人提供结果 | 实际版本、命令、数量、ASan/TSan、缺陷复测 |

需要截图时，将真实运行截图放在 `reports/p2/images/`，在报告中说明对应版本与测试范围。截图目录在实际有文件时创建，编译日志和临时输出放 `build/`，不提交数据库临时文件和编译产物。
