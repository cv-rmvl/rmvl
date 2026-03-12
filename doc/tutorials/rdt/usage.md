RMVL 快捷开发工具 {#tutorial_rdt}
============

RMVL Dev Tools（简称 `rdt`）是专门为 Linux 平台打造的 CLI 命令行工具，能够极大幅度简化 RMVL 的安装、开发、调试流程。`rdt` 的安装可直接参考 [rmvl-dev-tools 仓库](https://github.com/cv-rmvl/rmvl-dev-tools)，按照 README 中的说明进行安装即可。

`rdt` 的使用非常简单，安装完成后在终端输入以下命令即可查看帮助文档：

```bash
rmvl help
```

`rdt` 的 CLI 核心命令包括：

- rmvl：包含 `create`、`dev`、`update` 等子命令，主要用于 RMVL 项目的创建、构建、开发和更新。

  详细内容可参考 @subpage tutorial_rdt_rmvl 。

- lpss：包含 `node`、`topic`、`interface` 等子命令，主要用于 LPSS 相关的节点、话题、接口等资源的管理和调试。

  详细内容可参考 @subpage tutorial_rdt_lpss 。
