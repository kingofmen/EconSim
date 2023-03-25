// Package clientlib is a CLI interface to the Raubgraf engine.
// It is intended for development and debugging.
package clientlib

import (
	"errors"
	"fmt"
	"path/filepath"
	"strconv"
	"strings"

	"gogames/raubgraf/engine/handler"
	"gogames/util/coords"
)

type InputFormat int

const (
	width          = 256
	height         = 64
	msgLines       = 8
	vtxSepX        = 52
	vtxSepY        = 20
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

type startHandler struct{}
type loadGameHandler struct {
	fileList []string
	page     int
}

type gameHandler struct {
	gameID      int
	viewOffsetX int
	viewOffsetY int
	selectedVtx coords.Point
}

func newGameHandler(gid int) Handler {
	return &gameHandler{
		gameID:      gid,
		viewOffsetX: 0,
		viewOffsetY: 0,
		selectedVtx: coords.Nil(),
	}
}

func newLoadGameHandler() Handler {
	lgh := &loadGameHandler{
		fileList: make([]string, 0, 10),
		page:     0,
	}
	var err error
	dir := filepath.FromSlash("C:/Users/rolfa/Documents/Raubgraf/*prototext")
	lgh.fileList, err = filepath.Glob(dir)
	if err != nil {
		messagef("Could not patternise %q", dir)
		return StartScreenHandler()
	}
	return lgh
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

func showf(x, y int, format string, args ...interface{}) {
	show(x, y, fmt.Sprintf(format, args...))
}

func (h *startHandler) Format() InputFormat {
	return IFChar
}

func (h *gameHandler) Format() InputFormat {
	return IFChar
}

func (h *loadGameHandler) Format() InputFormat {
	return IFChar
}

func (h *gameHandler) drawVertex(row, col int, rEdge bool) {
	yloc := viewPortBottom - (row * vtxSepY) - h.viewOffsetY
	xloc := col*vtxSepX + (row%2)*(vtxSepX/2) + h.viewOffsetX
	show(xloc-2, yloc-2, ".....")
	show(xloc-3, yloc-1, ".     .")
	show(xloc-3, yloc-0, ".     .")
	show(xloc-3, yloc+1, ".     .")
	show(xloc-2, yloc+2, ".....")
	if h.selectedVtx.X() == col && h.selectedVtx.Y() == row {
		show(xloc, yloc, ".")
	}
	if col > 0 {
		show(xloc-vtxSepX+4, yloc, strings.Repeat("_", vtxSepX-7))
	}
	odd := row%2 == 1
	if row > 0 {
		for idx := 0; idx < vtxSepY-3; idx++ {
			if col > 0 || odd {
				show(xloc-5-idx, yloc+2+idx, "/")
			}
			if !rEdge || !odd {
				show(xloc+5+idx, yloc+2+idx, "\\")
			}
		}
	}
}

// Display draws the board state into the buffer.
func (h *gameHandler) Display() {
	clear()
	board, err := handler.GetGameState(h.gameID)
	if err != nil {
		messagef("Error getting game state %d: %v", h.gameID, err)
	}
	height, width := int(board.GetHeight()), int(board.GetWidth())
	if h.selectedVtx.IsNil() {
		h.selectedVtx = coords.New(height/2, width/2)
	}
	if h.selectedVtx.X() < 0 {
		h.selectedVtx[0] = 0
	}
	if h.selectedVtx.X() >= width {
		h.selectedVtx[0] = width - 1
	}
	if h.selectedVtx.Y() < 0 {
		h.selectedVtx[1] = 0
	}
	if h.selectedVtx.Y() >= height {
		h.selectedVtx[1] = height - 1
	}
	h.calcOffsets()

	for row := 0; row < height; row++ {
		for col := 0; col < width; col++ {
			h.drawVertex(row, col, col == width-1)
		}
	}
	printMessages()
}

// calcOffsets recalculates the viewport offsets.
func (h *gameHandler) calcOffsets() {
	h.viewOffsetX = width/2 - h.selectedVtx.X()*vtxSepX
	h.viewOffsetY = height/2 - h.selectedVtx.Y()*vtxSepY
}

func (h *loadGameHandler) Display() {
	clear()
	show(100, 20, "(p)rev, (n)ext, (b)ack")
	defer printMessages()
	if len(h.fileList) < 1 {
		show(104, 22, "No files found")
		return
	}
	start := h.page * 10
	end := start + 10
	if end > len(h.fileList) {
		end = len(h.fileList)
	}

	showf(101, 22, "Page %d / %d", h.page+1, h.numPages()+1)
	for idx, f := range h.fileList[start:end] {
		showf(101, 24+idx, "%d) %s", idx, f)
	}
}

// numPages calculates the number of pages of save files.
func (h *loadGameHandler) numPages() int {
	return (len(h.fileList) - 1) / 10
}

// Parse handles the input string, selecting the file to load.
func (h *loadGameHandler) Parse(inp string) (Handler, error) {
	if selected, err := strconv.Atoi(inp); err == nil {
		// TODO: Actually load and parse the file!
		messagef("Selected file %d", selected)
		return h, nil
	}
	switch strings.ToLower(inp) {
	case "b":
		return StartScreenHandler(), nil
	case "n":
		h.page++
	case "p":
		h.page--
	}
	if h.page < 0 {
		h.page = 0
	}
	if pages := h.numPages(); h.page > pages {
		h.page = pages
	}
	return h, nil
}

// Parse handles the input string.
func (h *gameHandler) Parse(inp string) (Handler, error) {
	switch strings.ToLower(inp) {
	case "q":
		// Quit.
		return h, QuitSignal
	case "\r":
	// TODO: Next turn.
	case "a":
		h.selectedVtx[0] -= 1
	case "d":
		h.selectedVtx[0] += 1
	case "s":
		h.selectedVtx[1] -= 1
	case "w":
		h.selectedVtx[1] += 1
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
	lines[34] = strings.Repeat(" ", 100) + "(L)oad game"

	for idx := range lines {
		ll := len(lines[idx])
		if ll >= width {
			lines[idx] = lines[idx][:width]
			continue
		}
		lines[idx] += strings.Repeat(" ", width-ll)
	}
	printMessages()
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
		return newLoadGameHandler(), nil
	}

	return h, nil
}

func StartScreenHandler() Handler {
	lines = make([]string, height)
	return &startHandler{}
}
