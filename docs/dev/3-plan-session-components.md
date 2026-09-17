# Session 组件编排计划

> 文档元数据
> - 文件编号：3
> - 文档类型：plan
> - 文件路径：docs/dev/3-plan-session-components.md
> - 文档版本：v1.0.0
> - 最后更新：2026-09-17
> - 关联需求：session 启动时自动拉起 mutter、desktop、panel，并使用 ibus + rime 输入法

## 1. 目标与成功标准

- 任务目标：将 `graceful-session` 默认启动路径从单个 `graceful-desktop` 进程升级为多个桌面组件编排。
- 成功标准：默认组件顺序包含 `mutter --replace`、`ibus-daemon --daemonize --xim`、`ibus engine rime`、`graceful-desktop`、`graceful-panel`；环境变量启用 ibus；命令行显式 `-- COMMAND...` 调试语义保持可用。
- 非目标：不实现 systemd user target、XDG autostart、D-Bus SessionManager、Wayland compositor session 入口。

## 2. 修改边界

- 修改范围：`session/` 组件模型、进程等待 API、manager 编排逻辑、session 单元测试、session 模块文档、索引和开发概览。
- 禁止触碰范围：不修改 greeter、desktop、panel 的实现；不安装软件包；不修改系统 LightDM 配置。
- 依赖关系：继续使用 GLib/GObject/GIO，不引入新构建依赖。

## 3. 安全门禁摘要

| 项 | 结论 |
|----|------|
| 风险矩阵结论 | L3 |
| 风险标签 | system,lifecycle,public-contract,product-flow |
| 验证范围 | 模块或关键链路 |
| 记录级别 | 独立文档 |
| 命令权限 | C0/C1 |
| 高风险开发门禁 | 是：C 逻辑、进程生命周期、公共 session 入口 |
| 破坏性操作 | 否 |

## 4. 执行计划

| 步骤 | 修改内容 | 验证方式 | 状态 |
|------|----------|----------|------|
| 1 | RED：新增环境变量和默认组件列表测试 | 定向构建/测试失败于缺失组件模型和 ibus 环境 | 完成 |
| 2 | GREEN：新增 `GracefulSessionComponent`，扩展环境和 manager 默认编排 | session 定向测试通过 | 完成 |
| 3 | 修正生命周期：为 `GracefulSessionProcess` 异步等待增加 `GTask` 包装 | 异步等待单元测试通过 | 完成 |
| 4 | 验证与文档：运行全量验证并更新长期上下文 | `ctest`、`git diff --check` | 完成 |
