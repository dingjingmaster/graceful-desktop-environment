# Debian 打包与部署计划

> 文档元数据
> - 文件编号：4
> - 文档类型：plan
> - 文件路径：docs/dev/4-plan-debian-package.md
> - 文档版本：v1.0.0
> - 最后更新：2026-09-17
> - 关联需求：执行 `make deb` 生成 deb，并安装到测试机后默认启用 Graceful 环境

## 1. 目标与成功标准

- 任务目标：新增项目级 deb 打包入口，打包 greeter/session/desktop/panel、LightDM greeter/session 入口、panel logo 和默认 LightDM 配置。
- 成功标准：`make deb` 可生成 `.deb`；包依赖声明覆盖运行时需要的 Ubuntu 包；远端测试机可通过 apt 安装该 deb；安装后 LightDM 默认使用 `graceful-greeter` 和 `graceful` session。
- 非目标：不重启 LightDM，不自动注销当前图形会话，不发布到 apt 仓库。

## 2. 修改边界

- 修改范围：根 `Makefile`、`packaging/` 打包脚本和模板、开发文档。
- 禁止触碰范围：不修改 greeter/session/desktop/panel 功能逻辑；不写入测试机密码到仓库；不执行显示管理器重启。
- 依赖关系：打包使用系统已有 `cmake`、`ctest`、`dpkg-deb`；远端安装使用 apt 解析运行时依赖。

## 3. 安全门禁摘要

| 项 | 结论 |
|----|------|
| 风险矩阵结论 | L3 |
| 风险标签 | system,release,external-effect,public-contract |
| 验证范围 | 模块或关键链路 |
| 记录级别 | 独立文档 |
| 命令权限 | C0/C1；远端 apt 安装按用户明确要求执行 |
| 高风险开发门禁 | 是：打包/部署与系统 LightDM 配置 |
| 破坏性操作 | 未重启服务，未删除用户数据 |

## 4. 执行计划

| 步骤 | 修改内容 | 验证方式 | 状态 |
|------|----------|----------|------|
| 1 | 新增 `make deb` 和手工 deb 构建脚本 | `make deb` | 完成 |
| 2 | 新增运行时依赖和 LightDM 默认配置 | `dpkg-deb -I/-c` | 完成 |
| 3 | 新增远端安装目标 | `REMOTE_PASSWORD=... make install-remote` | 完成 |
| 4 | 验证远端安装和默认配置 | `dpkg-query`、检查 `/etc/lightdm/lightdm.conf.d/50-graceful.conf` | 完成 |
