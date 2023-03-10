// Package main defines the binary for the dev client.
package main

import (
	"bufio"
	"errors"
	"log"
	"os"

	"gogames/raubgraf/clients/dev/clientlib"
	"golang.org/x/sys/windows"
)

func runGame() error {
	var origOutMode, origInMode uint32
	outHandle := windows.Handle(os.Stdout.Fd())
	if err := windows.GetConsoleMode(outHandle, &origOutMode); err != nil {
		return err
	}
	defer windows.SetConsoleMode(outHandle, origOutMode)
	outMode := origOutMode | windows.ENABLE_PROCESSED_OUTPUT | windows.ENABLE_VIRTUAL_TERMINAL_PROCESSING
	_ = windows.SetConsoleMode(outHandle, outMode)

	inHandle := windows.Handle(os.Stdin.Fd())
	if err := windows.GetConsoleMode(inHandle, &origInMode); err != nil {
		return err
	}
	inMode := origInMode &^ windows.ENABLE_LINE_INPUT
	_ = windows.SetConsoleMode(inHandle, inMode)
	defer windows.SetConsoleMode(inHandle, origInMode)

	handler := clientlib.StartScreenHandler()
	reader := bufio.NewReader(os.Stdin)
	var err error
	for {
		handler.Display()
		clientlib.Flip()
		var input string
		switch handler.Format() {
		case clientlib.IFString:
			input, err = reader.ReadString('\n')
			if err != nil {
				return err
			}
		case clientlib.IFChar:
			c, err := reader.ReadByte()
			if err != nil {
				return err
			}
			input = string(c)
		}
		handler, err = handler.Parse(input)
		if err != nil {
			if errors.Is(err, clientlib.QuitSignal) {
				break
			}
			return err
		}
	}
	return nil
}

func main() {
	if err := runGame(); err != nil {
		log.Printf("Exit due to error: %v", err)
	}
	os.Exit(0)
}
