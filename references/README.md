# 课程附带的 P1_src 参考代码

这里保存用户提供的课程附件 `P1_src.zip`。该包包含 Project 1 三个模块的已有实现，供小组阅读、对照和分析。原包保持原样；同时展开六个 `.h/.cpp` 文件，方便在 GitHub 直接查看。展开目录省略了 macOS 的 `__MACOSX` 元数据。

- [下载原始 P1_src.zip](P1_src.zip)
- [浏览展开后的代码](P1_src/)
- [当前正式接口约定](../docs/INTERFACES.md)

## 它与初始工程的关系

`bustub_initial.zip` 提供完整工程、依赖和测试，仓库根目录的 `src/` 就来自该初始工程；`P1_src.zip` 只提供三个模块的六个实现/头文件，不是独立完整项目，也不包含完整测试和构建配置。

参考包内部不带外层 `src/` 目录，对应关系如下。需要开发、编译和提交的路径是右列。

| 模块与负责人 | 附件中的代码 | 正式开发位置 |
| --- | --- | --- |
| 哈希表，2 号 | [头文件](P1_src/include/container/hash/extendible_hash_table.h)、[实现](P1_src/container/hash/extendible_hash_table.cpp) | [头文件](../src/include/container/hash/extendible_hash_table.h)、[实现](../src/container/hash/extendible_hash_table.cpp) |
| LRU-K，3 号 | [头文件](P1_src/include/buffer/lru_k_replacer.h)、[实现](P1_src/buffer/lru_k_replacer.cpp) | [头文件](../src/include/buffer/lru_k_replacer.h)、[实现](../src/buffer/lru_k_replacer.cpp) |
| 缓冲池，4 号 | [头文件](P1_src/include/buffer/buffer_pool_manager_instance.h)、[实现](P1_src/buffer/buffer_pool_manager_instance.cpp) | [头文件](../src/include/buffer/buffer_pool_manager_instance.h)、[实现](../src/buffer/buffer_pool_manager_instance.cpp) |

当前 CMake 的编译目录为正式工程目录；`references/P1_src/` 未加入构建。`submit-p1` 也只打包根 CMakeLists.txt 中 `P1_FILES` 指定的 `src/` 六文件。直接修改参考目录不会改变数据库程序；实现改动要提交到右列对应位置。

## 每位同学怎样使用

| 成员 | 阅读与行动 |
| --- | --- |
| 1 号 | 确认三组文件与正式接口的对应关系；审阅 PR 时检查参考实现是否已按契约修正，避免把整包复制进 src 后直接视为完成 |
| 2 号 | 阅读 Bucket、目录翻倍和指针重分配流程；重点分析满桶时更新旧键的顺序，完成右列正式实现和自己的基础测试 |
| 3 号 | 阅读 history/cache 容器及状态计数；依据倒数第 K 次访问定义重新核对排序，覆盖 K=1/2/3 与合法 ID 边界 |
| 4 号 | 阅读选帧、脏页写回、页表更新和 pin/unpin 的调用关系；修正 FlushAll 的锁处理及 Flush 后 dirty 状态 |
| 5 号 | 将下面问题写成独立回归用例，在正式实现上验证；在报告中区分附件已有实现、团队采用的设计和团队实际修复 |

## 阅读时需要核对的问题

下面结论来自对附件源码的静态阅读；尚未对参考包运行测试，不代表只有这些问题。

### 1 哈希表先拆桶再查重复键

位置：[extendible_hash_table.cpp 的外层 Insert](P1_src/container/hash/extendible_hash_table.cpp)。函数先执行 `while (dir_[IndexOf(key)]->IsFull())` 拆分，之后才遍历查找并更新已有键。因此，满桶中更新旧键也可能触发不必要的拆分。内部 `Bucket::Insert` 也先检查满桶并直接追加，没有独立实现头文件约定的重复键覆盖。

2 号应先处理已有键更新，再对放不下的新键拆桶；5 号加入“满桶更新后深度、桶数不变”的回归用例。

### 2 LRU-K 成熟帧按最近一次访问移动

位置：[lru_k_replacer.cpp 的 RecordAccess 和 Evict](P1_src/buffer/lru_k_replacer.cpp)。访问超过 K 次后，参考实现将帧移到 cache_list 前端，按最近一次访问调整；这不能普遍实现“倒数第 K 次访问时间最早”的淘汰顺序。

3 号应维护所需的 K 次时间记录。5 号可使用 K=2、A 历史为 1/9、B 历史为 2/3 的反例：应淘汰 A。另检查 `frame_id > replacer_size_` 的判断，合法上界应为严格小于容量，等于容量也应拒绝，并处理负数。

### 3 FlushAll 重复获取同一把普通互斥锁

位置：[buffer_pool_manager_instance.cpp 的 FlushAllPgsImp 和 FlushPgImp](P1_src/buffer/buffer_pool_manager_instance.cpp)。FlushAll 先持有 `latch_`，随后调用同样获取 `latch_` 的 FlushPgImp，对有效页会发生同一线程重复获取普通 `std::mutex`，有死锁问题。

4 号应把锁边界设计清楚，例如抽取要求调用者已持锁的内部写回方法；5 号用受超时约束的 FlushAll 用例验证。

### 4 显式 Flush 后没有清 dirty

同一参考文件中，FlushPgImp 写盘后直接返回，没有按正式头文件约定清除 dirty。4 号需补全状态维护，5 号检查“Flush 后 dirty=false，pin 和映射保持不变”。

## 使用步骤

1. 从个人指南打开正式头文件、正式测试和这里的对应参考实现。
2. 按方法逐项对照接口约定，理解实现目的，明确所需修正。
3. 在个人开发分支修改 `src/` 中本人负责的模块，保留既有测试断言并补充必要用例。
4. 按个人指南运行完整模块测试、提交 PR，并说明参考了哪些思路、实际完成了哪些实现或修复。

参考文件保留课程附件原貌。后续团队修复写入正式 `src/`，以便查看初始框架、附件实现和团队成果之间的区别。
