package udp

import (
	"fmt"
	"log"
	"os"
	"time"

	"github.com/dustin/go-humanize"
	"github.com/sirupsen/logrus"
)

// FileType struct equivalent to namedtuple in Python
type FileType struct {
	Name       string
	StartBlock uint64
	BlockCount uint64
	Size       uint64
	Created    string
}

// Constants
const (
	REPORT_COUNT  = 100000
	PAYLOAD_START = 10
	BLOCK_BYTES   = 512
	RX_BLOCKS     = 16
	RX_BYTES      = BLOCK_BYTES * RX_BLOCKS
	NUM_SEGMENTS  = 16
	RX_SEQ_LEN    = RX_BYTES * NUM_SEGMENTS
)

type DownloadTracker struct {
	FileDesc        FileType
	DestPath        string
	RxQueue         chan []byte
	RequestBlockCb  func(uint64)
	StopListeningCb func()
	StopFlag        bool
	IsStopped       bool
	IsFileReady     bool
	FileWriteQueue  chan [][]byte
	BytesWritten    uint64
	BytesReceived   uint64
	CurrentBlock    uint64
	BlockCount      uint64
	RxSegs          [][]byte
	RxSize          uint64
	MaxConChunks    uint
	NumAciveChunks  uint
}

// Initialize the download tracker
func NewDownloadTracker(fileDesc FileType, destPath string, rxQueue chan []byte, requestBlockCb func(uint64), stopListeningCb func()) *DownloadTracker {
	dlt := &DownloadTracker{
		FileDesc:        fileDesc,
		DestPath:        destPath,
		RxQueue:         rxQueue,
		RequestBlockCb:  requestBlockCb,
		StopListeningCb: stopListeningCb,
		FileWriteQueue:  make(chan [][]byte, 10), // Adjust buffer size as needed
		StopFlag:        false,
		IsStopped:       true,
		IsFileReady:     false,
		MaxConChunks:    2,
		NumAciveChunks:  0,
	}

	dlt.Init()
	return dlt
}

func (d *DownloadTracker) Init() {
	d.CurrentBlock = d.FileDesc.StartBlock
	d.BlockCount = 0
	d.BytesWritten = 0
	d.BytesReceived = 0
	d.ResetSegments()
}

// Resets segment data
func (d *DownloadTracker) ResetSegments() {
	d.RxSegs = make([][]byte, 0)
	d.RxSize = 0
}

// Starts the download and writing processes
func (d *DownloadTracker) Start() {
	go d.DoDownload()
}

// Waits until the file is ready to write
func (d *DownloadTracker) WaitForFileReady() {
	for !d.IsFileReady {
		time.Sleep(10 * time.Millisecond)
	}
}

// Stops the download process
func (d *DownloadTracker) Stop() {
	d.StopFlag = true
}

func (d *DownloadTracker) GetIsStopped() bool {
	return d.IsStopped
}

// Requests a block by invoking the callback
func (d *DownloadTracker) GetBlock(blockNum uint64) {
	readBlock := d.FileDesc.StartBlock + blockNum
	d.RequestBlockCb(readBlock)
}

// Processes a received segment
func (d *DownloadTracker) ProcessSegment(seg []byte) bool {
	reportedLen := (uint64(uint64(seg[3]))<<8 | uint64(seg[2])) * 2
	if uint64(len(seg)-PAYLOAD_START) == reportedLen {
		d.BytesReceived += reportedLen
		d.RxSize += reportedLen
		d.RxSegs = append(d.RxSegs, seg)
	}

	okToIncrement := d.RxSize == RX_SEQ_LEN
	if okToIncrement {
		if len(d.FileWriteQueue) < cap(d.FileWriteQueue) {
			// d.FileWriteQueue <- d.RxSegs
		} else {
			log.Println("File write queue is full")
		}
	}

	return okToIncrement
}

func (d *DownloadTracker) slowdownIfEndOfFile() {
	// Slow down toward the end of the file
	if d.MaxConChunks > 1 {
		numRemainingChunks := d.getCountRemainingChunks()
		if numRemainingChunks < d.MaxConChunks*50 {
			d.MaxConChunks = 1
		}
	}
}

func (d *DownloadTracker) requestBlockIfOk() {
	// Request
	if d.NumAciveChunks < d.MaxConChunks {
		d.NumAciveChunks += 1
		d.GetBlock(d.BlockCount)
		d.incrementChunk()
	}
}

// Performs the download loop
func (d *DownloadTracker) DoDownload() {
	logrus.Infof("Starting file download")
	d.IsStopped = false

	// go d.WriteFile()
	// d.WaitForFileReady()

	// Start listening in new thread
	go d.DoListening()

	for !d.StopFlag && !d.IsStopped {
		// d.slowdownIfEndOfFile()
		d.requestBlockIfOk()
	}

	logrus.Infof("%s bytes received", humanize.Comma(int64(d.BytesReceived)))
}

