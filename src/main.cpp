#include <stdio.h>
#include <csignal>
#include <atomic>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <ctime>
#include <iostream>

std::atomic<bool> running(true);
std::vector<std::string> file_names;
std::string download_path;

typedef struct {
    bool help;
    bool list;
    bool all;
    std::string file;
} app_args_t;

app_args_t _args;

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cerr << "Caught Ctrl+C! Exiting gracefully..." << std::endl;
        running = false;
    }
}

void initilialize_args() {
    _args.list = false;
    _args.all = false;
    _args.file = "";
}
// Function to parse command-line arguments and process the logic
void parse_arguments(int argc, char** argv) {
    initilialize_args();
    if (argc < 2) {
        std::cerr << "Invalid arguments" << std::endl;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            _args.help = true;
        }
        else if (arg == "--list") {
            _args.list = true;
        }
        else if (arg == "--all") {
            _args.all = true;
        }
        else if (arg == "--file") {
            _args.file = argv[++i];
        } else {
            std::cerr << "Invalid argument: " << arg << std::endl;
        }
    }
}

void print_usage(void) {
    printf("Usage TODO ...\n");
}

#include <udp_utils.h>
int main(int argc, char** argv) {
    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signal_handler);

    printf("Starting UDP RMM Downloader\n");
    parse_arguments(argc, argv);

    if (_args.help) {
        print_usage();
        return 0;
    }

    RxQueue rxq(16);
    Receiver rx(1234, &rxq);
    rx.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    rx.stop();

    // if (_args.list) {
    //     rmm.read_contents()
    //     rmm.print_files()
    // }



    return 0;
}
