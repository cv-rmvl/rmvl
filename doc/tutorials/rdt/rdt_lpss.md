LPSS CLI 工具 {#tutorial_rdt_lpss}
============

`rdt` 提供了 LPSS 的 CLI 工具，使用时可通过输入

```bash
lpss help
```

来查看具体帮助，简单来说包括以下子命令

- `create` —— 创建一个依赖 LPSS 的新项目
- `node` —— 节点工具，包括 `list`（列出当前系统中所有的 LPSS 节点）、`info`（查看指定节点的详细信息），例如计划查看 `lpss_node_1` 的信息，则可以输入

  ```bash
  lpss node info lpss_node_1
  ```

  计划查看 `/cur/joint_states` 话题的信息，则可以输入

  ```bash
  lpss topic info /cur/joint_states
  ```

- `topic` —— 话题工具，包括 `list`（列出当前系统中所有的 LPSS 话题）、`info`（查看指定话题的详细信息）、`echo`（订阅指定话题并打印消息内容）、`hz`（查看指定话题的发布频率）、`bw`（查看指定话题的带宽使用情况）
- `interface` —— 内置消息接口查看工具
- `graph` —— 前后端分离的 LPSS 节点图工具
- `viz` —— 前后端分离的 LPSS 3D 可视化工具，也可以不加前缀的使用 `lviz` 来启动
