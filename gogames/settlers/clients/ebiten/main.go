// Package main defines the dev-client binary.
package main

import (
	"fmt"
	"image"
	"image/color"
	"log"
	"math"
	"os"
	"path/filepath"

	"gogames/settlers/ai/cog"
	"gogames/settlers/content/loader"
	"gogames/settlers/engine/settlers"
	"gogames/tiles/triangles"
	"gogames/util/dna"
	"gogames/util/vector2d"

	"github.com/hajimehoshi/ebiten/v2"
	"github.com/hajimehoshi/ebiten/v2/ebitenutil"
	"github.com/hajimehoshi/ebiten/v2/inpututil"
)

const (
	toRadians = math.Pi / 180
	SixtyD    = 60 * toRadians
	root3half = 0.86602540378
	root3     = 1.73205080757

	edgeLength    = 100
	invEdgeLength = 0.01
	bdrWidth      = edgeLength / 4
	height        = 87

	kTerrainOnly    = "terrain_display"
	kFactionColor   = "faction_color_overlay"
	kFactionBorders = "faction_borders_overlay"
)

var (
	colorRed   = color.RGBA{R: 255, A: 255}
	colorGreen = color.RGBA{G: 255, A: 255}
	colorBlue  = color.RGBA{B: 255, A: 255}

	// uiState contains information needed for all components.
	uiState *commonState

	debug = true
)

// commonState contains global UI information.
type commonState struct {
	// currTmpl is the selected Template.
	currTmpl *settlers.Template
	// game contains the board and other game information.
	game *settlers.GameState
	// mouseDn stores the last position where the mouse was clicked.
	mouseDn image.Point
	// turnOver is true when the player has done his action.
	turnOver bool
	// rotation is the amount to rotate the next tile placement.
	rotation int
	// keys contains the currently pressed keys.
	keys map[ebiten.Key]bool
	// playerIndex is the currently active faction.
	playerIndex int
	// turnSignal receives end-of-turn notifications.
	turnSignal chan int
	// overlayButton points to the state button for overlays.
	// TODO: This should probably be a listener for state changes instead.
	overlayButton *stateButton
	// factions contains the players, human and AI.
	factions []*factionState
}

// currFaction returns the faction whose turn it is.
func (cs *commonState) currFaction() *factionState {
	if cs == nil {
		return nil
	}
	if cs.playerIndex >= len(cs.factions) {
		// Indicates we are running a tick.
		return nil
	}
	return cs.factions[cs.playerIndex]
}

// factionState contains state information about the game's factions.
type factionState struct {
	faction      *settlers.Faction
	human        bool
	displayName  string
	displayColor color.RGBA
	decider      settlers.Decider
}

func (f *factionState) isHuman() bool {
	if f == nil {
		return false
	}
	return f.human
}

type activable interface {
	isActive() bool
}

type drawable interface {
	activable
	draw(*ebiten.Image)
}

type handler interface {
	activable
	image.Image
	handleLeftClick()
}

type tileDrawFunc func(*ebiten.Image, *settlers.Tile)

// sliceToTriangle makes all pixels outside the standard triangle
// fully transparent.
func sliceToTriangle(img *ebiten.Image) {
	for y := 0; y < height; y++ {
		for x := 0; x < edgeLength/2; x++ {
			if float64(height-y) < root3*float64(x) {
				continue
			}
			img.Set(x, y, color.Transparent)
			img.Set(edgeLength-x, y, color.Transparent)
		}
	}
}

// alphaScale returns the input color multiplied by the given factor.
// TODO: Safetyize this and put it in a separate color-utility library.
func alphaScale(col color.RGBA, alpha float64) color.RGBA {
	return color.RGBA{
		R: uint8(alpha * float64(col.R)),
		G: uint8(alpha * float64(col.G)),
		B: uint8(alpha * float64(col.B)),
		A: uint8(alpha * float64(col.A)),
	}
}

// loadImg loads an ebiten Image.
func loadImg(base, fname string) (*ebiten.Image, error) {
	fpath := filepath.Join(base, "gfx", fname)
	img, _, err := ebitenutil.NewImageFromFile(fpath)
	return img, err
}

// stateButton is a button whose state changes upon clicking.
type stateButton struct {
	idx    int
	states []string
	loc    *image.Rectangle
	images []*ebiten.Image
}

func (sb *stateButton) getState() string {
	return sb.states[sb.idx]
}

func (sb *stateButton) increment() {
	if sb == nil {
		return
	}
	sb.idx++
	if sb.idx >= len(sb.states) {
		sb.idx = 0
	}
}

