# 需求、任务与模块变更索引

> 记录路由结论为“模块+索引”或“独立文档”的功能开发、bug 修复、调研、计划、总结、评审和模块文档变更。新需求/问题优先读取本索引，再按相关性展开模块文档和少量独立文档，避免加载完整历史上下文。

## 独立文档编号规则

- 记录级别为“模块+索引”时，功能开发和 bug 修复更新模块文档但不分配编号；记录级别为“无”时不创建本地文档。
- 只有新增独立任务、问题修复、调研、计划、总结、评审或需求变更文档时，才分配编号。
- 独立文档编号使用从 `1` 开始递增的正整数，不要求固定位数。
- 编号全局只在 `docs/dev/` 下递增，不按类型分别编号。
- 新增独立文档前，先检查本索引和 `docs/dev/` 现有编号文件名，取最大编号 + 1。
- 同一需求的多份独立文档使用同一编号，例如 `2-research-xxx.md`、`2-plan-xxx.md`、`2-summary-xxx.md`。
- 编号一旦分配不得复用；取消、废弃、拆分、合并也要在索引中保留记录并标注状态。
- 文件命名格式：`N-[type]-[slug].md`，其中 `type` 可取 `summary`、`task`、`fix`、`research`、`plan`、`review`。
- 模块文档命名格式：`modules/[module].md`。

## 索引

| 日期 | 级别 | 风险标签 | 模块 | 类型 | 关联文档 | 状态 | 摘要 |
|------|------|----------|------|------|----------|------|------|
| 2026-09-16 | L3 | system,lifecycle,public-contract | greeter | research/plan/summary/module | [1-research-greeter.md](1-research-greeter.md), [1-plan-greeter.md](1-plan-greeter.md), [1-summary-greeter.md](1-summary-greeter.md), [modules/greeter.md](modules/greeter.md) | 已完成 | 基于 GTK4 与 liblightdm-gobject 实现 LightDM greeter 最小登录闭环。 |
| 2026-09-17 | L3 | system,lifecycle,public-contract,product-flow | session | research/plan/summary/module | [2-research-session.md](2-research-session.md), [2-plan-session.md](2-plan-session.md), [2-summary-session.md](2-summary-session.md), [modules/session.md](modules/session.md) | 已完成 | 新增纯 C/GObject `graceful-session`，建立登录后的 session 根进程最小闭环。 |
| 2026-09-17 | L2 | system,public-contract | desktop | module | [modules/desktop.md](modules/desktop.md) | 已完成 | 新增 GTK4/GObject `graceful-desktop` 第一阶段，绘制随机壁纸背景并支持整图渐变切换。 |
| 2026-09-17 | L2 | system,public-contract | panel | module | [modules/panel.md](modules/panel.md) | 已完成 | 新增 GTK4/GObject `graceful-panel` 第一阶段，固定单 panel 与内置模块布局。 |
| 2026-09-17 | L2 | system,public-contract | desktop,panel | fix/module | [modules/desktop.md](modules/desktop.md), [modules/panel.md](modules/panel.md) | 已完成 | 修复 desktop/panel 作为普通 GTK 窗口运行的问题；X11/Xwayland 下设置 EWMH desktop/dock 角色、底部 strut，并按 root 几何持续同步尺寸和位置。 |
| 2026-09-17 | L2 | system,public-contract | panel | task/module | [modules/panel.md](modules/panel.md) | 已完成 | 改进 panel 任务区：移除空任务文案，X11/Xwayland 下显示普通应用窗口图标，hover 显示缩略图/兜底预览，并增加半透明磨砂观感。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,public-contract | panel | fix/module | [modules/panel.md](modules/panel.md) | 已完成 | 修复 panel 任务预览闪烁和层级不够稳的问题：任务集合不变时不重建按钮，移除任务 tooltip，并通过 EWMH client message 强化 above/dock 状态。 |
