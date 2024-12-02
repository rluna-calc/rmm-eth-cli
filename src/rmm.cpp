#include <rmm.h>
#include <iostream>
#include <string.h>

const char* TX_IP = "192.168.0.255";
constexpr uint32_t PORT_252 = 252;
constexpr uint32_t PORT_253 = 253;
constexpr uint32_t PORT_BROADCAST = 255;
constexpr uint32_t DISCOVER_SIZE = 8548;


const uint8_t IDENTITY_MSG[] = {0x00,0x05,0x00,0x01,0x00,
                                0x00,0x00,0x00,0x00,0x00};


using namespace std;

Rmm::Rmm() : _is_ready(false), _stop(false) {
    // _serial_number = "";
    // _model_number = "";

    _rxq = new RxQueue(16);

    _rx.push_back(new Receiver(PORT_252, _rxq));
    _rx.push_back(new Receiver(PORT_253, _rxq));

    _start();
}

Rmm::~Rmm() {
    stop_all();

    for (size_t i = 0; i < _rx.size(); i++) {
        delete _rx[i];
    }

    delete _rxq;
}

void Rmm::stop_all() {
    //TODO: dlt

    for (size_t i = 0; i < _rx.size(); i++) {
        _rx[i]->start();
    }
}

bool Rmm::is_ready() {
    bool ret = true;
    for (size_t i = 0; i < _rx.size(); i++) {
        ret &= _rx[i]->is_running();
    }

    return ret;
}

bool Rmm::tasks_pending() {
    //TODO
    // ret = False
    // if self.dlt is not None:
    //     ret = not self.dlt.is_stopped()

    // return ret
    return false; // Simulate that no tasks are pending
}

void Rmm::wait_for_ready() {
    bool ready = false;
    while (!ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ready = is_ready();
    }
}

 void Rmm::wait_for_threads() {
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

q_elem_t* Rmm::wait_for_rx() {
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

bool Rmm::search() {
    _send_jumbo_zeros();
    return _get_identity();
}

void Rmm::download(const char* filename) {
    std::cout << "Downloading file: " << filename << std::endl;
    // Simulate downloading a file (this could be expanded)
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Download complete for: " << filename << std::endl;
}

void Rmm::read_contents() {
    // // Simulate reading file contents
    // file_names = {"File1", "File2", "File3"};
}

void Rmm::print_files() {
    // std::cout << "Files on RMM:" << std::endl;
    // for (const auto& file : file_names) {
    //     std::cout << "  " << file << std::endl;
    // }
}

bool Rmm::_get_identity() {
    const uint8_t* buf = IDENTITY_MSG;
    printf("Sending discovery message...\n");
    send_udp_packet(TX_IP, PORT_BROADCAST, buf, sizeof(IDENTITY_MSG));

    const q_elem_t* resp = wait_for_rx();
    return _parse_identity_response(resp);
}

bool Rmm::_parse_identity_response(const q_elem_t* resp) {
    if (!resp) {
        return false;
    }

    printf("Ready to parse response...\n");


    return true;
}

void Rmm::_send_jumbo_zeros() {
    uint8_t buf[DISCOVER_SIZE];
    memset(buf, 0, DISCOVER_SIZE);
    send_udp_packet(TX_IP, PORT_253, buf, DISCOVER_SIZE);
}

// Start all Rx threads
void Rmm::_start() {
    for (size_t i = 0; i < _rx.size(); i++) {
        _rx[i]->start();
    }
}
