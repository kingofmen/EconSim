// Package settlers defines the core gameplay loop.
package settlers

import (
	"fmt"
)

type Activity int

const (
	CONSUME = iota // Eat, drink, and be merry; all that feeds the body.
	MEANING        // Ritual, sport, family, worship; all that makes life worth living.
	PRODUCE        // Investment, building, the creation of tools.
	MILITIA        // Drill, weapons, training, patrol; the defense of the other three.
)

// Faction models a tribe or nation.
type Faction struct {
	// Number of people not assigned to a specific settlement.
	Freemen int64
}

// Template contains constant settlement data.
type Template struct {
	people int64
}

// Settlement contains variable production data.
type Settlement struct {
	kind       *Template
	faction    *Faction
	priorities [4]int16
	people     int64
}

func (s *Settlement) Populate() {
	if s == nil {
		return
	}
	if s.faction == nil {
		return
	}

	want := s.kind.people - s.people
	if s.faction.Freemen < want {
		want = s.faction.Freemen
	}
	s.people += want
	s.faction.Freemen -= want
}

// Prioritize calculates the production fractions.
func (s *Settlement) Prioritize() {
	if s == nil {
		return
	}
	// TODO: Replace this placeholder stub with an actual calculation!
	s.priorities[CONSUME] = 250
	s.priorities[MEANING] = 250
	s.priorities[PRODUCE] = 250
	s.priorities[MILITIA] = 250
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
	if m == nil {
		return fmt.Errorf("Tick called on nil Map")
	}

	for _, s := range m.settlements {
		s.Prioritize()
		s.Populate()
	}

	return nil
}
