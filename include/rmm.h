#ifndef _RMM_H_
#define _RMM_H_

#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>

#include <iostream>
#include <iomanip>      // For std::setlocale
#include <locale>       // For std::locale, std::use_facet
#include <sstream>      // For std::stringstream

#include "udp_utils.h"
#include "rx_queue.h"
#include "print_utils.h"
#include "download_tracker.h"

const char* TX_IP = "192.168.0.255";
constexpr uint32_t PORT_252 = 252;
constexpr uint32_t PORT_253 = 253;
constexpr uint32_t PORT_BROADCAST = 255;
constexpr uint32_t DISCOVER_SIZE = 8548;

constexpr uint32_t REQUEST_LEN = 10; // bytes
constexpr uint8_t IDENTITY_MSG[REQUEST_LEN] = {0x00,0x05,0x00,0x01,0x00,
                                               0x00,0x00,0x00,0x00,0x00};

constexpr uint8_t REQ_BLOCK_MSG[REQUEST_LEN] = {0x00,0x03,0x00,0x01,0x00,
                                                0x00,0x00,0x00,0x00,0x00};

constexpr uint8_t REQ_RAWBLOCK_MSG[REQUEST_LEN] = {0x00,0x03,0x01,0x00,0x00,
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
    Rmm() : 
    _is_ready(false), 
    _stop(false),
    _dlt(nullptr)
    {
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
        if( _dlt ) {
            delete _dlt;
        }
    }
    
    bool search() {
        // _send_jumbo_zeros();
        return _get_identity();
    }

    void download(std::string filename) {
        read_contents();

        int32_t dl_idx = _find_matching_file_index(filename);
        if( dl_idx < 0) {
            printf("File not found.\n");
            return;
        }

        printf("Downloading file: %s\n", _files[dl_idx].filename.c_str());

        // Create Download Tracker
        auto cb = [this](uint64_t block_num) { _request_raw_block(block_num); };
        _dlt = new DownloadTracker(_files[dl_idx], ".", _rxq, cb);
        _dlt->start();

        //wait for dlt to have started

    }

    void _request_raw_block(uint64_t block_num) {
        uint8_t buf[REQUEST_LEN];
        memcpy(buf, REQ_RAWBLOCK_MSG, REQUEST_LEN);

        buf[REQUEST_LEN-1] = (uint8_t) (block_num & 0xFF);
        buf[REQUEST_LEN-2] = (uint8_t) ((block_num>>8) & 0xFF);
        buf[REQUEST_LEN-3] = (uint8_t) ((block_num>>16) & 0xFF);
        buf[REQUEST_LEN-4] = (uint8_t) ((block_num>>24) & 0xFF);

        printf("Requesting block %llu\n", block_num);
        print_buf(buf, REQUEST_LEN);
        UDP::send_udp_packet(TX_IP, PORT_252, buf, REQUEST_LEN);
    }

    void read_contents() {
        bool stop = false;
        int32_t request_val = 1;

        while (!stop) {
            request_val = _request_content_block(request_val);
            if (request_val == 0xff) {
                stop = true;
            }
        }    
    }

    int32_t _find_matching_file_index(std::string& filename) {
        int32_t dl_idx = -1;

        for (size_t i = 0; i < _files.size(); i++) {
            if( _files[i].filename == filename ) {
                dl_idx = (int32_t) i;
                break;
            }
        }

        return dl_idx;
    }

    void _date_string_template(char* str) {
        sprintf(str, "01/01/2019 00:01:08");
        str[19] = 0;
    }

    void _date_string_update(char* out, const char* in) {
        // Month
        out[0] = in[2];
        out[1] = in[3];

        // Day
        out[3] = in[0];
        out[4] = in[1];

        // Year
        out[6] = in[4];
        out[7] = in[5];
        out[8] = in[6];
        out[9] = in[7];

        // Hour
        out[11] = in[8];
        out[12] = in[9];

        // Minute
        out[14] = in[10];
        out[15] = in[11];

        // Second
        out[17] = in[12];
        out[18] = in[13];

        out[19] = 0;
    }

    void print_files() {
        std::string strout = "Files on RMM:\n\n";
        strout += "Filename    StartBlock      BlockCount     Size           Created";
        cout << strout << endl;

        char dtstr[20];
        _date_string_template(dtstr);

        for (const auto& f : _files) {
            cout << setw(12) << left << " " + f.filename;
            cout << setw(12) << right << to_string(f.start_block);
            cout << setw(12) << right << to_string(f.block_count);
            cout << setw(12) << right << to_string(f.file_size);
            
            _date_string_update(dtstr, f.created);
            cout <<  setw(23) << right << dtstr;
            cout << endl;
        }
    }

    bool tasks_pending() {
        bool ret = false;
        if( _dlt ) { 
            ret = !_dlt->get_is_stopped();
        }

        return ret;
    }

    void wait_for_ready() {
        bool ready = false;
        while (!ready) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            ready = is_ready();
        }
    }


    void stop_all() {
        if( _dlt  ) {
            _dlt->stop();
        }

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
        if( _dlt ) {
            _dlt->_th_process.join();
            _dlt->_th_request.join();
            _dlt->_th_write.join();
        }

        for (size_t i = 0; i < _rx.size(); i++) {
            if(_rx[i]->_th.joinable()) {
                _rx[i]->_th.join();
            }
        }
    }

    q_elem_t* wait_for_rx() {
        q_elem_t* elem = nullptr;
        
        for (int i = 0; i < 100; i++) {
            elem = _rxq->get();
            if (elem) {
                break;
            }

            this_thread::sleep_for(chrono::milliseconds(10));
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

    uint32_t _request_content_block(int32_t value) {
        uint8_t buf[REQUEST_LEN];
        memcpy(buf, REQ_BLOCK_MSG, REQUEST_LEN);

        buf[REQUEST_LEN-1] = value;
        UDP::send_udp_packet(TX_IP, PORT_252, buf, REQUEST_LEN);

        const q_elem_t* resp = wait_for_rx();

        size_t old_size = _files.size();
        _parse_content_block(resp);
        size_t new_size = _files.size();

        if (new_size > old_size) {
            value += 1;
        } else {
            value = 0xff;
        }

        return value;
    }

    void _parse_content_block(const q_elem_t* resp) {
        bool searching_for_file = true;
        std::string filename = "";
        uint8_t* block = nullptr;

        file_t my_file;

        for(int32_t i = 0; i < resp->len; i++) {
            if (searching_for_file) {
                if(!memcmp(&resp->buf[i], "File", 4)) {
                    block = (uint8_t*) &resp->buf[i];

                    searching_for_file = false;
                    my_file.filename = (const char*) block;

                    my_file.start_block = _unpack64(&block[56]);
                    my_file.block_count = _unpack64(&block[64]);
                    my_file.file_size = _unpack64(&block[72]);
                    memcpy(my_file.created, &block[80], 16);
                    my_file.created[16] = 0;

                    _files.push_back(my_file);
                    searching_for_file = true;
                }
            } 
        }
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

    // Helper function to asseble big endian numbers
    uint64_t _unpack64(uint8_t* buf) {
        uint64_t num = 0;
        for (int i = 0; i < 8; i++) {
            num = (num << (8 * 1)) | (uint64_t) buf[i];
        }

        return num;
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
    std::vector<file_t> _files;
    DownloadTracker* _dlt;
};

#endif
