cmake_minimum_required(VERSION 2.8.12)
project(threadpool11)

# Flags
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

set(threadpool11_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/include
)

set(threadpool11_LIBRARIES
  threadpool11
)

# Definitions
add_definitions(-Dthreadpool11_EXPORTING)

# Include directories
include_directories(
  include
)

# Library
add_library(threadpool11 STATIC
  src/pool.cpp
  src/worker.cpp
  include/threadpool11/pool.hpp
  include/threadpool11/worker.hpp
  include/threadpool11/utility.hpp
)

if (CMAKE_COMPILER_IS_GNUCXX)
  target_link_libraries(threadpool11 pthread)
endif()

if (UNIX)
  install(FILES include/threadpool11/threadpool11.hpp DESTINATION include/threadpool11)
  install(FILES include/threadpool11/worker.hpp DESTINATION include/threadpool11)
  install(FILES include/threadpool11/pool.hpp DESTINATION include/threadpool11)
  install(FILES include/threadpool11/utility.hpp DESTINATION include/threadpool11)
  install(TARGETS threadpool11 DESTINATION lib)
endif()

set(threadpool11_INCLUDE_DIRS ${threadpool11_INCLUDE_DIRS} PARENT_SCOPE)
set(threadpool11_LIBRARIES ${threadpool11_LIBRARIES} PARENT_SCOPE)
