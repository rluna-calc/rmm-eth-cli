package udp

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"log"
	"strings"
	"time"

	"github.com/dustin/go-humanize"
	"github.com/sirupsen/logrus"
)

const (
	PORT_252       = 252
	PORT_253       = 253
	PORT_BROADCAST = 255
	DISCOVER_SIZE  = 8548
	BUFFER_SIZE    = 9000
)

type Rmm struct {
	txIP      string
	files     []FileType
	receivers []*Receiver
	rxQueue   chan []byte
	dlt       *DownloadTracker
}

func NewRmm() *Rmm {
	rxQueue := make(chan []byte, 128)
	rmm := &Rmm{
		txIP:    "192.168.0.255",
		rxQueue: rxQueue,
	}

	// Initialize receivers for ports 252 and broadcast
	rmm.receivers = append(rmm.receivers, NewReceiver(PORT_252, rxQueue))
	rmm.receivers = append(rmm.receivers, NewReceiver(PORT_BROADCAST, rxQueue))

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

func (r *Rmm) TasksPending() bool {
	if r.dlt != nil {
		return !r.dlt.GetIsStopped()
	}
	return false
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
	for len(r.rxQueue) > 0 {
		<-r.rxQueue
	}
}

func (r *Rmm) waitForRx() []byte {
	select {
	case msg := <-r.rxQueue:
		return msg
	case <-time.After(time.Second):
		log.Println("Timeout waiting for response")
		return nil
	}
}

func (r *Rmm) parseIdentityResponse(resp []byte) bool {
	serialParts := bytes.Split(resp[20:40], []byte{0x00})
	serialNumber := strings.TrimSpace(string(serialParts[len(serialParts)-1]))

	modelParts := bytes.Split(resp[54:94], []byte{0x00})
	modelNumber := strings.TrimSpace(string(modelParts[len(modelParts)-1]))

	logrus.Infof("SerialNumber: %s", serialNumber)
	logrus.Infof("ModelNumber: %s", modelNumber)

	return serialNumber != "" && modelNumber != ""
}

func datetimeStrToObj(dtstr string) (time.Time, error) {
	// Check if the input string has the expected length
	if len(dtstr) < 16 {
		return time.Time{}, fmt.Errorf("invalid timestamp string length")
	}

	// Extracting components from the string
	day := dtstr[0:2]
	month := dtstr[2:4]
	year := dtstr[4:8]
	hour := dtstr[8:10]
	minute := dtstr[10:12]
	second := dtstr[12:14]

	// Parsing the string into a time.Time object using `time.Parse`
	// Construct the datetime string in a standard layout for parsing
	datetimeStr := fmt.Sprintf("%s-%s-%sT%s:%s:%sZ", year, month, day, hour, minute, second)
	t, err := time.Parse("2006-01-02T15:04:05Z", datetimeStr)
	if err != nil {
		return time.Time{}, err
	}

	// Return the parsed time in UTC
	return t.UTC(), nil
}

func (r *Rmm) PrintFiles() {
	strout := "Files on RMM:\n\n"
	strout += fmt.Sprintf("%-12s %15s %15s %18s %20s\n", "Filename", "StartBlock", "BlockCount", "Size", "Created")

	for _, f := range r.files {
		createdTime, err := datetimeStrToObj(f.Created)
		if err != nil {
			log.Println("Error parsing date:", err)
			continue
		}
		createdStr := createdTime.Format("01/02/2006 15:04:05")

		strout += fmt.Sprintf(
			" %-12s %15s %15s %18s %20s\n",
			f.Name,
			humanize.Comma(int64(f.StartBlock)),
			humanize.Comma(int64(f.BlockCount)),
			humanize.Comma(int64(f.Size)),
			createdStr,
		)
	}

	logrus.Infof(strout)
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
		logrus.Warnf("File %s not found on RMM", filename)
		return
	}

	logrus.Infof("Downloading file: %s", file.Name)
	r.dlt = NewDownloadTracker(file, dest, r.rxQueue, r.requestBlock, r.Stop)
	r.dlt.Start()

	// Wait for download to start
	for r.dlt.GetIsStopped() {
		time.Sleep(100 * time.Millisecond)
	}
}

func (r *Rmm) requestBlock(blockNum uint64) {
	// Define behavior for requesting a block
	hexString := fmt.Sprintf("000301000000%08X", blockNum)
	buf, _ := hex.DecodeString(hexString)
	Send(buf, r.txIP, PORT_252)
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
			Name:       "File" + string(block[:nullIndex]),
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
