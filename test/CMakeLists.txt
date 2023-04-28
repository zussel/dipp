include(FetchContent)

FetchContent_Declare(gtest
  QUIET
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        e2239ee6043f73722e7aa812a459f54a28552929 # release-1.11.0
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