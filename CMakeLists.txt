cmake_minimum_required(VERSION 3.12)

file(READ "${CMAKE_SOURCE_DIR}/VERSION" VER_RAW)
string(STRIP ${VER_RAW} VERSION)

project(
  hyprpicker
  DESCRIPTION "A blazing fast wayland wallpaper utility"
  VERSION ${VERSION})

set(CMAKE_MESSAGE_LOG_LEVEL "STATUS")

message(STATUS "Configuring hyprpicker!")

# Get git info hash and branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
  COMMAND bash -c "git show ${GIT_COMMIT_HASH} | head -n 5 | tail -n 1"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_MESSAGE
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
  COMMAND bash -c "git diff-index --quiet HEAD -- || echo \"dirty\""
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_DIRTY
  OUTPUT_STRIP_TRAILING_WHITESPACE)
#

find_program(WaylandScanner NAMES wayland-scanner)
message(STATUS "Found WaylandScanner at ${WaylandScanner}")
execute_process(
  COMMAND pkg-config --variable=pkgdatadir wayland-protocols
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE WAYLAND_PROTOCOLS_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Found wayland-protocols at ${WAYLAND_PROTOCOLS_DIR}")

function(protocol protoPath protoName external)
  if(external)
    execute_process(
      COMMAND ${WaylandScanner} client-header ${protoPath}
              ${protoName}-protocol.h WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    execute_process(
      COMMAND ${WaylandScanner} private-code ${protoPath}
              ${protoName}-protocol.c WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    # target_sources(hyprpicker PRIVATE ${protoName}-protocol.h)
    target_sources(hyprpicker PRIVATE ${protoName}-protocol.h
                                      ${protoName}-protocol.c)
  else()
    execute_process(
      COMMAND ${WaylandScanner} client-header
              ${WAYLAND_PROTOCOLS_DIR}/${protoPath} ${protoName}-protocol.h
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    execute_process(
      COMMAND ${WaylandScanner} private-code
              ${WAYLAND_PROTOCOLS_DIR}/${protoPath} ${protoName}-protocol.c
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    target_sources(hyprpicker PRIVATE ${protoName}-protocol.h
                                      ${protoName}-protocol.c)
  endif()
endfunction()

include_directories(.)
set(CMAKE_CXX_STANDARD 23)
add_compile_options(-DWLR_USE_UNSTABLE)
add_compile_options(
  -Wall
  -Wextra
  -Wno-unused-parameter
  -Wno-unused-value
  -Wno-missing-field-initializers
  -Wno-narrowing
  -Wno-pointer-arith)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(
  deps
  REQUIRED
  IMPORTED_TARGET
  wayland-client
  wayland-protocols
  xkbcommon
  cairo
  pango
  pangocairo
  libjpeg
  hyprutils>=0.2.0)

file(GLOB_RECURSE SRCFILES "src/*.cpp")

add_executable(hyprpicker ${SRCFILES})

protocol("protocols/wlr-layer-shell-unstable-v1.xml"
         "wlr-layer-shell-unstable-v1" true)
protocol("protocols/wlr-screencopy-unstable-v1.xml"
         "wlr-screencopy-unstable-v1" true)
protocol("stable/xdg-shell/xdg-shell.xml" "xdg-shell" false)
protocol("staging/cursor-shape/cursor-shape-v1.xml" "wp-cursor-shape-v1" false)
protocol("unstable/tablet/tablet-unstable-v2.xml" "tablet-unstable-v2" false)

target_compile_definitions(hyprpicker
                           PRIVATE "-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
target_compile_definitions(hyprpicker PRIVATE "-DGIT_BRANCH=\"${GIT_BRANCH}\"")
target_compile_definitions(
  hyprpicker PRIVATE "-DGIT_COMMIT_MESSAGE=\"${GIT_COMMIT_MESSAGE}\"")
target_compile_definitions(hyprpicker PRIVATE "-DGIT_DIRTY=\"${GIT_DIRTY}\"")

target_link_libraries(hyprpicker rt)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(hyprpicker PkgConfig::deps)

target_link_libraries(hyprpicker pthread ${CMAKE_THREAD_LIBS_INIT}
                      wayland-cursor)

if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -no-pie -fno-builtin")
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} -pg -no-pie -fno-builtin")
  set(CMAKE_SHARED_LINKER_FLAGS
      "${CMAKE_SHARED_LINKER_FLAGS} -pg -no-pie -fno-builtin")
endif(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)

install(TARGETS hyprpicker)
install(FILES ${CMAKE_SOURCE_DIR}/doc/hyprpicker.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
