// Package settlers defines the core gameplay loop.
package settlers

import (
	"gogames/util/dna"
)

// Faction models a tribe or nation.
type Faction struct {
	// Number of people not assigned to a specific settlement.
	Freemen int64

	// uuid is the unique identifying string of the faction.
	uuid dna.Sequence
}

func NewFaction(id string) *Faction {
	return &Faction{
		uuid: dna.FromString(id),
	}
}

func (f *Faction) DNA() dna.Sequence {
	if f == nil {
		return dna.FromString("00000000")
	}
	return f.uuid
}

// GameState holds the state of a specific game.
type GameState struct {
	Factions  []*Faction
	Templates []*Template
	Board     *Board
}