// layout packages the screen areas.
type layout struct {
	board    *boardComponent
	tiles    *tilesComponent
	display  *displayComponent
	draws    []drawable
	handlers []handler
}

type Game struct {
	// opts contains the transform and other options of the game.
	opts *ebiten.DrawImageOptions
	// areas contains the rectangles used to divide up the screen for displays.
	areas layout
}

func (g *Game) handleLeftClick() {
	for _, h := range g.areas.handlers {
		if !h.isActive() {
			continue
		}
		if !uiState.mouseDn.In(h.Bounds()) {
			continue
		}
		h.handleLeftClick()
		break
	}
}

func (g *Game) handleRightClick() {
	pos := image.Pt(ebiten.CursorPosition())
	if pos.In(g.areas.tiles.Bounds()) {
		return
	}
	amt := 1
	if uiState.keys[ebiten.KeyShiftLeft] || uiState.keys[ebiten.KeyShiftRight] {
		amt = -1
	}
	uiState.rotation += amt
}

func (g *Game) Update() error {
	// Always quit if told.
	if inpututil.IsKeyJustReleased(ebiten.KeyQ) {
		close(uiState.turnSignal)
		return ebiten.Termination
	}

	// Otherwise ignore player input if it's not the player's turn.
	if !uiState.currFaction().isHuman() {
		// We're waiting for AI processing.
		return nil
	}

	pressed := make([]ebiten.Key, 0, 5)
	pressed = inpututil.AppendPressedKeys(pressed)
	clear(uiState.keys)
	for _, k := range pressed {
		uiState.keys[k] = true
	}

	if inpututil.IsMouseButtonJustPressed(ebiten.MouseButtonLeft) {
		uiState.mouseDn = image.Pt(ebiten.CursorPosition())
	}
	if inpututil.IsMouseButtonJustReleased(ebiten.MouseButtonLeft) {
		g.handleLeftClick()
	}
	if inpututil.IsMouseButtonJustReleased(ebiten.MouseButtonRight) {
		g.handleRightClick()
	}

	if !uiState.turnOver {
		return nil
	}
	uiState.turnOver = false
	uiState.turnSignal <- uiState.playerIndex
	return nil
}

// trackTurns awaits turn-over signals from the players.
func (g *Game) trackTurns() {
	for pidx := range uiState.turnSignal {
		uiState.playerIndex = pidx + 1
		if uiState.playerIndex >= len(uiState.factions) {
			if err := uiState.game.RunTick(); err != nil {
				fmt.Printf("Board tick error: %v\n", err)
			}
			uiState.playerIndex = 0
		}
		go aiProcessing(uiState.game, uiState.currFaction())
	}
}

// getDeadBrain returns a brain-dead, unimplemented AI.
func getDeadBrain(n string) settlers.DeciderFunc {
	return func(gs *settlers.GameState, f *settlers.Faction) error {
		return fmt.Errorf("%s faction AI was not initialised, no move made.", n)
	}
}

// aiProcessing runs the player-input calculation. It is a placeholder.
// TODO: Write an actual AI library and call out to it.
func aiProcessing(state *settlers.GameState, fac *factionState) {
	if fac.isHuman() {
		fmt.Printf("Starting turn of human player %q\n", fac.displayName)
		return
	}

	fmt.Printf("Starting turn of AI player %q\n", fac.displayName)
	if fac.decider == nil {
		fac.decider = getDeadBrain(fac.displayName)
	}
	if err := fac.decider.Decide(uiState.game, fac.faction); err != nil {
		fmt.Printf("AI error, skipping %s turn: %v\n", fac.displayName, err)
	}
	uiState.turnSignal <- uiState.playerIndex
}

func drawRectangle(target *ebiten.Image, r *image.Rectangle, fill, edge color.Color, width float64) {
	x, y, dx, dy := float64(r.Min.X), float64(r.Min.Y), float64(r.Dx()), float64(r.Dy())
	ebitenutil.DrawRect(target, x, y, dx, dy, edge)
	ebitenutil.DrawRect(target, x+width, y+width, dx-2*width, dy-2*width, fill)
}

func (g *Game) Draw(screen *ebiten.Image) {
	for _, d := range g.areas.draws {
		if !d.isActive() {
			continue
		}
		d.draw(screen)
	}
}

func (g *Game) Layout(outsideWidth, outsideHeight int) (screenWidth, screenHeight int) {
	return 640, 480
}

