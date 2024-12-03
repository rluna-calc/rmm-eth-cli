
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# Enable testing
enable_testing()

# Add test executable
add_executable(
    test_downloader
    downloader_test.cc
 )

#  target_include_directories(
#     test_downloader
#     PRIVATE ../inc/
# )


target_link_libraries(
  test_downloader
  gtest
  gtest_main
  # GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_downloader)

# Add the test to CTest
# add_test(NAME DownloaderTest COMMAND test_downloader)
