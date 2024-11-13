package main

import (
	"downloader/internal/udp"
	"flag"
	"log"
	"os"
	"os/signal"
	"syscall"
)

func signalHandler(sig os.Signal) {
	log.Println("Caught Ctrl+C! Exiting gracefully...")
	// if globalRmm != nil {
	// 	globalRmm.stopAll()
	// 	time.Sleep(500 * time.Millisecond)
	// }
	os.Exit(0)
}

func main() {
	// Define command line arguments
	// list := flag.Bool("list", false, "List files on RMM")
	// fileFlag := flag.String("file", "", "The URL of the file to download.")
	// allFlag := flag.Bool("all", false, "Download all files on the RMM")
	// destFlag := flag.String("dest", "", "The local path to save the downloaded file.")
	// ignoreFlag := flag.String("ignore", "", "A list file numbers to ignore")
	verboseFlag := flag.Bool("verbose", false, "Print verbose output")

	flag.Parse()

	// Set logging level
	logLevel := log.New(os.Stdout, "", log.LstdFlags)
	if *verboseFlag {
		logLevel.SetFlags(log.LstdFlags | log.Lshortfile) // Show file names and line numbers for debugging
	}

	logLevel.Println("Starting UDP RMM")

	// rmm := &Rmm{}
	// globalRmm = rmm // For stopping rx threads
	// rmm.waitForReady()
	rmm := udp.NewRmm()
	rmm.Start()

	// rmm.SendJumboZeros()
	// rmm.GetIdentity()

	// logLevel.Println("Download files or other logic here")

	// rmm.Stop()

	// Search for RMM
	rmmFound := rmm.Search()
	if !rmmFound {
		log.Fatal("RMM was not found")
	}

	// if *listFlag {
	// 	rmm.readContents()
	// 	rmm.printFiles()

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
