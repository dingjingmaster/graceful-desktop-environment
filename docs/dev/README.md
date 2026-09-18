# 需求、任务与模块变更索引

> 记录路由结论为“模块+索引”或“独立文档”的功能开发、bug 修复、调研、计划、总结、评审和模块文档变更。新需求/问题优先读取本索引，再按相关性展开模块文档和少量独立文档，避免加载完整历史上下文。

## 独立文档编号规则

- 记录级别为“模块+索引”时，功能开发和 bug 修复更新模块文档但不分配编号；记录级别为“无”时不创建本地文档。
- 只有新增独立任务、问题修复、调研、计划、总结、评审或需求变更文档时，才分配编号。
- 独立文档编号使用从 `1` 开始递增的正整数，不要求固定位数。
- 编号全局只在 `docs/dev/` 下递增，不按类型分别编号。
- 新增独立文档前，先检查本索引和 `docs/dev/` 现有编号文件名，取最大编号 + 1。
- 同一需求的多份独立文档使用同一编号，例如 `2-research-xxx.md`、`2-plan-xxx.md`、`2-summary-xxx.md`。
- 编号一旦分配不得复用；取消、废弃、拆分、合并也要在索引中保留记录并标注状态。
- 文件命名格式：`N-[type]-[slug].md`，其中 `type` 可取 `summary`、`task`、`fix`、`research`、`plan`、`review`。
- 模块文档命名格式：`modules/[module].md`。

## 索引

