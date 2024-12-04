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
#include "rmm.h"
#include "rx_queue.h"
#include "udp_utils.h"
#include "spdlog/spdlog.h"

std::vector<std::string> file_names;
std::string download_path;

using rmm_t = Rmm<UdpTxRx>;

rmm_t* _rmm = nullptr;

typedef struct {
    bool help;
    bool list;
    bool all;
    std::string file;
    std::string dest;
} app_args_t;

app_args_t _args;

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cerr << "Caught Ctrl+C! Exiting gracefully..." << std::endl;
        _rmm->stop_all();
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
        }
        else if (arg == "--dest") {
            _args.dest = argv[++i];
        } else {
            std::cerr << "Invalid argument: " << arg << std::endl;
        }
    }
}

void print_usage(void) {
    printf("Usage TODO ...\n");
}

int main(int argc, char** argv) {
    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signal_handler);

    spdlog::set_level(spdlog::level::debug);
    // spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    // spdlog::set_pattern("[%H:%M:%S %z] [%^%L%$] [%&] %v");
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

    spdlog::debug("Starting UDP RMM Downloader");
    parse_arguments(argc, argv);

    
    if (_args.help) {
        print_usage();
        return 0;
    }

    rmm_t rmm;
    _rmm = &rmm; // store for graceful exit

    rmm.wait_for_ready();
    bool rmm_found = rmm.search();

    if (rmm_found) {
        spdlog::info("SerialNumber: %s", rmm._serial_number.c_str());
        spdlog::info("ModelNumber: %s", rmm._model_number.c_str());
    } else {
        spdlog::error("ERROR: RMM was not found");
        exit(1);
    }

    if (_args.list) {
        rmm.read_contents();
        rmm.print_files();
    }
    else if (_args.all && _args.dest != "") {

    }
    else if (_args.file != "" && _args.dest != "") {
        rmm.download(_args.file);
    }
    else {
        spdlog::error("Invalid arguments\n");
    }

    while( rmm.tasks_pending() ) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    rmm.stop_all();
    rmm.wait_for_threads();
    
    spdlog::info("Exiting\n");
    return 0;
}
