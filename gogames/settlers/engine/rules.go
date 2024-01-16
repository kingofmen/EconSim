package settlers

import (
	"fmt"

	"gogames/tiles/triangles"
)

// Rule models a requirement for a piece to be placed.
type Rule interface {
	Allow(bd *Board, fac *Faction, pos ...triangles.TriPoint) []error
}

type EmptyRule struct{}

func (r *EmptyRule) Allow(bd *Board, fac *Faction, pos ...triangles.TriPoint) []error {
	errs := make([]error, 0, len(pos))
	for _, coord := range pos {
		pieces, err := bd.GetPieces(coord)
		if err != nil {
			errs = append(errs, err)
			continue
		}
		if len(pieces) > 0 {
			errs = append(errs, fmt.Errorf("position %s is not empty", coord))
			continue
		}
	}
	return errs
}
