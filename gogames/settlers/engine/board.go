package settlers

import (
	"fmt"

	"gogames/tiles/triangles"
)

type Tile struct {
	*triangles.Surface
	pieces []*Piece
}

type Point struct {
	*triangles.Vertex
	pieces []*Piece
}

// Board contains the game board.
type Board struct {
	Tiles   []*Tile
	tileMap map[triangles.TriPoint]*Tile
	vtxMap  map[triangles.TriPoint]*Point
	pieces  []*Piece
}

// NewHex returns a six-sided board of radius r.
func NewHex(r int) (*Board, error) {
	if r < 1 {
		return nil, fmt.Errorf("hex board of radius %d would be empty", r)
	}
	b := &Board{
		// Hex consists of 6 large triangles; hence the number of tiles is 6
		// times the nth triangular number.
		Tiles:   make([]*Tile, 0, 3*r*(r+1)),
		tileMap: make(map[triangles.TriPoint]*Tile),
		vtxMap:  make(map[triangles.TriPoint]*Point),
		pieces:  make([]*Piece, 0, 10),
	}

	surfaces := make([]*triangles.Surface, 0, len(b.Tiles))
	for warp := r; warp > -r; warp-- {
		for weft := r; weft > -r; weft-- {
			for hex := r; hex > -r; hex-- {
				s, err := triangles.New(warp, weft, hex)
				if err != nil {
					// Invalid tripoint.
					continue
				}
				surfaces = append(surfaces, s)
				tile := &Tile{
					Surface: s,
					pieces:  make([]*Piece, 0, 3),
				}
				b.Tiles = append(b.Tiles, tile)
				b.tileMap[s.GetTriPoint()] = tile
			}
		}
	}

	if err := triangles.Tile(surfaces...); err != nil {
		return nil, err
	}

	// Vertices are created and given positions by Tile.
	for _, tile := range b.Tiles {
		for _, vtx := range tile.GetVertices() {
			point := &Point{
				Vertex: vtx,
				pieces: make([]*Piece, 0, 3),
			}
			b.vtxMap[vtx.GetTriPoint()] = point
		}
	}

	return b, nil
}

// checkShape returns any rules conflicts of placing the piece.
func (bd *Board) checkShape(pos triangles.TriPoint, fac *Faction, tmp *Template) []error {
	bad := make([]error, 0, len(tmp.shape.faces))
	facePos := make([]triangles.TriPoint, 0, len(tmp.shape.faces))
	for _, face := range tmp.shape.faces {
		coord := triangles.Sum(pos, face.pos)
		if _, ok := bd.tileMap[coord]; !ok {
			bad = append(bad, fmt.Errorf("tile position %s does not exist", coord))
			continue
		}
		facePos = append(facePos, coord)
		for _, rule := range face.rules {
			if errs := rule.Allow(bd, fac, coord); len(errs) > 0 {
				bad = append(bad, errs...)
			}
		}
	}
	for _, rule := range tmp.shape.faceRules {
		if errs := rule.Allow(bd, fac, facePos...); len(errs) > 0 {
			bad = append(bad, errs...)
		}
	}

	vertPos := make([]triangles.TriPoint, 0, len(tmp.shape.faces))
	for _, vert := range tmp.shape.verts {
		coord := triangles.Sum(pos, vert.pos)
		if _, ok := bd.vtxMap[coord]; !ok {
			bad = append(bad, fmt.Errorf("vertex position %s does not exist", coord))
			continue
		}
		vertPos = append(vertPos, coord)
		for _, rule := range vert.rules {
			if errs := rule.Allow(bd, fac, coord); len(errs) > 0 {
				bad = append(bad, errs...)
			}
		}
	}
	for _, rule := range tmp.shape.vertRules {
		if errs := rule.Allow(bd, fac, vertPos...); len(errs) > 0 {
			bad = append(bad, errs...)
		}
	}

	// TODO: Edges.

	return bad
}

// Place puts a Piece onto the board.
func (bd *Board) Place(pos triangles.TriPoint, fac *Faction, tmp *Template) []error {
	if bd == nil {
		return []error{fmt.Errorf("piece placed on nil Board")}
	}

	if errors := bd.checkShape(pos, fac, tmp); len(errors) > 0 {
		return errors
	}

	p := NewPiece(tmp, fac)
	bd.pieces = append(bd.pieces, p)
	for _, face := range tmp.shape.faces {
		if !face.occupied {
			continue
		}
		coord := triangles.Sum(pos, face.pos)
		tile := bd.tileMap[coord]
		tile.pieces = append(tile.pieces, p)
	}
	for _, vert := range tmp.shape.verts {
		if !vert.occupied {
			continue
		}
		coord := triangles.Sum(pos, vert.pos)
		point := bd.vtxMap[coord]
		point.pieces = append(point.pieces, p)
	}

	return nil
}

// Tick runs all economic and military logic for one timestep.
func (m *Board) Tick() error {
	if m == nil {
		return fmt.Errorf("Tick called on nil Board")
	}

	for _, s := range m.pieces {
		s.Prioritize()
		s.Populate()
		s.Produce()
	}

	return nil
}

func (bd *Board) GetPieces(loc triangles.TriPoint) ([]*Piece, error) {
	if bd == nil {
		return nil, fmt.Errorf("cannot get pieces from nil Board")
	}
	if tile := bd.tileMap[loc]; tile != nil {
		return tile.pieces, nil
	}
	if point := bd.vtxMap[loc]; point != nil {
		return point.pieces, nil
	}
	return nil, fmt.Errorf("position %s is not on the board", loc)
}
