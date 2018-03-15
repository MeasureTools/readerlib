cmake_minimum_required(VERSION 3.8)

enable_testing()
project (readerlib VERSION 0.1.0)

set (LIBNAME "readerlib")
set (STATIC_LIBNAME "readerlib_s")

# Set compiler options
set (CMAKE_C_STANDARD 11)
set (CMAKE_C_COMPILER             "/usr/bin/clang")
set (CMAKE_C_FLAGS                "-fPIC -Weverything -pthread")
set (CMAKE_C_FLAGS_DEBUG          "-g")
set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_COMPILER             "/usr/bin/clang++")
set (CMAKE_CXX_FLAGS                "-fPIC -Weverything -Wno-unused-parameter -Wno-weak-vtables -Wno-c++98-compat -Wno-padded -Wno-covered-switch-default -Wno-vla -Wno-vla-extension -pthread")
set (CMAKE_CXX_FLAGS_DEBUG          "-g -fsanitize=undefined -fsanitize=address -fsanitize=leak")
set (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

set (CMAKE_SHARED_LINKER_FLAGS "-v -lboost_system")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREAD ON)
find_package(Boost 1.65.0 REQUIRED COMPONENTS system filesystem regex)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

add_subdirectory (src)
add_subdirectory (test)
