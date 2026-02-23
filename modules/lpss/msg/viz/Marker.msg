uint8 TYPE_ARROW = 0
uint8 TYPE_CUBE = 1
uint8 TYPE_SPHERE = 2
uint8 TYPE_CYLINDER = 3
uint8 TYPE_LINE_STRIP = 4
uint8 TYPE_LINE_LIST = 5
uint8 TYPE_CUBE_LIST = 6
uint8 TYPE_SPHERE_LIST = 7
uint8 TYPE_POINTS = 8

uint8 ACTION_ADD = 0
uint8 ACTION_DELETE = 1
uint8 ACTION_DELETEALL = 2

Header header          # 提供时间戳和坐标系的头部信息
string ns              # 用于放置该对象的命名空间，为对象创建一个唯一的名称，与 id 配合使用
int32 id               # 对象 ID，用于指定操作和删除对象，与 ns 配合使用
uint8 type             # 对象类型
uint8 action           # 0 添加/修改对象，1 删除对象，2 删除所有对象，指定为删除对象时，type 等字段将被忽略
geometry/Pose pose     # Pose of the object 对象的姿态
geometry/Vector3 scale # 对象的缩放，1, 1, 1 表示默认大小
ColorRGBA color        # 颜色 [0.0-1.0]

# 只有当指定的类型对它们有用时才使用（例如 POINTS、LINE_STRIP、LINE_LIST）。对于其他类型，这些字段将被忽略。
geometry/Point[] points
ColorRGBA[] colors
