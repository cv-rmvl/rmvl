                                        3rdparty modules
This folder contains third-party libraries for popular vision modules used in the extra modules,
including some common visual tag localization and decoding.

On Linux or Windows, all libraries are automatically detected by CMake scripts. To use these versions
of the libraries instead of the system ones, the BUILD_<library_name> CMake flags should be used (e.g.,
BUILD_APRILTAG for the apriltag library).

-------------------------------------------------------------------------------------------------------
apriltag            Description     A visual fiducial system for the localization of robotic systems
                    License         apriltag is covered by the BSD 2-Clause License, see
                                    apriltag/LICENSE.md
                    Homepage        https://april.eecs.umich.edu/software/apriltag
                    CMake options   1. BUILD_APRILTAG to build this module (enabled by default)
                                    2. WITH_APRILTAG to enable apriltag support for the tag_detector
                                       module

open62541           Description     An open-source C++ implementation of OPC UA
                    License         open62541 is covered by the MPL-2.0 license, see open62541/LICENSE
                    Homepage        open62541.org
                    CMake options   1. BUILD_OPEN62541 to download and build this module (disabled by
                                       default)
                                    2. WITH_OPEN62541 to enable open62541 support for the opcua module
                                    