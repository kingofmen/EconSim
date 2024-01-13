package main

import (
	"image/color"
	"log"

	"gogames/settlers/engine/settlers"

	"github.com/hajimehoshi/ebiten/v2"
	_ "github.com/hajimehoshi/ebiten/v2/ebitenutil"
)

type Game struct {
	board   *settlers.Board
	testImg *ebiten.Image
	opts    *ebiten.DrawImageOptions
	count   float64
}

func (g *Game) Update() error {
	g.count++
	if g.count > 100 {
		g.count = 0
		g.opts.GeoM.Reset()
	}
	return nil
}

func (g *Game) Draw(screen *ebiten.Image) {
	g.opts.GeoM.Translate(g.count, g.count)
	screen.DrawImage(g.testImg, g.opts)
}

func (g *Game) Layout(outsideWidth, outsideHeight int) (screenWidth, screenHeight int) {
	return 320, 240
}

func main() {
	ebiten.SetWindowSize(640, 480)
	ebiten.SetWindowTitle("Hello, World!")
	board, err := settlers.NewHex(3)
	if err != nil {
		log.Fatal(err)
	}
	game := &Game{
		board:   board,
		testImg: ebiten.NewImage(10, 10),
		opts: &ebiten.DrawImageOptions{
			GeoM: ebiten.GeoM{},
		},
		count: 0,
	}
	game.testImg.Fill(color.White)
	if err := ebiten.RunGame(game); err != nil {
		log.Fatal(err)
	}
}