func (d *DownloadTracker) DoListening() {
	isFinished := false
	numRetries := 0
	bytesPrev := uint64(0)
	timePrev := time.Now()

	startTime := time.Now()

outerForLoop:
	for !d.StopFlag && !isFinished {
		d.ResetSegments()
		d.FlushRxQueue()
		okBlockIncrement := false

		d.GetBlock(d.BlockCount)

	innerForLoop:
		for i := 0; i < NUM_SEGMENTS; i++ {
			select {
			case segment := <-d.RxQueue:
				okBlockIncrement = d.ProcessSegment(segment)
				// logrus.Infof("num: %d, seg: %02x", segment[0], segment[9])
				if okBlockIncrement {
					break innerForLoop
				}
			case <-time.After(100 * time.Millisecond):
				logrus.Warnf("Rx queue empty on segment %d. Retrying block %d", i, d.CurrentBlock)
				numRetries++
				break innerForLoop
			}
		}

		// okBlockIncrement = true
		if okBlockIncrement {
			if d.BytesReceived >= d.FileDesc.Size {
				isFinished = true
				break outerForLoop
			} else {
				d.NumAciveChunks -= 1
			}
		}

		if d.BlockCount%REPORT_COUNT == 0 {
			bytesPrev, timePrev, isFinished = d.DoReporting(startTime, timePrev, bytesPrev, numRetries)
			numRetries = 0
		}
	}

	d.StopListeningCb()
	d.IsStopped = true
}

func (d *DownloadTracker) getCountRemainingChunks() uint {
	bytesNow := d.BlockCount * BLOCK_BYTES
	bytesRemaining := d.FileDesc.Size - bytesNow
	chunksRemaining := bytesRemaining / RX_SEQ_LEN
	return uint(chunksRemaining)
}

func (d *DownloadTracker) incrementChunk() {
	d.BlockCount += (1 << 8)
	d.CurrentBlock += (1 << 8)
}

// Reporting function to log download progress
func (d *DownloadTracker) DoReporting(startTime, lastTime time.Time, bytesPrev uint64, numRetries int) (uint64, time.Time, bool) {
	timeNow := time.Now()
	bytesNow := d.BlockCount * BLOCK_BYTES
	rate := float64(bytesNow-bytesPrev) / timeNow.Sub(lastTime).Seconds()
	endNow := false

	bytesRemaining := d.FileDesc.Size - bytesNow
	var remainingStr string
	if rate > 0 {
		timeRemaining := float64(bytesRemaining) / rate
		remainingStr = GetTimeStr(timeRemaining)
	} else {
		remainingStr = "infinity"
		endNow = true
	}

	if lastTime.After(startTime) {
		log.Printf("%.1f GB of %.1f GB at ==> %.2f MB/s | %s",
			float64(bytesNow)/1e9, float64(d.FileDesc.Size)/1e9, rate/1e6, remainingStr)
	}

	return bytesNow, timeNow, endNow
}

// Flushes the receive queue
func (d *DownloadTracker) FlushRxQueue() {
	for {
		select {
		case <-d.RxQueue:
		default:
			return
		}
	}
}

// Writes received data to file
func (d *DownloadTracker) WriteFile() {
	createdSecs := d.FileDesc.Created[:len(d.FileDesc.Created)-6]
	createdMicrosecs := d.FileDesc.Created[len(d.FileDesc.Created)-6:]
	filename := fmt.Sprintf("%s/%s_%s_%s.ch10", d.DestPath, d.FileDesc.Name, createdSecs, createdMicrosecs)
	logrus.Infof("Ready to write file: %s", filename)

	d.IsFileReady = true

	file, err := os.Create(filename)
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	for !d.StopFlag {
		select {
		case segments := <-d.FileWriteQueue:
			for _, segment := range segments {
				remainingBytes := d.FileDesc.Size - d.BytesWritten
				if remainingBytes < RX_BYTES {
					file.Write(segment[PAYLOAD_START : PAYLOAD_START+remainingBytes])
					d.BytesWritten += remainingBytes
				} else {
					file.Write(segment[PAYLOAD_START:])
					d.BytesWritten += RX_BYTES
				}
				if d.BytesWritten >= d.FileDesc.Size {
					d.StopFlag = true
					break
				}
			}
			file.Sync()
		case <-time.After(100 * time.Millisecond):
			logrus.Warnf("write_queue empty")
		}
	}

	logrus.Infof("%d bytes written to file", d.BytesWritten)
	logrus.Infof("File: %s", filename)
}

// Formats remaining time as a string
func GetTimeStr(seconds float64) string {
	if seconds >= 3600 {
		return fmt.Sprintf("%.2f hrs", seconds/3600)
	} else if seconds >= 60 {
		return fmt.Sprintf("%.1f mins", seconds/60)
	} else {
		return fmt.Sprintf("%.1f secs", seconds)
	}
}
