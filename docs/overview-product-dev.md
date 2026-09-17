# Graceful Desktop Environment 开发概览

> 文档元数据
> - 文档版本：v1.0.0
> - 最后更新：2026-09-17
> - 更新来源：docs/dev/modules/desktop.md
> - 关联产品文档：docs/overview-product.md

## 1. 技术栈

| 类别 | 技术/版本 | 用途 | 备注 |
|------|-----------|------|------|
| 语言 | C | 项目实现语言 | 可使用 GObject 组织模块 |
| 构建系统 | CMake 3.20+ | 构建 greeter/session/desktop/common | 使用 pkg-config 发现依赖 |
| 关键依赖 | GTK4 | greeter 图形界面和 desktop 背景窗口 | 本地验证版本 4.22.5 |
| 关键依赖 | gtk4-x11 / Xlib | LightDM X11 greeter 精确匹配显示器尺寸 | 裸 X 环境下用于强制设置 greeter 窗口几何 |
| 关键依赖 | liblightdm-gobject-1 | LightDM greeter API | 本地验证版本 1.32.0 |
| 关键依赖 | GLib/GObject | 对象模型和单元测试 | greeter 内部模型使用 |
| 关键依赖 | GIO/GSubprocess | session 子进程生命周期 | graceful-session 使用 |

## 2. 架构边界

- 模块划分：`greeter/` 放 LightDM greeter 私有 UI、认证和模型；`session/` 放登录后的 session 根进程；`desktop/` 放桌面壳进程；`common/` 只放 greeter/session/settings-daemon/desktop 共享的纯 C 公共库。
- 进程/线程/内核边界：`graceful-greeter` 是由 LightDM 启动的独立 greeter 进程；`graceful-session` 是由登录管理器启动的用户 session 根进程；`graceful-desktop` 是 session 默认核心命令。
- 客户端/服务端/驱动边界：greeter 通过 `liblightdm-gobject-1` 与 LightDM daemon 交互。
- 数据流：LightDM 用户列表/GTK 密码输入控件 -> greeter 登录模型 -> LightDM PAM prompt response；session 定义/环境 -> session manager -> GSubprocess；壁纸目录 -> wallpaper store -> wallpaper view -> GTK snapshot。
- 控制流：GTK application activate -> LightDM daemon connect -> 用户触发 authenticate -> prompt response -> authentication-complete -> start session -> `graceful-session` 启动核心命令 -> `graceful-desktop` 绘制背景 -> 核心命令退出后 session 结束。
- 外部依赖：LightDM、PAM、系统 session desktop 文件。

## 3. 关键接口

| 接口/协议/ABI | 调用方 | 提供方 | 兼容约束 | 说明 |
|---------------|--------|--------|----------|------|
| liblightdm-gobject-1 | greeter | LightDM | 按系统 LightDM 开发包 ABI | 认证、prompt、session 启动 |
| xgreeters desktop entry | LightDM | greeter 包 | 安装到 `${datadir}/xgreeters` | 让 LightDM 发现 `graceful-greeter` |
| xsessions desktop entry | Display Manager | session 包 | 安装到 `${datadir}/xsessions` | 让 LightDM 等登录管理器发现 `Graceful` session |
| graceful session file | graceful-session | 本地配置/数据目录 | `.session` 文件使用 `[GNOME Session]` 组 | 第一版读取 `Name` 和 `Kiosk` |
| desktop environment variables | graceful-desktop | 用户环境 | 环境变量为空或非法时回落默认值 | `GRACEFUL_DESKTOP_WALLPAPER_DIR`、`GRACEFUL_DESKTOP_WALLPAPER_INTERVAL` |

## 4. 数据与配置

