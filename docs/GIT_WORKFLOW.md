# Git 协作与推送指南

每人使用自己的 GitHub 账号与 Git 提交身份，在本仓库的个人分支开发，通过 Pull Request 合入 `main`。下文命令适用于 macOS、Linux、WSL 或 Git Bash。

## 1 接受邀请并克隆

先用自己的 GitHub 账号接受 `milaqihe/bustub-initial` 的协作者邀请。能浏览公开仓库不等于已经有推送权限。

```bash
git clone https://github.com/milaqihe/bustub-initial.git
cd bustub-initial
```

已有克隆的同学直接进入仓库目录，不必重复 clone。已配置 GitHub SSH 的同学也可使用：

```bash
git clone git@github.com:milaqihe/bustub-initial.git
```

HTTPS 推送使用你自己的 GitHub 登录/凭据管理器，GitHub 网页密码不能直接作为 Git HTTPS 密码；也可用已配置的 SSH。不要互借账号、Token 或私钥。网页上的登录账号、推送认证账号和 Git 提交署名需要分别正确设置。

## 2 设置你自己的提交身份

在当前仓库运行，先把下面两项替换成自己的 GitHub 用户名和账号中已验证的邮箱，或 GitHub 邮箱设置页面显示的 noreply 邮箱：

```bash
git config user.name "你的GitHub用户名"
git config user.email "你的GitHub提交邮箱"
git config --get user.name
git config --get user.email
git remote -v
```

`origin` 应指向 `milaqihe/bustub-initial`。不要照抄组长或其他同学的署名。这里的设置只作用于当前克隆。

## 3 各自使用哪个分支

| 编号 | 分支 | PR 标题示例 |
| --- | --- | --- |
| 1 号 | `team/01-integration` | `P1: unify build and integration` |
| 2 号 | `team/02-hash` | `P1: implement extendible hash table` |
| 3 号 | `team/03-lruk` | `P1: implement LRU-K replacer` |
| 4 号 | `team/04-buffer-pool` | `P1: implement buffer pool manager` |
| 5 号 | `team/05-quality` | `P1: add quality tests and acceptance report` |

每人的指南都提供了该分支的首次创建和提交命令。第一次执行 `git switch -c`；以后继续开发使用 `git switch 分支名`。若远端已有该分支但本地没有，执行 `git fetch origin` 后 `git switch --track origin/分支名`，不要另建同名但不相连的分支。

## 4 每天开始和提交前同步

先提交或妥善保存当前工作，再在自己的分支执行：

```bash
git status
git fetch origin
git merge origin/main
```

合入 `origin/main` 会引入组员已合并的实现，不应把它们重新复制成自己的实现。不要在有未保存改动时强制切分支或覆盖文件。

冲突出现时，用编辑器逐项处理冲突标记，向文件负责人确认预期，再执行 `git add 冲突文件的实际路径`、`git commit` 并重跑受影响测试。需要放弃本次合并时可用 `git merge --abort`。共享分支不要使用 `git push --force` 或 `git reset --hard` 处理冲突。

## 5 提交与推送

1. 跑完个人模块测试，查看 `git status`、`git diff`。
2. 按个人指南逐个暂存负责文件；避免 `git add .` 把临时文件与无关修改一起提交。
3. 执行 `git diff --cached` 确认本次内容，再 commit。
4. 首次 `git push -u origin 个人分支`，之后 `git push`。

不要提交 `build/`、编译产物、个人 IDE 设置、数据库临时文件、Token、私钥或课程提交 zip。测试输出保存在本地 `build/`；报告中记录命令、结果与必要的错误摘要。

## 6 在 GitHub 提交 PR

推送后打开[仓库 Pull requests](https://github.com/milaqihe/bustub-initial/pulls)，选择 **New pull request**：

- `base` 选择 `main`，`compare` 选择自己的 `team/0x-...` 分支。
- 阅读 **Files changed**，确认没有无关模块、整份源码复制或大量格式改动。
- 填写实现内容、测试结果、接口变化和依赖，提交 PR。未完成时使用 Draft。
- 在 PR 页面请求 1 号审阅；4 号参与 2、3 号接口审阅，5 号参与边界测试审阅。
- 审阅后在同一个分支继续 commit 和 push，原 PR 自动更新，不需要反复新建 PR。
- 由 1 号在满足合并条件后合入 `main`。其他成员不直接向 `main` 推送实现。

可复制以下描述内容，按实际填写：

```text
负责编号：
本次完成的功能：
修改文件：
对应接口是否变化：
基于 origin/main 的提交号：
运行环境和完整测试命令：
实际通过/失败数量：
新增边界用例与预期：
尚未解决的问题或依赖：
```

模块尚未完成时，把失败和依赖写清楚；不要写尚未运行的“全部通过”。

## 7 1 号合并与后续工作

1 号先在本地集成分支审阅变更、运行所需测试，再在 GitHub 合并对应 PR。集成顺序见[五人任务表](team/README.md)。当前原有 CI 只监听 `master`，1 号适配前以实际本地记录为合并依据。

PR 合并后，所有同学在干净工作区更新本地 main：

```bash
git switch main
git pull --ff-only origin main
```

继续同一未合并 PR 时回到原分支；已经合并的任务，后续修复从最新 main 开新分支，例如 `team/02-hash-fix-01`。这样不会把旧 PR 的提交重复带入下一轮。

## 常见错误怎么处理

| 现象 | 处理 |
| --- | --- |
| `403` / 没有写权限 | 确认已接受邀请、Git 认证账号是自己的协作者账号、origin 正确 |
| `Permission denied (publickey)` | SSH 尚未配置到自己的账号；配置自己的 SSH，或使用 HTTPS 登录 |
| `non-fast-forward` | 先 fetch，合入对应远端分支并处理冲突，再推送；不要强推 |
| `branch already exists` | 继续已有分支，使用 `git switch`；不要重复 `-c` |
| 本地 main 不能 `--ff-only` | main 出现个人提交或分叉，先保存工作，请 1 号一起整理，不强制覆盖 |
| PR 比较方向错误 | 应为 `main ← 自己的开发分支` |
| 修改文件超出本人归属 | 去掉无关改动，或明确请对应负责人共同审阅 |
