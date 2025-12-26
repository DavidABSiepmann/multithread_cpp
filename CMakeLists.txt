cmake_minimum_required(VERSION 3.13)
project(pipelines_cpp)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project layout
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)

# Collect source files from src/
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
if(NOT SOURCES)
  message(WARNING "No source files found in ${SRC_DIR}. Add sources or update CMakeLists.txt")
endif()

# Separate core library (sources without `main.cpp`) and executable
set(MAIN_SRC ${SRC_DIR}/main.cpp)
list(REMOVE_ITEM SOURCES ${MAIN_SRC})
add_library(pipelines_core STATIC ${SOURCES})
target_include_directories(pipelines_core PUBLIC ${INC_DIR})

add_executable(pipelines_cpp ${MAIN_SRC})
target_include_directories(pipelines_cpp PRIVATE ${INC_DIR})

find_package(Threads REQUIRED)
target_link_libraries(pipelines_cpp PRIVATE pipelines_core ${CMAKE_THREAD_LIBS_INIT})

# Tests: add `tests` subdirectory (it will fetch GoogleTest and build tests when present)
option(ENABLE_SANITIZERS "DEPRECATED: Use ENABLE_ASAN or ENABLE_TSAN instead" OFF)
option(ENABLE_ASAN "Enable Address Sanitizer (memory/leak detection)" OFF)
option(ENABLE_TSAN "Enable Thread Sanitizer (race condition detection)" OFF)
enable_testing()
# add tests subdir (tests/CMakeLists.txt will handle fetching GoogleTest)
add_subdirectory(tests)


# Bench helper target: runs bench script to produce CSVs and graphs
add_custom_target(run_bench
  COMMAND ${CMAKE_COMMAND} -E echo "Running bench script..."
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ${CMAKE_COMMAND} -E env bash ${CMAKE_SOURCE_DIR}/scripts/run_bench.sh
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Run benchmarks and generate outputs"
)

# Install target
install(TARGETS pipelines_cpp RUNTIME DESTINATION bin)
