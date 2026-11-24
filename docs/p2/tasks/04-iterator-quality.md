# 任务 4：迭代器、独立测试与报告汇总

认领人：待填写姓名 / GitHub。开发分支：`p2/04-iterator-quality`，PR 目标：`project2`。

你实现范围扫描的 IndexIterator 和树的 Begin/End，并负责独立补充测试、验收记录及报告汇总。各模块负责人仍提交自己的基础测试和模块说明。

## 你改哪些地方

- `src/include/storage/index/index_iterator.h`、`src/storage/index/index_iterator.cpp`。
- `src/include/storage/index/b_plus_tree.h`、`src/storage/index/b_plus_tree.cpp` 中 Begin()、Begin(key)、End()。
- 新增独立测试：`test/p2_quality/`，使用唯一的 `p2_quality_*_test.cpp` 文件名。
- 本人说明：`reports/p2/parts/04-iterator-quality.md`。
- 汇总文件：`reports/p2/report.md`、`reports/p2/acceptance.md` 和截图。

## 按顺序完成

1. 和 1 号确认叶子 next 链和键值读取接口，和 2 号确认最左端与指定 key 的定位方法。
2. 定义构造、结束态、页面资源归属、析构以及复制/移动行为。
3. 实现 IsEnd、解引用、前置递增、相等与不等比较；跨叶子页时正确获取新页并释放旧页。
4. 实现 Begin()、Begin(key)、End()；覆盖空树、单页、多页、key 不存在、页间 key 和大于最大值。
5. 在独立目录追加边界/回归用例，预期通过手算或独立有序集合确定。
6. 和 3 号验证删除合并后的叶链，和 5 号确定扫描与并发删除的锁配合。
7. 集成齐备后运行完整插入、显式启用的删除、并发和独立测试，再分别执行 ASan、TSan、压力与重复运行。
8. 记录被测提交、实际环境、线程数、操作次数、随机种子和超时；汇总大家的模块说明、树图及真实运行截图。

## 和谁交接

插入和叶子链可用后即可开始扫描联调，不必等待全部删除/并发实现。各人写本人 parts 文件，你负责汇总报告；1 号接入构建和 CI，你提供新增目标名及所需超时。

## 测试与验证

先按[构建指南](../BUILD.md)配置本轮独立目录。

新增例如 `test/p2_quality/p2_quality_iterator_test.cpp` 后重新配置并执行：

```bash
cmake -S . -B build/p2-asan
cmake --build build/p2-asan --target p2_quality_iterator_test -j 4
./build/p2-asan/test/p2_quality_iterator_test
```

新目标只有在相应测试文件实际创建后存在。与 2/3 号联调时，同时运行插入和显式启用的删除测试；最终把各次结果与实际 project2 提交对应。

## 进入分支与推送

先配置[本人 Git 身份](../GIT_WORKFLOW.md)，首次进入已经创建的分支：

```bash
git fetch origin
git switch --track origin/p2/04-iterator-quality
```

提交本人已完成的小改动，只暂存以下路径中本次实际修改的文件；新增用例或其他协商后的文件再逐项添加：

```bash
git status
git diff
git add src/include/storage/index/index_iterator.h src/storage/index/index_iterator.cpp src/include/storage/index/b_plus_tree.h src/storage/index/b_plus_tree.cpp test/p2_quality reports/p2
git diff --cached
git commit -m "P2: implement task 4"
git fetch origin
git merge origin/project2
# 依赖更新后重跑本任务相关测试
git push
```

打开[创建本任务 PR](https://github.com/milaqihe/bustub-initial/compare/project2...p2/04-iterator-quality?expand=1)，确认 base 为 project2。后续原分支 commit / push 会更新同一个 PR。模块报告写本人 parts 文件，4 号最后统一汇总。

## 完成标准

迭代器跨页与资源管理正确；独立用例验证边界和集成行为；报告与截图对应实际版本，发现的真实缺陷交负责人修复并复测。
