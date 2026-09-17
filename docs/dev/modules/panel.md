# Panel 模块文档

> 文档元数据
> - 文档类型：module
> - 文件路径：docs/dev/modules/panel.md
> - 文档版本：v1.0.0
> - 最后更新：2026-09-17
> - 维护范围：用户桌面 panel 进程

## 1. 模块边界

- 职责：提供 `graceful-panel`，作为登录后的单 panel 进程；第一阶段使用 GTK4/GObject 固定布局 `[菜单] [启动器...] [任务区] [网络] [音量] [电源] [时钟]`。
- 非职责：不提供多 panel，不提供动态插件系统，不读取 GSettings/配置文件，不实现 applet 持久化布局，不实现系统托盘、网络/音量/电源控制；原生 Wayland 任务缩略图需要后续 compositor/portal backend。
- 相关代码路径：`panel/`。
- 相关长期文档：`docs/overview-product.md`、`docs/overview-product-dev.md`。

## 2. 当前行为

- 用户可见行为：`graceful-panel` 启动 GTK4 应用，创建半透明磨砂观感的无边框 panel 窗口；左侧显示 logo 菜单按钮，点击后弹出开始菜单，提供搜索框、固定应用和按 desktop 分类分组的应用列表；固定应用可右键取消固定，其他应用可右键固定，应用列表项单击启动；左侧还显示终端、文件、设置启动器；中间任务区为空时不显示文案，有普通应用窗口时显示对应窗口图标按钮，鼠标悬停时显示窗口缩略图或标题/图标兜底；右侧显示 workspace 指示、按需显示的 tray 展开按钮、电源图标和两行本地时间。
- 启动器行为：终端按钮按 `graceful-terminal`、`gnome-terminal`、`terminator`、`mate-terminal` 顺序查找并启动；文件按钮按 `graceful-file`、`nautilus`、`caja` 顺序查找并启动；设置按钮按 `graceful-settings`、`gnome-control-center` 顺序查找并启动。
- 电源菜单行为：点击 Power 按钮弹出菜单，提供关机、重启、登出、锁屏；关机/重启通过 `systemctl poweroff/reboot` 执行，登出优先 `loginctl terminate-session $XDG_SESSION_ID` 并回退到 `loginctl terminate-user $USER`，锁屏按 `loginctl lock-session`、`gnome-screensaver-command -l`、`xdg-screensaver lock` 顺序尝试。
- Workspace/Tray 行为：workspace 指示按钮读取 X11/EWMH `_NET_CURRENT_DESKTOP` 和 `_NET_NUMBER_OF_DESKTOPS` 并显示 `当前/总数`；没有托盘项时隐藏 tray 展开按钮，有托盘项时显示自绘箭头按钮，点击后向上弹出透明菜单。菜单打开时箭头旋转为向下，关闭时旋转回向上。当前 tray 为 UI 和模型骨架，真实 StatusNotifier/XEmbed 协议接入在后续阶段完成。
- 系统监控行为：tray 和 Power 之间显示网速、CPU、内存三个紧凑状态项；网速读取 `/proc/net/dev` 汇总非 `lo` 网卡，上行在上、下载在下，单位自动切换 `B/KB/MB/GB/s` 并保留两位小数；CPU 读取 `/proc/stat` 计算占用，并在 `/sys/class/thermal` 或 `/sys/class/hwmon` 暴露温度时同一行显示温度；内存读取 `/proc/meminfo` 按 `MemAvailable` 计算使用百分比并单行显示。
- 开始菜单数据：通过 GIO `GAppInfo` 收集系统应用，通过 `GDesktopAppInfo` 读取 desktop `Categories` 并映射为菜单分组；`GracefulPanelAppEntry` 负责应用名称/描述/图标/启动信息、分类和搜索匹配，应用启动调用 GIO。
- 窗口协议：X11/Xwayland 下设置 `_NET_WM_WINDOW_TYPE_DOCK`、sticky/skip taskbar/skip pager/above 状态和底部 `_NET_WM_STRUT_PARTIAL`；运行期间每 500ms 按 X root 几何重新同步 panel 宽度、底部位置和保留区域。
- 任务区数据：X11/Xwayland 下读取 `_NET_CLIENT_LIST`，过滤 desktop/dock/skip-taskbar/unmapped 窗口；优先使用 `_NET_WM_ICON` 作为任务图标；hover 预览使用 X root 可见区域截图，失败时回落到标题和图标。
- 配置/接口/数据：固定应用状态保存在 `XDG_CONFIG_HOME/graceful/panel/pinned-apps.ini`，默认路径为 `~/.config/graceful/panel/pinned-apps.ini`；所有 item 为内置 GObject/GTK widget；任务窗口模型由 `GracefulPanelWindowInfo` 描述。
- 默认规则：单 panel、固定高度 42px、底部 dock top-level 窗口；时钟第一行以 `HH:MM:SS` 显示 24 小时时间，第二行以 `YYYY/MM/DD 周X` 显示日期和星期。

## 3. 关键约束

- 安全边界：应用启动仅通过 GIO desktop entry 或受控候选命令执行；Power 菜单点击具体动作后会调用系统电源/session 命令，测试和部署过程不自动触发这些动作；除固定应用配置外，不修改其他用户状态。
- 兼容性要求：构建依赖 `gtk4`、`gtk4-x11`、`x11`、`glib-2.0`、`gobject-2.0`、`gio-2.0`、`gio-unix-2.0`；主体使用 GObject 分模块，便于后续扩展真实菜单、任务列表和状态控制。
- 性能/稳定性要求：时钟、dock 协议和任务区均有定时刷新；GObject 引用、定时器、任务窗口信息、popover 父子关系和 X11 错误陷阱需要正确释放/处理。
- 禁止触碰范围：不把 desktop 背景绘制、session 编排或 greeter 认证逻辑放入 panel 模块。

