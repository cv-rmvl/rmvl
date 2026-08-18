# Rectangular row-major update for an occupancy grid.
# The rectangle must be inside the target map, data.size() must equal
# width * height, and base_revision must equal the target's current revision.
Header header
uint64 base_revision
uint64 revision
uint32 x
uint32 y
uint32 width
uint32 height
int8[] data
