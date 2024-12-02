#include <udp_utils.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <string.h>
#include <stdio.h>


Receiver::Receiver(int port, RxQueue* rxq) :
    _port(port),
    _sock(-1),
    _stop(false),
    _is_running(false),
    _q(rxq),
    _buf_idx(0)
{
    memset(_buffer_pool, 0, NUM_BUFFER_POOL * BUFFER_SIZE);
}

Receiver::~Receiver() {
    _close_socket();
}

void Receiver::start() {
    _th = std::thread(&Receiver::_run, this);
}

void Receiver::_run() {
    _sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sock < 0) {
        throw std::runtime_error("Failed to create socket.");
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(_port);

    if (bind(_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Failed to bind socket.");
    }

    printf("Listening for UDP packets on port %d ... \n", _port);

    uint8_t* rx_buf;
    sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);
    while (!_stop) {
        _is_running = true;

        // Get the receive buffer and increment index for next time
        rx_buf = _buffer_pool[_buf_idx++];
        _buf_idx = _buf_idx % NUM_BUFFER_POOL;

        int received_bytes = recvfrom(_sock, rx_buf, BUFFER_SIZE, 0,
                                        (struct sockaddr*)&sender_addr, &sender_addr_len);
        if (received_bytes < 0) {
            printf("Error receiving data \n");
            break;
        }

        _q->push(rx_buf);
        printf("Received %d bytes from %s\n", received_bytes, inet_ntoa(sender_addr.sin_addr));
    }

    _is_running = false;
    _close_socket();
}

void Receiver::_close_socket() {
    if (_sock != -1) {
        close(_sock);
        _sock = -1;
    }
}


// Function to send a UDP packet to a given IP and port
void send_packet(const char* ip, int port, const uint8_t* buffer, const int len) {
// void send_udp_packet(const std::string& ip, int port, const std::vector<uint8_t>& buffer) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Failed to create socket.");
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &dest_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid IP address.");
    }

    int sent_bytes = sendto(sock, buffer, len, 0,
                            (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_bytes < 0) {
        printf("Failed to send packet\n");
    } else {
        printf("Sent %d bytes to %s:%d\n", sent_bytes, ip, port);
    }

    close(sock);
}
