#ifndef _RMM_H_
#define _RMM_H_

#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <vector>
#include "udp_utils.h"
#include "rx_queue.h"


typedef struct {
    const char sernum;
    const char modnum;
}rmm_info_t;
struct Rmm {
    Rmm();
    ~Rmm();

    void initialize();
    bool search();
    void download(const char* filename);
    void read_contents();
    void print_files();
    bool tasks_pending();
    void wait_for_ready();
    void stop_all();
    bool is_ready();
    void wait_for_threads();
    q_elem_t* wait_for_rx();

    void _send_jumbo_zeros();
    void _start();
    bool _get_identity();
    bool _parse_identity_response(const q_elem_t* buf);


    bool _is_ready;
    bool _stop;
    std::vector<Receiver*> _rx;
    RxQueue* _rxq;


    std::string _serial_number;
    std::string _model_number;


};

#endif