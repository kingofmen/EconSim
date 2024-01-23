package main

import (
	//"fmt"
	"image"
	"image/color"
	"log"
	"math"

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

	edgeLength = 100
	height     = 87
)

var (
	colorRed = color.RGBA{R: 255}
)

type drawable interface {
	draw(*settlers.GameState, *ebiten.Image)
	isActive() bool
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
}

func (bc *boardComponent) draw(state *settlers.GameState, screen *ebiten.Image) {
	drawRectangle(screen, bc.Rectangle, color.Black, color.White, 1)

	for _, tile := range state.Board.Tiles {
		bc.gopts.GeoM.Reset()
		x, y := tile.XY()
		x *= edgeLength
		y *= -edgeLength // Triangles library has upwards y coordinate.
		bc.gopts.GeoM.Translate(x+bc.offset.X(), y+bc.offset.Y())
		if tile.Points(triangles.North) {
			screen.DrawImage(bc.upTri, bc.gopts)
		} else {
			screen.DrawImage(bc.dnTri, bc.gopts)
		}
	}
}

func (bc *boardComponent) initTriangles() {
	line := ebiten.NewImage(edgeLength, 1)
	line.Fill(color.White)
	centroidOffset := 28.86751
	extraSpace := 29

	bc.dnTri = ebiten.NewImage(edgeLength, height+extraSpace)
	bc.gopts.GeoM.Reset()
	bc.gopts.GeoM.Translate(0, centroidOffset)
	bc.dnTri.DrawImage(line, bc.gopts)

	bc.gopts.GeoM.Reset()
	bc.gopts.GeoM.Rotate(SixtyD)
	bc.gopts.GeoM.Translate(0, centroidOffset)
	bc.dnTri.DrawImage(line, bc.gopts)

	bc.gopts.GeoM.Reset()
	bc.gopts.GeoM.Rotate(2 * SixtyD)
	bc.gopts.GeoM.Translate(edgeLength-1, centroidOffset)
	bc.dnTri.DrawImage(line, bc.gopts)

	centroidOffset = 0
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

type tilesComponent struct {
	component
	// shapes contains thumbnails for the known templates.
	shapes map[string]*ebiten.Image
}

func (tc *tilesComponent) draw(state *settlers.GameState, screen *ebiten.Image) {
	drawRectangle(screen, tc.Rectangle, color.Black, color.White, 1)
	count := 0
	for _, tmpl := range state.Templates {
		thumb, ok := tc.shapes[tmpl.Key()]
		if !ok {
			continue
		}
		x, y := float64(count%3), float64(count/3)
		tc.gopts.GeoM.Reset()
		tc.gopts.GeoM.Translate(5+35*x, 5+35*y)
		screen.DrawImage(thumb, tc.gopts)
		count++
	}
}

func (tc *tilesComponent) initThumbs(state *settlers.GameState) {
	for _, tmpl := range state.Templates {
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
	}
}

// layout packages the screen areas.
type layout struct {
	total *boardComponent
	tiles *tilesComponent
	draws []drawable
}

type Game struct {
	// state contains the game state.
	state *settlers.GameState
	// opts contains the transform and other options of the current tile.
	opts *ebiten.DrawImageOptions
	// mouseDn stores the last position where the mouse was clicked.
	mouseDn vector2d.Vector
	// areas contains the rectangles used to divide up the screen for displays.
	areas layout
}

func (g *Game) handleClick(dn, up vector2d.Vector) {
	diff := vector2d.Diff(up, dn)
	if diff.Len() < 2 {
		// This is a click, ignore for now.
		return
	}
	// Click and drag.
	g.areas.total.offset.Add(diff.XY())
}

func (g *Game) Update() error {
	if inpututil.IsKeyJustReleased(ebiten.KeyQ) {
		return ebiten.Termination
	}

	if inpututil.IsMouseButtonJustPressed(ebiten.MouseButtonLeft) {
		g.mouseDn.SetInt(ebiten.CursorPosition())
	}
	if inpututil.IsMouseButtonJustReleased(ebiten.MouseButtonLeft) {
		mouseUp := vector2d.Zero()
		mouseUp.SetInt(ebiten.CursorPosition())
		g.handleClick(g.mouseDn, mouseUp)
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
		d.draw(g.state, screen)
	}
}

func (g *Game) Layout(outsideWidth, outsideHeight int) (screenWidth, screenHeight int) {
	return g.areas.total.Dx(), g.areas.total.Dy()
}

func (g *Game) initGraphics() {
	g.opts = &ebiten.DrawImageOptions{
		GeoM: ebiten.GeoM{},
	}
	g.areas.total.initTriangles()
	g.areas.tiles.initThumbs(g.state)
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
			offset: vector2d.New(100, 80),
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
	}
	l.draws = make([]drawable, 2)
	l.draws[0] = l.total
	l.draws[1] = l.tiles
	return l
}

func main() {
	ebiten.SetWindowSize(640, 480)
	ebiten.SetWindowTitle("Settlers Dev Client")

	board, err := settlers.NewHex(1)
	if err != nil {
		log.Fatal(err)
	}

	game := &Game{
		state: &settlers.GameState{
			Board:     board,
			Templates: settlers.DevTemplates(),
		},
		mouseDn: vector2d.Zero(),
		areas:   makeLayout(),
	}
	game.initGraphics()
	if err := ebiten.RunGame(game); err != nil {
		log.Fatal(err)
	}
}
