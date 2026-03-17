# Install script for directory: /Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-python/_deps/googletest-build/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-python/_deps/pybind11-build/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "python" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/rhubarb" TYPE MODULE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-python/rhubarb/_rhubarb.cpython-312-darwin.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/rhubarb/_rhubarb.cpython-312-darwin.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/rhubarb/_rhubarb.cpython-312-darwin.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" -x "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/rhubarb/_rhubarb.cpython-312-darwin.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "python" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/rhubarb/res/sphinx" TYPE DIRECTORY FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/pocketsphinx-rev13216/model/en-us/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "python" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/rhubarb/res/sphinx/acoustic-model" TYPE DIRECTORY FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/cmusphinx-en-us-5.2/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/pocketsphinx-rev13216/model/en-us/cmudict-en-us.dict")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/pocketsphinx-rev13216/model/en-us/en-us-phone.lm.bin")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/pocketsphinx-rev13216/model/en-us/en-us.lm.bin")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx/acoustic-model" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/cmusphinx-en-us-5.2/README")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx/acoustic-model" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/cmusphinx-en-us-5.2/feat.params")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx/acoustic-model" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/cmusphinx-en-us-5.2/feature_transform")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx/acoustic-model" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/cmusphinx-en-us-5.2/mdef")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx/acoustic-model" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/cmusphinx-en-us-5.2/means")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx/acoustic-model" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/cmusphinx-en-us-5.2/mixture_weights")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx/acoustic-model" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/cmusphinx-en-us-5.2/noisedict")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx/acoustic-model" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/cmusphinx-en-us-5.2/transition_matrices")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/res/sphinx/acoustic-model" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/lib/cmusphinx-en-us-5.2/variances")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/README.adoc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-flac-ffmpeg.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-float32-audacity.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-float32-audition.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-float32-ffmpeg.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-float32-soundforge.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-float64-ffmpeg.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-int16-audacity.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-int16-audition.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-int16-ffmpeg.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-int16-soundforge.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-int24-audacity.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-int24-audition.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-int24-ffmpeg.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-int24-soundforge.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-int32-ffmpeg.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-int32-soundforge.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-uint8-audition.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-uint8-ffmpeg.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-uint8-soundforge.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tests/resources" TYPE FILE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/rhubarb/tests/resources/sine-triangle-vorbis-ffmpeg.wav")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE EXECUTABLE FILES "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-python/rhubarb/rhubarb")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./rhubarb" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./rhubarb")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" -u -r "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./rhubarb")
    endif()
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/jhb/Documents/GitHub/rhubarb-lip-sync/build-python/rhubarb/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
