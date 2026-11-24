# P2 独立质量测试入口

任务 4 在此添加独立 GoogleTest 用例，文件名使用全仓库唯一的 `p2_quality_*_test.cpp`。当前只有任务说明，没有新增测试实现。

建议按需创建：

| 文件 | 对应目标 | 重点 |
| --- | --- | --- |
| `p2_quality_iterator_test.cpp` | `p2_quality_iterator_test` | 空树、指定起点、跨叶子页、结束态与资源释放 |
| `p2_quality_tree_test.cpp` | `p2_quality_tree_test` | 小容量多层分裂、借元素、合并、根收缩及有序集合对照 |
| `p2_quality_concurrent_test.cpp` | `p2_quality_concurrent_test` | 有明确预期的多线程最终集合、重复操作与超时 |

这些文件是待编写任务，不是现有测试。写好后重新执行 CMake，再构建相应目标；任务 1 将其接入 P2 专用入口与标签。

每个用例的预期独立于被测实现。原有模块基础测试由对应成员维护，新增质量测试用于补充覆盖。实际执行记录与截图见 [P2 验收入口](../../reports/p2/README.md)。
