# Graceful Desktop Environment 开发概览

> 文档元数据
> - 文档版本：v1.0.0
> - 最后更新：2026-09-17
> - 更新来源：docs/dev/modules/session.md
> - 关联产品文档：docs/overview-product.md

## 1. 技术栈

| 类别 | 技术/版本 | 用途 | 备注 |
|------|-----------|------|------|
| 语言 | C | 项目实现语言 | 可使用 GObject 组织模块 |
| 构建系统 | CMake 3.20+ | 构建 greeter/session/desktop/panel/common | 使用 pkg-config 发现依赖 |
| 关键依赖 | GTK4 | greeter 图形界面、desktop 背景窗口和 panel 窗口 | 本地验证版本 4.22.5 |
| 关键依赖 | gtk4-x11 / Xlib | LightDM X11 greeter、desktop 和 panel 精确匹配显示几何 | X11/Xwayland 下用于强制设置窗口角色、层级、strut 和几何 |
| 关键依赖 | liblightdm-gobject-1 | LightDM greeter API | 本地验证版本 1.32.0 |
| 关键依赖 | GLib/GObject | 对象模型和单元测试 | greeter 内部模型使用 |
| 关键依赖 | GIO/GSubprocess | session 子进程生命周期 | graceful-session 使用 |

## 2. 架构边界

- 模块划分：`greeter/` 放 LightDM greeter 私有 UI、认证和模型；`session/` 放登录后的 session 根进程；`desktop/` 放桌面壳背景进程；`panel/` 放桌面 panel 进程；`common/` 只放 greeter/session/settings-daemon/desktop/panel 共享的纯 C 公共库。
- 进程/线程/内核边界：`graceful-greeter` 是由 LightDM 启动的独立 greeter 进程；Graceful session 入口由 LightDM 启动 `graceful-session-launcher`；launcher 启动并托管 `mutter --wayland --display-server -- /usr/bin/graceful-session`，收到退出信号时负责 TERM/KILL mutter；`graceful-session` 是 mutter 启动的用户 session 根进程；默认 session 组件包含 `ibus-daemon --daemonize --xim`、`ibus engine rime`、`graceful-desktop`、`graceful-panel` 和 XDG Autostart 启动项。
- 客户端/服务端/驱动边界：greeter 通过 `liblightdm-gobject-1` 与 LightDM daemon 交互。
- 数据流：LightDM 用户列表/GTK 密码输入控件 -> greeter 登录模型 -> LightDM PAM prompt response；session 定义/环境 -> session manager -> GSubprocess；XDG autostart 目录 -> autostart desktop entry 过滤 -> GSubprocess；壁纸目录 -> wallpaper store -> wallpaper view -> GTK snapshot；panel 内置 item -> panel layout -> panel window；X11 `_NET_CLIENT_LIST`/窗口属性 -> panel window list -> task area 图标按钮和 hover 预览。
- 控制流：GTK application activate -> LightDM daemon connect -> 用户触发 authenticate -> prompt response -> authentication-complete -> start session -> mutter 作为 Wayland compositor 启动 -> `graceful-session` 设置环境并启动默认组件 -> desktop 绘制背景、panel 提供桌面入口 -> 任一必需后台组件退出后 session 清理其它组件并结束。
- 外部依赖：LightDM、PAM、系统 session desktop 文件。

## 3. 关键接口

