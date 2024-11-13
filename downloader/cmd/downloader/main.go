package main

import (
	"downloader/internal/udp"
	"flag"
	"fmt"
	"log"
	"os"
	"os/signal"
	"strings"
	"syscall"

	"github.com/sirupsen/logrus"
)

func signalHandler(sig os.Signal) {
	log.Println("Caught Ctrl+C! Exiting gracefully...")
	// if globalRmm != nil {
	// 	globalRmm.stopAll()
	// 	time.Sleep(500 * time.Millisecond)
	// }
	os.Exit(0)
}

// CustomFormatter formats log entries to match the desired format
type CustomFormatter struct{}

// Format formats the log entry
func (f *CustomFormatter) Format(entry *logrus.Entry) ([]byte, error) {
	// Format timestamp
	timestamp := entry.Time.Format("15:04:05.000000")

	// Format log level (convert to lowercase)
	level := strings.ToUpper(entry.Level.String())

	// Format the log message
	logMessage := fmt.Sprintf("%s - %s - %s\n", timestamp, level, entry.Message)
	return []byte(logMessage), nil
}
func main() {
	// Define command line arguments
	list := flag.Bool("list", false, "List files on RMM")
	// fileFlag := flag.String("file", "", "The URL of the file to download.")
	// allFlag := flag.Bool("all", false, "Download all files on the RMM")
	// destFlag := flag.String("dest", "", "The local path to save the downloaded file.")
	// ignoreFlag := flag.String("ignore", "", "A list file numbers to ignore")
	verboseFlag := flag.Bool("verbose", false, "Print verbose output")

	flag.Parse()

	// Set logging level
	logrus.SetFormatter(&CustomFormatter{})

	if *verboseFlag {
		logrus.SetLevel(logrus.DebugLevel)
	} else {
		logrus.SetLevel(logrus.InfoLevel)
	}

	logrus.Debug("Starting UDP RMM")

	// rmm := &Rmm{}
	// globalRmm = rmm // For stopping rx threads
	// rmm.waitForReady()
	rmm := udp.NewRmm()
	rmm.Start()

	// Search for RMM
	rmmFound := rmm.Search()
	if !rmmFound {
		log.Fatal("RMM was not found")
	}

	if *list {
		rmm.ReadContents()
		rmm.PrintFiles()
	}

	// } else if *allFlag && *destFlag != "" {
	// 	rmm.readContents()
	// 	rmm.printFiles()

	// 	// Convert the ignore list into an array of integers
	// 	var ignoreList []int
	// 	if *ignoreFlag != "" {
	// 		for _, numStr := range strings.Split(*ignoreFlag, ",") {
	// 			num, err := strconv.Atoi(numStr)
	// 			if err == nil {
	// 				ignoreList = append(ignoreList, num)
	// 			}
	// 		}
	// 	}

	// 	logLevel.Printf("Starting download of all files to: %s\n", *destFlag)
	// 	logLevel.Printf("Ignoring files: %v\n", ignoreList)

	// 	for _, file := range rmm.files {
	// 		fileNum, err := strconv.Atoi(file.name[strings.LastIndex(file.name, "File")+4:])
	// 		if err != nil {
	// 			continue
	// 		}
	// 		// Skip ignored files
	// 		shouldExclude := false
	// 		for _, ignored := range ignoreList {
	// 			if fileNum == ignored {
	// 				shouldExclude = true
	// 				break
	// 			}
	// 		}
	// 		if shouldExclude {
	// 			continue
	// 		}

	// 		logLevel.Printf("Downloading file %s\n", file.name)
	// 		rmm.download(file.name, *destFlag)

	// 		// Wait for download to finish
	// 		for rmm.tasksPending() {
	// 			time.Sleep(200 * time.Millisecond)
	// 		}
	// 	}

	// 	logLevel.Printf("Downloaded %d files to %s\n", len(rmm.files), *destFlag)

	// } else if *fileFlag != "" && *destFlag != "" {
	// 	rmm.download(*fileFlag, *destFlag)
	// } else {
	// 	log.Fatal("Invalid arguments")
	// }

	// // Wait for tasks to end
	// for rmm.tasksPending() {
	// 	time.Sleep(200 * time.Millisecond)
	// }

	// rmm.stopAll()
	// rmm.waitForThreads()
}

func init() {
	// Set up signal handling
	c := make(chan os.Signal, 1)
	signal.Notify(c, syscall.SIGINT, syscall.SIGTERM)
	go func() {
		for sig := range c {
			signalHandler(sig)
		}
	}()
}