- 核心数据结构：`GracefulGreeterLoginModel` 保存用户名、密码和 session key；`GracefulSessionDefinition`、`GracefulSessionEnvironment`、`GracefulSessionProcess`、`GracefulSessionManager` 分别管理 session 定义、环境变量、子进程和编排；`GracefulDesktopConfig`、`GracefulWallpaperStore`、`GracefulWallpaperTransition`、`GracefulWallpaperView`、`GracefulDesktopApp` 分别管理桌面配置、壁纸目录、过渡进度、绘制和 GTK 应用主体。
- 配置文件/参数：`greeter/graceful-greeter.desktop` 描述 LightDM greeter 入口；`/etc/lightdm/graceful-greeter.conf` 可配置 `[Greeter] Background=/path/to/image`；`session/graceful.desktop` 描述 Display Manager session 入口；`graceful-session --session=SESSION -- [COMMAND...]` 可指定 session 和核心命令；`GRACEFUL_DESKTOP_WALLPAPER_DIR` 和 `GRACEFUL_DESKTOP_WALLPAPER_INTERVAL` 控制桌面壁纸目录和切换间隔。
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
| session 生命周期 | 核心命令启动失败、异常退出、环境变量错误会导致登录后立即退出 | session 单元测试、命令行冒烟、后续 LightDM 集成验证 | docs/dev/modules/session.md |
| desktop 背景窗口 | GTK 全屏窗口在不同 WM/Wayland 组合下的层级和覆盖行为 | desktop 单元测试、构建、图形环境冒烟 | docs/dev/modules/desktop.md |

## 6. 构建与验证

- 构建命令：`cmake -S . -B build && cmake --build build`。
- 单元测试：`ctest --test-dir build --output-on-failure`。
- 集成验证：在测试机 LightDM 配置中启用 graceful greeter 后执行真实登录。
- 静态检查：当前未配置专用静态检查。
- 高风险验证：登录链路需在真实 LightDM/PAM 环境补充集成验证。
- 最小人工验证步骤：安装 greeter 后配置 LightDM greeter-session，重启 LightDM 或进入测试显示环境，验证成功/失败登录。
- Session 命令行冒烟：`build/session/graceful-session -- /bin/sh -c 'exit 0'`。
- Desktop 图形冒烟：`GRACEFUL_DESKTOP_WALLPAPER_DIR=/path/to/images build/desktop/graceful-desktop`。

## 7. 发布与回滚

- 产物：`graceful-greeter`、`graceful-session`、`graceful-desktop` 可执行文件，`graceful-greeter.desktop` 和 `graceful.desktop`。
- 安装/部署方式：CMake install 将二进制安装到 `${bindir}`，xgreeters 文件安装到 `${datadir}/xgreeters`，xsessions 文件安装到 `${datadir}/xsessions`。
- 配置变更：系统 LightDM 配置不由当前构建自动修改。
- 升级步骤：替换 greeter/session/desktop 二进制和 desktop entry。
- 回滚步骤：恢复旧 greeter 包或切换 LightDM greeter-session。
- 止损条件：无法登录时从 TTY 切换 LightDM greeter-session 或回滚包。

## 8. 观测与排障

- 关键日志：LightDM 日志和 greeter 标准错误输出。
- 指标/告警：当前无。
- 常见故障：无法连接 LightDM daemon、认证失败、session key 无效、session 启动失败、壁纸目录不存在/为空或图片加载失败。
- 排障入口：`docs/dev/modules/greeter.md`、`docs/dev/modules/session.md`、`docs/dev/modules/desktop.md`。

## 9. 文档索引

- 需求与任务索引：docs/dev/README.md
- 产品概览：docs/overview-product.md
- 按需片段模板：.dj-agent/fragments/
- 关键任务文档：
  - docs/dev/1-summary-greeter.md：LightDM greeter 第一版实现总结。
  - docs/dev/2-summary-session.md：Session 根进程第一版实现总结。
  - docs/dev/modules/desktop.md：Desktop 背景进程模块上下文。

## 10. 变更记录

| 日期 | 变更 | 影响 | 关联文档 |
|------|------|------|----------|
| 2026-09-16 | 新增 GTK4 + LightDM greeter 架构与验证入口 | 建立 greeter 子系统开发基线 | docs/dev/1-summary-greeter.md |
| 2026-09-17 | 新增纯 C/GObject session 根进程架构与验证入口 | 建立 session 子系统开发基线 | docs/dev/2-summary-session.md |
| 2026-09-17 | 新增 GTK4/GObject desktop 背景进程架构与验证入口 | 建立 desktop 子系统开发基线 | docs/dev/modules/desktop.md |
