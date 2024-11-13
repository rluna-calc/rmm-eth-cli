package udp

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"log"
	"net"
	"time"

	"github.com/sirupsen/logrus"
)

const (
	PORT_252       = 252
	PORT_253       = 253
	PORT_BROADCAST = 255
	DISCOVER_SIZE  = 8548
	BUFFER_SIZE    = 9000
)

type FileType struct {
	Name       string
	StartBlock uint64
	BlockCount uint64
	Size       uint64
	Created    string
}

func getLocalIP() (string, error) {
	conn, err := net.Dial("udp", "8.8.8.8:80")
	if err != nil {
		return "", err
	}
	defer conn.Close()

	localAddr := conn.LocalAddr().(*net.UDPAddr)
	return localAddr.IP.String(), nil
}

type Rmm struct {
	txIP         string
	files        []FileType
	receivers    []*Receiver
	messageQueue chan []byte
}

func NewRmm() *Rmm {
	messageQueue := make(chan []byte, 100)
	rmm := &Rmm{
		txIP:         "192.168.0.255",
		messageQueue: messageQueue,
	}

	// Initialize receivers for ports 252 and broadcast
	rmm.receivers = append(rmm.receivers, NewReceiver(PORT_252, messageQueue))
	rmm.receivers = append(rmm.receivers, NewReceiver(PORT_BROADCAST, messageQueue))

	return rmm
}

func (r *Rmm) Start() {
	for _, receiver := range r.receivers {
		receiver.Start()
	}
}

func (r *Rmm) Stop() {
	for _, receiver := range r.receivers {
		receiver.Stop()
	}
}

func (r *Rmm) SendJumboZeros() {
	buf := make([]byte, DISCOVER_SIZE)
	Send(buf, r.txIP, PORT_253)
}

func (r *Rmm) GetIdentity() bool {
	r.flushRxQueue()
	hexString := "00050001000000000000"
	buf, _ := hex.DecodeString(hexString)
	Send(buf, r.txIP, PORT_BROADCAST)

	resp := r.waitForRx()
	return r.parseIdentityResponse(resp)
}

func (r *Rmm) Search() bool {
	r.SendJumboZeros()
	return r.GetIdentity()
}

func (r *Rmm) flushRxQueue() {
	for len(r.messageQueue) > 0 {
		<-r.messageQueue
	}
}

func (r *Rmm) waitForRx() []byte {
	select {
	case msg := <-r.messageQueue:
		return msg
	case <-time.After(time.Second):
		log.Println("Timeout waiting for response")
		return nil
	}
}

func (r *Rmm) parseIdentityResponse(resp []byte) bool {
	serialNumber := string(resp[20:40])
	modelNumber := string(resp[54:94])

	logrus.Infof("SerialNumber: %s", serialNumber)
	logrus.Infof("ModelNumber: %s", modelNumber)

	return serialNumber != "" && modelNumber != ""
}

func (r *Rmm) PrintFiles() {
	log.Println("Files on RMM:")
	for _, f := range r.files {
		log.Printf("%-12s %-15d %-15d %-18d %-20s", f.Name, f.StartBlock, f.BlockCount, f.Size, f.Created)
	}
}

func (r *Rmm) Download(filename, dest string) {
	r.ReadContents()

	// Find the file in the list
	var file FileType
	for _, f := range r.files {
		if f.Name == filename {
			file = f
			break
		}
	}

	if file.Name == "" {
		log.Printf("File %s not found on RMM", filename)
		return
	}

	log.Printf("Downloading file: %s", file.Name)
	// Implement actual download logic here
}

func (r *Rmm) ReadContents() {
	r.files = r.readContentsFromRMM()
}

func (r *Rmm) readContentsFromRMM() []FileType {
	var files []FileType
	stop := false
	requestVal := 1
	oldVal := requestVal

	for !stop {
		logrus.Debug(requestVal)
		logrus.Debug(oldVal)
		requestVal, newFiles := r.requestContentBlock(oldVal)
		if requestVal == 0xFF {
			stop = true
		}
		files = append(files, newFiles...)
		oldVal = requestVal
	}

	return files
}

func (r *Rmm) requestContentBlock(value int) (int, []FileType) {
	hexString := fmt.Sprintf("000300010000000000%02X", value)
	buf, _ := hex.DecodeString(hexString)
	Send(buf, r.txIP, PORT_252)

	resp := r.waitForRx()
	newFiles := r.parseContentBlock(resp)

	if len(newFiles) > 0 {
		value++
	} else {
		value = 0xFF
	}

	return value, newFiles
}

func (r *Rmm) parseContentBlock(resp []byte) []FileType {
	var files []FileType

	fileInfoBlocks := split(resp, []byte("File"))
	for i, block := range fileInfoBlocks[1:] {
		logrus.Debugf("Block %d\n%s", i, hex.EncodeToString(block))
		// Parse block data to extract file info
		nullIndex := bytes.IndexByte(block, 0x00)
		file := FileType{
			Name:       string(block[:nullIndex]),
			StartBlock: binary.BigEndian.Uint64(block[52 : 52+8]),
			BlockCount: binary.BigEndian.Uint64(block[60 : 60+8]),
			Size:       binary.BigEndian.Uint64(block[68 : 68+8]),
			Created:    string(block[76:92]),
		}
		files = append(files, file)
	}

	return files
}

func split(s, sep []byte) [][]byte {
	var result [][]byte
	n := 0
	for i := 0; i < len(s)-len(sep)+1; i++ {
		if string(s[i:i+len(sep)]) == string(sep) {
			result = append(result, s[n:i])
			n = i + len(sep)
		}
	}
	result = append(result, s[n:])
	return result
}
