// Package dna models two-part identity strings.
package dna

import (
	"fmt"
	"strings"
)

type Lineage int

const (
	Paternal Lineage = iota
	Maternal
	Full
)

type Sequence struct {
	// Tags that define the male and female
	// lines of descent.
	ydna  string
	mtdna string
}

// New returns a new Sequence.
func New(pat, mat string) *Sequence {
	return &Sequence{
		ydna:  pat,
		mtdna: mat,
	}
}

// Make returns the paternal and maternal DNA in a single string.
func Make(pat, mat string) string {
	return fmt.Sprintf("%s %s", pat, mat)
}

func (s *Sequence) String() string {
	if s == nil {
		return "unknown"
	}
	return Make(s.ydna, s.mtdna)
}

// Match returns true if the sequence matches.
func (s *Sequence) Match(seq string, lin Lineage) bool {
	switch lin {
	case Paternal:
		return strings.HasPrefix(seq, s.ydna)
	case Maternal:
		return strings.HasSuffix(seq, s.mtdna)
	case Full:
		return seq == s.String()
	}
	return false
}
