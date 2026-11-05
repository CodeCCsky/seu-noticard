# seu-noticard · 东南大学通知台

把东南大学教务处、学生处的通知抓下来，抽取成卡片，用本地网页按主题展示；支持已读和稍后提醒。面向 Windows 10/11 环境

> **状态**：开发中，尚未发布可运行版本。进度与待办见 [docs/status.md](docs/status.md)

## 功能

- [ ] 多站聚合：定时抓取教务处、学生处等学校站点的通知，按栏目汇总到一个页面
- [ ] 详情补全：逐条读取详情页，提取时间、地点、主办、截止日期，使事项一目了然
- [ ] 主题分类：按选课、考试、讲座、竞赛、推免、奖助、培养、其他 8 类自动打标签，分类明确不烧心
- [ ] 卡片墙：按栏目和主题筛选，新通知高亮，截止日期显示倒计时
- [ ] 阅读管理：标记已读、设置提醒、隐藏，到点自动置顶
- [ ] 本地优先：只监听 127.0.0.1，数据存本机 SQLite，可选 AI 抽取（需联网，不主动开启）
- [ ] 离线可用：自动缓存新闻数据，没有校园网也能查看

## 技术栈

| 层 | 选型 |
|---|---|
| 语言 | C++14（VS2026 + v141 工具集，兼容 VS2017） |
| 网络 | WinHTTP（客户端）、WinSock2（本地服务） |
| 存储 | SQLite amalgamation，单文件 |
| 界面 | HTML/CSS/JS，WebView2 运行时 |
| 构建 | CMake ≥ 3.15 |

除 SQLite 源码和系统 WebView2 运行时外无第三方依赖；目前不需要 Node、npm、VC++ 运行库

## 目录结构

~~~text
seu-noticard/
  CMakeLists.txt
  README.md  AGENTS.md
  docs/          学习、状态、Git 规范
  third_party/   sqlite3.c sqlite3.h
  config/        sites.json tags.json fields.json
  web/           index.html app.css app.js
  tools/         embed.py
  src/
    model/       types.h
    util/        text date
    proto/       json
    net/         http
    parse/       html list fields
    store/       db
    app/         config crawl tag remind
    view/        viewmodel json_view
    web/         server assets
    entry/       mcp（P1）
  tests/         test_main.cpp
  fixtures/      页面快照（只读）
  data/          运行时数据库，demo.db 随交付
~~~

## 构建

前置：Windows 10/11、VS2026（工作负载"使用 C++ 的桌面开发"）、v141 工具集、CMake ≥ 3.15

工程建好后执行：

~~~text
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -T v141 -DCMAKE_SYSTEM_VERSION=10.0
cmake --build build --config Release
~~~

产物：build\x64-Release\seu-noticard.exe

## 运行

（待实现）

~~~text
seu-noticard.exe
~~~

启动后打开桌面窗口

## 配置

（待实现）config/ 下三个文件：站点与解析提示、标签词表、字段别名

## 文档

- [主力学习路线](docs/学习-主力.md)
- [新手两周上手](docs/学习-新手.md)
- [项目状态](docs/status.md)
- [Git 规范](docs/git.md)

## 已知限制

- 微信外链只保留标题和链接，不抓正文（微信公众号限制，比较难爬取）
- 内嵌 PDF 和纯附件页只显示标题，不解析附件
- 时间、地点、截止字段尽力而为，抽不到就留空
- 抓取限速每 3 秒 1 次请求，不做高频轮询防止被墙

## AI 使用说明

AI 用于文档编写、评审、测试用例讨论；项目大纲、方向与 C++ 代码由**能工智人**（人类）编写。详细记录见 docs/ai-usage.md（暂时还没有）

## 许可证

MIT，具体见 [LICENSE](LICENSE)
