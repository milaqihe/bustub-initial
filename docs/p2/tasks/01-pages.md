# 任务 1：页面基础与公共集成

负责人：**吴佳忆**；GitHub：`wjy676`。开发分支：`p2/01-pages`，PR 目标：`project2`。

你实现 BPlusTreePage、InternalPage、LeafPage 的页面原语，和 2/3 号先确定 size、容量、定位和搬移接口，并维护 P2 的公共构建入口。三类页面是其他任务的共同依赖，优先提交一个小的基础 PR。

## 你改哪些地方

- `src/include/storage/page/b_plus_tree_page.h`、`src/storage/page/b_plus_tree_page.cpp`
- `src/include/storage/page/b_plus_tree_internal_page.h`、`src/storage/page/b_plus_tree_internal_page.cpp`
- `src/include/storage/page/b_plus_tree_leaf_page.h`、`src/storage/page/b_plus_tree_leaf_page.cpp`
- 公共配置：`CMakeLists.txt`、`test/CMakeLists.txt`、必要的 `cmake/` / `.github/workflows/` 配置。
- 本人模块说明：`reports/p2/parts/01-pages.md`。

## 按顺序完成

1. 实现页类型、根判断、size 增减、max/min size、父页和自身编号、LSN 等现有方法。
2. 实现内部页、叶子页 Init；初始化各自布局、容量和叶子 next_page_id。
3. 实现现有 KeyAt / SetKeyAt / ValueAt / next 等访问方法；内部页第 0 个 key 无效但 value 有效。
4. 和 2/3 号约定定位、插入、删除、搬移辅助方法，明确调用时是否已经获取页面与锁，再分批实现。页面数组保持当前页内布局。
5. 添加页面基础测试：初始化、字段读写、容量计数、根页例外、next 链和实际新增搬移操作。
6. 增加 P2 专用构建/测试入口、P2 标签和超时；接入 4 号独立测试；把 P2 CI 的 push / PR 目标配置为 project2 和 p2 任务分支。
7. 保留 P1 的 check-p1 回归；统一 P2 格式、lint、clang-tidy 和包含 16 个源码文件的打包。
8. 在本人说明中给出三类页布局、size 规则、接口签名和实际基础测试结果。

## 和谁交接

和 2/3 号一起确认叶子与内部节点的满页/最小占用规则，以及页面搬移后 parent_page_id 的维护责任；和 4 号交接叶子 next 链；和 5 号确认页面原语不重复获取树层已经持有的锁。接口签名填入 `docs/p2/INTERFACES.md`。

## 测试与验证

现有仓库没有单独的页面测试文件，需要先新增例如 `test/storage/b_plus_tree_page_test.cpp`，再构建对应目标。当前 P2 功能尚未实现，不需要靠修改其他人的树算法让基础页面 PR 变绿。

```bash
cmake --build build/p2-asan --target b_plus_tree_page_test -j 4
./build/p2-asan/test/b_plus_tree_page_test
cmake --build build/p2-asan --target check-p1 -j 4
```

第一条新目标只在你创建相应用例并重新配置后存在。完整配置见[构建指南](../BUILD.md)。

## 进入分支与推送

先按[本人身份与推送指南](../GIT_WORKFLOW.md)配置 Git；分支已创建，本地首次使用：

```bash
git fetch origin
git switch --track origin/p2/01-pages
```

提交本人已完成的小改动时，暂存下列路径中这次实际修改的文件；新增测试、公共配置等再按实际逐项 git add：

```bash
git status
git diff
git add src/include/storage/page/b_plus_tree_page.h src/storage/page/b_plus_tree_page.cpp src/include/storage/page/b_plus_tree_internal_page.h src/storage/page/b_plus_tree_internal_page.cpp src/include/storage/page/b_plus_tree_leaf_page.h src/storage/page/b_plus_tree_leaf_page.cpp reports/p2/parts/01-pages.md
git diff --cached
git commit -m "P2: implement B+ tree page primitives"
git fetch origin
git merge origin/project2
# 依赖更新后重跑本任务相关测试
git push
```

打开[创建本任务 PR](https://github.com/milaqihe/bustub-initial/compare/project2...p2/01-pages?expand=1)，确认 base 为 project2。后续在原分支继续 commit / push，同一个 PR 自动更新。报告写入本人 parts 文件，不直接覆盖其他成员章节。

## 完成标准

三类页面与已约定原语可用，有独立基础测试；公共构建入口能发现 P2 用例；2/3/4/5 能按文档调用；本人模块说明、截图与实际测试结果完整。
