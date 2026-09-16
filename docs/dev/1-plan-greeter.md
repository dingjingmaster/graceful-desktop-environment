# LightDM Greeter 开发计划

> 文档元数据
> - 文件编号：1
> - 文档类型：plan
> - 文件路径：docs/dev/1-plan-greeter.md
> - 文档版本：v1.0.0
> - 最后更新：2026-09-16
> - 关联需求：基于 GTK4 实现 LightDM greeter
> - 关联调研：[docs/dev/1-research-greeter.md](1-research-greeter.md)

## 1. 目标与成功标准

- 任务目标：实现 `graceful-greeter` 的最小 GTK4 + LightDM 登录闭环。
- 成功标准：项目可构建；greeter 内部模型测试通过；`main.c` 能连接 LightDM、处理认证提示、认证成功后启动所选 session。
- 前置条件：系统提供 `gtk4` 与 `liblightdm-gobject-1` 开发包。
- 非目标：不安装到系统、不启用 LightDM 配置、不实现主题/电源/远程登录/锁屏。

## 2. 修改边界

- 最大修改范围：`greeter/`、顶层 `CMakeLists.txt`、`docs/dev/`。
- 禁止触碰范围：`common/` 不新增 greeter 私有代码；不修改系统配置；不执行 sudo/systemctl/git 提交。
- 影响模块/文件：`greeter/CMakeLists.txt`、`greeter/main.c`、新增 greeter 内部模型与测试、开发文档。
- 依赖关系：新增构建依赖 `gtk4`、`liblightdm-gobject-1`，测试依赖 `glib-2.0`/`gobject-2.0`。

## 3. 安全门禁摘要

| 项 | 结论 |
|----|------|
| 风险矩阵结论 | L3 |
| 风险标签 | system,lifecycle,public-contract |
| 验证范围 | 模块或关键链路 |
| 记录级别 | 独立文档 |
| 命令权限 | C0/C1 |
| 高风险开发门禁 | 是，涉及登录链路和外部 LightDM API |
| 破坏性操作 | 否 |
| 用户确认事项 | 无 |
| 止损/回滚方案 | 回退本次新增/修改的 greeter 与 docs 文件；未触碰系统配置 |

## 4. Bug 修复计划

不适用。

## 5. 执行计划

| 步骤 | 修改内容 | 验证方式 | 状态 |
|------|----------|----------|------|
| 1 | 新增 greeter 内部登录模型测试 | 已确认缺少模型时构建失败；新增密码空格测试时先失败 | 完成 |
| 2 | 实现 greeter 登录模型 GObject | `greeter-login-model-test` 通过 | 完成 |
| 3 | 接入 GTK4 + LightDM UI/认证流程和 CMake 依赖 | `cmake --build build` 通过 | 完成 |
| 4 | 更新模块文档与索引状态 | 人工审阅文档一致性 | 完成 |

## 6. 验证计划

- 基础验证：`cmake -S . -B build`、`cmake --build build`、`ctest --test-dir build --output-on-failure` 均通过。
- 高风险验证：代码审查 LightDM 错误路径、GObject 引用释放、GTK 信号生命周期、session key 选择边界。
- 验证环境：本地开发环境。
- 不可执行验证项：未在当前环境启用真实 LightDM greeter，未做 PAM 登录实测。
- 残余风险：真实显示管理器环境下的 PAM 提示组合、用户列表权限和 session 启动失败路径仍需集成验证。
