cmake_minimum_required(VERSION 3.13)

# Only add tests if there are test sources
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
if(NOT TEST_SOURCES)
  message(STATUS "No test sources found in tests/")
  return()
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(unit_tests ${TEST_SOURCES})
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

# expose path to the pipelines_cpp binary for CLI tests
target_compile_definitions(unit_tests PRIVATE TEST_BIN_PATH=\"${CMAKE_BINARY_DIR}/pipelines_cpp\")

# Link with project core so tests can use global symbols
target_link_libraries(unit_tests PRIVATE pipelines_core GTest::gtest_main Threads::Threads)

# Optionally enable sanitizers for tests
if(ENABLE_SANITIZERS)
  message(STATUS "Enabling sanitizers for unit_tests")
  target_compile_options(unit_tests PRIVATE -fsanitize=address -fsanitize=thread -g)
  target_link_options(unit_tests PRIVATE -fsanitize=address -fsanitize=thread)
endif()

add_test(NAME unit_tests COMMAND unit_tests)
