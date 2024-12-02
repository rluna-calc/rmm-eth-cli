#include <rmm.h>
#include <iostream>

const char* TX_IP = "192.168.0.255";
constexpr uint32_t PORT_252 = 252;
constexpr uint32_t PORT_253 = 253;
constexpr uint32_t PORT_BROADCAST = 255;
constexpr uint32_t DISCOVER_SIZE = 8548;


Rmm::Rmm() : _is_ready(false), _stop(false) {
    _rxq = new RxQueue(16);

    _rx.push_back(new Receiver(PORT_252, _rxq));
    _rx.push_back(new Receiver(PORT_253, _rxq));

    _start();
}

Rmm::~Rmm() {
    stop_all();

    for (int i = 0; i < _rx.size(); i++) {
        delete _rx[i];
    }

    delete _rxq;
}

void Rmm::stop_all() {
    //TODO: dlt

    for (int i = 0; i < _rx.size(); i++) {
        _rx[i]->start();
    }
}

bool Rmm::is_ready() {
    bool ret = true;
    for (int i = 0; i < _rx.size(); i++) {
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

    for (int i = 0; i < _rx.size(); i++) {
        if(_rx[i]->_th.joinable()) {
            _rx[i]->_th.join();
        }
    }
}

void Rmm::search() {
    // const char buf[] = {};
    // std::cout << "Sending discovery message..." << std::endl;
    // std::vector<uint8_t> buf(8548, 0); // Simulated discovery message
    // send_udp_packet("192.168.0.255", PORT_253, buf);
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



// Start all Rx threads
void Rmm::_start() {
    for (int i = 0; i < _rx.size(); i++) {
        _rx[i]->start();
    }
}
