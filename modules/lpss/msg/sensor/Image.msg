# This message contains an uncompressed image
# (0, 0) is at top-left corner of image

Header header # Header timestamp should be acquisition time of image
              # Header frame_id should be optical frame of camera
              # origin of frame should be optical center of camera
              # +x should point to the right in the image
              # +y should point down in the image
              # +z should point into to plane of the image
              # If the frame_id here and the frame_id of the CameraInfo
              # message associated with the image conflict
              # the behavior is undefined

uint32 height # image height, that is, number of rows
uint32 width  # image width, that is, number of columns

string encoding # Encoding of pixels -- channel meaning, ordering
                  # contain "rgb8", "bgr8", "mono8", "mono16", "rgba8",
                  # "bgra8", "bayer_rggb8", "bayer_bggr8",
                  # "bayer_rggb16", "bayer_bggbr16", "yuv422", "yuv420"
uint8[] data    # actual matrix data, size is (step * rows)