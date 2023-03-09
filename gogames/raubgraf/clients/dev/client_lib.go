// Package clientlib is a CLI interface to the Raubgraf engine.
// It is intended for development and debugging.
package clientlib

import (
	"fmt"
)

type Handler interface {
	Display()
	Parse(inp string) (Handler, error)
}

type startHandler struct {
}

func (h *startHandler) Display() {
	fmt.Printf("\033[H")
}

func (h *startHandler) Parse(inp string) (Handler, error) {
	if inp == "quit" {
		return h, fmt.Errorf("Quit command received")
	}
	return h, nil
}

func StartScreenHandler() Handler {
	return &startHandler{}
}
