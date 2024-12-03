# ----------------------------------------------------------------------------
#   Eigen3
# ----------------------------------------------------------------------------

project(
  Eigen3
  LANGUAGES CXX
)

option(BUILD_TESTING "Build the testing tree." OFF)

rmvl_download(Eigen3 GIT "https://gitlab.com/libeigen/eigen.git : 3.4.0")
set(EIGEN3_VERSION_STRING "3.4.0" CACHE INTERNAL "Eigen3 version")
