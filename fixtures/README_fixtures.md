# fixtures 说明

> 2026-11-02 用带 Chrome UA 的请求抓取，间隔 3 秒，UTF-8 无 BOM
> 测试只读这些文件，**不联网**。站点改版后重新抓一份，文件名带日期
> 用于编写时参考和离线测试，实际测试中一定要抓真实网页！

## 列表页

| 文件 | 来源 | 期望 |
|---|---|---|
| `list_jwc_jwxx_20261102.htm` | https://jwc.seu.edu.cn/jwxx/list.htm | 15 条，容器 `div.news-list-item`；链接 **15/15 是单引号** `href='...'`；日期在 `div.news-item-date-ym`，**位于 <a> 之后**，必须按块取 |
| `list_jwc_xjgl_20261102.htm` | https://jwc.seu.edu.cn/xjgl/list.htm | 同上 15 条 |
| `list_xsc_tzgg_20261102.htm` | https://xsc.seu.edu.cn/64952/list.htm | 15 条，容器 `li.news`；其中 **6 条是微信外链** `mp.weixin.qq.com`；页脚另有 8 个 `<tr>`（不是列表，别被误导） |

## 详情页

| 文件 | 来源 | 特征 | 期望 |
|---|---|---|---|
| `detail_jwc_pdf_20261030.htm` | /2026/1030/c21678a588177/page.htm | 正文是内嵌 PDF：`<span pdfsrc="...pdf">` | 切成正文区后**文本为空** → `detail_status=3`，摘要退化为标题 |
| `detail_jwc_exam_20261029.htm` | /2026/1027/c21678a587732/page.htm | 散文式："将于如下时间进行：" + "课程名：2026-12-16 19:00" | **没有"时间：/地点："标签**；用标签规则应抽不到，用日期模式才可能命中 |
| `detail_jwc_xueji_20261014.htm` | /2026/1013/c21679a585911/page.htm | 有"二、时间安排 即日起至2026年10月23日"；6 个附件链接 | 能抽到 `time`（或至少日期）；`has_attachment=true` |
| `detail_jwc_zhuanzhuanye_20260916.htm` | /2026/0915/c21679a583204/page.htm | "2、申请时间：\n2026年9月16日至2026年9月18日下午17:00"；3 个附件 | 能抽到 `deadline`/`time`；标签后是换行，规则要能吃下 |
| `detail_xsc_jobfair_20251023.htm` | /2025/1203/c64952a547606/page.htm | 开头就是"时间：2025年10月25日（周六）9:00—12:00 / 地点：九龙湖…"；含大表格 | 两个字段都能抽到；正文含表格，`</td>` 必须转换行 |
| `detail_xsc_pdf_20250814.htm` | /2025/1203/c64952a547604/page.htm | 正文只有一个 PDF 附件链接 | 文本几乎为空 → `detail_status=3`，`has_attachment=true` |

## 必须覆盖的回归点

1. **单引号 href**：只认双引号的扫描器会在这个 fixtures 上返回 0 条。
2. **块级日期**：用"链接前后 ±800 字节找日期"的做法，会把 `list_xsc_tzgg` 里 2025-12-03 的条目错标成 2026-04-01。
3. **正文区**：整页 `strip_tags` 会让摘要以"首页 / 部门简介 / 领导分工…"开头。
4. **PDF 页**：必须能识别并跳过，不能当成"解析失败"反复重试。
5. **附件**：只记布尔值，不下载。
