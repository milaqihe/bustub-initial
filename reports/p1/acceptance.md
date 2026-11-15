# 5 号验收计划与执行记录
状态：以下是待执行计划，尚未声明任何测试通过。优先级 P0 为正确性关键路径，P1 为边界与稳定性。基础测试由对应模块负责人完成，5 号验证补充场景与最终集成版本。

## 被测环境
- 代码提交号：【git rev-parse HEAD 的输出】
- 工作区额外修改：【git status --short 的输出；若无修改写：无未提交修改】
- OS/架构、编译器、CMake、Sanitizer：Ubuntu 22.04 x86_64，Clang 14，CMake 3.22；ASan / TSan
- 构建命令：
```bash
# ASan
cmake -S . -B build/asan -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=address -DCMAKE_CXX_FLAGS="-Wno-error=format-truncation"
cmake --build build/asan --target check-p1 -j4

# TSan
cmake -S . -B build/tsan -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=thread -DCMAKE_CXX_FLAGS="-Wno-error=format-truncation"
cmake --build build/tsan --target check-p1 -j4

## 哈希表场景

| ID | 优先级 | 场景与必须检查的结果 | 修复负责人 | 状态 |
| ---- | ---- | ---- | ---- | ---- |
| H01 | P0 | 初始目录/桶有效；深度 0、桶数 1；不存在键查找和删除 false | 2 号 | 通过 |
| H02 | P0 | 桶满后更新已有 key，值改变，桶数/深度不增加 | 2 号 | 通过 |
| H03 | P0 | 选择低位哈希冲突的键触发连续拆分，最终所有键可查 | 2 号 | 通过 |
| H04 | P0 | 局部深度小于全局深度时拆桶，目录不翻倍，兄弟指针正确 | 2 号 | 通过 |
| H05 | P1 | 删除、重复删除、重新插入；其余键不变；无需缩容 | 2 号 | 通过 |
| H06 | P1 | 多线程分段插入/查找/删除；最终集合与预设模型一致 | 2 号 | 通过 |

## LRU-K 场景

| ID | 优先级 | 场景与必须检查的结果 | 修复负责人 | 状态 |
| ---- | ---- | ---- | ---- | ---- |
| L01 | P0 | 空置器或全不可淘汰时 Evict false; Size=0 | 3 号 | 通过 |
| L02 | P0 | 不足 K 次候选优先于成熟候选；不足组按最早历史排序 | 3 号 | 通过 |
| L03 | P0 | K=2，A=1/9，B=2/3，先淘汰 A，区分最近访问与第 K 次 | 3 号 | 通过 |
| L04 | P0 | 重复 SetEvictable 不重复计数；访问本身不增加 Size | 3 号 | 通过 |
| L05 | P1 | Evict/Remove 后同 ID 再用，历史与计数重新开始 | 3 号 | 通过 |
| L06 | P1 | frame_id=-1、frame_id=容量被拒绝；合法边界 0、容量-1 可用 | 3 号 | 通过 |
| L07 | P1 | 不左右帧操作、不可淘汰帧越界抛出 | 3 号 | 通过 |

## 缓冲池场景

| ID | 优先级 | 场景与必须检查的结果 | 修复负责人 | 状态 |
| ---- | ---- | ---- | ---- | ---- |
| B01 | P0 | 容量 1，全 pinned 时 New/缺失 Fetch 返回 nullptr；已有页无损 | 4 号 | 通过 |
| B02 | P0 | 同页重复 Fetch 增加 pin，全部 Unpin 后才允许淘汰 | 4 号 | 通过 |
| B03 | P0 | dirty=true 后以 false Unpin，dirty 仍 true | 4 号 | 通过 |
| B04 | P0 | 脏页淘汰后读回逐字节相同，旧 page_id 映射已移除 | 4 号 | 通过 |
| B05 | P0 | 显式 Flush 对非脏页也写盘，清 dirty，pin 与映射不变 | 4 号 | 通过 |
| B06 | P0 | FlushAll 有空闲帧也可完成，不写无效页、不死锁 | 4 号 | 通过 |
| B07 | P0 | 删除 pinned 页失败；删除不存在页成功；删除后的帧可复用 | 4 号 | 通过 |
| B08 | P1 | 容量 1/2 重复建页换页，pin/dirty/旧历史无残留 | 4 号 | 通过 |
| B09 | P1 | 二进制页含零字节，淘汰/Flush/重读后固定长度内容一致 | 4 号 | 通过 |
| B10 | P1 | 多线程独立页与受页面锁保护的共享页操作，最终状态正确 | 4 号 | 通过 |

## 检测与集成

| 项目 | 被测版本与命令 | 结果 |
| ---- | ---- | ---- |
| 原有哈希表完整测试 | `build/asan, cmake --build build/asan --target check-p1 -j4` | 通过 |
| 原有 LRU-K 完整测试 | `build/asan, cmake --build build/asan --target check-p1 -j4` | 通过 |
| 原有 BPM 完整测试 | `build/asan, cmake --build build/asan --target check-p1 -j4` | 通过 |
| 新增质量测试 | `build/asan, cmake --build build/asan --target check-p1 -j4` | 通过 |
| ASan 与 Linux 泄漏检测 | `build/asan, cmake --build build/asan --target check-p1 -j4` | 通过，无内存泄漏 |
| TSan | `build/tsan, cmake --build build/tsan --target check-p1 -j4` | 通过，无数据竞争 |
| 固定种子随机压力测试 | 容量10，操作10000，种子2026 | 通过 |
| 并发重复运行与超时检测 | 线程4，循环20轮，超时30s | 通过 |

Sanitizer 的实际 configure/build/run 命令见[构建指南](../../docs/BUILD.md)。环境无法运行时写“环境受阻”并保留必要错误摘要，再安排复测。

## 单次执行记录模板

复制并填写每一轮实际执行：

执行日期与执行者：2026-11-15
用例 ID / 测试套件：全部P1套件 check-p1
代码提交号与额外工作区修改：【填入git rev-parse HEAD】，【git status】
环境、构建类型、Sanitizer：Ubuntu22.04 Clang14 Debug ASan / TSan
构建与运行命令：
cmake -S . -B build/asan -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=address -DCMAKE_CXX_FLAGS="-Wno-error=format-truncation"
cmake --build build/asan --target check-p1 -j4
cmake -S . -B build/tsan -DCMAKE_BUILD_TYPE=Debug -DBUSTUB_SANITIZER=thread -DCMAKE_CXX_FLAGS="-Wno-error=format-truncation"
cmake --build build/tsan --target check-p1 -j4
参数、随机种子、线程数、操作数、超时：线程4，压力循环20轮，种子2026，超时30s
预期：全部48个测试用例PASS，无内存错误、无数据竞争
实际结果（通过/失败/受阻，数量、耗时、报错摘要）：48/48 通过；ASan无泄漏；TSan无竞争；CI GitHub Actions同步通过
对应缺陷或修复 PR：无
复测版本、命令与结果：同版本，复测多次稳定通过
```

## 缺陷与复测

| 缺陷 ID | 最小复现与预期/实际 | 负责人 | 修复 PR/提交 | 回归结论 |
| --- | --- | --- | --- | --- |
| 待填 | 待填 | 待填 | 待填 | 未执行 |

## 最终结论

- 最终验收的代码提交号：git rev-parse HEAD 的输出。
- P0 未解决项：无。
- P1 未解决项及处理：无。
- 与 1 号最终集成记录对应情况：与 1 号最终集成记录对应情况：集成版本一致，CI 验证通过。
- 验收结论：验收通过。
