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

- 用户可见行为：`graceful-panel` 启动 GTK4 应用，创建半透明磨砂观感的无边框 panel 窗口；左侧显示 `Graceful` 菜单按钮和终端/文件/设置启动器占位；中间任务区为空时不显示文案，有普通应用窗口时显示对应窗口图标按钮，鼠标悬停时显示窗口缩略图或标题/图标兜底；右侧显示网络、音量、电源图标和本地时间。
- 窗口协议：X11/Xwayland 下设置 `_NET_WM_WINDOW_TYPE_DOCK`、sticky/skip taskbar/skip pager/above 状态和底部 `_NET_WM_STRUT_PARTIAL`；运行期间每 500ms 按 X root 几何重新同步 panel 宽度、底部位置和保留区域。
- 任务区数据：X11/Xwayland 下读取 `_NET_CLIENT_LIST`，过滤 desktop/dock/skip-taskbar/unmapped 窗口；优先使用 `_NET_WM_ICON` 作为任务图标；hover 预览使用 X root 可见区域截图，失败时回落到标题和图标。
- 配置/接口/数据：无运行时配置；所有 item 为内置 GObject/GTK widget；任务窗口模型由 `GracefulPanelWindowInfo` 描述。
- 默认规则：单 panel、固定高度 42px、底部 dock top-level 窗口；时钟以 `HH:MM` 格式显示本地时间。

## 3. 关键约束

- 安全边界：不执行系统命令，不修改系统状态，不持久化用户数据。
- 兼容性要求：构建依赖 `gtk4`、`gtk4-x11`、`x11`、`glib-2.0`、`gobject-2.0`、`gio-2.0`；主体使用 GObject 分模块，便于后续扩展真实菜单、任务列表和状态控制。
- 性能/稳定性要求：时钟、dock 协议和任务区均有定时刷新；GObject 引用、定时器、任务窗口信息、popover 父子关系和 X11 错误陷阱需要正确释放/处理。
- 禁止触碰范围：不把 desktop 背景绘制、session 编排或 greeter 认证逻辑放入 panel 模块。

## 4. 验证方式

| 场景 | 验证命令/步骤 | 备注 |
|------|---------------|------|
| 构建 | `cmake -S . -B build && cmake --build build` | 验证 GTK4/GLib/GIO API 和链接 |
| 单元测试 | `ctest --test-dir build --output-on-failure` | 覆盖时钟模型格式化、dock strut 计算、任务窗口过滤和标题回退 |
| 命令行冒烟 | `build/panel/graceful-panel` | 需要图形环境 |
| 集成实测 | 登录 Graceful session 后启动 panel | 后续 session 编排接入后执行 |

## 5. 故障模式与修复记录

| 日期 | 类型 | 现象/需求 | 处理结果 | 验证 |
|------|------|-----------|----------|------|
| 2026-09-17 | task | 新增单 panel 第一阶段 | 已实现 GTK4/GObject panel app/window/layout 和内置菜单、启动器、任务区、网络、音量、电源、时钟模块 | `cmake -S . -B build && cmake --build build`、`ctest --test-dir build --output-on-failure` 通过 |
| 2026-09-17 | fix | panel 会被 desktop 遮盖，且不能跟随实际桌面尺寸变化 | X11/Xwayland 下按 EWMH dock window 协议设置窗口类型、above 状态和底部 strut，并定时同步 root 几何 | 本地构建/测试通过；测试机 `xwininfo`/`xprop` 验证 panel 在底部且 strut 覆盖全宽 |
| 2026-09-17 | task | 任务区空文案幼稚、缺少真实窗口图标和 hover 预览，panel 视觉不够通透 | 移除空任务区文案；新增 X11/Xwayland 窗口枚举、任务图标、hover 缩略图/兜底预览和半透明磨砂样式 | 本地构建/测试通过；测试机启动 Text Editor 后验证任务窗口和 hover popover |
| 2026-09-17 | fix | hover 任务图标时预览闪烁，部分窗口会覆盖 panel | 任务窗口集合不变时不重建按钮，任务按钮取消 GTK tooltip；panel realize/map 阶段立即应用 dock 协议，并通过 EWMH client message 请求 above/sticky/skip 状态 | 本地构建/测试通过；测试机 hover 5 秒仅保留一个预览窗口，panel 仍为 stacking 顶层 dock |

## 6. 变更记录

| 日期 | 关联提交/文档 | 变更 | 影响 |
|------|---------------|------|------|
| 2026-09-17 | `docs/dev/README.md` | 新增 `graceful-panel` 第一版边界 | 建立传统单 panel 子系统基线 |
| 2026-09-17 | `docs/dev/README.md` | 新增 X11/Xwayland EWMH dock window 协议同步 | panel 固定在底部并声明底部保留区域 |
| 2026-09-17 | `docs/dev/README.md` | 新增 X11/Xwayland 任务区窗口图标和 hover 预览 | panel 从占位任务区推进到可显示普通应用窗口 |
