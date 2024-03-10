package main

import (
	"fmt"
	"image"
	"image/color"
	"log"
	"math"
	"os"
	"path/filepath"

	"gogames/settlers/engine/settlers"
	"gogames/tiles/triangles"
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
	height        = 87
)

var (
	colorRed   = color.RGBA{R: 255}
	colorGreen = color.RGBA{G: 255}
	colorBlue  = color.RGBA{B: 255}

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

// component is a part of the UI.
type component struct {
	*image.Rectangle
	active bool
	gopts  *ebiten.DrawImageOptions
}

func (c *component) isActive() bool {
	return c.active
}

type boardComponent struct {
	component
	// baseTri is the base, unfilled triangle tile.
	baseTri *ebiten.Image
	// offset is the view position for the map.
	offset vector2d.Vector
	// pieceTris contains the triangle images to use for templates.
	pieceTris map[string][]*ebiten.Image
	// pieceVtxs contains the vertex images to use for templates.
	pieceVtxs map[string][]*ebiten.Image
	// pieceNums contains the image indices for each tile.
	pieceNums map[triangles.TriPoint]map[string]int
}

// optsForTile translates the graphics matrix to the tile position.
func (bc *boardComponent) optsForTile(tp triangles.TriPoint) {
	bc.gopts.GeoM.Reset()
	x, y, w, h := tp.Bounds()
	cx, cy := 0.0, 0.0
	if tp.Points(triangles.South) {
		bc.gopts.GeoM.Rotate(toRadians * 180.0)
		cx += w * edgeLength
		cy += h * edgeLength
	}
	x *= edgeLength
	y *= -edgeLength // Triangles library has upwards y coordinate.
	bc.gopts.GeoM.Translate(float64(bc.Min.X)+x+bc.offset.X()+cx, y+bc.offset.Y()+cy)
}

func (bc *boardComponent) optsForVertex(tp triangles.TriPoint) {
	bc.gopts.GeoM.Reset()
	x, y := tp.XY()
	x *= edgeLength
	y *= -edgeLength // Triangles library has upwards y coordinate.
	bc.gopts.GeoM.Translate(float64(bc.Min.X)+x+bc.offset.X()-10, y+bc.offset.Y()-10)
}

func (bc *boardComponent) draw(screen *ebiten.Image) {
	drawRectangle(screen, bc.Rectangle, color.Black, color.White, 1)
	for _, tile := range uiState.game.Board.Tiles {
		bc.optsForTile(tile.GetTriPoint())
		screen.DrawImage(bc.baseTri, bc.gopts)
		pnums := bc.pieceNums[tile.GetTriPoint()]
		for _, p := range tile.Pieces() {
			idx := pnums[p.GetKey()]
			imgs := bc.pieceTris[p.GetKey()]
			if idx >= len(imgs) {
				continue
			}
			if img := imgs[idx]; img != nil {
				screen.DrawImage(img, bc.gopts)
			}
		}
	}

	for _, pt := range uiState.game.Board.Points {
		pieces := pt.Pieces()
		if len(pieces) < 1 {
			continue
		}
		bc.optsForVertex(pt.GetTriPoint())
		pnums := bc.pieceNums[pt.GetTriPoint()]
		for _, p := range pieces {
			idx := pnums[p.GetKey()]
			imgs := bc.pieceVtxs[p.GetKey()]
			if idx >= len(imgs) {
				continue
			}
			if img := imgs[idx]; img != nil {
				screen.DrawImage(img, bc.gopts)
			}
		}
	}

	ebiten.SetCursorShape(ebiten.CursorShapeDefault)
	pos := image.Pt(ebiten.CursorPosition())
	if !pos.In(bc.Bounds()) {
		return
	}
	targetTile := bc.getTileAt(pos)
	if targetTile == nil || uiState.currTmpl == nil {
		return
	}
	// TODO: Cache this information to avoid the every-frame evaluation.
	// Also reuse it below instead of recalculating the rotations.
	if errs := uiState.game.Board.CheckShape(targetTile.GetTriPoint(), nil, uiState.currTmpl, settlers.Turns(uiState.rotation)); len(errs) > 0 {
		ebiten.SetCursorShape(ebiten.CursorShapeNotAllowed)
	} else {
		ebiten.SetCursorShape(ebiten.CursorShapeCrosshair)
	}

	pTris, pVtxs := bc.pieceTris[uiState.currTmpl.Key()], bc.pieceVtxs[uiState.currTmpl.Key()]
	bc.gopts.ColorScale.ScaleAlpha(0.5)
	tris, verts := uiState.currTmpl.OccupiesFrom(targetTile.GetTriPoint(), settlers.Turns(uiState.rotation))
	for idx, tri := range tris[:len(pTris)] {
		bc.optsForTile(tri)
		screen.DrawImage(pTris[idx], bc.gopts)
	}
	for idx, vert := range verts[:len(pVtxs)] {
		bc.optsForVertex(vert)
		screen.DrawImage(pVtxs[idx], bc.gopts)
	}
	bc.gopts.ColorScale.Reset()
}

// getTileAt returns the tile containing the given point in the window.
func (bc *boardComponent) getTileAt(pos image.Point) *settlers.Tile {
	// TODO: Check that we're actually in the board component and not, e.g.,
	// the tile display.
	// Note that coordinates are relative to game window, not board component.
	bPos := pos.Sub(bc.Min)
	coord := vector2d.New(float64(bPos.X)-bc.offset.X(), bc.offset.Y()-float64(bPos.Y)).Mul(invEdgeLength)
	target := triangles.FromXY(coord.XY())
	return uiState.game.Board.GetTile(target)
}

func (bc *boardComponent) handleLeftClick() {
	pos := image.Pt(ebiten.CursorPosition())
	diff := pos.Sub(uiState.mouseDn)
	if diff.X*diff.X+diff.Y*diff.Y > 4 {
		// Click and drag.
		bc.offset.AddInt(diff.X, diff.Y)
		return
	}

	target := bc.getTileAt(pos)
	if target == nil {
		return
	}
	if uiState.currTmpl == nil {
		return
	}
	if errs := uiState.game.Board.Place(target.GetTriPoint(), nil, uiState.currTmpl, settlers.Turns(uiState.rotation)); len(errs) > 0 {
		for _, err := range errs {
			fmt.Printf("%s/%s: %v\n", target.GetTriPoint(), uiState.currTmpl.Key(), err)
		}
	} else {
		tris, verts := uiState.currTmpl.OccupiesFrom(target.GetTriPoint(), settlers.Turns(uiState.rotation))
		for idx, tri := range tris {
			bc.pieceNums[tri][uiState.currTmpl.Key()] = idx
		}
		for idx, vert := range verts {
			bc.pieceNums[vert][uiState.currTmpl.Key()] = idx
		}

		uiState.turnOver = true
	}
}

func (bc *boardComponent) initTriangles() {
	line := ebiten.NewImage(edgeLength, 1)
	line.Fill(color.White)

	bc.baseTri = ebiten.NewImage(edgeLength, height)
	bc.gopts.GeoM.Reset()
	bc.gopts.GeoM.Translate(0, height-1)
	bc.baseTri.DrawImage(line, bc.gopts)

	bc.gopts.GeoM.Reset()
	bc.gopts.GeoM.Rotate(-SixtyD)
	bc.gopts.GeoM.Translate(0, height-1)
	bc.baseTri.DrawImage(line, bc.gopts)

	bc.gopts.GeoM.Reset()
	bc.gopts.GeoM.Rotate(-2 * SixtyD)
	bc.gopts.GeoM.Translate(edgeLength-1, height-1)
	bc.baseTri.DrawImage(line, bc.gopts)

	for _, tile := range uiState.game.Board.Tiles {
		bc.pieceNums[tile.GetTriPoint()] = make(map[string]int)
	}
	for _, pt := range uiState.game.Board.Points {
		bc.pieceNums[pt.GetTriPoint()] = make(map[string]int)
	}
}

func (bc *boardComponent) loadTriangleImage(key, base, fname string) error {
	fpath := filepath.Join(base, "gfx", fname)
	img, _, err := ebitenutil.NewImageFromFile(fpath)
	if err != nil {
		return err
	}
	for y := 0; y < height; y++ {
		for x := 0; x < edgeLength/2; x++ {
			if float64(height-y) < root3*float64(x) {
				continue
			}
			img.Set(x, y, color.Transparent)
			img.Set(edgeLength-x, y, color.Transparent)
		}
	}
	bc.pieceTris[key] = append(bc.pieceTris[key], img)
	return nil
}

func (bc *boardComponent) loadVertexImage(key, base, fname string) error {
	fpath := filepath.Join(base, "gfx", fname)
	img, _, err := ebitenutil.NewImageFromFile(fpath)
	if err != nil {
		return err
	}
	img.Set(0, 0, color.Transparent)
	img.Set(0, 1, color.Transparent)
	img.Set(1, 0, color.Transparent)
	img.Set(19, 19, color.Transparent)
	img.Set(18, 19, color.Transparent)
	img.Set(19, 18, color.Transparent)
	img.Set(19, 0, color.Transparent)
	img.Set(19, 1, color.Transparent)
	img.Set(18, 0, color.Transparent)
	img.Set(0, 19, color.Transparent)
	img.Set(0, 18, color.Transparent)
	img.Set(1, 19, color.Transparent)
	bc.pieceVtxs[key] = append(bc.pieceVtxs[key], img)
	return nil
}

func (bc *boardComponent) initTemplates() error {
	wd, err := os.Getwd()
	if err != nil {
		return err
	}
	if err := bc.loadTriangleImage("village", wd, "ManorVillage.png"); err != nil {
		return err
	}
	if err := bc.loadVertexImage("temple", wd, "WoodenChurch.png"); err != nil {
		return err
	}
	if err := bc.loadVertexImage("tower", wd, "WatchTower.png"); err != nil {
		return err
	}
	if err := bc.loadTriangleImage("outfields", wd, "Outfields1.png"); err != nil {
		return err
	}
	if err := bc.loadTriangleImage("outfields", wd, "Outfields2.png"); err != nil {
		return err
	}
	if err := bc.loadTriangleImage("outfields", wd, "Outfields3.png"); err != nil {
		return err
	}
	return nil
}

type tilesComponent struct {
	component
	// shapes contains thumbnails for the known templates.
	shapes map[string]*ebiten.Image
	// subs are the locations to draw the thumbnails.
	subs []image.Rectangle
}

func (tc *tilesComponent) draw(screen *ebiten.Image) {
	drawRectangle(screen, tc.Rectangle, color.Black, color.White, 1)
	for i, tmpl := range uiState.game.Templates {
		thumb, ok := tc.shapes[tmpl.Key()]
		if !ok {
			continue
		}
		tc.gopts.GeoM.Reset()
		tc.gopts.GeoM.Translate(float64(tc.subs[i].Min.X), float64(tc.subs[i].Min.Y))
		screen.DrawImage(thumb, tc.gopts)
	}
}

func (tc *tilesComponent) handleLeftClick() {
	pos := image.Pt(ebiten.CursorPosition())
	for i, tmpl := range uiState.game.Templates {
		if !pos.In(tc.subs[i]) {
			continue
		}
		uiState.currTmpl = tmpl
		break
	}
}

func (tc *tilesComponent) initThumbs() {
	for count, tmpl := range uiState.game.Templates {
		thumb := ebiten.NewImage(30, 30)
		rect := thumb.Bounds()
		drawRectangle(thumb, &rect, color.Black, color.White, 1)
		tris, verts := tmpl.Occupies()
		for _, tri := range tris {
			c := triangles.TriPoint{tri.A(), tri.B(), tri.C() + 1}
			x, y := c.XY()
			x *= 4.7
			y *= -4.7
			x += 15
			y += 15
			if c.Points(triangles.North) {
			} else {
				thumb.Set(int(x+2.5), int(y-1.5), color.White)
				thumb.Set(int(x+1.5), int(y-1.5), color.White)
				thumb.Set(int(x+0.5), int(y-1.5), color.White)
				thumb.Set(int(x-0.5), int(y-1.5), color.White)
				thumb.Set(int(x-1.5), int(y-1.5), color.White)

				thumb.Set(int(x+1.5), int(y-0.5), color.White)
				thumb.Set(int(x+0.5), int(y-0.5), color.White)
				thumb.Set(int(x-0.5), int(y-0.5), color.White)

				thumb.Set(int(x+0.5), int(y+0.5), color.White)

				thumb.Set(int(x+0.5), int(y+1.5), color.White)
			}
		}
		for _, vert := range verts {
			c := triangles.TriPoint{vert.A(), vert.B(), vert.C() + 1}
			x, y := c.XY()
			x *= 4.7
			y *= -4.7
			x += 15
			y += 15
			thumb.Set(int(x+0.5), int(y+0.5), colorRed)
		}
		tc.shapes[tmpl.Key()] = thumb
		x, y := count%3, count/3
		tc.subs = append(tc.subs, image.Rectangle{image.Pt(5+35*x, 5+35*y), image.Pt(35+35*x, 35+35*y)})
	}
}

// layout packages the screen areas.
type layout struct {
	board    *boardComponent
	tiles    *tilesComponent
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
	if inpututil.IsKeyJustReleased(ebiten.KeyQ) {
		return ebiten.Termination
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

	if uiState.turnOver {
		if err := uiState.game.Board.Tick(); err != nil {
			fmt.Printf("Board tick error: %v\n", err)
		}
		uiState.turnOver = false
	}
	return nil
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
	if err := g.areas.board.initTemplates(); err != nil {
		return err
	}
	g.areas.tiles.initThumbs()
	return nil
}

func makeRect(x, y, w, h int) *image.Rectangle {
	rect := image.Rect(x, y, w, h)
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
			offset:    vector2d.New(250, 180),
			pieceTris: make(map[string][]*ebiten.Image),
			pieceVtxs: make(map[string][]*ebiten.Image),
			pieceNums: make(map[triangles.TriPoint]map[string]int),
		},
		tiles: &tilesComponent{
			component: component{
				Rectangle: makeRect(0, 0, 110, 480),
				active:    true,
				gopts: &ebiten.DrawImageOptions{
					GeoM: ebiten.GeoM{},
				},
			},
			shapes: make(map[string]*ebiten.Image),
		},
		draws:    make([]drawable, 2),
		handlers: make([]handler, 2),
	}
	l.draws[0] = l.board
	l.draws[1] = l.tiles
	// Note that handle order differs.
	l.handlers[0] = l.tiles
	l.handlers[1] = l.board
	return l
}

func main() {
	ebiten.SetWindowSize(640, 480)
	ebiten.SetWindowTitle("Settlers Dev Client")

	board, err := settlers.NewHex(4)
	if err != nil {
		log.Fatal(err)
	}

	uiState = &commonState{
		game: &settlers.GameState{
			Board:     board,
			Templates: settlers.DevTemplates(),
		},
		mouseDn:  image.Pt(0, 0),
		rotation: 0,
		keys:     make(map[ebiten.Key]bool),
	}

	game := &Game{
		areas: makeLayout(),
	}
	if err := game.initGraphics(); err != nil {
		log.Fatalf("Error initialising graphics: %v", err)
	}
	fmt.Println("Starting game.")
	if err := ebiten.RunGame(game); err != nil {
		log.Fatal(err)
	}
	fmt.Println("Goodbye!")
}
