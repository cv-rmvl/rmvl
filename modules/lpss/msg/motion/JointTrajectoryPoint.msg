# 各个关节的位置，单位取决于特定的关节类型：旋转关节为 rad，线性关节为 m
float64[] positions

# 各个关节的速度，单位取决于特定的关节类型：旋转关节为 rad/s，线性关节为 m/s
float64[] velocities

# 各个关节的加速度，单位取决于特定的关节类型：旋转关节为 rad/s^2，线性关节为 m/s^2
float64[] accelerations

# 各个关节的力矩或力，旋转关节为 N*m，线性关节为 N
float64[] effort

# 从轨迹开始到达这个轨迹点的期望时间，单位为 ms
int64 time_from_start
