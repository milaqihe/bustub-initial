# 5 号质量测试入口

本目录已创建，供 5 号添加独立的 GoogleTest 用例。当前没有新增测试实现或通过记录。详细任务见[5 号指南](../../docs/team/member-05.md)，预期规则见[接口约定](../../docs/INTERFACES.md)。

| 待创建的文件 | 构建目标 | 建议套件名 |
| --- | --- | --- |
| `p1_quality_hash_test.cpp` | `p1_quality_hash_test` | `P1QualityHashTest` |
| `p1_quality_lruk_test.cpp` | `p1_quality_lruk_test` | `P1QualityLRUKTest` |
| `p1_quality_bpm_test.cpp` | `p1_quality_bpm_test` | `P1QualityBPMTest` |

每份文件包含自己的 `#include "gtest/gtest.h"` 和所需模块头文件，测试放在 `namespace bustub` 下；公共构建已经链接 GoogleTest/gmock_main，不需要自己定义 main。文件 basename 必须与全仓库其他测试不同。

现有 `test/CMakeLists.txt` 会递归发现本层 `*test.cpp`，添加后从仓库根重新执行 `cmake -S . -B build`。例如先只写好哈希质量测试：

```bash
cmake -S . -B build
cmake --build build --target p1_quality_hash_test -j 4
./build/test/p1_quality_hash_test
```

模块基础测试仍由 2、3、4 号在原文件维护。补充用例应验证边界、接口不变量或真实缺陷，保留原有断言，不以删除失败测试完成验收。
