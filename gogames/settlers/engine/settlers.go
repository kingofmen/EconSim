// Package settlers defines the core gameplay loop.
package settlers

import (
	"gogames/util/dna"
)

// DeciderFunc makes a move on the board as the given faction.
type DeciderFunc func(gs *GameState, f *Faction) error

func (df DeciderFunc) Decide(gs *GameState, f *Faction) error {
	return df(gs, f)
}

// Faction models a tribe or nation.
type Faction struct {
	// Number of people not assigned to a specific settlement.
	Freemen int64

	// uuid is the unique identifying string of the faction.
	uuid dna.Sequence
}

// NewFaction returns a faction pointer with the given DNA.
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

// Decider is the AI interface.
type Decider interface {
	Decide(gs *GameState, f *Faction) error
}
