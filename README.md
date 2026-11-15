# seu-noticard · 东南大学通知台

seu-noticard 是一个东南大学校园通知聚合工具，使用 C++14 开发，面向 Windows 10/11 环境。
计划抓取教务处和学生处的通知，在本地页面中集中展示，支持主题分类、已读标记和稍后提醒。数据保存在本机，已经抓取的通知可以离线查看。

> **状态**：🚧这是一个课程项目，目前处于开发阶段🚧 
> 
> 进度与待办见 [docs/status.md](docs/status.md)

## 功能

- [ ] 抓取教务处、学生处指定栏目的通知
- [ ] 提取正文摘要，并尝试识别时间、地点和截止日期
- [ ] 按主题分类，在本地页面中展示
- [ ] 标记已读、隐藏通知、设置稍后提醒
- [ ] 将通知保存在 SQLite 数据库中，供离线查看

> 这些功能会逐步实现，目前暂不能作为完整工具使用

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

## 开发

### 环境

- Windows 10/11，x64
- Visual Studio 2026，安装“使用 C++ 的桌面开发”工作负载
- MSVC v141 工具集
- CMake，版本需要支持 Visual Studio 2026 生成器

项目使用 C++14。开发时使用 v141 工具集，兼容 Visual Studio 2017 编译环境

除 SQLite 源码和系统 WebView2 运行时外无第三方依赖；目前暂时不需要 Node、npm、VC++ 等运行库

### 构建

在仓库根目录执行：

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -T v141 -DCMAKE_SYSTEM_VERSION="10.0"
cmake --build build --config Release
```

按上述方式构建时，可执行文件默认位于：

```text
build/Release/seu-noticard.exe
```

构建输出保存在 `build/`，不加入版本控制

### 运行

（待实现）

~~~text
seu-noticard.exe
~~~

启动后打开桌面窗口

### 代码格式

使用 clang-format 默认配置统一 C++ 代码格式。格式规则确定后，将配置保存在仓库根目录的 `.clang-format` 中。

添加配置文件后，可以在仓库根目录格式化指定文件：

```powershell
clang-format -i -style=file src/util/text.h
```

`-i` 表示直接修改文件，`-style=file` 表示读取格式配置。

只格式化自己修改的文件，不处理第三方源码。

## 配置

（待实现）config/ 下三个文件：站点与解析提示、标签词表、字段别名

> AI 配置仍在考虑中

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
