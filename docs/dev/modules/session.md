# Session 模块文档

> 文档元数据
> - 文档类型：module
> - 文件路径：docs/dev/modules/session.md
> - 文档版本：v1.0.0
> - 最后更新：2026-09-17
> - 维护范围：用户 session 根进程

## 1. 模块边界

- 职责：提供 `graceful-session-launcher` 和 `graceful-session`；launcher 作为 Display Manager 等待的 Wayland session 入口负责启动/终止外层 mutter，`graceful-session` 作为 mutter 启动后的用户 session 根进程负责读取 session 定义、设置基础环境变量、启动默认桌面组件、拉起 XDG Autostart 启动项，并随核心组件退出结束 session。
- 非职责：不负责 greeter 登录认证、不提供完整 GNOME SessionManager D-Bus API、不管理 systemd user targets、不实现电源/锁屏/session restore。
- 相关代码路径：`session/`。
- 相关长期文档：`docs/dev/2-research-session.md`、`docs/dev/2-plan-session.md`、`docs/dev/2-summary-session.md`。

## 2. 当前行为

- 用户可见行为：Display Manager 中可出现 `Graceful` Wayland session；session 入口执行 `graceful-session-launcher`，launcher 启动 `mutter --wayland --display-server -- /usr/bin/graceful-session`，并在收到 SIGTERM/SIGINT/SIGHUP 时先 TERM、后 KILL 外层 mutter；`graceful-session` 默认启动 `ibus-daemon --daemonize --xim`、`ibus engine rime`、安装目录下的 `graceful-desktop` 和 `graceful-panel`；核心组件启动成功后扫描 XDG Autostart 并拉起启动项；默认桌面模式正常退出或收到 SIGTERM/SIGINT 时也会确认式结束父级 mutter，使 LightDM 能回到 greeter；`graceful-session -- [COMMAND...]` 会保留单命令调试模式并等待 `COMMAND` 退出。
- 配置/接口/数据：支持 `--session=SESSION` 选择 session id；读取 `$XDG_CONFIG_HOME/graceful-session/sessions`、`$XDG_CONFIG_DIRS/graceful-session/sessions`、`$XDG_DATA_DIRS/graceful-session/sessions` 中的 `SESSION.session` 文件；内置 `graceful` session。
- 默认规则：未提供命令时优先使用 `GRACEFUL_SESSION_COMMAND`，否则运行内置桌面组件列表；设置 `DESKTOP_SESSION`、`GDMSESSION`、`XDG_CURRENT_DESKTOP`、`XDG_SESSION_TYPE=wayland`、`GDK_BACKEND=x11`、`NO_AT_BRIDGE=1`、`GTK_IM_MODULE=ibus`、`QT_IM_MODULE=ibus` 和 `XMODIFIERS=@im=ibus`；移除 LightDM Xsession wrapper 注入的 `GTK_MODULES`，并以精确环境启动 session 子进程，避免被父进程环境重新继承后反复激活旧 AT-SPI bridge；XDG Autostart 读取 `$XDG_CONFIG_HOME/autostart` 和 `$XDG_CONFIG_DIRS/autostart`，用户项覆盖系统同名 desktop id。

## 3. 关键约束

- 安全边界：不持久化用户数据，不执行系统级操作，不修改 LightDM 配置。
- 兼容性要求：构建依赖 `glib-2.0`、`gobject-2.0`、`gio-2.0`；安装 `graceful-session-launcher`、`graceful-session` 到 `${bindir}`，安装 `graceful.desktop` 到 `${datadir}/wayland-sessions`。
- 性能/稳定性要求：子进程启动失败和异常退出需返回非零状态；GObject 引用、字符串数组和 `GSubprocess` 需要正确释放。
- 禁止触碰范围：greeter 私有认证/UI 逻辑不放入 session 模块。

## 4. 验证方式

| 场景 | 验证命令/步骤 | 备注 |
|------|---------------|------|
| 构建 | `cmake -S . -B build && cmake --build build` | 验证 GLib/GIO API 和链接 |
| 单元测试 | `ctest --test-dir build --output-on-failure` | 覆盖 session 定义、环境变量、默认组件列表、子进程退出状态、异步等待、父 mutter 匹配判断和 launcher mutter 命令构造 |
| Autostart 定向测试 | `build/session/session-autostart-test` | 覆盖 Hidden、OnlyShowIn、NotShowIn、TryExec、用户覆盖系统项和 Exec 字段码处理 |
| 命令行冒烟 | `build/session/graceful-session -- /bin/sh -c 'exit 0'` | 可验证 CLI 启动闭环 |
| 集成实测 | 安装后在 Display Manager 选择 Graceful session 登录 | 后续有 desktop 核心进程后执行 |

## 5. 故障模式与修复记录

