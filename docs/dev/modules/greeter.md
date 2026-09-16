# Greeter 模块文档

> 文档元数据
> - 文档类型：module
> - 文件路径：docs/dev/modules/greeter.md
> - 文档版本：v1.0.0
> - 最后更新：2026-09-16
> - 维护范围：LightDM greeter 登录入口

## 1. 模块边界

- 职责：提供 LightDM greeter 的 GTK4 图形登录界面，处理用户名、密码、session 选择、PAM 提示响应和认证成功后的 session 启动。
- 非职责：不负责用户 session 进程管理、settings-daemon、desktop shell、系统 LightDM 配置安装。
- 相关代码路径：`greeter/`。
- 相关长期文档：`docs/dev/1-research-greeter.md`、`docs/dev/1-plan-greeter.md`。

## 2. 当前行为

- 用户可见行为：第一版提供全屏 greeter、居中登录框、可配置背景图、用户下拉列表、密码输入、session 下拉选择、登录按钮和状态信息；用户名只允许从下拉列表选择，不提供手动用户名输入框。
- 配置/接口/数据：通过 `liblightdm-gobject-1` 与 LightDM daemon 交互；可在 `/etc/lightdm/graceful-greeter.conf` 的 `[Greeter]` 段配置 `Background=/path/to/image`。
- 默认规则：未配置背景图时使用纯色背景；LightDM secret prompt 不显示 `Password:` 文案；无法连接 LightDM daemon 时显示错误并禁用登录；认证失败后清空密码。

## 3. 关键约束

- 安全边界：不记录密码；密码只在 GTK 输入控件与 LightDM prompt 响应之间短暂使用。
- 兼容性要求：构建依赖 `gtk4`、`gtk4-x11` 和 `liblightdm-gobject-1`。
- 性能/稳定性要求：登录流程错误路径必须释放 GObject 引用和动态分配内存。
- 禁止触碰范围：greeter 私有状态与 UI 逻辑不放入 `common/`。

## 4. 验证方式

| 场景 | 验证命令/步骤 | 备注 |
|------|---------------|------|
| 构建 | `cmake -S . -B build && cmake --build build` | 验证 GTK4/LightDM API 兼容 |
| 单元测试 | `ctest --test-dir build --output-on-failure` | 覆盖 greeter 内部登录模型和配置读取 |
| 集成实测 | 在测试机 LightDM 配置中启用 graceful greeter 后登录 | 2026-09-16 已在测试机验证 LightDM 启动 greeter |
| 显示尺寸 | `xdpyinfo` 对比 `xwininfo -root -tree` 中 `Graceful` 窗口尺寸 | 裸 X 下应与 root window 同宽同高 |

## 5. 故障模式与修复记录

| 日期 | 类型 | 现象/需求 | 处理结果 | 验证 |
|------|------|-----------|----------|------|
| 2026-09-16 | task | 新增 LightDM greeter 最小登录闭环 | 已实现 GTK4 全屏 UI、居中登录框、可配置背景图、LightDM 认证回调、用户列表、session 选择与 xgreeters 描述文件 | `cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure` 通过；测试机 LightDM active，`Graceful` 窗口为 `1280x800+0+0` |
| 2026-09-16 | fix | 输入密码后 `Log In` 按钮仍不可点击 | 密码输入变化后同步刷新登录按钮敏感状态 | `cmake --build build`、`ctest --test-dir build --output-on-failure`、`git diff --check` 通过；测试机 LightDM active，`Graceful` 窗口为 `1280x800+0+0` |

## 6. 变更记录

| 日期 | 关联提交/文档 | 变更 | 影响 |
|------|---------------|------|------|
| 2026-09-16 | `docs/dev/1-plan-greeter.md` | 定义并实现 GTK4 + LightDM greeter 第一版边界 | 新增 greeter 子系统 |