| 日期 | 级别 | 风险标签 | 模块 | 类型 | 关联文档 | 状态 | 摘要 |
|------|------|----------|------|------|----------|------|------|
| 2026-09-16 | L3 | system,lifecycle,public-contract | greeter | research/plan/summary/module | [1-research-greeter.md](1-research-greeter.md), [1-plan-greeter.md](1-plan-greeter.md), [1-summary-greeter.md](1-summary-greeter.md), [modules/greeter.md](modules/greeter.md) | 已完成 | 基于 GTK4 与 liblightdm-gobject 实现 LightDM greeter 最小登录闭环。 |
| 2026-09-17 | L3 | system,lifecycle,public-contract,product-flow | session | research/plan/summary/module | [2-research-session.md](2-research-session.md), [2-plan-session.md](2-plan-session.md), [2-summary-session.md](2-summary-session.md), [modules/session.md](modules/session.md) | 已完成 | 新增纯 C/GObject `graceful-session`，建立登录后的 session 根进程最小闭环。 |
| 2026-09-17 | L2 | system,public-contract | desktop | module | [modules/desktop.md](modules/desktop.md) | 已完成 | 新增 GTK4/GObject `graceful-desktop` 第一阶段，绘制随机壁纸背景并支持整图渐变切换。 |
| 2026-09-17 | L2 | system,public-contract | panel | module | [modules/panel.md](modules/panel.md) | 已完成 | 新增 GTK4/GObject `graceful-panel` 第一阶段，固定单 panel 与内置模块布局。 |
| 2026-09-17 | L2 | system,public-contract | desktop,panel | fix/module | [modules/desktop.md](modules/desktop.md), [modules/panel.md](modules/panel.md) | 已完成 | 修复 desktop/panel 作为普通 GTK 窗口运行的问题；X11/Xwayland 下设置 EWMH desktop/dock 角色、底部 strut，并按 root 几何持续同步尺寸和位置。 |
| 2026-09-17 | L2 | system,public-contract | panel | task/module | [modules/panel.md](modules/panel.md) | 已完成 | 改进 panel 任务区：移除空任务文案，X11/Xwayland 下显示普通应用窗口图标，hover 显示缩略图/兜底预览，并增加半透明磨砂观感。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,public-contract | panel | fix/module | [modules/panel.md](modules/panel.md) | 已完成 | 修复 panel 任务预览闪烁和层级不够稳的问题：任务集合不变时不重建按钮，移除任务 tooltip，并通过 EWMH client message 强化 above/dock 状态。 |
| 2026-09-17 | L1 | product-flow | panel | task/module | [modules/panel.md](modules/panel.md) | 已完成 | 将 panel 菜单按钮从 `Graceful` 文本改为 `data/2.png` logo，并安装为 panel 数据文件。 |
| 2026-09-17 | L2 | system,product-flow | panel | task/module | [modules/panel.md](modules/panel.md) | 已完成 | 在 panel 中新增开始菜单第一版：GTK4 popover、应用搜索、固定应用、应用列表和 GIO 应用启动。 |
| 2026-09-17 | L2 | system,product-flow | panel | task/module | [modules/panel.md](modules/panel.md) | 已完成 | 优化开始菜单：固定应用持久化到 `~/.config/graceful/panel/pinned-apps.ini`，支持右键 pin/unpin、按 desktop 分类分组显示和单击启动。 |
| 2026-09-17 | L2 | system,product-flow | panel | task/module | [modules/panel.md](modules/panel.md) | 已完成 | 优化 panel 启动器和时钟：终端/文件/设置按钮按候选命令 fallback 启动，时钟改为两行显示秒、日期和星期。 |
| 2026-09-17 | L2 | system,lifecycle,product-flow | panel | task/module | [modules/panel.md](modules/panel.md) | 已完成 | Power 按钮新增关机、重启、登出、锁屏菜单，并通过受控系统命令执行对应动作。 |
| 2026-09-17 | L2 | system,product-flow | panel | task/module | [modules/panel.md](modules/panel.md) | 已完成 | 右侧状态区移除网络/声音按钮，新增 workspace 指示和 tray 展开按钮/透明 popover 骨架；无托盘项隐藏按钮，菜单开合带箭头旋转动画。 |
| 2026-09-17 | L2 | system,product-flow | panel | task/module | [modules/panel.md](modules/panel.md) | 已完成 | panel 在 tray 和 Power 之间新增网速、CPU、内存实时监控项，读取 `/proc`/`/sys` 并紧凑显示。 |
| 2026-09-17 | L3 | system,lifecycle,public-contract,product-flow | session | plan/summary/module | [3-plan-session-components.md](3-plan-session-components.md), [3-summary-session-components.md](3-summary-session-components.md), [modules/session.md](modules/session.md) | 已完成 | LightDM Wayland session 先启动 mutter，再由 graceful-session 编排 ibus/rime、desktop 和 panel，保留显式命令调试路径。 |
| 2026-09-17 | L2 | system,product-flow | panel | task/module | [modules/panel.md](modules/panel.md) | 已完成 | panel 将 CPU/MEM 合并为一个资源状态组件，MEM 在上、CPU 在下。 |
| 2026-09-17 | L1 | product-flow | panel | task/module | [modules/panel.md](modules/panel.md) | 已完成 | 固定网速与资源状态组件预留宽度，CPU 不再显示温度，减少右侧布局跳动。 |
| 2026-09-17 | L3 | system,release,external-effect,public-contract | packaging | plan/summary | [4-plan-debian-package.md](4-plan-debian-package.md), [4-summary-debian-package.md](4-summary-debian-package.md) | 已完成 | 新增 `make deb` 和远端安装目标；deb 声明 Ubuntu 运行时依赖并安装 LightDM 默认 Graceful 配置。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,product-flow | session | fix/module | [modules/session.md](modules/session.md) | 已完成 | 修复 Graceful 登录后立即回到 greeter：移除 mutter 不支持的 `--replace`，并固定 desktop/panel 使用安装路径。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,release,product-flow | session | fix/module | [modules/session.md](modules/session.md) | 已完成 | 修复 mutter 放在 LightDM Xsession 子进程中抢占 DRM 失败的问题：Graceful 改为 Wayland session，由 mutter 作为外层 compositor 启动 graceful-session。 |
| 2026-09-17 | L2 | system,lifecycle,product-flow | session | module | [modules/session.md](modules/session.md) | 已完成 | session 支持 XDG Autostart，核心组件启动后拉起 `/etc/xdg/autostart` 和用户 autostart 中匹配 Graceful 的启动项。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,product-flow | session | module | [modules/session.md](modules/session.md) | 已完成 | 修复 mutter Wayland session 中 desktop/panel 启动但层级和定位不生效：session 子进程默认设置 `GDK_BACKEND=x11`。 |
| 2026-09-17 | L2 | system,lifecycle,public-contract,product-flow | panel | module | [modules/panel.md](modules/panel.md) | 已完成 | panel 实现 `org.kde.StatusNotifierWatcher`，支持 StatusNotifier/AppIndicator 托盘项注册并在 tray 展开菜单中显示。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,product-flow | session | module | [modules/session.md](modules/session.md) | 已完成 | 修复进入桌面慢：session 子进程移除 `GTK_MODULES`、设置 `NO_AT_BRIDGE=1`，并以精确环境启动，避免旧 AT-SPI bridge 反复激活。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,product-flow | panel | module | [modules/panel.md](modules/panel.md) | 已完成 | 修复 tray 展开面板显示和交互：托盘项改为正方形图标按钮，支持左键激活、右键 DBusMenu 菜单和菜单项点击回传。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,product-flow | session | module | [modules/session.md](modules/session.md) | 已完成 | 修复点击登出后 greeter 不回来：默认桌面模式正常退出和 SIGTERM 登出路径都会确认式结束父级 mutter，避免 LightDM 一直等待外层 compositor。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,product-flow | panel | module | [modules/panel.md](modules/panel.md) | 已完成 | tray 增加 XEmbed legacy 托盘接管，补齐未注册 StatusNotifierItem 的进程，并缩小展开面板图标尺寸。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,product-flow | panel | module | [modules/panel.md](modules/panel.md) | 已完成 | 修复 panel 登出仍可能卡住：优先结束当前 Graceful 会话外层 mutter，loginctl 仅作为兜底。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,product-flow | desktop,panel | module | [modules/desktop.md](modules/desktop.md), [modules/panel.md](modules/panel.md) | 已完成 | desktop/panel 改为非唯一 GTK application，避免核心壳进程启动时因 session bus 应用注册 NoReply 卡顿。 |
| 2026-09-17 | L2 | bug-e2,system,lifecycle,public-contract,product-flow | session | module | [modules/session.md](modules/session.md), [../overview-product-dev.md](../overview-product-dev.md) | 已完成 | 新增 `graceful-session-launcher` 托管外层 mutter，Wayland session 入口改为 launcher，修复注销/LightDM 重启时 mutter 残留导致卡死。 |
| 2026-09-17 | L3 | bug-e2,system,lifecycle,public-contract,product-flow | panel | module | [modules/panel.md](modules/panel.md) | 已完成 | 修复托盘协议兼容性：XEmbed 过滤非 `_XEMBED_INFO` 窗口并嵌入到弹出面板 X surface，SNI watcher 不再混入 legacy `xembed:*` 项。 |
