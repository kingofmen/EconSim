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

// component is a part of the UI.
type component struct {
	*image.Rectangle
	active bool
}

// layout packages the screen areas.
type layout struct {
	total component
	tiles component
}

type Game struct {
	// state contains the game state.
	state *settlers.GameState
	// upTri and dnTri are graphical triangles.
	upTri *ebiten.Image
	dnTri *ebiten.Image
	// shapes contains thumbnails for the known templates.
	shapes map[string]*ebiten.Image
	// opts contains the transform and other options of the current tile.
	opts *ebiten.DrawImageOptions
	// offset is the view position for the map.
	offset vector2d.Vector
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
	g.offset.Add(diff.XY())
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

func (g *Game) drawBoard(screen *ebiten.Image) {
	drawRectangle(screen, g.areas.total.Rectangle, color.Black, color.White, 1)

	for _, tile := range g.state.Board.Tiles {
		g.opts.GeoM.Reset()
		x, y := tile.XY()
		x *= edgeLength
		y *= -edgeLength // Triangles library has upwards y coordinate.
		g.opts.GeoM.Translate(x+g.offset.X(), y+g.offset.Y())
		if tile.Points(triangles.North) {
			screen.DrawImage(g.upTri, g.opts)
		} else {
			screen.DrawImage(g.dnTri, g.opts)
		}
	}
}

func (g *Game) drawTiles(screen *ebiten.Image) {
	drawRectangle(screen, g.areas.tiles.Rectangle, color.Black, color.White, 1)
	count := 0
	for _, tmpl := range g.state.Templates {
		thumb, ok := g.shapes[tmpl.Key()]
		if !ok {
			continue
		}
		x, y := float64(count%3), float64(count/3)
		g.opts.GeoM.Reset()
		g.opts.GeoM.Translate(5+35*x, 5+35*y)
		screen.DrawImage(thumb, g.opts)
		count++
	}
}

func (g *Game) Draw(screen *ebiten.Image) {
	if g.areas.total.active {
		g.drawBoard(screen)
	}
	if g.areas.tiles.active {
		g.drawTiles(screen)
	}
}

func (g *Game) Layout(outsideWidth, outsideHeight int) (screenWidth, screenHeight int) {
	return g.areas.total.Dx(), g.areas.total.Dy()
}

func (g *Game) initTriangle() {
	line := ebiten.NewImage(edgeLength, 1)
	line.Fill(color.White)
	centroidOffset := 28.86751
	extraSpace := 29

	g.opts = &ebiten.DrawImageOptions{
		GeoM: ebiten.GeoM{},
	}
	g.dnTri = ebiten.NewImage(edgeLength, height+extraSpace)
	g.opts.GeoM.Reset()
	g.opts.GeoM.Translate(0, centroidOffset)
	g.dnTri.DrawImage(line, g.opts)

	g.opts.GeoM.Reset()
	g.opts.GeoM.Rotate(SixtyD)
	g.opts.GeoM.Translate(0, centroidOffset)
	g.dnTri.DrawImage(line, g.opts)

	g.opts.GeoM.Reset()
	g.opts.GeoM.Rotate(2 * SixtyD)
	g.opts.GeoM.Translate(edgeLength-1, centroidOffset)
	g.dnTri.DrawImage(line, g.opts)

	centroidOffset = 0
	g.upTri = ebiten.NewImage(edgeLength, height)
	g.opts.GeoM.Reset()
	g.opts.GeoM.Translate(0, height-1)
	g.upTri.DrawImage(line, g.opts)

	g.opts.GeoM.Reset()
	g.opts.GeoM.Rotate(-SixtyD)
	g.opts.GeoM.Translate(0, height-1)
	g.upTri.DrawImage(line, g.opts)

	g.opts.GeoM.Reset()
	g.opts.GeoM.Rotate(-2 * SixtyD)
	g.opts.GeoM.Translate(edgeLength-1, height-1)
	g.upTri.DrawImage(line, g.opts)

	for _, tmpl := range g.state.Templates {
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
		g.shapes[tmpl.Key()] = thumb
	}
}

func makeRect(x, y, w, h int) *image.Rectangle {
	rect := image.Rect(x, y, w, h)
	return &rect
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
		shapes:  make(map[string]*ebiten.Image),
		offset:  vector2d.New(100, 80),
		mouseDn: vector2d.Zero(),
		areas: layout{
			total: component{
				Rectangle: makeRect(0, 0, 640, 480),
				active:    true,
			},
			tiles: component{
				Rectangle: makeRect(0, 0, 110, 480),
				active:    true,
			},
		},
	}
	game.initTriangle()
	if err := ebiten.RunGame(game); err != nil {
		log.Fatal(err)
	}
}
