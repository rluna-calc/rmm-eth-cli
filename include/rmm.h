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

const uint8_t REQ_BLOCK_MSG[] = {0x00,0x03,0x00,0x01,0x00,
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
        bool stop = false;
        int32_t request_val = 1;

        vector<string> new_files;
        while (!stop) {
            request_val = _request_content_block(request_val, new_files);
            if (request_val == 0xff) {
                stop = true;
            }

            //TODO: files extend
        }    
    
        //TODO: save file list to member variable
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

    uint32_t _request_content_block(int32_t value, vector<string>& new_files) {
        uint8_t buf[sizeof(REQ_BLOCK_MSG)];
        uint32_t buf_len = sizeof(REQ_BLOCK_MSG);

        memcpy(buf, REQ_BLOCK_MSG, buf_len);
        buf[buf_len-1] = value;

        UDP::send_udp_packet(TX_IP, PORT_252, buf, buf_len);

        const q_elem_t* resp = wait_for_rx();

        new_files = _parse_content_block(resp);
        if (new_files.size() > 0) {
            value += 1;
        } else {
            value = 0xff;
        }

        return value;
    }

    vector<string> _parse_content_block(const q_elem_t* resp) {
        vector<string> new_files;


        return new_files;
    }

    bool _get_identity() {
        const uint8_t* buf = IDENTITY_MSG;
        // printf("Sending discovery message...\n");
        UDP::send_udp_packet(TX_IP, PORT_BROADCAST, buf, sizeof(IDENTITY_MSG));

        const q_elem_t* resp = wait_for_rx();
        return _parse_identity_response(resp);
    }

    bool _parse_identity_response(const q_elem_t* resp) {
        if (!resp) {
            return false;
        }

        // Extract serial number (bytes 20-40)
        _serial_number = _trim_end((char*) &resp->buf[50], 25);
        _model_number = _trim_end((char*) &resp->buf[94], 40);
        
        return true;
    }

    // Helper function to trim leading/trailing whitespaces
    const char* _trim_end(char* p, uint32_t max_len) {
        char* ret = nullptr;
        
        enum {
            INITIAL,
            IN_FIRST_NULL,
            IN_TEXT,
            FOUND_LAST_NULL,
            FINISHED,
        } state;

        state = INITIAL;

        for (uint32_t i = 0; i < max_len; i++) {
            switch(state) {
                case INITIAL: 
                    state = ((*p == 0) || (*p == 0x20)) ? IN_FIRST_NULL : INITIAL;
                    break;
                case IN_FIRST_NULL: 
                    state = ((*p == 0) || (*p == 0x20)) ? IN_FIRST_NULL : IN_TEXT;
                    break;
                case IN_TEXT: 
                    state = (*p == 0) ? FINISHED : IN_TEXT;
                    break;
                default:
                    break;
            }

            if(state == FINISHED) {
                ret = ++p;
                break;
            }

            p--; 
        }

        return (const char*) ret;
    }

    bool _is_ready;
    bool _stop;
    std::vector<UDP*> _rx;
    RxQueue* _rxq;

    std::string _serial_number;
    std::string _model_number;
};

#endif
