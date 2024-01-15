package settlers

import (
	"fmt"

	"gogames/tiles/triangles"
)

// Board contains the game board.
type Board struct {
	Tiles       []*triangles.Surface
	tileMap     map[triangles.TriPoint]*triangles.Surface
	settlements []*Piece
}

// NewHex returns a six-sided board of radius r.
func NewHex(r int) (*Board, error) {
	if r < 1 {
		return nil, fmt.Errorf("hex board of radius %d would be empty", r)
	}
	b := &Board{
		// Hex consists of 6 large triangles; hence the number is 6
		// times the nth triangular number.
		Tiles:       make([]*triangles.Surface, 0, 3*r*(r+1)),
		tileMap:     make(map[triangles.TriPoint]*triangles.Surface),
		settlements: make([]*Piece, 0, 10),
	}
	for warp := r; warp > -r; warp-- {
		for weft := r; weft > -r; weft-- {
			for hex := r; hex > -r; hex-- {
				s, err := triangles.New(warp, weft, hex)
				if err != nil {
					// Invalid tripoint.
					continue
				}
				b.Tiles = append(b.Tiles, s)
				b.tileMap[s.GetTriPoint()] = s
			}
		}
	}

	if err := triangles.Tile(b.Tiles...); err != nil {
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

		for _, allow := range tmp.shape.faceRules {
			if err := allow.Triangle(start, fac); err != nil {
				bad = append(bad, err)
			}
		}
		for _, allow := range face.rules {
			if err := allow.Triangle(start, fac); err != nil {
				bad = append(bad, err)
			}
		}
		for _, vert := range face.verts {
			vtx := start.GetVertex(vert.dir)
			if vtx == nil {
				bad = append(bad, fmt.Errorf("cannot apply vertex rules to nil vertex (%s -> %s)", coord, vert.dir))
				continue
			}
			for _, allow := range tmp.shape.vertRules {
				if err := allow.Vertex(vtx, fac); err != nil {
					bad = append(bad, err)
				}
			}
			for _, allow := range vert.rules {
				if err := allow.Vertex(vtx, fac); err != nil {
					bad = append(bad, err)
				}
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
