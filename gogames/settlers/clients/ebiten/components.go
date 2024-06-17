package main

import (
	"fmt"
	"image"
	"image/color"
	"os"

	"gogames/settlers/engine/settlers"
	"gogames/tiles/triangles"
	"gogames/util/dna"
	"gogames/util/vector2d"

	"github.com/hajimehoshi/ebiten/v2"
	"github.com/hajimehoshi/ebiten/v2/ebitenutil"
	"github.com/hajimehoshi/ebiten/v2/text"
	"golang.org/x/image/font/basicfont"
)

// component is a part of the UI.
type component struct {
	*image.Rectangle
	active bool
	gopts  *ebiten.DrawImageOptions
}

// isActive returns true if the component is currently active.
func (c *component) isActive() bool {
	return c.active
}

// boardComponent displays the triangular tiles and their content.
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
	// facColOverlay contains color overlay triangles for each faction.
	facColOverlay map[dna.Sequence]*ebiten.Image
	// facBdrOverlay contains color overlay edges for each faction.
	facBdrOverlay map[dna.Sequence]*ebiten.Image
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

// drawTilePieces draws the placed pieces on the tile.
func (bc *boardComponent) drawTilePieces(screen *ebiten.Image, tile *settlers.Tile) {
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

// drawFactionColor fills the tile with the owning faction's color.
func (bc *boardComponent) drawFactionColor(screen *ebiten.Image, tile *settlers.Tile) {
	fac := tile.Controller()
	if fac == nil {
		return
	}
	if img := bc.facColOverlay[fac.DNA()]; img != nil {
		screen.DrawImage(img, bc.gopts)
	}
}

// drawFactionBorders draws the pieces and then overlays the faction color at the
// borders with other factions.
// TODO: Better composition of these operations.
func (bc *boardComponent) drawFactionBorders(screen *ebiten.Image, tile *settlers.Tile) {
	bc.drawTilePieces(screen, tile)
	fac := tile.Controller()
	if fac == nil {
		return
	}
	img := bc.facBdrOverlay[fac.DNA()]
	if img == nil {
		return
	}
	geom := bc.gopts.GeoM
	for dir, nbs := range tile.Neighbours() {
		nbt := uiState.game.Board.GetTile(nbs.GetTriPoint())
		if nbt == nil {
			continue
		}
		if f := nbt.Controller(); f.DNA() != fac.DNA() {
			rotate := ebiten.GeoM{}
			switch dir {
			case triangles.NorthEast, triangles.SouthWest:
				rotate.Rotate(-2 * SixtyD)
				rotate.Translate(bdrWidth, 3*height/2)
			case triangles.NorthWest, triangles.SouthEast:
				rotate.Rotate(2 * SixtyD)
				rotate.Translate(edgeLength+bdrWidth, height/2)
			}
			rotate.Concat(geom)
			bc.gopts.GeoM = rotate
			screen.DrawImage(img, bc.gopts)
		}
	}
	bc.gopts.GeoM = geom
}

// drawTiles draws each tile using the provided function.
func (bc *boardComponent) drawTiles(screen *ebiten.Image, tf tileDrawFunc) {
	for _, tile := range uiState.game.Board.Tiles {
		bc.optsForTile(tile.GetTriPoint())
		screen.DrawImage(bc.baseTri, bc.gopts)
		tf(screen, tile)
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
}

// draw draws the game board.
func (bc *boardComponent) draw(screen *ebiten.Image) {
	drawRectangle(screen, bc.Rectangle, color.Black, color.White, 1)
	switch uiState.overlayButton.getState() {
	case kFactionColor:
		bc.drawTiles(screen, bc.drawFactionColor)
	case kFactionBorders:
		bc.drawTiles(screen, bc.drawFactionBorders)
	case kTerrainOnly:
		bc.drawTiles(screen, bc.drawTilePieces)
	default:
		// Do nothing.
	}

	// Now draw transparent candidate piece.
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
	place := settlers.NewPlacement(target.GetTriPoint(), uiState.currTmpl).WithRotation(settlers.Turns(uiState.rotation)).WithFaction(uiState.factions[uiState.playerIndex].faction)
	if errs := uiState.game.Board.Place(place); len(errs) > 0 {
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

// initOverlays creates faction-color images.
// TODO: Use DrawTriangles method of ebiten.Image instead.
func (bc *boardComponent) initOverlays() {
	step := float64(1.0)
	step /= float64(bdrWidth)

	for _, fac := range uiState.factions {
		dna := fac.faction.DNA()
		colOverlay := ebiten.NewImage(edgeLength, height)
		colOverlay.Fill(fac.displayColor)
		sliceToTriangle(colOverlay)
		bc.facColOverlay[dna] = colOverlay

		bdrOverlay := ebiten.NewImage(edgeLength, height)
		for i := 0; i < bdrWidth; i++ {
			col := alphaScale(fac.displayColor, 1.0-step*float64(i))
			ebitenutil.DrawLine(bdrOverlay, 0, float64(height-i), edgeLength, float64(height-i), col)
		}
		sliceToTriangle(bdrOverlay)
		bc.facBdrOverlay[dna] = bdrOverlay
	}
}

func (bc *boardComponent) loadTriangleImage(key, base, fname string) error {
	img, err := loadImg(base, fname)
	if err != nil {
		return err
	}
	sliceToTriangle(img)
	bc.pieceTris[key] = append(bc.pieceTris[key], img)
	return nil
}

func (bc *boardComponent) loadVertexImage(key, base, fname string) error {
	img, err := loadImg(base, fname)
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

// tilesComponent displays buttons showing the different templates.
type tilesComponent struct {
	component
	// shapes contains thumbnails for the known templates.
	shapes map[string]*ebiten.Image
	// subs are the locations to draw the thumbnails.
	subs []image.Rectangle
}

// draw draws the template buttons.
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

// displayComponent contains buttons controlling the board display.
type displayComponent struct {
	component
	// factions controls the faction colour overlays.
	factions *stateButton
}

// draw draws the display-control buttons.
func (dc *displayComponent) draw(screen *ebiten.Image) {
	drawRectangle(screen, dc.Rectangle, color.Black, color.White, 1)
	for _, sb := range []*stateButton{dc.factions} {
		drawRectangle(screen, sb.loc, color.Black, color.White, 1)
		if img := sb.images[sb.idx]; img != nil {
			dc.gopts.GeoM.Reset()
			dc.gopts.GeoM.Translate(float64(sb.loc.Min.X), float64(sb.loc.Min.Y))
			screen.DrawImage(img, dc.gopts)
		}
	}
}

// initButtons loads up the state images.
func (dc *displayComponent) initButtons() error {
	wd, err := os.Getwd()
	if err != nil {
		return err
	}
	terrain, err := loadImg(wd, "terrain.png")
	if err != nil {
		return err
	}
	dc.factions.images[0] = terrain
	facCol, err := loadImg(wd, "factionColor.png")
	if err != nil {
		return err
	}
	dc.factions.images[1] = facCol
	borders, err := loadImg(wd, "borders.png")
	if err != nil {
		return err
	}
	dc.factions.images[2] = borders

	return nil
}

// handleLeftClick sets the overlays in response to clicks.
func (dc *displayComponent) handleLeftClick() {
	pos := image.Pt(ebiten.CursorPosition())
	if !pos.In(dc.Bounds()) {
		return
	}
	for _, sb := range []*stateButton{dc.factions} {
		if !pos.In(*sb.loc) {
			continue
		}
		sb.increment()
		break
	}
}

// infoComponent displays the current state of a selected tile.
type infoComponent struct {
	component
	curr *settlers.Tile
}

// draw displays the tile state.
func (ic *infoComponent) draw(screen *ebiten.Image) {
	if ic == nil {
		return
	}
	drawRectangle(screen, ic.Rectangle, color.Black, color.White, 1)
	text.Draw(screen, "Test", basicfont.Face7x13, 5, 315, color.White)
	if ic.curr == nil {
		return
	}
}

// handleLeftClick does nothing.
func (ic *infoComponent) handleLeftClick() {}
