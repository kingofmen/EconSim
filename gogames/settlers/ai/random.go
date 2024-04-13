// Package cog contains AI implementations.
package cog

import (
	"errors"
	"fmt"
	"math/rand"

	"gogames/settlers/engine/settlers"
	"gogames/tiles/triangles"
)

// move stores a legal move.
type move struct {
	pos   triangles.TriPoint
	tmpl  *settlers.Template
	turns settlers.Rotate
}

// NewRandom returns a randomly-moving AI.
// This is intended only for development!
func NewRandom() settlers.DeciderFunc {
	return func(gs *settlers.GameState, f *settlers.Faction) error {
		moves := make([]move, 0, len(gs.Board.Tiles))
		for _, tile := range gs.Board.Tiles {
			for _, tmpl := range gs.Templates {
				for _, r := range []settlers.Rotate{settlers.None, settlers.Once, settlers.Twice} {
					if errs := gs.Board.CheckShape(tile.GetTriPoint(), f, tmpl, r); len(errs) > 0 {
						continue
					}
					moves = append(moves, move{tile.GetTriPoint(), tmpl, r})
				}
			}
		}

		if len(moves) == 0 {
			return fmt.Errorf("no legal moves")
		}

		rnd := rand.Int() % len(moves)
		action := moves[rnd]
		return errors.Join(gs.Board.Place(action.pos, f, action.tmpl, action.turns)...)
	}
}
