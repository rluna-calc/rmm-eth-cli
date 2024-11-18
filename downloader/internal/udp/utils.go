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
	addr_str := fmt.Sprintf(":%d", r.port)
	// conn, err := net.ListenPacket("udp", addr)
	addr, _ := net.ResolveUDPAddr("udp", addr_str)
	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		logrus.Fatalf("Error listening on port %d: %v", r.port, err)
		return
	}
	defer conn.Close()

	logrus.Debugf("Listening for UDP packets on port %d...", r.port)

	// // Get the file descriptor for the connection
	// fd, err := conn.(*net.UDPConn).File()
	// if err != nil {
	// 	logrus.Fatal(err)
	// }
	// defer fd.Close()

	// Set the receive buffer size to 1 MB
	// err = syscall.SetsockoptInt(syscall.Handle(fd.Fd()), syscall.SOL_SOCKET, syscall.SO_RCVBUF, 1024*1024)
	// if err != nil {
	// 	logrus.Fatal(err)
	// }

	conn.SetReadBuffer(1024 * 1024)
	logrus.Debug("UDP receive buffer size set to 1 MB")

	// buffer := make([]byte, BUFFER_SIZE)
	// Create a slice of byte slices
	bufferPool := make([][]byte, 32)
	for i := range bufferPool {
		bufferPool[i] = make([]byte, BUFFER_SIZE) // Each element is a byte array of size BUFFER_SIZE
	}
	bufferCount := 0

	for !r.stopListening {
		buffer := bufferPool[bufferCount]
		bufferCount = (bufferCount + 1) & 0x1F

		numBytes, _, err := conn.ReadFrom(buffer)
		go r.processRx(buffer, numBytes)

		if err != nil {
			if r.stopListening {
				return
			}
			continue
		}
	}
	r.isRunning = false
}

func (r *Receiver) processRx(buffer []byte, numBytes int) {
	if numBytes < 20 {
		return
	}

	// Send the data to the channel
	select {
	case r.messageChannel <- buffer[:numBytes]: // This will be non-blocking if there's space in the buffer
		// Successfully sent to the channel
	default:
		// Optional: Handle the case where the channel is full, such as logging or discarding
		logrus.Warn("Dropped packet due to full channel buffer")
	}
}
