# 此处表示从坐标系 header.frame_id，转换至坐标系 child_frame_id 时，时间戳为 header.stamp
# 除 header.frame_id 外，还需在报头中添加 child_frame_id 以在独立消息中完整传递变换的引用信息。

# Header 中的 frame id 用作此变换的参考坐标系
Header header

# 此变换指向的子坐标系的 frame id
string child_frame_id

# 从 header.frame_id 到 child_frame_id 的三维平移和旋转，也即 child_frame_id 相对于 header.frame_id 的变换
geometry/Transform transform
