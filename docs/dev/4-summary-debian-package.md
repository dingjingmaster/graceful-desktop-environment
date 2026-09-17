# Debian 打包与部署总结

> 文档元数据
> - 文件编号：4
> - 文档类型：summary
> - 文件路径：docs/dev/4-summary-debian-package.md
> - 文档版本：v1.0.0
> - 完成日期：2026-09-17
> - 关联需求：执行 `make deb` 生成 deb，并安装到测试机后默认启用 Graceful 环境
> - 关联计划：docs/dev/4-plan-debian-package.md

## 1. 最终结果

- 新增 `make deb`，生成 `build/deb/graceful-desktop-environment_0.1.0_amd64.deb`。
- 新增 `make install-remote` / `make deploy-deb`，默认安装到 `dingjing@192.168.122.195`；密码通过 `REMOTE_PASSWORD` 环境变量传入，不写入仓库。
- deb 包安装 `/usr/bin/graceful-greeter`、`/usr/bin/graceful-session`、`/usr/bin/graceful-desktop`、`/usr/bin/graceful-panel`、xgreeter/Wayland session 入口、panel logo 和 `/etc/lightdm/lightdm.conf.d/50-graceful.conf`。
- LightDM 默认配置为 `greeter-session=graceful-greeter` 和 `user-session=graceful`，不再默认进入 Ubuntu GNOME session。
- 包安装后会清理旧版 `/usr/share/xsessions/graceful.desktop`，避免 LightDM 继续把 Graceful 当 Xsession 启动。

## 2. 运行时依赖

- 核心运行时：`libc6`、`libglib2.0-0t64 | libglib2.0-0`、`libgtk-4-1`、`liblightdm-gobject-1-0`、`libx11-6`。
- 桌面会话：`lightdm`、`mutter`、`ibus`、`ibus-rime`。
- panel 启动器/系统动作依赖：`gnome-terminal | terminator | mate-terminal`、`nautilus | caja`、`gnome-control-center`、`gnome-screensaver | xdg-utils`、`systemd`。

## 3. 验证结果

- 本地执行 `make deb` 成功，打包过程完成 CMake Release 构建和 19 个测试。
- `dpkg-deb -I` 确认依赖字段写入包控制信息。
- `dpkg-deb -c` 确认包内包含四个可执行文件、LightDM 入口、Wayland session 入口、logo 和 LightDM 默认配置。
- 远端执行安装成功，apt 补装窗口管理器和终端依赖并安装 `graceful-desktop-environment 0.1.0`。
- 远端检查 `/etc/lightdm/lightdm.conf.d/50-graceful.conf` 内容正确。

## 4. 未执行项

- 未重启 LightDM，避免打断当前测试机图形会话。
- 未执行真实登出/重登验证；配置将在下一次 LightDM 启动或登录流程中生效。
