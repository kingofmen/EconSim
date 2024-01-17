package settlers

import (
	"fmt"

	"gogames/tiles/triangles"
)

// Rule models a requirement for a piece to be placed.
type Rule interface {
	Allow(bd *Board, fac *Faction, pos ...triangles.TriPoint) []error
}

// EmptyRule allows placement if the target is empty.
type EmptyRule struct{}

// Allow implements Rule.
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

// HasRule allows placement if some number of tiles have previously been placed.
type HasRule struct {
	required map[string]int
}

// HasKeys returns a HasRule requiring the provided keys.
func HasKeys(keys ...string) *HasRule {
	ret := &HasRule{
		required: make(map[string]int),
	}
	for _, key := range keys {
		ret.required[key]++
	}
	return ret
}

// Allow implements Rule.
func (r *HasRule) Allow(bd *Board, fac *Faction, pos ...triangles.TriPoint) []error {
	errs := make([]error, 0, len(pos))
	found := make(map[string]int)
	for _, coord := range pos {
		pieces, err := bd.GetPieces(coord)
		if err != nil {
			errs = append(errs, err)
			continue
		}
		for _, piece := range pieces {
			found[piece.GetKey()]++
		}
	}
	for n, c := range r.required {
		if c <= found[n] {
			continue
		}
		errs = append(errs, fmt.Errorf("require %d %s, found %d", c, n, found[n]))
	}

	return errs
}
