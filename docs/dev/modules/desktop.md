# Desktop 模块文档

> 文档元数据
> - 文档类型：module
> - 文件路径：docs/dev/modules/desktop.md
> - 文档版本：v1.0.0
> - 最后更新：2026-09-17
> - 维护范围：用户桌面壳进程

## 1. 模块边界

- 职责：提供 `graceful-desktop`，作为登录后的桌面壳进程；第一阶段负责从指定目录随机读取图片，以 GTK4 桌面窗口绘制桌面背景，并在切换时做整图渐变过渡。
- 非职责：暂不提供桌面图标、右键菜单、文件/文件夹创建、设置中心入口、终端入口、Wayland layer-shell 后端或完整窗口管理能力。
- 相关代码路径：`desktop/`。
- 相关长期文档：`docs/overview-product.md`、`docs/overview-product-dev.md`。

## 2. 当前行为

- 用户可见行为：`graceful-desktop` 启动非唯一 GTK4 应用，创建无边框桌面窗口；从壁纸目录随机选择图片，以 cover 模式铺满窗口；切换壁纸时旧图淡出、新图淡入；目录不存在、目录为空或图片加载失败时绘制带浅色 `Graceful Linux` 字样的内置极简线条背景。
- 窗口协议：X11/Xwayland 下设置 `_NET_WM_WINDOW_TYPE_DESKTOP` 和 sticky/skip taskbar/skip pager/below 状态；运行期间每 500ms 按 X root 几何重新同步窗口尺寸和层级，适配分辨率或虚拟 root 尺寸变化。
- 配置/接口/数据：支持环境变量 `GRACEFUL_DESKTOP_WALLPAPER_DIR` 指定壁纸目录；支持 `GRACEFUL_DESKTOP_WALLPAPER_INTERVAL` 指定随机切换间隔，单位秒。
- 默认规则：未指定壁纸目录时使用 `~/Pictures/Wallpapers`；未指定或非法间隔时使用 300 秒。
- 图片格式：第一阶段支持 `.jpg`、`.jpeg`、`.png`、`.webp`、`.bmp` 后缀；当前只扫描指定目录的直接子文件，不递归。

## 3. 关键约束

- 安全边界：不修改用户文件，不删除图片，不写系统配置。
- 兼容性要求：构建依赖 `gtk4`、`gtk4-x11`、`x11`、`glib-2.0`、`gobject-2.0`、`gio-2.0`；桌面主体使用 GObject 组织，便于后续扩展菜单、图标和后端。
- 性能/稳定性要求：图片目录每次切换前重新扫描；过渡绘制使用 GTK snapshot opacity 叠加，默认 1200ms；兜底背景使用 GTK snapshot/Cairo 直接绘制，不依赖外部图片文件；GObject 引用、GPtrArray 字符串、tick callback 和 GTK 资源需要正确释放。
- 禁止触碰范围：不把 greeter/session 认证和进程编排逻辑放入 desktop 模块。

## 4. 验证方式

| 场景 | 验证命令/步骤 | 备注 |
|------|---------------|------|
| 构建 | `cmake -S . -B build && cmake --build build` | 验证 GTK4/GLib/GIO API 和链接 |
| 单元测试 | `ctest --test-dir build --output-on-failure` | 覆盖配置读取、图片过滤、空目录错误和过渡进度 |
| 命令行冒烟 | `GRACEFUL_DESKTOP_WALLPAPER_DIR=/path/to/images build/desktop/graceful-desktop` | 需要图形环境；可用不存在目录验证兜底背景 |
| 集成实测 | 登录 Graceful session 后观察背景窗口 | 后续测试机部署验证 |

## 5. 故障模式与修复记录

| 日期 | 类型 | 现象/需求 | 处理结果 | 验证 |
|------|------|-----------|----------|------|
| 2026-09-17 | task | 新增桌面背景进程第一阶段 | 已实现 GObject 主体框架、GTK4 全屏背景窗口、随机壁纸扫描、定时切换和整图渐变过渡 | `cmake -S . -B build && cmake --build build`、`ctest --test-dir build --output-on-failure` 通过 |
| 2026-09-17 | fix | 桌面窗口会遮盖 panel 或不能随分辨率变化铺满 | X11/Xwayland 下按 EWMH desktop window 协议设置窗口类型与状态，并定时同步 X root 几何 | 本地构建/测试通过；测试机 `xwininfo` 验证 desktop 尺寸跟随 root |
| 2026-09-17 | fix | desktop 启动时可能因 GTK application 唯一实例注册等待 session bus | `graceful-desktop` 改为 `G_APPLICATION_NON_UNIQUE`，避免桌面壳依赖 session bus 注册唯一应用名 | 全量构建、22 个测试和 `git diff --check` 通过 |

## 6. 变更记录

| 日期 | 关联提交/文档 | 变更 | 影响 |
|------|---------------|------|------|
| 2026-09-17 | `docs/dev/README.md` | 新增 `graceful-desktop` 第一阶段边界 | 为 session 默认核心命令提供最小桌面壳 |
| 2026-09-17 | `docs/dev/README.md` | 新增壁纸切换整图渐变过渡 | 切换壁纸时旧图淡出、新图淡入 |
| 2026-09-17 | `docs/dev/README.md` | 新增 X11/Xwayland EWMH desktop window 协议同步 | desktop 不再作为普通 GTK 窗口参与遮盖 panel |
