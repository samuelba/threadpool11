cmake_minimum_required(VERSION 2.8.12)
project(threadpool11_demo)

include_directories(
  ${threadpool11_INCLUDE_DIRS}
)

add_executable(threadpool11_demo
  src/main.cpp
)
add_executable(threadpool11_tests
  src/tests.cpp
)
add_executable(threadpool11_openmp
  src/openmp.cpp
)

set_target_properties(threadpool11_openmp PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fopenmp"
                                                     LINK_FLAGS "${LINK_FLAGS} -fopenmp")

target_link_libraries(threadpool11_demo
  ${threadpool11_LIBRARIES}
)
target_link_libraries(threadpool11_tests
  ${threadpool11_LIBRARIES}
)
target_link_libraries(threadpool11_openmp
  ${threadpool11_LIBRARIES}
)
