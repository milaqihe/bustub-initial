# 任务 2：单点查询、插入与分裂

认领人：待填写姓名 / GitHub。开发分支：`p2/02-search-insert`，PR 目标：`project2`。

你负责单线程 B+ 树的查找与插入主链，提供 3/4/5 号复用的找叶子页方法，并正确处理多层分裂和新根。

## 你改哪些地方

- `src/include/storage/index/b_plus_tree.h`、`src/storage/index/b_plus_tree.cpp` 中构造、IsEmpty、GetRootPageId、GetValue、Insert、查询与插入辅助方法。
- `test/storage/b_plus_tree_insert_test.cpp`；按需要追加独立查询基础用例。
- 本人模块说明：`reports/p2/parts/02-search-insert.md`。
- 页面原语由 1 号维护；Remove 属于 3 号，Begin/End 属于 4 号。

## 按顺序完成

1. 和 1 号确定内部页的查找规则、叶子页定位、插入和搬移接口。
2. 实现空树判断与实际根页编号，添加公共找叶子页方法，明确返回页面的释放责任。
3. 实现 GetValue：按 comparator 路由到叶子页，命中返回 RID，不存在返回 false。
4. 实现首次插入建根、普通有序插入；重复键返回 false，不覆盖旧记录。
5. 实现叶子分裂与 next 链维护；把分隔键传到父节点。
6. 实现内部节点分裂、孩子 parent_page_id 更新、向上连续分裂及创建新根。
7. root_page_id 变化时调用 UpdateRootPageId，把根记录写入 header 页。
8. 保证修改页面的 dirty 与 Fetch/New 对应的 Unpin；给出小容量连续分裂例子。

## 和谁交接

共享遍历由你首先设计，3 号用于定位删除页，4 号用于 Begin(key)，5 号将在其上加锁。先在 `INTERFACES.md` 写清参数、返回页和资源归属。只改查询/插入相关函数，不重写 Remove 或 Begin/End。

## 测试与验证

```bash
cmake --build build/p2-asan --target b_plus_tree_insert_test -j 4
./build/p2-asan/test/b_plus_tree_insert_test
```

该目标目前包含 3 项（其中 ScaleTestC1 就是本仓库的规模测试），部分用例会扫描整棵树，需要 4 号迭代器接入。早期可先用 GetValue 验证插入结果，PR 中写清实际运行范围和依赖；最终必须回到完整目标。

覆盖空树、重复键、升序/降序/固定种子随机插入、连续多层分裂、叶链顺序与 header 根编号。

## 进入分支与推送

先按[本人身份与推送指南](../GIT_WORKFLOW.md)配置 Git；分支已创建，本地首次使用：

```bash
git fetch origin
git switch --track origin/p2/02-search-insert
```

提交本人已完成的小改动时，暂存下列路径中这次实际修改的文件；新增测试、公共配置等再按实际逐项 git add：

```bash
git status
git diff
git add src/include/storage/index/b_plus_tree.h src/storage/index/b_plus_tree.cpp test/storage/b_plus_tree_insert_test.cpp reports/p2/parts/02-search-insert.md
git diff --cached
git commit -m "P2: implement B+ tree search and insertion"
git fetch origin
git merge origin/project2
# 依赖更新后重跑本任务相关测试
git push
```

打开[创建本任务 PR](https://github.com/milaqihe/bustub-initial/compare/project2...p2/02-search-insert?expand=1)，确认 base 为 project2。后续在原分支继续 commit / push，同一个 PR 自动更新。报告写入本人 parts 文件，不直接覆盖其他成员章节。

## 完成标准

唯一键查询/插入正确，分裂后所有旧记录可查且叶链有序，根页更新正确；原有插入/规模用例与新增基础用例通过，交接公共遍历和分裂说明。
