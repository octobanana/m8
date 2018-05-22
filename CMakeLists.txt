cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

set (TARGET m8)
project (${TARGET})

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (DEBUG_FLAGS "-Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused -std=c++17 -g")
set (RELEASE_FLAGS "-std=c++17 -s -O3")
set (GCOV_FLAGS "-fprofile-arcs -ftest-coverage")

# set (CMAKE_CXX_FLAGS "")
# set (CMAKE_C_FLAGS "")

set (CMAKE_CXX_FLAGS_DEBUG ${DEBUG_FLAGS})
set (CMAKE_C_FLAGS_DEBUG ${DEBUG_FLAGS})
set (CMAKE_CXX_FLAGS_RELEASE ${RELEASE_FLAGS})
set (CMAKE_C_FLAGS_RELEASE ${RELEASE_FLAGS})
set (CMAKE_EXE_LINKER_FLAGS_DEBUG ${GCOV_FLAGS})

message ("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")

include_directories(
  ./src
  ./
)

set (SOURCES
  src/main.cc
  src/m8.cc
  src/ast.cc
  src/reader.cc
  src/writer.cc
  src/lexer.cc
  src/parser.cc
  src/m8_macros.cc
  src/user_macros.cc
  src/sys_command.cc
  src/http.cc
  src/crypto.cc
  src/string.cc
  src/ansi_escape_codes.cc
)

set (HEADERS
)

add_executable (
  ${TARGET}
  ${SOURCES}
  ${HEADERS}
)

target_link_libraries (
  ${TARGET}
  pthread
  stdc++fs
  crypto
  curl
  # boost_serialization
)

install (TARGETS ${TARGET} DESTINATION "/usr/local/bin")
