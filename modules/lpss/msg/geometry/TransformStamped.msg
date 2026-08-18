# 此消息表示 child_frame_id 在 header.frame_id 中的位姿，时间戳为 header.stamp
# 其变换方向记为 T_parent_child，可将 child_frame_id 中的坐标映射到 header.frame_id

# Header 中的 frame id 用作此变换的参考坐标系
Header header

# 此变换指向的子坐标系的 frame id
string child_frame_id

# child_frame_id 相对于 header.frame_id 的三维平移和旋转
geometry/Transform transform
