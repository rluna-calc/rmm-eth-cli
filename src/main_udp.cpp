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
#include "rx_queue.h"
#include "udp_utils.h"
#include "print_utils.h"
#include "time_utils.h"

typedef struct {
    bool help;
    std::string tx;
    uint32_t port;
    bool rx;
    std::string message;
    bool hex;
} app_args_t;

app_args_t _args;
bool g_stop = false;

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cerr << "Caught Ctrl+C! Exiting gracefully..." << std::endl;
        g_stop = true;
    }
}

void initilialize_args() {
    _args.tx = "";
    _args.port = 0;
    _args.rx = false;
    _args.message = "";
    _args.hex = false;
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
        else if (arg == "--tx") {
            _args.tx = argv[++i];
        }
        else if (arg == "--port") {
            _args.port = atoi(argv[++i]);
        }
        else if (arg == "--rx") {
            _args.rx = true;
        }
        else if (arg == "--message") {
            _args.message = argv[++i];
        }
        else if (arg == "--hex") {
            _args.hex = true;
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

    // printf("Starting UDP RMM Downloader\n");
    parse_arguments(argc, argv);

    if (_args.help) {
        print_usage();
        return 0;
    }

    if (_args.tx != "" && _args.port > 0) {
        uint8_t tx_buf[] = "Once upon a midnight dreary, while I pondered, weak and weary,";
        UdpTxRx::send_udp_packet(_args.tx.c_str(), _args.port, tx_buf, sizeof(tx_buf));
    }
    else if (_args.rx && _args.port > 0) {
        RxQueue rxq(32);
        UdpTxRx rx(_args.port, &rxq);
        rx.start();
        q_elem_t* elem;
        printf("Listening on port %d\n", _args.port);
        while( !g_stop ) {
            elem = rxq.get(200);
            if( elem ) {
                // print_buf(elem->buf, elem->len);
                printf("%lu: %d bytes\n", time_since_epoch_microsecs(), elem->len);
            }
        }

        printf("Stopping...\n");
        rx.stop();
        if (rx._th.joinable()) { rx._th.join(); }
    }
    else {
        printf("Invalid arguments\n");
    }

    
    printf("Exiting\n");
    return 0;
}
