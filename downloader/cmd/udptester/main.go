package main

import (
	"downloader/internal/udp"
	"flag"
	"fmt"
	"log"
	"os"
)

func main() {
	// Define command line arguments
	tx := flag.Bool("tx", false, "Set mode to transmit")
	ip := flag.String("ip", "", "Destination IP address")
	port := flag.Int("port", -1, "Rx or Tx port")
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
	var txBuf []byte
	if *hex {
		var err error
		txBuf, err = hexToBytes(*message)
		if err != nil {
			log.Fatalf("Invalid hex message: %v", err)
		}
	} else {
		txBuf = []byte(*message)
	}

	// Send the message if transmitting mode is set
	if *tx && *ip != "" && *port != 0 {
		log.Printf("Sending message '%s' to (%s, %d)", *message, *ip, *port)
		udp.Send(txBuf, *ip, *port)
		os.Exit(0)
	}
}

// func init() {
// 	// Set up signal handling
// 	c := make(chan os.Signal, 1)
// 	signal.Notify(c, syscall.SIGINT, syscall.SIGTERM)
// 	go func() {
// 		for sig := range c {
// 			signalHandler(sig)
// 		}
// 	}()
// }

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
