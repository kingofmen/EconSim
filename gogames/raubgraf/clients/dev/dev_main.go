// Package main defines the binary for the dev client.
package main

import (
	"bufio"
	"log"
	"os"

	"gogames/raubgraf/clients/dev/clientlib"
	"golang.org/x/sys/windows"
)

func main() {
	var outMode uint32
	out := windows.Handle(os.Stdout.Fd())
	if err := windows.GetConsoleMode(out, &outMode); err == nil {
		outMode |= windows.ENABLE_PROCESSED_OUTPUT | windows.ENABLE_VIRTUAL_TERMINAL_PROCESSING
		_ = windows.SetConsoleMode(out, outMode)
	}

	handler := clientlib.StartScreenHandler()
	scanner := bufio.NewScanner(os.Stdin)
	var err error
	for {
		handler.Display()
		clientlib.Flip()
		scanner.Scan()
		if err := scanner.Err(); err != nil {
			log.Printf("Error during input scan: %v", err)
			break
		}
		inp := scanner.Text()
		handler, err = handler.Parse(inp)
		if err != nil {
			log.Printf("Error parsing input %q: %v", inp, err)
			break
		}
	}

	os.Exit(0)
}