## 4. 验证方式

| 场景 | 验证命令/步骤 | 备注 |
|------|---------------|------|
| 构建 | `cmake -S . -B build && cmake --build build` | 验证 GTK4/GLib/GIO API 和链接 |
| 单元测试 | `ctest --test-dir build --output-on-failure` | 覆盖启动器候选命令解析、Power action 命令映射、workspace 标签格式化、tray item 数据模型、系统监控解析/格式化、时钟模型格式化、dock strut 计算、任务窗口过滤、标题回退、开始菜单应用搜索/分类和固定应用持久化 |
| 命令行冒烟 | `build/panel/graceful-panel` | 需要图形环境 |
| 集成实测 | 登录 Graceful session 后启动 panel | 后续 session 编排接入后执行 |

## 5. 故障模式与修复记录

| 日期 | 类型 | 现象/需求 | 处理结果 | 验证 |
|------|------|-----------|----------|------|
| 2026-09-17 | task | 新增单 panel 第一阶段 | 已实现 GTK4/GObject panel app/window/layout 和内置菜单、启动器、任务区、网络、音量、电源、时钟模块 | `cmake -S . -B build && cmake --build build`、`ctest --test-dir build --output-on-failure` 通过 |
| 2026-09-17 | fix | panel 会被 desktop 遮盖，且不能跟随实际桌面尺寸变化 | X11/Xwayland 下按 EWMH dock window 协议设置窗口类型、above 状态和底部 strut，并定时同步 root 几何 | 本地构建/测试通过；测试机 `xwininfo`/`xprop` 验证 panel 在底部且 strut 覆盖全宽 |
| 2026-09-17 | task | 任务区空文案幼稚、缺少真实窗口图标和 hover 预览，panel 视觉不够通透 | 移除空任务区文案；新增 X11/Xwayland 窗口枚举、任务图标、hover 缩略图/兜底预览和半透明磨砂样式 | 本地构建/测试通过；测试机启动 Text Editor 后验证任务窗口和 hover popover |
| 2026-09-17 | fix | hover 任务图标时预览闪烁，部分窗口会覆盖 panel | 任务窗口集合不变时不重建按钮，任务按钮取消 GTK tooltip；panel realize/map 阶段立即应用 dock 协议，并通过 EWMH client message 请求 above/sticky/skip 状态 | 本地构建/测试通过；测试机 hover 5 秒仅保留一个预览窗口，panel 仍为 stacking 顶层 dock |
| 2026-09-17 | task | 菜单按钮显示 `Graceful` 文本不美观 | 菜单按钮改为加载 `data/2.png` logo；安装后读取 `/usr/local/share/graceful/panel/menu-logo.png` | 本地构建/测试通过；测试机部署 logo 并启动 panel |
| 2026-09-17 | task | 基于 Budgie Menu 的简洁理念实现开始菜单第一版 | 新增 GTK4/GObject 开始菜单 popover、应用 entry/index 模型、搜索、固定应用和应用启动 | 本地构建/测试通过；测试机点击 logo 后出现开始菜单窗口 |
| 2026-09-17 | task | 开始菜单需要支持固定/取消固定和按 desktop 分类显示 | 新增固定应用配置 `~/.config/graceful/panel/pinned-apps.ini`；开始菜单支持 pinned 右键取消固定、应用列表右键固定、分类显示和单击启动 | 本地构建/测试通过 |
| 2026-09-17 | task | panel 启动器需要真实执行程序，时钟需要显示更详细 | 启动器按 graceful/桌面默认候选命令顺序解析并启动；时钟改为两行显示 `HH:MM:SS` 和 `YYYY/MM/DD 周X` | 本地构建/测试通过 |
| 2026-09-17 | task | Power 按钮需要提供关机、重启、登出、锁屏 | 新增 Power popover 菜单和 `panel-power-action` 命令映射/执行模块 | 本地构建/测试通过；未自动触发真实电源/session 动作 |
| 2026-09-17 | task | 右侧状态区需要移除网络/声音并增加 workspace/tray | 移除 Network/Volume 按钮；新增 workspace 指示按钮和 tray 展开按钮/透明 popover/model 骨架；无托盘项时隐藏 tray 按钮，菜单开合时箭头带旋转动画 | 本地构建/测试通过 |
| 2026-09-17 | task | tray 和 Power 之间需要显示网速、CPU、内存状态 | 新增 net speed、CPU、MEM 三个 GTK/GObject 状态项和 `panel-system-monitor-model` 解析/格式化模块 | 本地构建/测试通过 |

## 6. 变更记录

| 日期 | 关联提交/文档 | 变更 | 影响 |
|------|---------------|------|------|
| 2026-09-17 | `docs/dev/README.md` | 新增 `graceful-panel` 第一版边界 | 建立传统单 panel 子系统基线 |
| 2026-09-17 | `docs/dev/README.md` | 新增 X11/Xwayland EWMH dock window 协议同步 | panel 固定在底部并声明底部保留区域 |
| 2026-09-17 | `docs/dev/README.md` | 新增 X11/Xwayland 任务区窗口图标和 hover 预览 | panel 从占位任务区推进到可显示普通应用窗口 |
