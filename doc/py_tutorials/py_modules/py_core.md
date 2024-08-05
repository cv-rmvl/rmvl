适用于 Python 的 core 模块 {#tutorial_py_core}
=======================

@author 赵曦
@date 2024/08/05
@version 1.0
@brief RMVL-Python core 模块的使用示例

@next_tutorial{tutorial_py_algorithm}

@tableofcontents

------

## 定时模块

```python
import rm

# 定时器
t = rm.timer.now()
print("now = {:.6f}".format(t))
```
