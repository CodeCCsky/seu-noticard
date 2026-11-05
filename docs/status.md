# 项目状态

> 这里放会变的东西：进度、待办、待定问题。稳定规则在 [AGENTS.md](../AGENTS.md)
> 这个文档由 AI Agent 维护。Agent 读这个，但是记得做完要在这里同步！

## 当前阶段

W0 收尾：接口冻结接近完成，准备开始编码。

## 已完成

- 数据模型 src/model/types.h
- 纯函数接口 src/util/text.h
- 页面快照 fixtures/（3 个列表页 + 6 个详情页）
- 两份学习文档
- AGENTS.md、README 骨架、本文件

## 下一步

1. 其余模块头文件（db、http、parse、view、web）
2. CMakeLists.txt + main.cpp 空跑
3. docs/api.md + web/mock.json
4. 前端三件套

## 待建

- config/sites.json、tags.json、fields.json
- web/index.html、app.css、app.js
- tools/embed.py
- tests/test_main.cpp
- docs/ai-usage.md

## 待定

- 桌面壳：WebView2，W4 接入；Tauri 和 Electron 不再考虑。
- data/demo.db 预置：等抓取跑通后生成。
- 老师机器能否访问校园网：决定 demo.db 是否必须。