| 日期 | 类型 | 现象/需求 | 处理结果 | 验证 |
|------|------|-----------|----------|------|
| 2026-09-17 | task | 新增 session 进程最小闭环 | 已实现纯 C/GObject 模块化 session definition/environment/process/manager 和 Display Manager session 入口 | `cmake -S . -B build && cmake --build build`、`ctest --test-dir build --output-on-failure`、`git diff --check`、CLI 冒烟通过 |
| 2026-09-17 | task | session 默认需要拉起窗口管理器、桌面、panel 和 ibus+rime | 新增 session component 模型；LightDM Wayland session 入口由 mutter 启动，`graceful-session` 默认编排 ibus-daemon、ibus rime、graceful-desktop、graceful-panel；输入法环境改为 ibus | 全量构建、20 个测试、CLI 冒烟和 `git diff --check` 通过 |
| 2026-09-17 | fix | 登录 Graceful 后立即回到 greeter，不能进入桌面 | 当前 Ubuntu 的 mutter 50.1 不支持 `--replace`，且不应作为 Xsession 子组件启动；移除该参数并将 mutter 放到 Wayland session 入口外层，同时让 desktop/panel 使用安装 bindir 的绝对路径 | 本地构建、20 个测试、CLI 冒烟、`git diff --check` 通过；远端 deb 重装完成，确认无旧 Xsession 入口、无 `metacity`/`--replace` |
| 2026-09-17 | fix | 移除 `--replace` 后仍无法进入桌面 | 当前 mutter 50.1 应作为会话外层 Wayland compositor 启动；将 Graceful session 改为 LightDM Wayland session，入口执行 `mutter --wayland --display-server -- /usr/bin/graceful-session`，`graceful-session` 不再把 mutter 当子组件启动 | 本地构建、20 个测试、CLI 冒烟、deb 构建、远端重装和入口文件核对通过；真实登录待人工验证 |
| 2026-09-17 | task | session 需要拉起 `/etc/xdg/autostart` 等 XDG 启动项 | 新增 `GracefulSessionAutostart`，核心组件启动成功后扫描 XDG Autostart；支持 Hidden、OnlyShowIn、NotShowIn、TryExec、`X-GNOME-Autostart-enabled=false`、用户同名覆盖系统项和基础 Exec 字段码处理 | 全量构建、20 个测试、CLI 冒烟、deb 构建和远端重装通过 |
| 2026-09-17 | fix | Wayland session 中 desktop/panel 启动但桌面层和 panel dock 定位不生效 | 保持 mutter 作为外层 Wayland compositor，session 子进程环境增加 `GDK_BACKEND=x11`，让现有 GTK4 desktop/panel 通过 Xwayland 使用已实现的 X11/EWMH 定位协议 | session 定向测试通过，等待全量和远端验证 |
| 2026-09-17 | fix | 进入桌面慢，desktop/panel 约 20 秒后才完成 GTK application 注册 | LightDM wrapper 注入 `GTK_MODULES=gail:atk-bridge`，GTK 子进程反复激活 AT-SPI 且出现授权失败；session 子进程环境移除 `GTK_MODULES`、设置 `NO_AT_BRIDGE=1`，并用精确环境启动子进程，避免父进程变量重新继承 | `session-environment-test`、`session-process-test`、CLI 环境冒烟、全量构建和 21 个测试通过；远端验证见安装记录 |
| 2026-09-17 | fix | 点击登出后卡住，greeter 不回来 | LightDM 等待的进程是外层 mutter；`loginctl terminate-session` 会先给 `graceful-session` 发送 SIGTERM，导致 main 正常返回后的收尾逻辑可能来不及执行。默认桌面模式新增 SIGTERM/SIGINT 处理，并复用父 compositor 匹配与确认式退出收尾：先向父级 mutter 发送 SIGTERM，短等后如果同一 mutter 仍存活则 SIGKILL；显式命令调试模式不触发 | 本地构建、22 个测试、deb 构建、`git diff --check`、远端重装和入口/二进制核对通过；远端当前已回到 greeter 且无 mutter/graceful-session 残留 |
| 2026-09-17 | fix | 注销或重启 LightDM 时仍可能只剩外层 mutter 卡住 | 新增 `graceful-session-launcher` 作为 LightDM 等待的 session 入口，由 launcher 启动 mutter 并在收到 SIGTERM/SIGINT/SIGHUP 时负责 TERM/KILL mutter；Wayland session desktop entry 改为执行 launcher | `session-launcher-test`、全量 23 个测试和 `git diff --check` 通过 |

## 6. 变更记录

| 日期 | 关联提交/文档 | 变更 | 影响 |
|------|---------------|------|------|
| 2026-09-17 | `docs/dev/2-plan-session.md` | 新增 `graceful-session` 第一版边界 | 建立登录后的 session 根进程基线 |
| 2026-09-17 | `docs/dev/3-plan-session-components.md` | session 默认组件编排 | 将 Graceful session 升级为最小桌面环境启动链 |
