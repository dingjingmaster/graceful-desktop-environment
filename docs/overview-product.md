# Graceful Desktop Environment 产品概览

> 文档元数据
> - 文档版本：v1.0.0
> - 最后更新：2026-09-16
> - 更新来源：docs/dev/1-summary-greeter.md

## 1. 产品定位

- 目标用户：需要轻量、可控、GTK 技术栈桌面环境的 Linux 用户和开发者。
- 核心问题：提供一个最小但可逐步扩展的桌面环境，从登录入口到 session、设置服务和桌面壳逐步闭环。
- 核心价值：组件边界清楚，便于按 greeter、session、settings-daemon、desktop 独立演进。
- 非目标：不复刻完整 GNOME Shell；不在第一阶段提供完整控制中心、文件管理器或主题系统。

## 2. 功能边界

- 核心功能：当前已实现 LightDM greeter 的最小图形登录入口。
- 不支持功能：当前不支持电源菜单、远程登录、锁屏、主题系统和辅助功能专项适配。
- 关键对象：greeter、session、settings-daemon、desktop。
- 关键状态：登录前、认证中、认证失败、认证成功并启动 session。

## 3. 关键场景

| 场景 | 用户目标 | 成功标准 | 异常/边界 |
|------|----------|----------|-----------|
| LightDM 登录 | 用户在全屏 greeter 中选择账号，输入密码并选择 session | 认证成功后启动所选 session | 认证失败时清空密码并保留登录界面 |

## 4. 核心流程

```text
1. LightDM 启动 graceful-greeter。
2. greeter 连接 LightDM daemon 并展示 GTK4 登录窗口。
3. 用户从列表选择用户名，输入密码并选择 session。
4. greeter 通过 liblightdm-gobject 响应 PAM prompt。
5. 认证成功后，greeter 请求 LightDM 启动所选 session。
```

## 5. 产品规则

- 权限规则：登录认证交由 LightDM/PAM 处理，greeter 不自行校验密码。
- 状态流转：认证失败回到可登录状态；认证成功进入启动 session 状态。
- 异常处理：无法连接 LightDM daemon 时禁用登录输入并显示错误。
- 兼容约束：当前 greeter 面向 LightDM greeter 机制。
- 用户可见行为：第一版 UI 全屏显示，背景图可配置，登录框居中；提供用户列表、密码、session 选择、登录按钮和状态提示。

## 6. 文档索引

- 需求与任务索引：docs/dev/README.md
- 开发概览：docs/overview-product-dev.md
- 关键任务文档：
  - docs/dev/1-summary-greeter.md：LightDM greeter 第一版实现总结。

## 7. 变更记录

| 日期 | 变更 | 影响 | 关联文档 |
|------|------|------|----------|
| 2026-09-16 | 新增 LightDM greeter 产品行为 | 建立登录入口第一版 | docs/dev/1-summary-greeter.md |
