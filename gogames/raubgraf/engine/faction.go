// Package faction contains objects for keeping track
// of game-element loyalties.
package faction

import (
	"gogames/raubgraf/engine/dna"
)

type FactionState int

const (
	Ghost FactionState = iota
	Computer
	Human
)

// Faction models a clan descended from a male-female pair
// with uniquely identifying Y-chromosome and mitochondrial
// DNA.
type Faction struct {
	*dna.Sequence

	state FactionState
}

// New returns a new Faction.
func New(pat, mat string) *Faction {
	return &Faction{
		Sequence: dna.New(pat, mat),
	}
}

// GetDNA returns the full DNA string of the faction,
// maternal and paternal both.
func (f *Faction) GetDNA() string {
	if f == nil {
		return ""
	}
	return f.Sequence.String()
}

// Command returns true if the Faction is allowed to give orders
// to an entity with the provided DNA.
func (f *Faction) Command(seq *dna.Sequence) bool {
	if f == nil {
		return false
	}
	// Give orders only to patrilineally descended entities.
	return f.Sequence.Match(seq, dna.Paternal)
}

// IsHuman returns true if the faction is being played by a human.
func (f *Faction) IsHuman() bool {
	if f == nil {
		return false
	}
	return f.state == Human
}

// Score returns the contribution weight of an entity with
// the provided DNA to the faction's score.
func (f *Faction) Score(seq *dna.Sequence) int {
	if f == nil {
		return 0
	}
	w := 0
	if f.Sequence.Match(seq, dna.Paternal) {
		w++
	}
	if f.Sequence.Match(seq, dna.Maternal) {
		w++
	}
	return w
}
