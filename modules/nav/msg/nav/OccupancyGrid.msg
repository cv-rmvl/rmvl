# Full row-major occupancy grid: index = y * info.width + x.
# data values: -1 unknown, 0 free, 100 occupied, intermediate values are
# probabilities or costs. data.size() must equal info.width * info.height.
Header header
nav/MapMetaData info
uint64 revision
int8[] data
