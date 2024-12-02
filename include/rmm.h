#include <stdint.h>
#include <stdbool.h>
#include <udp_utils.h>
#include <vector>

// A simple UDP receiver class to listen for incoming packets
struct Rmm {
    Rmm();
    ~Rmm();

    void initialize();
    void search();
    void download(const char* filename);
    void read_contents();
    void print_files();
    bool tasks_pending();
    void wait_for_ready();
    void stop_all();
    bool is_ready();
    void wait_for_threads();

    void _start();

    bool _is_ready;
    bool _stop;
    std::vector<Receiver*> _rx;
};
