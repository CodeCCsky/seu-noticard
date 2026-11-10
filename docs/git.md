# Git 协作规范

> 3 人、5 周、Windows + VS2026、无 CI。目标只有一个：main 随时能编译，三个人不互相堵。
> 这份文件随实际使用调整；稳定规则在 [AGENTS.md](../AGENTS.md)。
> 这份文档使用大肥鱼 deepseek-v4.1 flash 1017 生成
> 读这个的大概是 Agent，所以一定要记得检查当前分支对不对！如果不对先提醒你的用户！

## 1. 分支模型：main + 短分支

- main 是唯一长期分支，随时可编译、可运行。
- 功能分支从 main 拉，寿命不超过 3 天，合完删掉。
- 不搞 Git Flow。3 个人用不上 develop/release/hotfix 三层。

~~~text
main ──●────●──────●────●──►
        \         /    /
         ●──●──●  /    feat/parse-list
                   ●──●
                   feat/web-ui
~~~

## 2. 分支命名

格式：`<type>/<scope>-<short>`

| 段 | 取值 | 例 |
|---|---|---|
| type | feat / fix / docs / chore / test | feat |
| scope | 模块：parse / db / web / ui / docs | parse |
| short | 2–4 个英文小写词，连字符 | list |

例：`feat/parse-list`、`fix/db-lock`、`docs/readme`、`chore/gitignore`。

两个人改同一个模块时，加名字首字母：`feat/parse-html-b`。

## 3. 提交信息

格式：

~~~text
<type>(<scope>): <subject>

<body>
~~~

- subject 用中文，动词开头，不超过 50 字，不加句号。
- type 用 feat / fix / docs / test / build / chore / refactor。
- body 写为什么改、影响什么。不写怎么改，diff 里有。
- 一个提交只做一件事。格式化、改名、功能分开提交。

例：

~~~text
feat(db): 增加详情队列查询

详情页抓取需要按发布时间倒序取待抓条目。
新增 pending_details，带 limit 参数。
~~~

## 4. 推送前自检

1. 跑 `git status`，确认没有 build/、.vs/、*.db、.scratch/、.probe/。
2. 能构建就跑一次 `--self-test`；不能构建就在 PR 里写“未构建 + 原因”。
3. 把 main 合进自己的分支，解决冲突后再推。
4. 检查分支名和提交信息。

## 5. PR 流程

1. 推送分支，在 GitHub 或 Gitee 开 PR。
2. 标题用提交信息第一行。
3. 描述写四段：改了什么 / 为什么 / 怎么验证 / 影响哪些模块。
4. 至少 1 人 review，清单见下。
5. Squash and merge，合并后删分支。

### Review 清单

- 架构边界：parse、util、proto 是否碰了 net/store/view；view 是否碰了 SQL；网络是否只出现在 `net/http.cpp`；SQL 是否只出现在 `store/db.cpp`；`web/server.cpp` 是否有业务逻辑。
- C++14：有没有 C++17 特性。
- 错误处理：是否使用 nc::Status，是否忽略返回值。
- 命名：`nc`、PascalCase、snake_case、`DbItem`/`ItemState`。
- 生成物：有没有把 build/、.vs/、*.db 带进来。
- 无关改动：有没有顺手格式化、顺手重构。

## 6. main 保护

在仓库设置里开启：

- 禁止直接 push main；
- 合并前需要 1 个 approve；
- 禁止 force push；
- 合并后自动删分支。

W0 搭骨架阶段可以临时直推，骨架稳定后开启保护。

## 7. 冲突处理

冲突集中在三个文件：`CMakeLists.txt`、`src/model/types.h`、docs。

- 这三个文件归 owner 改，其他人只提需求。
- 解决冲突时保留双方意图，不要整段删。
- 不要用 `checkout --ours` 或 `--theirs` 一把梭。
- 解决完重新构建一次。

## 8. 回滚

- 已推送的错误提交：用 `git revert <sha>`，不要改写公共历史。
- 未推送的本地提交：可以 reset，但先确认没人基于它工作。
- 不要在 main 上做 interactive rebase。

## 9. 标签

| 标签 | 时间 | 含义 |
|---|---|---|
| v0.1 | W2 末 | 命令行能抓、能入库 |
| v0.5 | W4 末 | 界面能用 |
| v1.0 | 交付 | 打包完成 |

只给 main 打标签。标签说明写一句这次交付的范围。

## 10. 行尾与二进制

Windows 和 WSL 混用会改行尾。fixtures 是快照，字节变了测试就会挂。

.gitattributes :
~~~text
* text=auto
*.cpp text eol=lf
*.h text eol=lf
*.md text eol=lf
*.json text eol=lf
*.html text eol=lf
*.css text eol=lf
*.js text eol=lf
fixtures/** -text
*.db binary
*.pdf binary
*.png binary
~~~

`fixtures/** -text` 让 Git 不做行尾转换，快照保持原样。

## 11. 每日节奏

开工前：

~~~text
git switch main
git pull --ff-only
~~~

开工：

~~~text
git switch -c feat/xxx
~~~

收工：

~~~text
git add <文件>
git commit -m "feat(xxx): ..."
git push -u origin feat/xxx
~~~

合并前：把 main 合进分支，本地构建一次。
合并后：删分支，回到 main 拉一次。

## 12. 新人最小命令集

~~~text
git switch main
git pull --ff-only
git switch -c feat/text-trim
# 改代码
git status
git add src/util/text.cpp
git commit -m "feat(text): 实现 trim"
git push -u origin feat/text-trim
~~~

PR 在网页上开。不会的命令先问 AI，再不会的在群里问，不要猜着执行。

## 13. 禁止

- force push 到 main 或别人的分支。
- 提交 build/、.vs/、*.db、.scratch/、.probe/。
- 改 .gitignore 绕过检查。
- 一个 PR 混多个不相关改动。
- 直接 push main（W0 之后）。
- 删除别人的分支。
