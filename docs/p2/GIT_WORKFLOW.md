# P2 同步、提交与 PR

## 1 分工与本人身份

按[已确认分工](README.md#分支与成员)进入本人任务分支，已有 P1 克隆可直接使用。每位同学使用自己的 GitHub 账号和已验证邮箱提交；不要照抄别人的署名。

```bash
git config user.name "你的GitHub用户名"
git config user.email "你的GitHub已验证邮箱或noreply邮箱"
git config --get user.name
git config --get user.email
git remote -v
```

本仓库为 `milaqihe/bustub-initial`。已有 P1 克隆可以直接使用，不需要另建仓库。

## 2 进入已经创建好的任务分支

下面以任务 2 为例；其他任务将分支换成自己的名字。

```bash
git fetch origin
git switch --track origin/p2/02-search-insert
```

本地已经有同名分支时用 `git switch p2/02-search-insert`。五个任务分支均由同一份 `project2` 基线建立，含完整 P1 和本轮指南，不必再次创建远端分支。

## 3 同步公共集成成果

先保存或提交本人工作，在自己的任务分支执行：

```bash
git fetch origin
git merge origin/project2
```

以 `origin/project2` 为本轮依赖来源，其他成员已合并的实现会自然进入分支。不要复制他人的整个文件，也不要用强推覆盖共享成果。

## 4 测试、提交和推送

每份[任务指南](README.md)末尾给出了具体暂存路径和示例 commit。只暂存这次确实修改的文件：

```bash
git status
git diff
# 按任务指南逐项 git add
git diff --cached
git commit -m "P2: describe the completed change"
git push
```

远端拒绝 non-fast-forward 时，先 fetch 并合入对应远端分支处理实际冲突，不使用 force 覆盖。

## 5 PR 的目标分支

- `base`：**project2**。
- `compare`：本人 `p2/0x-...` 分支。
- 描述本次功能、基线、实际测试目标、通过/失败数量、新增用例和依赖；未完成时标 Draft。
- 共享函数、页面布局、辅助方法或锁责任发生变化时，请受影响负责人一起审阅。
- 由组长何佳绮（任务 5）安排审阅，通过后合入 `project2`；所有成员随后同步它。
- 只有第二次实验完整验收和报告汇总后，组长才创建 `project2 → main` 的最终 PR。第一轮的 main 提交保持可查看。

```text
任务编号 / 姓名：
修改的函数与文件：
基于 project2 的提交号：
完成的功能：
新增或变更的辅助接口：
测试环境、目标、完整命令和实际数量：
依赖与尚未完成项：
模块说明和运行截图：
```

每位同学在自己的 `reports/p2/parts/` 文件填写模块说明，4 号统一汇总 `reports/p2/report.md`，避免五个人同时改同一份报告。
