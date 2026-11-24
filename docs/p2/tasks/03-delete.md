# 任务 3：删除与再平衡

认领人：待填写姓名 / GitHub。开发分支：`p2/03-delete`，PR 目标：`project2`。

你负责 Remove、兄弟借元素、节点合并、根收缩和空树处理。先确定页面原语与公共定位接口，再实现单线程删除；并发锁由 5 号后续接入。

## 你改哪些地方

- `src/include/storage/index/b_plus_tree.h`、`src/storage/index/b_plus_tree.cpp` 中 Remove、再平衡和删除辅助方法。
- `test/storage/b_plus_tree_delete_test.cpp`。
- 本人模块说明：`reports/p2/parts/03-delete.md`。
- 查询和 Insert 由 2 号维护；页内删除、搬移原语由 1 号维护。

## 按顺序完成

1. 复用 2 号的找叶子页方法，处理空树、不存在键和正常删除。
2. 删除叶子键值，维护 size；需要时更新上层分隔键。
3. 节点低于最小占用时，向合适的左右兄弟借元素，调整父分隔键和相关孩子的父页编号。
4. 无法借元素时合并叶子页，接好 next 链并移除父节点对应目录项。
5. 实现内部节点的借元素、分隔键下推、合并和向上递归再平衡。
6. 处理根只有一个孩子时降低树高、删除最后一条记录后空树；root_page_id 改变时更新 header 记录。
7. 通过 BufferPoolManager 回收废页，保证 dirty、pin 和释放顺序正确。
8. 提供正常删除、左右借元素、叶子/内部合并、根收缩的小容量示例与图。

## 和谁交接

和 1 号确定页内删除、首尾搬移和 size 规则；复用 2 号的定位与根更新约定；和 4 号验证合并后的扫描。单线程齐备后，5 号接入兄弟锁和事务待删除页集合。

## 测试与验证

先按[构建指南](../BUILD.md)配置本轮独立目录。

```bash
cmake --build build/p2-asan --target b_plus_tree_delete_test -j 4
./build/p2-asan/test/b_plus_tree_delete_test --gtest_also_run_disabled_tests
```

两个原始删除用例目前都带 DISABLED_ 前缀，必须显式启用并确认实际执行。默认 0 用例不能算通过。完整树级删除测试依赖插入；早期可先构造页面验证借元素/合并，2 号合入后同步 project2 并复跑。

## 进入分支与推送

先配置[本人 Git 身份](../GIT_WORKFLOW.md)，首次进入已经创建的分支：

```bash
git fetch origin
git switch --track origin/p2/03-delete
```

提交本人已完成的小改动，只暂存以下路径中本次实际修改的文件；新增用例或其他协商后的文件再逐项添加：

```bash
git status
git diff
git add src/include/storage/index/b_plus_tree.h src/storage/index/b_plus_tree.cpp test/storage/b_plus_tree_delete_test.cpp reports/p2/parts/03-delete.md
git diff --cached
git commit -m "P2: implement task 3"
git fetch origin
git merge origin/project2
# 依赖更新后重跑本任务相关测试
git push
```

打开[创建本任务 PR](https://github.com/milaqihe/bustub-initial/compare/project2...p2/03-delete?expand=1)，确认 base 为 project2。后续原分支 commit / push 会更新同一个 PR。模块报告写本人 parts 文件，4 号最后统一汇总。

## 完成标准

借元素、叶子/内部合并、根收缩和空树正确；删除后剩余键可查且扫描有序；显式启用的原有删除用例和新增基础测试通过，页面资源正确回收。
