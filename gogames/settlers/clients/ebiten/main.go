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
	handleClick()
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
	// upTri and dnTri are graphical triangles.
	upTri *ebiten.Image
	dnTri *ebiten.Image
	// offset is the view position for the map.
	offset vector2d.Vector
	// pieces contains the images to use for templates.
	pieces map[string]*ebiten.Image
}

func (bc *boardComponent) draw(screen *ebiten.Image) {
	drawRectangle(screen, bc.Rectangle, color.Black, color.White, 1)
	for _, tile := range uiState.game.Board.Tiles {
		bc.gopts.GeoM.Reset()
		x, y, w, h := tile.Bounds()
		cx, cy := 0.0, 0.0
		if tile.Points(triangles.South) {
			bc.gopts.GeoM.Rotate(toRadians * 180.0)
			cx += w * edgeLength
			cy += h * edgeLength
		}
		x *= edgeLength
		y *= -edgeLength // Triangles library has upwards y coordinate.
		bc.gopts.GeoM.Translate(x+bc.offset.X()+cx, y+bc.offset.Y()+cy)
		screen.DrawImage(bc.upTri, bc.gopts)
		for _, p := range tile.Pieces() {
			if img := bc.pieces[p.GetKey()]; img != nil {
				screen.DrawImage(img, bc.gopts)
			}
		}
	}

	for _, pt := range uiState.game.Board.Points {
		pieces := pt.Pieces()
		if len(pieces) < 1 {
			continue
		}
		bc.gopts.GeoM.Reset()
		x, y := pt.GetTriPoint().XY()
		x *= edgeLength
		y *= -edgeLength // Triangles library has upwards y coordinate.
		bc.gopts.GeoM.Translate(x+bc.offset.X()-10, y+bc.offset.Y()-10)
		for _, p := range pieces {
			if img := bc.pieces[p.GetKey()]; img != nil {
				screen.DrawImage(img, bc.gopts)
			}
		}
	}

	pos := image.Pt(ebiten.CursorPosition())
	targetTile := bc.getTileAt(pos)
	if targetTile == nil || uiState.currTmpl == nil {
		ebiten.SetCursorShape(ebiten.CursorShapeDefault)
	} else {
		// TODO: Cache this information to avoid the every-frame evaluation.
		if errs := uiState.game.Board.CheckShape(targetTile.GetTriPoint(), nil, uiState.currTmpl); len(errs) > 0 {
			ebiten.SetCursorShape(ebiten.CursorShapeNotAllowed)
		} else {
			ebiten.SetCursorShape(ebiten.CursorShapeCrosshair)
		}
	}
}

// getTileAt returns the tile containing the given point in the window.
func (bc *boardComponent) getTileAt(pos image.Point) *settlers.Tile {
	// TODO: Check that we're actually in the board component and not, e.g.,
	// the tile display.
	// Note that coordinates are relative to game window, not board component.
	coord := vector2d.New(float64(pos.X)-bc.offset.X(), bc.offset.Y()-float64(pos.Y)).Mul(invEdgeLength)
	target := triangles.FromXY(coord.XY())
	return uiState.game.Board.GetTile(target)
}

func (bc *boardComponent) handleClick() {
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
	if errs := uiState.game.Board.Place(target.GetTriPoint(), nil, uiState.currTmpl); len(errs) > 0 {
		for _, err := range errs {
			fmt.Printf("%s/%s: %v\n", target.GetTriPoint(), uiState.currTmpl.Key(), err)
		}
	} else {
		uiState.turnOver = true
	}
}