func (g *Game) initGraphics() error {
	g.opts = &ebiten.DrawImageOptions{
		GeoM: ebiten.GeoM{},
	}
	g.areas.board.initTriangles()
	g.areas.board.initOverlays()
	if err := g.areas.board.initTemplates(); err != nil {
		return err
	}
	if err := g.areas.display.initButtons(); err != nil {
		return err
	}
	g.areas.tiles.initThumbs()
	return nil
}

func makeRect(x0, y0, x1, y1 int) *image.Rectangle {
	rect := image.Rect(x0, y0, x1, y1)
	return &rect
}

func makeLayout() layout {
	l := layout{
		board: &boardComponent{
			component: component{
				Rectangle: makeRect(110, 0, 640, 480),
				active:    true,
				gopts: &ebiten.DrawImageOptions{
					GeoM: ebiten.GeoM{},
				},
			},
			offset:        vector2d.New(250, 180),
			pieceTris:     make(map[string][]*ebiten.Image),
			pieceVtxs:     make(map[string][]*ebiten.Image),
			pieceNums:     make(map[triangles.TriPoint]map[string]int),
			facColOverlay: make(map[dna.Sequence]*ebiten.Image),
			facBdrOverlay: make(map[dna.Sequence]*ebiten.Image),
		},
		tiles: &tilesComponent{
			component: component{
				Rectangle: makeRect(0, 0, 110, 400),
				active:    true,
				gopts: &ebiten.DrawImageOptions{
					GeoM: ebiten.GeoM{},
				},
			},
			shapes: make(map[string]*ebiten.Image),
		},
		display: &displayComponent{
			component: component{
				Rectangle: makeRect(0, 400, 110, 480),
				active:    true,
				gopts: &ebiten.DrawImageOptions{
					GeoM: ebiten.GeoM{},
				},
			},
			factions: &stateButton{
				states: []string{kTerrainOnly, kFactionColor, kFactionBorders},
				images: make([]*ebiten.Image, 3),
				loc:    makeRect(5, 405, 25, 425),
			},
		},
		draws:    make([]drawable, 3),
		handlers: make([]handler, 3),
	}
	l.draws[0] = l.board
	l.draws[1] = l.tiles
	l.draws[2] = l.display
	// Note that handle order differs.
	l.handlers[0] = l.tiles
	l.handlers[1] = l.display
	l.handlers[2] = l.board
	return l
}

// devFactions returns default factions for development.
func devFactions() []*factionState {
	return []*factionState{
		&factionState{
			faction:      settlers.NewFaction("player01"),
			human:        true,
			displayColor: color.RGBA{R: 135, G: 51, B: 55, A: 255},
			displayName:  "Red",
		},
		&factionState{
			faction:      settlers.NewFaction("abcd1234"),
			displayName:  "Blue",
			displayColor: color.RGBA{R: 65, G: 105, B: 225, A: 255},
			decider:      cog.NewRandom(),
		},
	}
}

func main() {
	ebiten.SetWindowSize(640, 480)
	ebiten.SetWindowTitle("Settlers Dev Client")

	board, err := settlers.NewHex(4)
	if err != nil {
		log.Fatal(err)
	}
	game := &Game{
		areas: makeLayout(),
	}

	wd, err := os.Getwd()
	if err != nil {
		log.Fatal(err)
	}

	webs, err := loader.Webs(filepath.Join(wd, "data", "webs", "*.pb.txt"))
	if err != nil {
		log.Fatal(err)
	}
	buckets, err := loader.Buckets(filepath.Join(wd, "data", "buckets", "*.pb.txt"))
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("Loaded %d webs, %d buckets.\n", len(webs), len(buckets))

	facStates := devFactions()
	uiState = &commonState{
		game: &settlers.GameState{
			Factions:  make([]*settlers.Faction, len(facStates)),
			Board:     board,
			Templates: settlers.DevTemplates(),
			Webs:      webs,
			Buckets:   buckets,
		},
		mouseDn:       image.Pt(0, 0),
		rotation:      0,
		keys:          make(map[ebiten.Key]bool),
		playerIndex:   0,
		turnSignal:    make(chan int),
		overlayButton: game.areas.display.factions,
		factions:      facStates,
	}
	for i, fs := range uiState.factions {
		uiState.game.Factions[i] = fs.faction
	}

	if err := game.initGraphics(); err != nil {
		log.Fatalf("Error initialising graphics: %v", err)
	}
	fmt.Println("Starting game.")
	go game.trackTurns()
	if err := ebiten.RunGame(game); err != nil {
		log.Fatal(err)
	}
	fmt.Println("Goodbye!")
}
