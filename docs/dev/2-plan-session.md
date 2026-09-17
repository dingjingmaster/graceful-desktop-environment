# Session 进程开发计划

> 文档元数据
> - 文件编号：2
> - 文档类型：plan
> - 文件路径：docs/dev/2-plan-session.md
> - 文档版本：v1.0.0
> - 最后更新：2026-09-17
> - 关联需求：实现 graceful session 进程，纯 C，并尽可能用 GObject 按功能分模块
> - 关联调研：docs/dev/2-research-session.md

## 1. 目标与成功标准

- 任务目标：新增 `graceful-session`，作为 LightDM 登录成功后启动的用户 session 根进程。
- 成功标准：可构建安装；可读取内置/文件 session 定义；可构造 session 环境变量；可启动并等待核心子进程；核心子进程退出后 session 退出；单元测试覆盖关键行为。
- 前置条件：系统提供 GLib/GObject/GIO 和 CMake。
- 非目标：完整 D-Bus SessionManager、systemd user target、XDG autostart、inhibitor、电源/锁屏/session restore。

## 2. 修改边界

- 最大修改范围：新增 `session/` 源码和测试；顶层 CMake 增加 `add_subdirectory(session)`；新增 session 相关 docs/dev 文档和索引；必要时更新产品/开发概览。
- 禁止触碰范围：不修改 greeter 认证和 UI；不修改系统配置；不执行安装/重启 LightDM。
- 影响模块/文件：`session/`、`CMakeLists.txt`、`docs/dev/*`、`docs/overview-product*.md`。
- 依赖关系：使用 GLib/GObject/GIO，不引入新非系统依赖。

## 3. 安全门禁摘要

| 项 | 结论 |
|----|------|
| 风险矩阵结论 | L3 |
| 风险标签 | system,lifecycle,public-contract,product-flow |
| 验证范围 | 模块或关键链路 |
| 记录级别 | 独立文档 |
| 命令权限 | C0/C1 |
| 高风险开发门禁 | 是：C 逻辑、进程生命周期、公共 session 入口 |
| 破坏性操作 | 否 |
| 用户确认事项 | 无 |
| 止损/回滚方案 | 回退本次新增 session 目录、CMake 入口和文档；LightDM 可继续选择其它 session |

## 4. 执行计划

| 步骤 | 修改内容 | 验证方式 | 状态 |
|------|----------|----------|------|
| 1 | RED：新增 session 单元测试、构建入口和计划文档 | `cmake -S . -B build` 失败于缺失 `session-definition.c` 和 `main.c` | 完成 |
| 2 | GREEN：实现 session definition/environment/process/manager GObject 模块和 CLI | `cmake -S . -B build && cmake --build build` 通过 | 完成 |
| 3 | 验证：运行 session 测试、全量 ctest、diff 检查 | `ctest --test-dir build --output-on-failure`、`git diff --check`、CLI 冒烟通过 | 完成 |
| 4 | Summary：更新模块文档、索引、概览和总结文档 | 人工审阅 diff 与文档一致性 | 完成 |

## 5. 验证计划

- 基础验证：CMake configure/build；session 单元测试；全量 ctest；`git diff --check`。
- 高风险验证：进程退出状态、错误路径、GObject 引用释放、环境变量覆盖。
- 验证环境：当前本地工作区。
- 不可执行验证项：真实 LightDM 登录后的 session 集成，本轮不重启系统服务。
- 残余风险：第一版不管理 systemd user services 和 XDG autostart，真实桌面组件接入后需扩展。
