package main

import (
	"downloader/internal/udp"
	"flag"
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall"
)

func main() {
	// Define command line arguments
	tx := flag.Bool("tx", false, "Set mode to transmit")
	// rx := flag.Bool("rx", false, "Set mode to receive")
	ip := flag.String("ip", "", "Destination IP address")
	port := flag.Int("port", 12345, "Rx or Tx port")
	message := flag.String("message", "Hello UDP!", "Message to send")
	hex := flag.Bool("hex", false, "Interpret message as a hexadecimal string")
	verboseFlag := flag.Bool("verbose", false, "Print verbose output")

	flag.Parse()

	// Set logging level
	logLevel := log.New(os.Stdout, "", log.LstdFlags)
	if *verboseFlag {
		logLevel.SetFlags(log.LstdFlags | log.Lshortfile) // Show file names and line numbers for debugging
	}

	// If message should be in hex, convert it
	var txBuf []byte = checkHex(*message, *hex)

	// Send the message if transmitting mode is set
	if *tx && *ip != "" && *port != 0 {
		log.Printf("Sending message '%s' to (%s, %d)", *message, *ip, *port)
		udp.Send(txBuf, *ip, *port)
		os.Exit(0)
	}

	var rxChan = make(chan []byte, 20)
	log.Printf("Starting receiver on port %d", *port)
	var receiver = udp.NewReceiver(*port, rxChan)
	receiver.Start()

	// Set up a channel to listen for interrupt signals (e.g., Ctrl+C)
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM) // Catch SIGINT (Ctrl+C) and SIGTERM

	// Main loop to process received messages
	for {
		select {
		case msg, ok := <-rxChan:
			if !ok {
				// Channel is closed and drained
				log.Println("All messages processed. Exiting...")
				return
			}
			// Process received message
			log.Printf("Main thread received: %s", msg)

		case sig := <-sigChan:
			// Handle interrupt signal
			log.Printf("Received signal: %v. Exiting...", sig)
			return
		}
	}
}

func checkHex(message string, isHex bool) []byte {
	var txBuf []byte
	if isHex {
		var err error
		txBuf, err = hexToBytes(message)
		if err != nil {
			log.Fatalf("Invalid hex message: %v", err)
		}
	} else {
		txBuf = []byte(message)
	}

	return txBuf
}

// hexToBytes converts a hex string to a byte slice
func hexToBytes(s string) ([]byte, error) {
	var bytes []byte
	for i := 0; i < len(s); i += 2 {
		hexByte := s[i : i+2]
		var b byte
		_, err := fmt.Sscanf(hexByte, "%x", &b)
		if err != nil {
			return nil, fmt.Errorf("failed to parse hex: %v", err)
		}
		bytes = append(bytes, b) // Append the byte value, not an int
	}
	return bytes, nil
}
