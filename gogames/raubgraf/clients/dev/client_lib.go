// Package clientlib is a CLI interface to the Raubgraf engine.
// It is intended for development and debugging.
package clientlib

import (
	"errors"
	"fmt"
	"strconv"
	"strings"

	"gogames/raubgraf/engine/handler"
)

type InputFormat int

const (
	width          = 256
	height         = 64
	msgLines       = 8
	vtxSep         = 60
	viewPortBottom = height - msgLines

	IFString InputFormat = iota
	IFChar
)

var (
	QuitSignal = errors.New("quit")
	lines      []string
	messages   []string
)

type Handler interface {
	Display()
	Parse(inp string) (Handler, error)
	Format() InputFormat
}

type startHandler struct {
}

type gameHandler struct {
	gameID      int
	viewOffsetX int
	viewOffsetY int
}

func newGameHandler(gid int) *gameHandler {
	return &gameHandler{
		gameID:      gid,
		viewOffsetX: 0,
		viewOffsetY: 0,
	}
}

func clear() {
	for idx := range lines {
		lines[idx] = strings.Repeat(" ", width)
	}
}

// message adds a string to the message log.
func message(msg string) {
	if len(messages) < msgLines {
		messages = append(messages, msg)
		return
	}

	// Don't append-and-reslice here as it may extend the
	// underlying array indefinitely.
	for idx := 0; idx < msgLines-1; idx++ {
		messages[idx] = messages[idx+1]
	}
	messages[msgLines-1] = msg
}

// messagef adds a formatted message to the log.
func messagef(format string, args ...interface{}) {
	message(fmt.Sprintf(format, args...))
}

// Flip prints the display lines to screen.
func Flip() {
	fmt.Printf("\033[H")
	for _, l := range lines {
		fmt.Println(l)
	}
}

// printMessages blits the message log into the display lines.
func printMessages() {
	for idx, msg := range messages {
		lines[height-msgLines+idx] = fmt.Sprintf("%s%s", msg, strings.Repeat(" ", width-len(msg)))
	}
}

// show inserts 'val' at (x, y).
func show(x, y int, val string) {
	if x+len(val) < 0 || y < 0 {
		return
	}
	if x >= width || y >= height-msgLines {
		return
	}
	if x < 0 {
		val = val[-x:]
		x = 0
	}
	if x+len(val) > width {
		val = val[:width-x]
	}
	lines[y] = lines[y][:x] + val + lines[y][x+len(val):]
}

func (h *startHandler) Format() InputFormat {
	return IFChar
}

func (h *gameHandler) Format() InputFormat {
	return IFChar
}

func (h *gameHandler) drawVertex(row, col int) {
	yloc := viewPortBottom - (row * vtxSep / 3) + h.viewOffsetY
	xloc := col*vtxSep + (row%2)*(vtxSep/2) + h.viewOffsetX
	show(xloc-2, yloc-2, ".....")
	show(xloc-3, yloc-1, ".     .")
	show(xloc-3, yloc-0, ".     .")
	show(xloc-3, yloc+1, ".     .")
	show(xloc-2, yloc+2, ".....")
}

func (h *gameHandler) Display() {
	clear()
	board, err := handler.GetGameState(h.gameID)
	if err != nil {
		messagef("Error getting game state %d: %v", h.gameID, err)
	}
	for row := 0; row < int(board.GetHeight()); row++ {
		for col := 0; col < int(board.GetWidth()); col++ {
			h.drawVertex(row, col)
		}
	}
	printMessages()
}

func (h *gameHandler) Parse(inp string) (Handler, error) {
	switch strings.ToLower(inp) {
	case "q":
		// Quit.
		return h, QuitSignal
	case "\r":
	// TODO: Next turn.
	case "a":
		h.viewOffsetX += 1
	case "d":
		h.viewOffsetX -= 1
	case "s":
		h.viewOffsetY += 1
	case "w":
		h.viewOffsetY -= 1
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
		gid, err := handler.CreateGame(size, size)
		if err != nil {
			return h, err
		}
		return newGameHandler(gid), nil
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
