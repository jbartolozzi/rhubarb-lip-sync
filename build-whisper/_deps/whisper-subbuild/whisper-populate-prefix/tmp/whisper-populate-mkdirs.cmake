# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-whisper/_deps/whisper-src")
  file(MAKE_DIRECTORY "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-whisper/_deps/whisper-src")
endif()
file(MAKE_DIRECTORY
  "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-whisper/_deps/whisper-build"
  "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-whisper/_deps/whisper-subbuild/whisper-populate-prefix"
  "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-whisper/_deps/whisper-subbuild/whisper-populate-prefix/tmp"
  "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-whisper/_deps/whisper-subbuild/whisper-populate-prefix/src/whisper-populate-stamp"
  "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-whisper/_deps/whisper-subbuild/whisper-populate-prefix/src"
  "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-whisper/_deps/whisper-subbuild/whisper-populate-prefix/src/whisper-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-whisper/_deps/whisper-subbuild/whisper-populate-prefix/src/whisper-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-whisper/_deps/whisper-subbuild/whisper-populate-prefix/src/whisper-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
