# Session 组件编排总结

> 文档元数据
> - 文件编号：3
> - 文档类型：summary
> - 文件路径：docs/dev/3-summary-session-components.md
> - 文档版本：v1.0.0
> - 完成日期：2026-09-17
> - 关联需求：session 启动时自动拉起 mutter、desktop、panel，并使用 ibus + rime 输入法
> - 关联计划：docs/dev/3-plan-session-components.md

## 1. 最终结果

- Display Manager 的 `Graceful` Wayland session 入口先启动 `mutter --wayland --display-server -- /usr/bin/graceful-session`；`graceful-session` 默认按顺序启动 `ibus-daemon --daemonize --xim`、`ibus engine rime`、安装目录下的 `graceful-desktop` 和 `graceful-panel`。
- 输入法环境固定设置 `GTK_IM_MODULE=ibus`、`QT_IM_MODULE=ibus`、`XMODIFIERS=@im=ibus`；会话类型固定设置 `XDG_SESSION_TYPE=wayland`。
- `ibus-daemon` 和 `ibus engine rime` 是可选一次性组件，失败只打印警告，避免输入法缺失阻止登录；desktop、panel 是必需后台组件。
- 核心组件启动成功后，`graceful-session` 会扫描 `$XDG_CONFIG_HOME/autostart` 和 `$XDG_CONFIG_DIRS/autostart`，按 XDG Autostart 规则拉起启动项；启动项失败只打印警告，不结束桌面 session。
- 显式 `graceful-session -- COMMAND...` 和 `GRACEFUL_SESSION_COMMAND` 仍保留单命令调试路径。
- 2026-09-17 修正：Ubuntu 26.04 的 mutter 50.1 不支持传统 X11 WM 的 `--replace`；Graceful 改为 Wayland session，由 mutter 作为最外层 compositor 启动 `graceful-session`。desktop/panel 继续使用安装 bindir 的绝对路径，避免使用历史手工部署到 `/usr/local/bin` 的旧二进制。

## 2. 关键改动

- 新增 `GracefulSessionComponent` 描述默认组件命令、必需性和一次性/后台运行模式。
- `GracefulSessionManager` 默认路径启动组件列表，并等待任一必需后台组件退出；退出时终止其它后台组件。
- `GracefulSessionProcess` 增加基于 `GTask` 的异步等待 API，保证回调 source object 仍是 `GracefulSessionProcess`。
- 新增组件列表、输入法环境、XDG Autostart 过滤和异步等待单元测试。

## 3. 验证结果

- 验证环境：本地工作区 `/data/code/graceful-desktop-environment`。
- 已执行：session 定向构建与测试；全量 `cmake --build build`、`ctest --test-dir build --output-on-failure`、`git diff --check`、命令行冒烟。
- 未执行：真实 LightDM 登录集成和 mutter 图形会话人工验证。
