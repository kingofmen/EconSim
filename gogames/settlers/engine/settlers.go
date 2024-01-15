// Package settlers defines the core gameplay loop.
package settlers

import (
	"gogames/util/coords"
)

// Faction models a tribe or nation.
type Faction struct {
	// Number of people not assigned to a specific settlement.
	Freemen int64
}

// Rule models a requirement for a piece to be placed.
type Rule interface {
	Allow(pos coords.Point, fac *Faction, board *Board) error
}
