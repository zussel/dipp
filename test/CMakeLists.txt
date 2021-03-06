include(FetchContent)

FetchContent_Declare(gtest
  QUIET
  URL https://github.com/google/googletest/archive/release-1.11.0.tar.gz
)
# configure build of googletest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gtest)

add_executable(unit_tests tests.cpp)

include_directories("../include")

target_link_libraries(unit_tests gtest gtest_main)

# automatic discovery of unit tests
include(GoogleTest)
gtest_discover_tests(unit_tests
  PROPERTIES
  LABELS "dipp"
  DISCOVERY_TIMEOUT  # how long to wait (in seconds) before crashing
    240
  )