# src/CMakeLists.txt

# Create the executable or library
add_executable(
    downloader
    main.cpp
    udp_utils.cpp
    rx_queue.cpp
    print_utils.cpp
)

# Link libraries or other dependencies (if needed)
# target_link_libraries(downloader some_library)
