// Package settlers defines the core gameplay loop.
package settlers

// Faction models a tribe or nation.
type Faction struct {
	// Number of people not assigned to a specific settlement.
	Freemen int64
}

// GameState holds the state of a specific game.
type GameState struct {
	Factions  []*Faction
	Templates []*Template
	Board     *Board
}
