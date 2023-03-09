// Package main defines the binary for the dev client.
package main

import (
	//"flag"
	"fmt"
	"log"
	"os"
	//"gogames/raubgraf/clients/devclient"
	"golang.org/x/sys/windows"
)

func main() {
	log.Printf("Hello, dev client!")
	var outMode uint32
	out := windows.Handle(os.Stdout.Fd())
	if err := windows.GetConsoleMode(out, &outMode); err == nil {
		outMode |= windows.ENABLE_PROCESSED_OUTPUT | windows.ENABLE_VIRTUAL_TERMINAL_PROCESSING
		_ = windows.SetConsoleMode(out, outMode)
	}
	fmt.Printf("\033[H")
	fmt.Printf("Bwah-hah.")
	os.Exit(0)
}