| 接口/协议/ABI | 调用方 | 提供方 | 兼容约束 | 说明 |
|---------------|--------|--------|----------|------|
| liblightdm-gobject-1 | greeter | LightDM | 按系统 LightDM 开发包 ABI | 认证、prompt、session 启动 |
| xgreeters desktop entry | LightDM | greeter 包 | 安装到 `${datadir}/xgreeters` | 让 LightDM 发现 `graceful-greeter` |
| wayland-sessions desktop entry | Display Manager | session 包 | 安装到 `${datadir}/wayland-sessions` | 让 LightDM 等登录管理器发现 `Graceful` Wayland session |
| graceful session file | graceful-session | 本地配置/数据目录 | `.session` 文件使用 `[GNOME Session]` 组 | 第一版读取 `Name` 和 `Kiosk` |
| XDG Autostart desktop entry | graceful-session | `$XDG_CONFIG_HOME/autostart`、`$XDG_CONFIG_DIRS/autostart` | 用户同名 entry 覆盖系统 entry；遵守 Hidden、OnlyShowIn、NotShowIn、TryExec 和 GNOME autostart enabled 字段 | 拉起输入法、托盘、agent 等会话启动项 |
| desktop environment variables | graceful-desktop | 用户环境 | 环境变量为空或非法时回落默认值 | `GRACEFUL_DESKTOP_WALLPAPER_DIR`、`GRACEFUL_DESKTOP_WALLPAPER_INTERVAL` |
| panel built-in layout | graceful-panel | 代码内置 | 第一版无运行时配置 | 固定单 panel 与内置 item 顺序 |
| StatusNotifierWatcher | graceful-panel | D-Bus session bus | 提供 `org.kde.StatusNotifierWatcher` bus name 和 `/StatusNotifierWatcher` 对象 | AppIndicator/StatusNotifier 进程向 panel 注册托盘项 |
| EWMH desktop/dock window | graceful-desktop / graceful-panel | X11/Xwayland 窗口管理器 | 每 500ms 按 X root 几何重新同步 | desktop 使用 desktop window 类型；panel 使用 dock window 类型和底部 strut |
| EWMH task list | graceful-panel | X11/Xwayland 窗口管理器和客户端窗口 | 过滤 desktop/dock/skip-taskbar/unmapped 窗口 | panel 任务区显示普通应用窗口图标，hover 时用可见区域截图生成缩略图 |

## 4. 数据与配置

- 核心数据结构：`GracefulGreeterLoginModel` 保存用户名、密码和 session key；`GracefulSessionDefinition`、`GracefulSessionEnvironment`、`GracefulSessionComponent`、`GracefulSessionAutostart`、`GracefulSessionProcess`、`GracefulSessionManager` 分别管理 session 定义、环境变量、默认组件、XDG Autostart、子进程和编排；`GracefulDesktopConfig`、`GracefulWallpaperStore`、`GracefulWallpaperTransition`、`GracefulWallpaperView`、`GracefulDesktopApp` 分别管理桌面配置、壁纸目录、过渡进度、绘制和 GTK 应用主体；`GracefulPanelApp`、`GracefulPanelWindow`、`GracefulPanelLayout`、`GracefulPanelWindowInfo`、`GracefulPanelWindowList` 和各内置 item 管理 panel 应用、窗口、布局、任务窗口模型/读取和显示组件。
- 配置文件/参数：`greeter/graceful-greeter.desktop` 描述 LightDM greeter 入口；`/etc/lightdm/graceful-greeter.conf` 可配置 `[Greeter] Background=/path/to/image`；`session/graceful.desktop` 描述 Display Manager Wayland session 入口并执行 `graceful-session-launcher`；`graceful-session --session=SESSION -- [COMMAND...]` 可指定 session 和单命令调试入口；`GRACEFUL_SESSION_COMMAND` 可覆盖默认组件编排；session 子进程默认设置 `GDK_BACKEND=x11`，使 desktop/panel 在 mutter Wayland session 中通过 Xwayland 使用现有 X11/EWMH 定位协议；session 子进程同时移除 `GTK_MODULES` 并设置 `NO_AT_BRIDGE=1`，避免旧 AT-SPI bridge 拖慢 GTK 启动；`GRACEFUL_DESKTOP_WALLPAPER_DIR` 和 `GRACEFUL_DESKTOP_WALLPAPER_INTERVAL` 控制桌面壁纸目录和切换间隔。
- 持久化数据：无。
- 迁移/兼容规则：无历史数据迁移。
- 敏感信息处理：密码不写日志、不持久化；认证失败和 reset 时清空模型密码。

## 5. 高风险区域

