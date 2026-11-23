# P1 独立质量测试

本目录包含 5 号编写的 10 项独立补充测试，与三个模块的 38 项基础测试一起接入 `check-p1`。

| 文件与目标 | 用例数 | 主要覆盖 |
| --- | --- | --- |
| [p1_quality_lruk_test.cpp](p1_quality_lruk_test.cpp) / `p1_quality_lruk_test` | 4 | K=3 的第 K 次访问排序、Remove 后复用与计数、帧编号边界、多线程精确计数 |
| [p1_quality_bpm_test.cpp](p1_quality_bpm_test.cpp) / `p1_quality_bpm_test` | 6 | 容量 1/2、全 pinned、脏页重载、FlushAll、删除复用、固定种子换页、独立页与共享页并发 |

哈希表的 12 项用例位于 [test/container/hash/extendible_hash_table_test.cpp](../container/hash/extendible_hash_table_test.cpp)。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=address
cmake --build build --target check-p1 -j 4
```

运行结果与截图见[验收记录](../../reports/p1/acceptance.md)和[截图入口](../../reports/p1/README.md)。
