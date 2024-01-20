package main

import (
	"fmt"
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
	userExit = fmt.Errorf("User exit")
)

type layout struct {
	total image.Rectangle
	tiles image.Rectangle
}

type Game struct {
	board   *settlers.Board
	upTri   *ebiten.Image
	dnTri   *ebiten.Image
	opts    *ebiten.DrawImageOptions
	offset  vector2d.Vector
	mouseDn vector2d.Vector
	areas   layout
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
		return userExit
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

func drawRectangle(target *ebiten.Image, r *image.Rectangle, fill, edge color.Color) {
	x, y, dx, dy := float64(r.Min.X), float64(r.Min.Y), float64(r.Dx()), float64(r.Dy())
	ebitenutil.DrawRect(target, x, y, dx, dy, fill)
	ebitenutil.DrawRect(target, x+1, y+1, dx-2, dy-2, edge)
}

func (g *Game) Draw(screen *ebiten.Image) {
	drawRectangle(screen, &g.areas.total, color.White, color.Black)

	for _, tile := range g.board.Tiles {
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

	drawRectangle(screen, &g.areas.tiles, color.White, color.Black)
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
}

func main() {
	ebiten.SetWindowSize(640, 480)
	ebiten.SetWindowTitle("Settlers Dev Client")

	board, err := settlers.NewHex(1)
	if err != nil {
		log.Fatal(err)
	}
	game := &Game{
		board:   board,
		offset:  vector2d.New(100, 80),
		mouseDn: vector2d.Zero(),
		areas: layout{
			total: image.Rect(0, 0, 640, 480),
			tiles: image.Rect(0, 0, 120, 480),
		},
	}
	game.initTriangle()
	if err := ebiten.RunGame(game); err != nil {
		if err == userExit {
			fmt.Println("Goodbye!")
		} else {
			log.Fatal(err)
		}
	}
}