| 风险区域 | 关注点 | 验证方式 | 关联文档 |
|----------|--------|----------|----------|
| 内存/生命周期 | GObject 引用、GPtrArray 字符串释放、GTK widget 生命周期 | 构建、单元测试、代码审查 | docs/dev/1-summary-greeter.md |
| ABI/API/协议 | LightDM greeter API、xgreeters desktop entry | CMake 构建链接、安装规则审查 | docs/dev/1-summary-greeter.md |
| 权限/系统调用 | 登录认证由 LightDM/PAM 承担 | 当前未执行真实登录；后续测试机集成验证 | docs/dev/1-summary-greeter.md |
| 显示几何 | 裸 X 下无窗口管理器，GTK fullscreen 不保证覆盖 root window | X11 root/window 几何检查 | docs/dev/modules/greeter.md |
| session 生命周期 | 必需组件启动失败或异常退出会导致登录后退出；输入法可选组件失败只警告 | session 单元测试、命令行冒烟、后续 LightDM 集成验证 | docs/dev/modules/session.md |
| XDG Autostart | autostart entry 过滤不完整可能误启动或漏启动系统托盘/agent；启动项异常不能拖垮 session | session-autostart 单元测试、真实登录日志检查 | docs/dev/modules/session.md |
| desktop 背景窗口 | GTK 全屏窗口在不同 WM/Wayland 组合下的层级和覆盖行为 | desktop 单元测试、构建、图形环境冒烟 | docs/dev/modules/desktop.md |
| panel 窗口层级 | GTK 普通窗口在不同 WM/Wayland 组合下不一定具备 dock/layer 行为 | panel 单元测试、构建、图形环境冒烟 | docs/dev/modules/panel.md |
| panel 任务预览 | X11 窗口截图可能因窗口状态触发异步错误；原生 Wayland 不允许普通客户端直接抓取其他窗口 | X11 error trap、任务模型单元测试、测试机 hover 冒烟 | docs/dev/modules/panel.md |
| panel 托盘协议 | StatusNotifier/AppIndicator 依赖 D-Bus watcher；XEmbed 老托盘协议尚未接入 | watcher 地址解析和 tray model 单元测试、测试机 D-Bus name 检查 | docs/dev/modules/panel.md |

## 6. 构建与验证

- 构建命令：`cmake -S . -B build && cmake --build build`。
- 单元测试：`ctest --test-dir build --output-on-failure`。
- 集成验证：在测试机 LightDM 配置中启用 graceful greeter 后执行真实登录。
- 静态检查：当前未配置专用静态检查。
- 高风险验证：登录链路需在真实 LightDM/PAM 环境补充集成验证；desktop/panel 几何需用 `xwininfo`、`xprop` 或后续 Wayland 后端工具验证窗口角色、尺寸和保留区域。
- 最小人工验证步骤：安装 greeter 后配置 LightDM greeter-session，重启 LightDM 或进入测试显示环境，验证成功/失败登录。
- Session 命令行冒烟：`build/session/graceful-session -- /bin/sh -c 'exit 0'`。
- Desktop 图形冒烟：`GRACEFUL_DESKTOP_WALLPAPER_DIR=/path/to/images build/desktop/graceful-desktop`。
- Panel 图形冒烟：`build/panel/graceful-panel`。

## 7. 发布与回滚

- 产物：`graceful-greeter`、`graceful-session-launcher`、`graceful-session`、`graceful-desktop`、`graceful-panel` 可执行文件，`graceful-greeter.desktop` 和 `graceful.desktop`。
- 安装/部署方式：CMake install 将二进制安装到 `${bindir}`，xgreeters 文件安装到 `${datadir}/xgreeters`，Wayland session 文件安装到 `${datadir}/wayland-sessions`；`make deb` 生成 Debian 包，包含 LightDM 默认 Graceful 配置。
- 配置变更：Debian 包安装 `/etc/lightdm/lightdm.conf.d/50-graceful.conf`，设置 `greeter-session=graceful-greeter` 和 `user-session=graceful`。
- 升级步骤：替换 greeter/session/desktop 二进制和 desktop entry。
- 回滚步骤：恢复旧 greeter 包或切换 LightDM greeter-session。
- 止损条件：无法登录时从 TTY 切换 LightDM greeter-session 或回滚包。

