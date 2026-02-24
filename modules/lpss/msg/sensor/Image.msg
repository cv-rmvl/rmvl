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

int32 height # image height, that is, number of rows
int32 width  # image width, that is, number of columns

uint8 encoding_rgb8 = 0
uint8 encoding_bgr8 = 1
uint8 encoding_mono8 = 2
uint8 encoding_mono16 = 3
uint8 encoding_rgba8 = 4
uint8 encoding_bgra8 = 5
uint8 encoding_bayer_rggb8 = 6
uint8 encoding_bayer_bggr8 = 7
uint8 encoding_bayer_rggb16 = 8
uint8 encoding_bayer_bggbr16 = 9
uint8 encoding_yuv422 = 10
uint8 encoding_yuv420 = 11

uint8 encoding # Encoding of pixels -- channel meaning, ordering
               # contain "rgb8", "bgr8", "mono8", "mono16", "rgba8",
               # "bgra8", "bayer_rggb8", "bayer_bggr8",
               # "bayer_rggb16", "bayer_bggbr16", "yuv422", "yuv420"
uint8[] data   # actual matrix data