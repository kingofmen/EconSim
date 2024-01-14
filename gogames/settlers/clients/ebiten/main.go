package main

import (
	"image/color"
	"log"
	"math"

	"gogames/settlers/engine/settlers"
	"gogames/tiles/triangles"

	"github.com/hajimehoshi/ebiten/v2"
	_ "github.com/hajimehoshi/ebiten/v2/ebitenutil"
)

const (
	toRadians = math.Pi / 180
	SixtyD    = 60 * toRadians

	edgeLength = 100
	height     = 87
)

type Game struct {
	board *settlers.Board
	upTri *ebiten.Image
	dnTri *ebiten.Image
	opts  *ebiten.DrawImageOptions
	count float64
}

func (g *Game) Update() error {
	g.count++
	if g.count > 100 {
		g.count = 0
	}
	return nil
}

func (g *Game) Draw(screen *ebiten.Image) {
	for _, tile := range g.board.Tiles {
		g.opts.GeoM.Reset()
		x, y := tile.XY()
		x *= edgeLength
		y *= -edgeLength // Triangles library has upwards y coordinate.
		g.opts.GeoM.Translate(x+100, y+80)
		if tile.Points(triangles.North) {
			screen.DrawImage(g.upTri, g.opts)
		} else {
			screen.DrawImage(g.dnTri, g.opts)
		}
	}
}

func (g *Game) Layout(outsideWidth, outsideHeight int) (screenWidth, screenHeight int) {
	return 320, 240
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
		board: board,
		count: 0,
	}
	game.initTriangle()
	if err := ebiten.RunGame(game); err != nil {
		log.Fatal(err)
	}
}