## 8. 观测与排障

- 关键日志：LightDM 日志和 greeter 标准错误输出。
- 指标/告警：当前无。
- 常见故障：无法连接 LightDM daemon、认证失败、session key 无效、session 启动失败、壁纸目录不存在/为空或图片加载失败、panel 未处于预期屏幕边缘。
- 排障入口：`docs/dev/modules/greeter.md`、`docs/dev/modules/session.md`、`docs/dev/modules/desktop.md`、`docs/dev/modules/panel.md`。

## 9. 文档索引

- 需求与任务索引：docs/dev/README.md
- 产品概览：docs/overview-product.md
- 按需片段模板：.dj-agent/fragments/
- 关键任务文档：
  - docs/dev/1-summary-greeter.md：LightDM greeter 第一版实现总结。
  - docs/dev/2-summary-session.md：Session 根进程第一版实现总结。
  - docs/dev/3-summary-session-components.md：Session 默认桌面组件编排总结。
  - docs/dev/4-summary-debian-package.md：Debian 打包和测试机安装总结。
  - docs/dev/modules/desktop.md：Desktop 背景进程模块上下文。
  - docs/dev/modules/panel.md：Panel 进程模块上下文。

## 10. 变更记录

| 日期 | 变更 | 影响 | 关联文档 |
|------|------|------|----------|
| 2026-09-16 | 新增 GTK4 + LightDM greeter 架构与验证入口 | 建立 greeter 子系统开发基线 | docs/dev/1-summary-greeter.md |
| 2026-09-17 | 新增纯 C/GObject session 根进程架构与验证入口 | 建立 session 子系统开发基线 | docs/dev/2-summary-session.md |
| 2026-09-17 | 新增 GTK4/GObject desktop 背景进程架构与验证入口 | 建立 desktop 子系统开发基线 | docs/dev/modules/desktop.md |
| 2026-09-17 | 新增 GTK4/GObject panel 架构与验证入口 | 建立 panel 子系统开发基线 | docs/dev/modules/panel.md |
| 2026-09-17 | 新增 desktop/panel X11/Xwayland EWMH 窗口协议同步 | desktop 作为桌面窗口铺满 root；panel 作为底部 dock 声明 strut 并跟随 root 几何变化 | docs/dev/modules/desktop.md, docs/dev/modules/panel.md |
| 2026-09-17 | 新增 panel X11/Xwayland 任务窗口模型、图标和 hover 预览 | panel 任务区可显示普通应用窗口，空任务区不再显示占位文案 | docs/dev/modules/panel.md |
| 2026-09-17 | Graceful session 通过 mutter Wayland compositor 拉起 ibus/rime、desktop 和 panel | Graceful session 具备最小 Wayland 桌面环境启动链 | docs/dev/3-summary-session-components.md |
| 2026-09-17 | session 支持 XDG Autostart | 登录后自动拉起 `/etc/xdg/autostart` 和用户 autostart 中匹配 Graceful 的启动项 | docs/dev/modules/session.md |
| 2026-09-17 | session 子进程默认使用 `GDK_BACKEND=x11` | desktop/panel 在 mutter Wayland session 中通过 Xwayland 复用 X11/EWMH 桌面层和 dock 定位协议 | docs/dev/modules/session.md |
| 2026-09-17 | session 子进程禁用旧 AT-SPI bridge | 避免 LightDM Xsession wrapper 注入的 `GTK_MODULES=gail:atk-bridge` 导致 GTK 应用注册和显示延迟 | docs/dev/modules/session.md |
| 2026-09-17 | panel 实现 StatusNotifierWatcher | AppIndicator/StatusNotifier 托盘程序可向 panel 注册并在 tray 菜单中显示 | docs/dev/modules/panel.md |
| 2026-09-17 | 新增 Debian 包构建和 LightDM 默认配置安装 | 可通过 `make deb` 产出 deb，并在测试机默认启用 Graceful greeter/session | docs/dev/4-summary-debian-package.md |
