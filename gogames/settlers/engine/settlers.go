// Package settlers defines the core gameplay loop.
package settlers

import (
	"gogames/util/coords"
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

// Rule models a requirement for a piece to be placed.
type Rule interface {
	Allow(pos coords.Point, fac *Faction, board *Board) error
}

// Template contains information shared across a class of game pieces,
// such as shape and production capability.
type Template struct {
	people int64

	required []Rule
}

// Piece is an instantiation of a Template, placed on the board.
type Piece struct {
	kind       *Template
	faction    *Faction
	priorities [4]int16
	people     int64
}

func NewPiece(tmp *Template, fac *Faction) *Piece {
	return &Piece{
		kind:       tmp,
		faction:    fac,
		priorities: [4]int16{250, 250, 250, 250},
		people:     0,
	}
}

func (s *Piece) Populate() {
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
func (s *Piece) Prioritize() {
	if s == nil {
		return
	}
	// TODO: Replace this placeholder stub with an actual calculation!
	s.priorities[CONSUME] = 250
	s.priorities[MEANING] = 250
	s.priorities[PRODUCE] = 250
	s.priorities[MILITIA] = 250
}

func (s *Piece) Produce() {
	if s == nil {
		return
	}
}
