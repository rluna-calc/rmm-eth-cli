#ifndef _RMM_H_
#define _RMM_H_

#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include "udp_utils.h"
#include "rx_queue.h"
#include "print_utils.h"

const char* TX_IP = "192.168.0.255";
constexpr uint32_t PORT_252 = 252;
constexpr uint32_t PORT_253 = 253;
constexpr uint32_t PORT_BROADCAST = 255;
constexpr uint32_t DISCOVER_SIZE = 8548;


const uint8_t IDENTITY_MSG[] = {0x00,0x05,0x00,0x01,0x00,
                                0x00,0x00,0x00,0x00,0x00};

constexpr uint32_t BUFFER_POOL_SIZE = 32;

using namespace std;


typedef struct {
    const char sernum;
    const char modnum;
}rmm_info_t;

template <
    typename UDP

>
struct Rmm {
    Rmm() : _is_ready(false), _stop(false) {
        _serial_number = "";
        _model_number = "";

        _rxq = new RxQueue(BUFFER_POOL_SIZE);

        _rx.push_back(new UDP(PORT_252, _rxq));
        _rx.push_back(new UDP(PORT_BROADCAST, _rxq));

        _start();
    }

    ~Rmm() {
        stop_all();

        for (size_t i = 0; i < _rx.size(); i++) {
            delete _rx[i];
        }

        delete _rxq;
    }
    
    bool search() {
        // _send_jumbo_zeros();
        return _get_identity();
    }

    void download(const char* filename) {
        std::cout << "Downloading file: " << filename << std::endl;
        // Simulate downloading a file (this could be expanded)
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Download complete for: " << filename << std::endl;
    }

    void read_contents() {
        // // Simulate reading file contents
        // file_names = {"File1", "File2", "File3"};
    }

    void print_files() {
        // std::cout << "Files on RMM:" << std::endl;
        // for (const auto& file : file_names) {
        //     std::cout << "  " << file << std::endl;
        // }
    }

    bool tasks_pending() {
        //TODO
        // ret = False
        // if self.dlt is not None:
        //     ret = not self.dlt.is_stopped()

        // return ret
        return false; // Simulate that no tasks are pending
    }

    void wait_for_ready() {
        bool ready = false;
        while (!ready) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            ready = is_ready();
        }
    }


    void stop_all() {
        //TODO: dlt

        for (size_t i = 0; i < _rx.size(); i++) {
            _rx[i]->stop();
        }
    }

    bool is_ready() {
        bool ret = true;
        for (size_t i = 0; i < _rx.size(); i++) {
            ret &= _rx[i]->is_running();
        }

        return ret;
    }

    void wait_for_threads() {
            //TODO
            // if self.dlt is not None:
            //     self.dlt.thread_dl.join()
            //     self.dlt.thread_write.join()

            for (size_t i = 0; i < _rx.size(); i++) {
                if(_rx[i]->_th.joinable()) {
                    _rx[i]->_th.join();
                }
            }
        }

        q_elem_t* wait_for_rx() {
        q_elem_t* elem = nullptr;
        
        for (int i = 0; i < 10; i++) {
            elem = _rxq->get();
            if (elem) {
                break;
            }

            this_thread::sleep_for(chrono::milliseconds(100));
        }

        return elem;
    }


    void _send_jumbo_zeros() {
        uint8_t buf[DISCOVER_SIZE];
        memset(buf, 0, DISCOVER_SIZE);
        UDP::send_udp_packet(TX_IP, PORT_253, buf, DISCOVER_SIZE);
    }

    void _start() {
        for (size_t i = 0; i < _rx.size(); i++) {
            _rx[i]->start();
        }
    }

    bool _get_identity() {
        const uint8_t* buf = IDENTITY_MSG;
        printf("Sending discovery message...\n");
        UDP::send_udp_packet(TX_IP, PORT_BROADCAST, buf, sizeof(IDENTITY_MSG));

        const q_elem_t* resp = wait_for_rx();
        return _parse_identity_response(resp);
    }

    bool _parse_identity_response(const q_elem_t* resp) {
        if (!resp) {
            return false;
        }

        printf("Ready to parse response...%d bytes\n", resp->len);
        print_buf(resp->buf, resp->len);

        // Extract serial number (bytes 20-40)
        std::vector<uint8_t> serial_number_bytes(&resp->buf[20], &resp->buf[40]);
        // Find the null byte and trim
        auto null_pos = std::find(serial_number_bytes.begin(), serial_number_bytes.end(), 0);
        serial_number_bytes.resize(null_pos - serial_number_bytes.begin());

        // Convert to string and trim leading/trailing whitespace
        _serial_number = _trim(std::string(serial_number_bytes.begin(), serial_number_bytes.end()));

        return true;
    }

    // Helper function to trim leading/trailing whitespaces
    std::string _trim(const std::string& str) {
        const auto first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return ""; // No content
        const auto last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, last - first + 1);
    }

    bool _is_ready;
    bool _stop;
    std::vector<UDP*> _rx;
    RxQueue* _rxq;

    std::string _serial_number;
    std::string _model_number;
};

#endif