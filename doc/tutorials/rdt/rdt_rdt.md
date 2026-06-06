RDT CLI 工具 {#tutorial_rdt_rdt}
============

@author 赵曦
@date 2026/06/06
@version 1.0
@brief RDT 命令行工具的使用教程

@next_tutorial{tutorial_rdt_lpss}

@tableofcontents

---

### 前言

RMVL Dev Tools（简称 `rdt`）提供了 RMVL 安装、更新、开发、提交与移除等常用工作流的命令行入口，能够减少手动输入重复命令的成本。本工具的主入口为 `rdt`，可通过输入

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> help</div>
</div>

来查看具体帮助。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> <span class="comment">&lt;command&gt;</span> [args...]</div>
</div>

@dl_begin{命令}
@dl_item{help,显示详细帮助信息}
@dl_item{create,创建一个新的 RMVL 模块}
@dl_item{update,更新 RMVL 或 rdt 工具}
@dl_item{dev,开始开发 RMVL}
@dl_item{git,执行常用 Git 工作流}
@dl_item{remove,移除 RMVL 组件}
@dl_item{version,显示 rdt 工具版本}
@dl_end

此工具支持的根命令有 `rdt`、`%lpss` 和 `lviz`，更多信息请参考官方手册:

- [用户手册](https://cv-rmvl.github.io/)
- [Doxygen](https://cv-rmvl.github.io/docs/2.x/)
- [GitHub](https://github.com/cv-rmvl/rmvl)

### create 命令

创建一个新的 RMVL 模块。该命令会在当前目录下生成主模块的基本目录结构和必要文件，也可以同时创建若干依赖主模块的子模块。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> create <span class="comment">&lt;module_name&gt;</span> [sub_module_1 [sub_module_2] ...]</div>
</div>

@param module_name 要创建的主模块名称
@param sub_module_<n> 可选的子模块名称，可输入多个

主模块会作为 RMVL 中的独立模块，生成的核心 CMake 命令形如

<div class="fragment">
<div class="line"><span class="keyword">rmvl_add_module</span>(</div>
<div class="line">&nbsp;&nbsp;&lt;module_name&gt;</div>
<div class="line">&nbsp;&nbsp;<span class="keyword">DEPENDS</span> core</div>
<div class="line">)</div>
</div>

如果指定了子模块，每个子模块也会作为独立模块，并依赖主模块，核心 CMake 命令形如

<div class="fragment">
<div class="line"><span class="keyword">rmvl_add_module</span>(</div>
<div class="line">&nbsp;&nbsp;&lt;sub_module&gt;</div>
<div class="line">&nbsp;&nbsp;<span class="keyword">DEPENDS</span> &lt;module_name&gt;</div>
<div class="line">)</div>
</div>

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 创建 my_module 主模块，并包含 sub1、sub2 和 sub3 三个子模块</span></div>
<div class="line"><span class="keywordflow">rdt</span> create my_module sub1 sub2 sub3</div>
</div>

### update 命令

更新 RMVL 代码、文档、库文件或 rdt 工具。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> update <span class="comment">[help | tool | doc | code | lib | all]</span> [args...]</div>
</div>

@dl_begin{命令}
@dl_item{help,显示此帮助信息}
@dl_item{tool,更新 rdt 工具到最新版本，并自动更新 RMVL 代码}
@dl_item{doc,执行 Doxygen 文档生成，并推送到 `cv-rmvl.github.io` 仓库}
@dl_item{code,更新 RMVL 仓库至最新的 `2.x` 分支代码}
@dl_item{lib,执行完整的编译安装流程以更新 RMVL 动态/静态库}
@dl_item{all,依次执行 `code` 和 `lib` 两个步骤，即更新代码并以 Release 模式编译安装}
@dl_end

@warning `rdt update code` 会在 RMVL 仓库中执行 `git stash`、`git checkout 2.x` 和 `git reset --hard origin/2.x`，请在执行前确认本地更改已经妥善保存。

#### doc 子命令

生成 Doxygen 文档，并将生成结果提交推送到 `cv-rmvl.github.io` 仓库的指定文档目录中。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> update doc <span class="comment">&lt;folder&gt;</span></div>
</div>

@param folder 文档存放的文件夹名称，例如 `2.x`

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 生成当前 RMVL 的 Doxygen 文档，并发布到 docs/2.x</span></div>
<div class="line"><span class="keywordflow">rdt</span> update doc 2.x</div>
</div>

#### lib 子命令

编译并安装 RMVL 动态/静态库。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> update lib <span class="comment">&lt;mode&gt;</span></div>
</div>

@param mode 编译模式，包括 `release` 和 `debug`

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 以 Release 模式编译并安装 RMVL</span></div>
<div class="line"><span class="keywordflow">rdt</span> update lib release</div>
<div class="line"><span class="comment"># 以 Debug 模式编译并安装 RMVL</span></div>
<div class="line"><span class="keywordflow">rdt</span> update lib debug</div>
</div>

#### 常用更新示例

<div class="fragment">
<div class="line"><span class="comment"># 仅更新 RMVL 代码</span></div>
<div class="line"><span class="keywordflow">rdt</span> update code</div>
<div class="line"><span class="comment"># 更新代码后以 Release 模式编译安装</span></div>
<div class="line"><span class="keywordflow">rdt</span> update all</div>
<div class="line"><span class="comment"># 更新 rdt 工具，执行时会先更新 RMVL 代码</span></div>
<div class="line"><span class="keywordflow">rdt</span> update tool</div>
</div>

### dev 命令

使用常见开发工具打开本地 RMVL 仓库。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> dev <span class="comment">[help | code | nvim | dir]</span></div>
</div>

@dl_begin{命令}
@dl_item{help,显示此帮助信息}
@dl_item{code,在 Visual Studio Code 中打开本地 RMVL}
@dl_item{nvim,在 Neovim 中打开本地 RMVL}
@dl_item{dir,Linux 上使用 Nautilus 打开本地 RMVL}
@dl_end

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 使用 VS Code 打开 RMVL 仓库</span></div>
<div class="line"><span class="keywordflow">rdt</span> dev code</div>
<div class="line"><span class="comment"># 使用 Neovim 打开 RMVL 仓库</span></div>
<div class="line"><span class="keywordflow">rdt</span> dev nvim</div>
</div>

### git 命令

执行 RMVL 开发中常用的 Git 工作流。该命令提供交互式界面，会引导输入提交类型、影响范围、摘要、详细说明等内容。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> git <span class="comment">[help | commit | squash | reword | newbr | update]</span></div>
</div>

@dl_begin{命令}
@dl_item{help,显示此帮助信息}
@dl_item{commit,执行 `git add .` 和 `git commit` 提交本地更改}
@dl_item{squash,创建临时提交并压缩至上一个提交}
@dl_item{reword,修改上一个提交的消息，不修改提交内容}
@dl_item{newbr,创建新分支并应用提交}
@dl_item{update,适用于 RMVL 的更新组合拳，将从 `upstream` 更新本地仓库，并推送至 `origin`}
@dl_end

#### commit 子命令

提交当前仓库中的本地更改。执行后会进入交互式流程，并生成符合约定式提交风格的 commit message。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> git commit</div>
</div>

该命令最终会执行

<div class="fragment">
<div class="line"><span class="keywordflow">git</span> add .</div>
<div class="line"><span class="keywordflow">git</span> commit -m <span class="comment">&lt;commit_title&gt;</span></div>
</div>

若填写了详细说明，则会额外通过第二个 `-m` 写入 commit body。

#### squash 子命令

将当前本地更改压缩到上一个提交中，并保留上一个提交的消息。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> git squash</div>
</div>

该命令会创建临时提交，然后执行软重置和 amend，核心流程为

<div class="fragment">
<div class="line"><span class="keywordflow">git</span> add .</div>
<div class="line"><span class="keywordflow">git</span> commit -m <span class="comment">"rdt squash temporary commit"</span></div>
<div class="line"><span class="keywordflow">git</span> reset <span class="comment">\-\-soft</span> HEAD~1</div>
<div class="line"><span class="keywordflow">git</span> commit <span class="comment">\-\-amend</span> <span class="comment">\-\-no-edit</span></div>
</div>

执行过程中可以选择是否在压缩完成后执行 `git push --force-with-lease`。

#### reword 子命令

修改上一个提交的消息，但不修改提交内容。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> git reword</div>
</div>

该命令会重新收集提交消息，并执行

<div class="fragment">
<div class="line"><span class="keywordflow">git</span> commit <span class="comment">\-\-amend</span> <span class="comment">\-\-only</span> -m <span class="comment">&lt;commit_title&gt;</span></div>
</div>

若填写了详细说明，则会额外通过第二个 `-m` 写入 commit body。

#### newbr 子命令

创建新分支，并将当前本地更改提交到这个新分支。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> git newbr</div>
</div>

该命令会在交互式流程中要求输入新分支名称和提交消息，并可以选择是否推送到 `origin`。核心流程为

<div class="fragment">
<div class="line"><span class="keywordflow">git</span> switch -c <span class="comment">&lt;branch_name&gt;</span></div>
<div class="line"><span class="keywordflow">git</span> add .</div>
<div class="line"><span class="keywordflow">git</span> commit -m <span class="comment">&lt;commit_title&gt;</span></div>
<div class="line"><span class="keywordflow">git</span> push -u origin <span class="comment">&lt;branch_name&gt;</span></div>
</div>

如果选择不推送到远程，则不会执行最后一步。

#### update 子命令

执行适用于 RMVL 的分支同步流程。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> git update</div>
</div>

该命令将依次执行以下操作：

<div class="fragment">
<div class="line"><span class="keywordflow">git</span> checkout 2.x</div>
<div class="line"><span class="keywordflow">git</span> pull upstream 2.x</div>
<div class="line"><span class="keywordflow">git</span> push origin 2.x</div>
<div class="line"><span class="keywordflow">git</span> checkout master</div>
<div class="line"><span class="keywordflow">git</span> reset <span class="comment">\-\-hard</span> 2.x</div>
<div class="line"><span class="keywordflow">git</span> remote prune origin</div>
<div class="line"><span class="keywordflow">git</span> push origin master</div>
<div class="line"><span class="keywordflow">git</span> push upstream master</div>
</div>

@warning `rdt git update` 会切换分支、重置 `master` 分支并向远程仓库推送，请仅在确认本地仓库状态、远程地址和权限都正确时使用。

### remove 命令

移除 RMVL 组件或 rdt 工具。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> remove <span class="comment">[help | tool | lib]</span></div>
</div>

@dl_begin{命令}
@dl_item{help,显示此帮助信息}
@dl_item{tool,移除 `rmvl-dev-tools` 工具}
@dl_item{lib,移除 RMVL 动态/静态库}
@dl_end

#### tool 子命令

移除 `rmvl-dev-tools` 工具。执行后会进入交互式流程，可选择是否同时移除本地 RMVL 仓库和 rdt 工具仓库。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> remove tool</div>
</div>

@warning 该命令可能删除本地仓库目录，请根据交互提示谨慎选择。

#### lib 子命令

移除已安装到系统目录中的 RMVL 库文件、头文件、CMake 配置和文档。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> remove lib</div>
</div>

该命令会移除以下路径中的 RMVL 安装内容：

<div class="fragment">
<div class="line">/usr/local/lib/librmvl_*</div>
<div class="line">/usr/local/lib/cmake/RMVL</div>
<div class="line">/usr/local/include/RMVL</div>
<div class="line">/usr/local/share/doc/RMVL</div>
</div>

### version 命令

显示 rdt 工具版本。

**用法**

<div class="fragment">
<div class="line"><span class="keywordflow">rdt</span> version <span class="comment">[log]</span></div>
</div>

@param log 可选参数，指定后会显示当前版本和详细更新日志，否则仅显示版本号

**示例**

<div class="fragment">
<div class="line"><span class="comment"># 显示当前 rdt 版本</span></div>
<div class="line"><span class="keywordflow">rdt</span> version</div>
<div class="line"><span class="comment"># 显示当前版本和详细更新日志</span></div>
<div class="line"><span class="keywordflow">rdt</span> version log</div>
</div>
