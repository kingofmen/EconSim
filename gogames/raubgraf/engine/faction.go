// Package faction contains objects for keeping track
// of game-element loyalties.
package faction

import (
	"fmt"
	"strings"
)

// Faction models a clan descended from a male-female pair
// with uniquely identifying Y-chromosome and mitochondrial
// DNA.
type Faction struct {
	// DNA tags that define the male and female
	// lines of descent.
	ydna  string
	mtdna string
}

// GetDNA returns the full DNA string of the faction,
// maternal and paternal both.
func (f *Faction) GetDNA() string {
	if f == nil {
		return ""
	}
	return MakeDNA(f.ydna, f.mtdna)
}

// MakeDNA returns the paternal and maternal DNA in a single string.
func MakeDNA(pat, mat string) string {
	return fmt.Sprintf("%s %s", pat, mat)
}

// Command returns true if the Faction is allowed to give orders
// to an entity with the provided DNA.
func (f *Faction) Command(dna string) bool {
	if f == nil {
		return false
	}
	// Give orders only to patrilineally descended entities.
	return strings.HasPrefix(dna, f.ydna)
}

// Score returns the contribution weight of an entity with
// the provided DNA to the faction's score.
func (f *Faction) Score(dna string) int {
	if f == nil {
		return 0
	}
	w := 0
	if strings.HasPrefix(dna, f.ydna) {
		w++
	}
	if strings.HasSuffix(dna, f.mtdna) {
		w++
	}
	return w
}
