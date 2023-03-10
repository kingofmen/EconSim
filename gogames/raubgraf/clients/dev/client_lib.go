// Package clientlib is a CLI interface to the Raubgraf engine.
// It is intended for development and debugging.
package clientlib

import (
	"errors"
	"fmt"
	"strconv"
	"strings"

	"gogames/raubgraf/engine/engine"
)

type InputFormat int

const (
	width  = 256
	height = 64

	IFString InputFormat = iota
	IFChar
)

var (
	QuitSignal = errors.New("quit")
	lines      []string
)

type Handler interface {
	Display()
	Parse(inp string) (Handler, error)
	Format() InputFormat
}

type startHandler struct {
}

type gameHandler struct {
	game *engine.RaubgrafGame
}

func clear() {
	for idx := range lines {
		lines[idx] = strings.Repeat(" ", width)
	}
}

func Flip() {
	fmt.Printf("\033[H")
	for _, l := range lines {
		fmt.Println(l)
	}
}

func (h *startHandler) Format() InputFormat {
	return IFChar
}

func (h *gameHandler) Format() InputFormat {
	return IFChar
}

func (h *gameHandler) Display() {
	clear()
}

func (h *gameHandler) Parse(inp string) (Handler, error) {
	if strings.ToLower(inp) == "q" {
		return h, QuitSignal
	}
	return h, nil
}

func (h *startHandler) Display() {
	clear()
	lines[10] = "                RRRRRRRRRR              AAAAAAAAA          UUUU           UUUU  BBBBBBBB          GGGGGGGGG   RRRRRRRRRR             AAAAAAAAA         FFFFFFFFFFFFF"
	lines[11] = "                 RRRRRRRRRR            AAAAAAAAAAA          UUU           UUU    BBBBBBBB        GGGGGGGGGGG   RRRRRRRRRR           AAAAAAAAAAA         FFFFFFFFFFF "
	lines[12] = "                 RRR    RRRR          AAA       AAA         UUU           UUU    BBB   BBB      GGG       GGG  RRR    RRRR         AAA       AAA        FFF         "
	lines[13] = "                 RRR     RRRR        AAA         AAA        UUU           UUU    BBB    BBB    GGG        GGG  RRR     RRRR       AAA         AAA       FFF         "
	lines[14] = "                 RRR     RRRR       AAA           AAA       UUU           UUU    BBB    BBB    GGG             RRR     RRRR      AAA           AAA      FFF         "
	lines[15] = "                 RRR    RRRR        AAA           AAA       UUU           UUU    BBB   BBB     GGG             RRR    RRRR       AAA           AAA      FFF         "
	lines[16] = "                 RRRRRRRRRR         AAA           AAA       UUU           UUU    BBBBBBBB      GGG             RRRRRRRRRR        AAA           AAA      FFFFFFFFF   "
	lines[17] = "                 RRRRRRRR           AAAAAAAAAAAAAAAAA       UUU           UUU    BBBBBBBB      GGG             RRRRRRRR          AAAAAAAAAAAAAAAAA      FFFFFFFF    "
	lines[18] = "                 RRRRRR            AAAAAAAAAAAAAAAAAAA      UUU           UUU    BBB   BBB     GGG             RRRRRR           AAAAAAAAAAAAAAAAAAA     FFF         "
	lines[19] = "                 RRR RRR           AAA             AAA      UUU           UUU    BBB   BBB     GGG     GGGGGG  RRR RRR          AAA             AAA     FFF         "
	lines[20] = "                 RRR  RRR          AAA             AAA      UUU           UUU    BBB    BBB    GGG     GGGGGG  RRR  RRR         AAA             AAA     FFF         "
	lines[21] = "                 RRR   RRR         AAA             AAA      UUU           UUU    BBB    BBB    GGG     G  GGG  RRR   RRR        AAA             AAA     FFF         "
	lines[22] = "                 RRR    RRR       AAA               AAA      UUU         UUU     BBB    BBB    GGG        GGG  RRR    RRR      AAA               AAA    FFF         "
	lines[23] = "                 RRR     RRR      AAA               AAA       UUUUUUUUUUUUU      BBBBBBBBB      GGGGGGGGGGGG   RRR     RRR     AAA               AAA    FFF         "
	lines[24] = "                 RRR      RRR    AAAA               AAAA        UUUUUUUUU       BBBBBBBBB        GGGGGGGGGG    RRR      RRR   AAAA               AAAA   FFF         "

	lines[30] = strings.Repeat(" ", 100) + "(Q)uit"
	lines[31] = strings.Repeat(" ", 100) + "New game (5)x5"
	lines[32] = strings.Repeat(" ", 100) + "New game (1)0x10"
	lines[33] = strings.Repeat(" ", 100) + "New game (2)0x20"
	lines[34] = strings.Repeat(" ", 100) + "(L)oad game (not implemented)"

	for idx := range lines {
		ll := len(lines[idx])
		if ll >= width {
			lines[idx] = lines[idx][:width]
			continue
		}
		lines[idx] += strings.Repeat(" ", width-ll)
	}
}

func (h *startHandler) Parse(inp string) (Handler, error) {
	if strings.ToLower(inp) == "q" {
		return h, QuitSignal
	}
	if inp == "1" || inp == "2" || inp == "5" {
		size, err := strconv.Atoi(inp)
		if err != nil {
			return h, err
		}
		if size < 5 {
			size *= 10
		}
		game, err := engine.NewGame(size, size)
		if err != nil {
			return h, err
		}
		return &gameHandler{game: game}, nil
	}
	if strings.ToLower(inp) == "l" {
		return h, fmt.Errorf("Game loading not implemented.")
	}

	return h, nil
}

func StartScreenHandler() Handler {
	lines = make([]string, height)
	return &startHandler{}
}
