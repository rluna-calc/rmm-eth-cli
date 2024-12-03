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
    _q(rxq)
{
    memset(_elem.buf, 0, RX_BUFFER_SIZE);
    _elem.len = 0;
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

    // Set receive buffer size (1MB buffer)
    int buffer_size = 1024 * 1024;
    if (setsockopt(_sock, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size)) < 0) {
        throw std::runtime_error("Failed to set socket receive buffer size.");
    }
    
    // Set socket timeout to 0.5 seconds (500 ms)
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;  // 0.5 seconds
    if (setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        throw std::runtime_error("Failed to set socket timeout.");
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

    sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);
    while (!_stop) {
        _is_running = true;

        _elem.len = recvfrom(_sock, _elem.buf, RX_BUFFER_SIZE, 0,
                                        (struct sockaddr*)&sender_addr, &sender_addr_len);
        if (_elem.len < 20) {
            // printf("Error receiving data \n");
            continue;
        }


        _q->push(&_elem);
        printf("Received %d bytes from %s\n", _elem.len, inet_ntoa(sender_addr.sin_addr));

        printf("%p\n", _elem.buf);
        for (int i = 0; i < 20; i++) {
            printf("0x%02x,", _elem.buf[i]);
        }
        printf("\n");
    }

    _is_running = false;
    _close_socket();
    printf("Finished listening on port %d\n", _port);
}

void Receiver::_close_socket() {
    if (_sock != -1) {
        close(_sock);
        _sock = -1;
    }
}


// Function to send a UDP packet to a given IP and port
void send_udp_packet(const char* ip, int port, const uint8_t* buffer, const int len) {
    // Create socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Failed to create socket.");
    }

    // Allow broadcasting
    int broadcast = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        printf("Error setting socket option for broadcast: %s\n",strerror(errno));
        close(sock);
        return;
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
