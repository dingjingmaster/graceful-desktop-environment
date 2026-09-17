# Session 进程调研报告

> 文档元数据
> - 文件编号：2
> - 文档类型：research
> - 文件路径：docs/dev/2-research-session.md
> - 文档版本：v1.0.0
> - 最后更新：2026-09-17
> - 关联需求：实现 graceful session 进程，纯 C，并尽可能用 GObject 按功能分模块

## 1. 问题与边界

- 问题描述：项目已有 LightDM greeter，但认证成功后缺少可由 LightDM 启动的用户 session 根进程。
- 调研目的：参考 gnome-session 的职责，确定 graceful-session 第一版可验证的最小闭环。
- 包含：session 定义读取、基础环境变量、核心子进程启动与退出状态处理、LightDM xsessions 入口。
- 不包含：完整 GNOME D-Bus SessionManager API、inhibitor、systemd user target 集成、XDG autostart、session restore、电源管理和锁屏。
- 非目标：不复刻完整 GNOME Shell/gnome-session。
- 禁止触碰范围：不修改 greeter 认证逻辑；不修改系统 LightDM 配置；不引入非 C 运行时实现。

## 2. 当前证据

- 现有实现/现状：仓库已有 `greeter/`、`common/`；`docs/overview-product.md` 将 session 列为后续核心对象。
- 已知约束：项目使用 C、CMake、GLib/GObject；greeter 已通过 LightDM session key 启动系统 session。
- 关键参考：GNOME Session Manager 负责启动桌面核心组件和登录时应用；现代 GNOME 文档强调 session 定义文件与 systemd user target，但本项目第一版先做独立最小闭环。
- 证据不足项：真实 LightDM 登录后的 session 集成需要测试机验证。

## 3. 安全门禁摘要

| 项 | 结论 |
|----|------|
| 风险矩阵初判 | L3 |
| 风险标签 | system,lifecycle,public-contract,product-flow |
| 命令权限 | C0/C1；不执行 C2/C3 |
| 高风险开发门禁 | 是：C 逻辑、进程生命周期、公共 session 入口 |
| 破坏性操作 | 否 |
| 用户已有修改 | 否，`git status --short` 为空 |
| 用户确认事项 | 用户已确认进入实现 |

## 4. 候选方案

| 方案 | 核心思路 | 优点 | 风险/代价 | 适用条件 |
|------|----------|------|-----------|----------|
| A | 第一版实现独立 session 根进程，直接用 GSubprocess 管理核心进程 | 依赖少、可本地单测、能尽快闭环 LightDM 登录 | 暂不支持 systemd user units 与 D-Bus 管理 | 当前项目阶段 |
| B | 直接对齐 GNOME，接入 systemd user target 与 SessionManager D-Bus API | 长期能力完整 | 实现大、验证环境复杂、超出当前需求 | 桌面组件成熟后 |

## 5. 推荐结论

- 推荐方案：方案 A。
- 取舍理由：当前目标是建立 greeter 后的 session 根进程；先保证启动、环境和退出闭环，再扩展服务管理能力。
- 需要进入 Plan 的关键约束：纯 C；核心逻辑用 GObject 分模块；TDD；文档记录为独立文档和 session 模块文档。
- 需要用户确认的问题：无。
- 后续验证方向：CMake 构建、session 单元测试、全量 ctest、diff 空白检查；真实 LightDM 集成后续补测。
