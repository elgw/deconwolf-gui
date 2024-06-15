#
#  This is a file to be interpreted by cmake (https://cmake.org/)
#  Please note that all dependencies have to be available first.
#
# Typical Usage
#
#   mkdir build
#   cd build
#   cmake ..
#   cmake --build .
#
#   Either install directly with
#   cmake install
#
#   Or produce a package that can be installed with the
#   system package manager, for example
#   cpack -G RPM
#   cpack -G DEB
#
#
#  On Windows, use:
#
#   mkdir build
#   cd build
#   cmake `-T ClangCL -A x64` ..
#   cmake --build .
#
# Variations/Options
# - On Windows, use
# - To Specify the compiler:
#   CC=clang cmake ..
# - To see command lines:
#   cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ..
# - See what is defined with
#   cmake -L . or cmake --LAH .
#
#
# cmake documentation and tutorials:
# https://cmake.org/cmake/help/latest/
# https://cliutils.gitlab.io/modern-cmake/
#

# TODO:
# Check https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4996?view=msvc-170

cmake_minimum_required(VERSION 3.9)

project(deconwolf-gui
  DESCRIPTION "GUI for deconwolf"
  LANGUAGES C)

set (CMAKE_C_STANDARD 11)

# UNIX, WIN32, WINRT, CYGWIN, APPLE are environment variables
# as flags set by default system
if(UNIX)
  add_definitions(-DUNIX)
elseif(WIN32)
  add_definitions(-DWINDOWS)  
endif()

#
# Default build type is RELEASE
#
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


#
# Add source files
#
add_executable(dw_gui
  src/dw_gui.c
  src/dw_app.c
  src/dw_app_runner.c
  src/dw_app_window.c
  resources.c
  src/dw_channel.c
  src/dw_scope.c
  src/dw_conf.c
  src/dw_file.c
  src/dw_colors.c)


#
# Parse version number from src/version.h
#

file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h DW_GUI_VERSION_MAJOR
  REGEX "#define[ ]+DW_GUI_VERSION_MAJOR[ ]+[0-9]+")
string(REGEX MATCH " ([0-9]+)" _ ${DW_GUI_VERSION_MAJOR})
set(DW_GUI_VERSION_MAJOR "${CMAKE_MATCH_1}")

file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h DW_GUI_VERSION_MINOR
  REGEX "#define[ ]+DW_GUI_VERSION_MINOR[ ]+[0-9]+")
string(REGEX MATCH " ([0-9]+)" _ ${DW_GUI_VERSION_MINOR})
set(DW_GUI_VERSION_MINOR "${CMAKE_MATCH_1}")

file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h DW_GUI_VERSION_PATCH
  REGEX "#define[ ]+DW_GUI_VERSION_PATCH[ ]+[0-9]+")
string(REGEX MATCH "[ ]([0-9]+)" _ ${DW_GUI_VERSION_PATCH})
set(DW_GUI_VERSION_PATCH "${CMAKE_MATCH_1}")

project(deconwolf-gui VERSION "${DW_GUI_VERSION_MAJOR}.${DW_GUI_VERSION_MINOR}.${DW_GUI_VERSION_PATCH}")

#
# Set MSVC runtime to MultiThreaded (/MT)
#

if(WIN32)
  cmake_policy(SET CMP0091 NEW)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

#
# Link time optimization
#

# https://cmake.org/cmake/help/latest/module/CheckIPOSupported.html
include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)
if(result)
  message("Enabling IPO")
  set_property(TARGET dw_gui PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_JOBS 8)
else()
  message(WARNING "IPO is not supported: ${output}")
endif()

#
# Architecture optimizations
#
if(ENABLE_NATIVE_OPTIMIZATION)
  include(CheckCCompilerFlag)
  CHECK_C_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
  if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
  endif()
  CHECK_C_COMPILER_FLAG("-mtune=native" COMPILER_SUPPORTS_MTUNE_NATIVE)
  if(COMPILER_SUPPORTS_MTUNE_NATIVE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mtune=native")
  endif()
endif()


#
# Math library, if needed
#

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
  target_link_libraries(dw_gui ${MATH_LIBRARY})
endif()


#
# GTK3
#
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
target_link_libraries(dw_gui ${GTK4_LIBRARIES})
target_include_directories(dw_gui PRIVATE ${GTK4_INCLUDE_DIRS})
target_link_directories(dw_gui PRIVATE ${GTK4_LIBRARY_DIRS})


#
# Prevent terminal from popping up on windows
#
if(WIN32)
target_link_options(dw_gui PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
# target_link_options(dw_gui PRIVATE -mwindows)
endif()


#
# Compile resources (images)
#

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

set(GRESOURCE_C resources.c)
set(GRESOURCE_XML src/gresources.xml)
add_custom_command(
  OUTPUT "resources.c"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${GLIB_COMPILE_RESOURCES}
  ARGS
  --target=${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
  --generate-source ${GRESOURCE_XML}
  VERBATIM
  MAIN_DEPENDENCY ${GRESOURCE_XML}
  DEPENDS
  resources/deconwolf.png
  )

add_custom_target(
  dummy-resource
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
  )

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
  PROPERTIES GENERATED TRUE
  )

add_dependencies(dw_gui dummy-resource)

# Enable -flto (GCC) and similar if available, see
# https://cmake.org/cmake/help/latest/policy/CMP0069.html
check_ipo_supported(RESULT result)
if(result)
  message("Enabling IPO")
  set_property(DIRECTORY PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

include(GNUInstallDirs)
message("LIBEXECDIR= ${CMAKE_INSTALL_LIBEXECDIR}")
message("BINDIR= ${CMAKE_INSTALL_BINDIR}")


install(TARGETS dw_gui)
install(FILES "${CMAKE_SOURCE_DIR}/resources/deconwolf.desktop"
  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications/" )
install(FILES "${CMAKE_SOURCE_DIR}/resources/deconwolf.png"
  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps/" )


# Install generators
# Debian
# https://cmake.org/cmake/help/latest/cpack_gen/deb.html
# RPM
# https://cmake.org/cmake/help/latest/cpack_gen/rpm.html
# Windows
# https://cmake.org/cmake/help/latest/cpack_gen/nsis.html
# Macos
# https://cmake.org/cmake/help/latest/cpack_gen/bundle.html
#

# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html


SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Erik Wernersson") #required
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON) # resolve dependencies automatically
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.github.com/elgw/deconwolf-gui/")
SET(CPACK_DEBIAN_PACKAGE_SECTION "science")
INCLUDE(CPack)
