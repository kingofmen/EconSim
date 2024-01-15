package settlers

import (
	"fmt"

	"gogames/tiles/triangles"
)

type Tile struct {
	*triangles.Surface
}

// Board contains the game board.
type Board struct {
	Tiles       []*Tile
	tileMap     map[triangles.TriPoint]*Tile
	settlements []*Piece
}

// NewHex returns a six-sided board of radius r.
func NewHex(r int) (*Board, error) {
	if r < 1 {
		return nil, fmt.Errorf("hex board of radius %d would be empty", r)
	}
	b := &Board{
		// Hex consists of 6 large triangles; hence the number of tiles is 6
		// times the nth triangular number.
		Tiles:       make([]*Tile, 0, 3*r*(r+1)),
		tileMap:     make(map[triangles.TriPoint]*Tile),
		settlements: make([]*Piece, 0, 10),
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
				tile := &Tile{s}
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

// Place puts a Piece onto the board.
func (bd *Board) Place(pos triangles.TriPoint, fac *Faction, tmp *Template) []error {
	if bd == nil {
		return []error{fmt.Errorf("piece placed on nil Board")}
	}

	bad := make([]error, 0, len(tmp.shape.faces))
	for _, face := range tmp.shape.faces {
		coord := triangles.Sum(pos, face.pos)
		start, ok := bd.tileMap[coord]
		if !ok {
			bad = append(bad, fmt.Errorf("required position %s not on the board", coord))
			continue
		}

		for _, rule := range tmp.shape.faceRules {
			if err := rule.Allow(start, fac); err != nil {
				bad = append(bad, err)
			}
		}
		for _, rule := range face.rules {
			if err := rule.Allow(start, fac); err != nil {
				bad = append(bad, err)
			}
		}

		vertices := make([]*triangles.Vertex, 0, len(face.verts))
		for _, vert := range face.verts {
			vtx := start.GetVertex(vert.dir)
			if vtx == nil {
				bad = append(bad, fmt.Errorf("cannot apply vertex rules to nil vertex (%s -> %s)", coord, vert.dir))
				continue
			}
			vertices = append(vertices, vtx)
			for _, rule := range vert.rules {
				if err := rule.Allow(start, fac, vtx); err != nil {
					bad = append(bad, err)
				}
			}
		}
		for _, rule := range tmp.shape.vertRules {
			if err := rule.Allow(start, fac, vertices...); err != nil {
				bad = append(bad, err)
			}
		}
		// TODO: Edges.
	}

	if len(bad) > 0 {
		return bad
	}

	bd.settlements = append(bd.settlements, NewPiece(tmp, fac))
	return nil
}

// Tick runs all economic and military logic for one timestep.
func (m *Board) Tick() error {
	if m == nil {
		return fmt.Errorf("Tick called on nil Board")
	}

	for _, s := range m.settlements {
		s.Prioritize()
		s.Populate()
		s.Produce()
	}

	return nil
}
