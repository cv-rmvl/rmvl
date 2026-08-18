# Pose of child_frame_id in header.frame_id at header.stamp.
# Twist is the velocity of child_frame_id expressed in child_frame_id.
# A common frame pair is odom -> base_link.
Header header
string child_frame_id
geometry/PoseWithCovariance pose
geometry/TwistWithCovariance twist
