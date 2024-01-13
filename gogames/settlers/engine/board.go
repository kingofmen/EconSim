package settlers

import (
	"fmt"

	"gogames/tiles/triangles"
	"gogames/util/coords"
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
func (m *Board) Place(pos coords.Point, fac *Faction, tmp *Template) []error {
	if m == nil {
		return []error{fmt.Errorf("piece placed on nil Board")}
	}
	bad := make([]error, 0, len(tmp.required))
	for _, rule := range tmp.required {
		if err := rule.Allow(pos, fac, m); err != nil {
			bad = append(bad, err)
		}
	}
	if len(bad) > 0 {
		return bad
	}

	m.settlements = append(m.settlements, NewPiece(tmp, fac))
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
