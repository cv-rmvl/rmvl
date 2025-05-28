# ----------------------------------------------------------------------------
#   pybind11
# ----------------------------------------------------------------------------

project(
  pybind11
  LANGUAGES C
)

set(ver "2.13.5")

rmvl_download(pybind11 GIT "https://github.com/pybind/pybind11.git@v${ver}")
set(pybind11_VERSION "${ver}" CACHE INTERNAL "pybind11 version")
