package settlers

import (
	"fmt"

	"gogames/tiles/triangles"
)

type Tile struct {
	*triangles.Surface
	facePieces []*Piece
	vtxPieces  map[*triangles.Vertex][]*Piece
}

// Board contains the game board.
type Board struct {
	Tiles   []*Tile
	tileMap map[triangles.TriPoint]*Tile
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
					Surface:    s,
					facePieces: make([]*Piece, 0, 3),
					vtxPieces:  make(map[*triangles.Vertex][]*Piece),
				}
				b.Tiles = append(b.Tiles, tile)
				b.tileMap[s.GetTriPoint()] = tile
			}
		}
	}

	if err := triangles.Tile(surfaces...); err != nil {
		return nil, err
	}
	return b, nil
}

// checkShape returns any rules conflicts of placing the piece.
func (bd *Board) checkShape(pos triangles.TriPoint, fac *Faction, tmp *Template) []error {
	bad := make([]error, 0, len(tmp.shape.faces))
	for _, face := range tmp.shape.faces {
		coord := triangles.Sum(pos, face.pos)
		tile, ok := bd.tileMap[coord]
		if !ok {
			bad = append(bad, fmt.Errorf("required position %s not on the board", coord))
			continue
		}

		for _, rule := range tmp.shape.faceRules {
			if err := rule.Allow(tile, fac); err != nil {
				bad = append(bad, err)
			}
		}
		for _, rule := range face.rules {
			if err := rule.Allow(tile, fac); err != nil {
				bad = append(bad, err)
			}
		}

		vertices := make([]*triangles.Vertex, 0, len(face.verts))
		for _, vert := range face.verts {
			vtx := tile.GetVertex(vert.dir)
			if vtx == nil {
				bad = append(bad, fmt.Errorf("cannot apply vertex rules to nil vertex (%s -> %s)", coord, vert.dir))
				continue
			}
			vertices = append(vertices, vtx)
			for _, rule := range vert.rules {
				if err := rule.Allow(tile, fac, vtx); err != nil {
					bad = append(bad, err)
				}
			}
		}
		for _, rule := range tmp.shape.vertRules {
			if err := rule.Allow(tile, fac, vertices...); err != nil {
				bad = append(bad, err)
			}
		}
		// TODO: Edges.
	}

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
		coord := triangles.Sum(pos, face.pos)
		tile := bd.tileMap[coord]
		tile.facePieces = append(tile.facePieces, p)
		for _, vert := range face.verts {
			vtx := tile.GetVertex(vert.dir)
			tile.vtxPieces[vtx] = append(tile.vtxPieces[vtx], p)
		}
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
