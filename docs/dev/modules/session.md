# Session 模块文档

> 文档元数据
> - 文档类型：module
> - 文件路径：docs/dev/modules/session.md
> - 文档版本：v1.0.0
> - 最后更新：2026-09-17
> - 维护范围：用户 session 根进程

## 1. 模块边界

- 职责：提供 `graceful-session`，作为登录成功后的用户 session 根进程；负责读取 session 定义、设置基础环境变量、启动默认桌面组件并随核心组件退出结束 session。
- 非职责：不负责 greeter 登录认证、不提供完整 GNOME SessionManager D-Bus API、不管理 systemd user targets、不处理 XDG autostart、不实现电源/锁屏/session restore。
- 相关代码路径：`session/`。
- 相关长期文档：`docs/dev/2-research-session.md`、`docs/dev/2-plan-session.md`、`docs/dev/2-summary-session.md`。

## 2. 当前行为

- 用户可见行为：Display Manager 中可出现 `Graceful` session；默认启动 `mutter --replace`、`ibus-daemon --daemonize --xim`、`ibus engine rime`、`graceful-desktop` 和 `graceful-panel`；`graceful-session -- [COMMAND...]` 会保留单命令调试模式并等待 `COMMAND` 退出。
- 配置/接口/数据：支持 `--session=SESSION` 选择 session id；读取 `$XDG_CONFIG_HOME/graceful-session/sessions`、`$XDG_CONFIG_DIRS/graceful-session/sessions`、`$XDG_DATA_DIRS/graceful-session/sessions` 中的 `SESSION.session` 文件；内置 `graceful` session。
- 默认规则：未提供命令时优先使用 `GRACEFUL_SESSION_COMMAND`，否则运行内置桌面组件列表；设置 `DESKTOP_SESSION`、`GDMSESSION`、`XDG_CURRENT_DESKTOP`、`GTK_IM_MODULE=ibus`、`QT_IM_MODULE=ibus` 和 `XMODIFIERS=@im=ibus`。

## 3. 关键约束

- 安全边界：不持久化用户数据，不执行系统级操作，不修改 LightDM 配置。
- 兼容性要求：构建依赖 `glib-2.0`、`gobject-2.0`、`gio-2.0`；安装 `graceful.desktop` 到 `${datadir}/xsessions`。
- 性能/稳定性要求：子进程启动失败和异常退出需返回非零状态；GObject 引用、字符串数组和 `GSubprocess` 需要正确释放。
- 禁止触碰范围：greeter 私有认证/UI 逻辑不放入 session 模块。

## 4. 验证方式

| 场景 | 验证命令/步骤 | 备注 |
|------|---------------|------|
| 构建 | `cmake -S . -B build && cmake --build build` | 验证 GLib/GIO API 和链接 |
| 单元测试 | `ctest --test-dir build --output-on-failure` | 覆盖 session 定义、环境变量、默认组件列表、子进程退出状态和异步等待 |
| 命令行冒烟 | `build/session/graceful-session -- /bin/sh -c 'exit 0'` | 可验证 CLI 启动闭环 |
| 集成实测 | 安装后在 Display Manager 选择 Graceful session 登录 | 后续有 desktop 核心进程后执行 |

## 5. 故障模式与修复记录

| 日期 | 类型 | 现象/需求 | 处理结果 | 验证 |
|------|------|-----------|----------|------|
| 2026-09-17 | task | 新增 session 进程最小闭环 | 已实现纯 C/GObject 模块化 session definition/environment/process/manager 和 xsessions 入口 | `cmake -S . -B build && cmake --build build`、`ctest --test-dir build --output-on-failure`、`git diff --check`、CLI 冒烟通过 |
| 2026-09-17 | task | session 默认需要拉起窗口管理器、桌面、panel 和 ibus+rime | 新增 session component 模型；默认编排 mutter、ibus-daemon、ibus rime、graceful-desktop、graceful-panel；输入法环境改为 ibus | 全量构建、19 个测试、CLI 冒烟和 `git diff --check` 通过 |

## 6. 变更记录

| 日期 | 关联提交/文档 | 变更 | 影响 |
|------|---------------|------|------|
| 2026-09-17 | `docs/dev/2-plan-session.md` | 新增 `graceful-session` 第一版边界 | 建立登录后的 session 根进程基线 |
| 2026-09-17 | `docs/dev/3-plan-session-components.md` | session 默认组件编排 | 将 Graceful session 升级为最小桌面环境启动链 |
