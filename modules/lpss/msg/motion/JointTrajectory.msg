# Header 用于指定轨迹的坐标系和参考时间
Header header

# 轨迹点中活动关节的名称，这些名称是有序的，并且必须与每个 JointTrajectoryPoint 中的值相对应。
string[] joint_names

# 轨迹点数组，描述了每个时间点关节的位置、速度、加速度和/或力矩。
motion/JointTrajectoryPoint[] points
