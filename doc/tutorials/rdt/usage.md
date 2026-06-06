RMVL 快捷开发工具 {#tutorial_rdt}
============

RMVL Dev Tools（简称 `rdt`）是一个 CLI 命令行工具，能够极大幅度简化 RMVL 的安装、开发、调试流程，支持 Bash 和 PowerShell。手动执行 `rdt` 的安装可直接参考 [rmvl-dev-tools 仓库](https://github.com/cv-rmvl/rmvl-dev-tools)，按照 README 中的说明进行安装即可。

@remark 对于 Linux 用户推荐使用如下的一键安装命令，打开终端后输入以下内容，根据提示操作即可完成 RMVL 以及 rdt 工具的安装：
<div class="fragment">
<div class="line"><span class="keywordflow">wget</span> https://cv-rmvl.github.io/install <span class="comment">-qO</span> - | <span class="keywordflow">bash</span></div>
</div>

`rdt` 的使用非常简单，安装完成后在终端输入以下命令即可查看帮助文档：

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> help</div>
</div>

`rdt` 的 CLI 核心命令包括：

- rdt：包含 `create`、`dev`、`update` 等子命令，主要用于 RMVL 项目的创建、构建、开发和更新。

  详细内容可参考 @subpage tutorial_rdt_rdt 。

- lpss：包含 `node`、`topic`、`interface` 等子命令，主要用于 LPSS 相关的节点、话题、接口等资源的管理和调试。

  详细内容可参考 @subpage tutorial_rdt_lpss 。
