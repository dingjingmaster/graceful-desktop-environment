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