func (bc *boardComponent) initTriangles() {
	line := ebiten.NewImage(edgeLength, 1)
	line.Fill(color.White)

	bc.dnTri = ebiten.NewImage(edgeLength, height)
	bc.gopts.GeoM.Reset()
	bc.dnTri.DrawImage(line, bc.gopts)

	bc.gopts.GeoM.Reset()
	bc.gopts.GeoM.Rotate(SixtyD)
	bc.dnTri.DrawImage(line, bc.gopts)

	bc.gopts.GeoM.Reset()
	bc.gopts.GeoM.Rotate(2 * SixtyD)
	bc.gopts.GeoM.Translate(edgeLength-1, 0)
	bc.dnTri.DrawImage(line, bc.gopts)

	bc.upTri = ebiten.NewImage(edgeLength, height)
	bc.gopts.GeoM.Reset()
	bc.gopts.GeoM.Translate(0, height-1)
	bc.upTri.DrawImage(line, bc.gopts)

	bc.gopts.GeoM.Reset()
	bc.gopts.GeoM.Rotate(-SixtyD)
	bc.gopts.GeoM.Translate(0, height-1)
	bc.upTri.DrawImage(line, bc.gopts)

	bc.gopts.GeoM.Reset()
	bc.gopts.GeoM.Rotate(-2 * SixtyD)
	bc.gopts.GeoM.Translate(edgeLength-1, height-1)
	bc.upTri.DrawImage(line, bc.gopts)
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
	bc.pieces[key] = img
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
	bc.pieces["temple"] = ebiten.NewImage(20, 20)
	bnds := bc.pieces["temple"].Bounds()
	drawRectangle(bc.pieces["temple"], &bnds, color.Black, colorRed, 2)
	bc.pieces["tower"] = ebiten.NewImage(20, 20)
	bnds = bc.pieces["tower"].Bounds()
	drawRectangle(bc.pieces["tower"], &bnds, color.Black, colorBlue, 2)
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

func (tc *tilesComponent) handleClick() {
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
	total    *boardComponent
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

func (g *Game) handleClick() {
	for _, h := range g.areas.handlers {
		if !h.isActive() {
			continue
		}
		if !uiState.mouseDn.In(h.Bounds()) {
			continue
		}
		h.handleClick()
		break
	}
}

func (g *Game) Update() error {
	if inpututil.IsKeyJustReleased(ebiten.KeyQ) {
		return ebiten.Termination
	}

	if inpututil.IsMouseButtonJustPressed(ebiten.MouseButtonLeft) {
		uiState.mouseDn = image.Pt(ebiten.CursorPosition())
	}
	if inpututil.IsMouseButtonJustReleased(ebiten.MouseButtonLeft) {
		g.handleClick()
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
	return g.areas.total.Dx(), g.areas.total.Dy()
}

func (g *Game) initGraphics() error {
	g.opts = &ebiten.DrawImageOptions{
		GeoM: ebiten.GeoM{},
	}
	g.areas.total.initTriangles()
	if err := g.areas.total.initTemplates(); err != nil {
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
		total: &boardComponent{
			component: component{
				Rectangle: makeRect(0, 0, 640, 480),
				active:    true,
				gopts: &ebiten.DrawImageOptions{
					GeoM: ebiten.GeoM{},
				},
			},
			offset: vector2d.New(250, 180),
			pieces: make(map[string]*ebiten.Image),
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
	l.draws[0] = l.total
	l.draws[1] = l.tiles
	// Note that handle order differs.
	l.handlers[0] = l.tiles
	l.handlers[1] = l.total
	return l
}

func main() {
	ebiten.SetWindowSize(640, 480)
	ebiten.SetWindowTitle("Settlers Dev Client")

	board, err := settlers.NewHex(1)
	if err != nil {
		log.Fatal(err)
	}

	uiState = &commonState{
		game: &settlers.GameState{
			Board:     board,
			Templates: settlers.DevTemplates(),
		},
		mouseDn: image.Pt(0, 0),
	}

	game := &Game{
		areas: makeLayout(),
	}
	if err := game.initGraphics(); err != nil {
		log.Fatalf("Error initialising graphics: %v")
	}
	fmt.Println("Starting game.")
	if err := ebiten.RunGame(game); err != nil {
		log.Fatal(err)
	}
	fmt.Println("Goodbye!")
}
