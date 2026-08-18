# All poses must use the same frame as header.frame_id.
# header.stamp is the path generation time; pose stamps may be zero when the
# path is not time-parameterized.
Header header
geometry/PoseStamped[] poses
