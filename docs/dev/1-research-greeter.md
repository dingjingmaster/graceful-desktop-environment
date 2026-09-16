# LightDM Greeter 调研报告

> 文档元数据
> - 文件编号：1
> - 文档类型：research
> - 文件路径：docs/dev/1-research-greeter.md
> - 文档版本：v1.0.0
> - 最后更新：2026-09-16
> - 关联需求：基于 GTK4 实现 LightDM greeter

## 1. 问题与边界

- 问题描述：为 graceful desktop environment 增加第一个 LightDM greeter，实现图形登录入口。
- 调研目的：确认最小可用 greeter 的依赖、认证流程和模块边界。
- 包含：GTK4 窗口、LightDM daemon 连接、用户名/密码输入、session 选择、PAM 认证响应、认证成功后启动 session。
- 不包含：主题系统、头像、远程登录、电源菜单、锁屏、辅助功能、HiDPI 专项适配。
- 非目标：不在 `common/` 放 greeter 私有逻辑；不实现 session、settings-daemon、desktop。
- 禁止触碰范围：不修改系统 LightDM 配置，不安装/启用 greeter，不执行 sudo/systemctl。

## 2. 当前证据

- 现有实现/现状：仓库已有 `greeter/main.c` 空入口，顶层 CMake 已加入 `common` 与 `greeter`。
- 已知约束：项目为纯 C；可用 GObject 拆分模块；`common` 只放多个组件共享的纯 C 库。
- 复现环境/替代过程：本地可通过 `pkg-config` 找到 `gtk4 4.22.5` 与 `liblightdm-gobject-1 1.32.0`。
- 关键日志/数据/用户反馈：用户明确要求 greeter 基于 LightDM，窗口管理器后续使用 Mutter。
- Bug 证据等级：不适用。
- 根因/待验证根因：不适用。
- 证据不足项：未在真实 LightDM 会话中实测启动和 PAM 交互，本阶段以构建、单元测试和代码路径审查为主。

## 3. 安全门禁摘要

| 项 | 结论 |
|----|------|
| 风险矩阵初判 | L3 |
| 风险标签 | system,lifecycle,public-contract |
| 命令权限 | C0/C1 |
| 高风险开发门禁 | 是，涉及登录链路、生命周期和外部 LightDM API |
| 破坏性操作 | 否 |
| 用户已有修改 | 是，已有 CMake/common/greeter 初始文件；基于现状增量修改 |
| 用户确认事项 | 已确认 GTK4、纯 C、greeter 代码放 `greeter/`、`common/` 仅放公共纯 C 库 |

## 4. 候选方案

| 方案 | 核心思路 | 优点 | 风险/代价 | 适用条件 |
|------|----------|------|-----------|----------|
| A | 单文件 `main.c` 完成 UI 与 LightDM 状态机 | 代码量最少 | 难测试，后续容易膨胀 | 只做一次性原型 |
| B | `main.c` 负责 UI/LightDM，greeter 内部 GObject 模型负责登录输入状态 | 可测试，边界清楚，仍保持最小实现 | 文件略多 | 当前第一版 |
| C | 完整拆分 app/window/auth/session 多个 GObject 类型 | 扩展性强 | 第一版过度设计 | 主题、电源、用户列表等功能明确后 |

## 5. 推荐结论

- 推荐方案：方案 B。
- 取舍理由：第一版需要先证明 LightDM 登录闭环，同时保留可测的纯 C/GObject 模块；避免把 greeter 私有逻辑放入 `common`。
- 需要进入 Plan 的关键约束：只新增 greeter 内部模块和测试；CMake 增加 GTK4、LightDM、GLib 测试依赖；不修改系统配置。
- 需要用户确认的问题：无。
- 后续验证方向：`ctest` 覆盖模型行为，CMake 构建覆盖 GTK4/LightDM API 兼容性，真实 LightDM 登录留到集成环境验证。

## 6. 参考资料

- LightDM Development：greeter 使用 `liblightdm`，GObject 项目使用 GObject 变体。
- 本机头文件：`/usr/include/lightdm-gobject-1/lightdm/greeter.h`、`session.h`、`user.h`。
