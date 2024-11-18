package udp

import (
	"fmt"
	"net"

	"github.com/sirupsen/logrus"
)

func Send(buf []byte, ip string, port int) {
	// Resolve UDP address
	udpAddress := &net.UDPAddr{
		IP:   net.ParseIP(ip),
		Port: port,
	}

	// Dial the UDP connection
	conn, err := net.DialUDP("udp", nil, udpAddress)
	if err != nil {
		logrus.Fatalf("Error creating UDP connection: %v", err)
	}
	defer conn.Close()

	// Send the UDP packet
	_, err = conn.Write(buf)
	if err != nil {
		logrus.Fatalf("Error sending UDP message: %v", err)
	}

	// Log the sent message length
	logrus.Debugf("Sent: %d bytes to %s:%d", len(buf), ip, port)
}

type Receiver struct {
	port           int
	stopListening  bool
	isRunning      bool
	messageChannel chan []byte
}

func NewReceiver(port int, messageChannel chan []byte) *Receiver {
	return &Receiver{
		port:           port,
		messageChannel: messageChannel,
	}
}

func (r *Receiver) Start() {
	go r.listen()
}

func (r *Receiver) Stop() {
	r.stopListening = true
}

func (r *Receiver) listen() {
	addr := fmt.Sprintf(":%d", r.port)
	conn, err := net.ListenPacket("udp", addr)
	if err != nil {
		logrus.Fatalf("Error listening on port %d: %v", r.port, err)
		return
	}
	defer conn.Close()

	logrus.Debugf("Listening for UDP packets on port %d...", r.port)

	for !r.stopListening {
		buffer := make([]byte, BUFFER_SIZE)
		num_bytes, addr, err := conn.ReadFrom(buffer)
		if err != nil {
			if r.stopListening {
				return
			}
			continue
		}

		// // Filter out packets from self or too small to be valid
		// localIP, err := getLocalIP()
		// if err == nil && addr.(*net.UDPAddr).IP.String() == localIP {
		// 	continue
		// }

		if num_bytes < 20 {
			continue
		}

		logrus.Debugf("Received message: %d bytes from %s", num_bytes, addr.String())
		r.messageChannel <- buffer[:num_bytes]
	}
	r.isRunning = false
}
