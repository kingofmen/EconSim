// Package settlers defines the core gameplay loop.
package settlers

// Faction models a tribe or nation.
type Faction struct {
	// Number of people not assigned to a specific settlement.
	Freemen int64
}

// Settlement contains the production logic.
type Settlement struct {
}

// Map contains the game board.
type Map struct {
	settlements []*Settlement
}

// Place puts a Settlement onto the map.
func (m *Map) Place() error {
	return nil
}

// Tick runs all economic and military logic for one timestep.
func (m *Map) Tick() error {
	return nil
}
