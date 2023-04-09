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

func (s *Sequence) Copy() *Sequence {
	if s == nil {
		return nil
	}
	return New(s.ydna, s.mtdna)
}

// String returns the full sequence.
func (s *Sequence) String() string {
	if s == nil {
		return "unknown"
	}
	return Make(s.ydna, s.mtdna)
}

// Partial returns the requested part of the sequence.
func (s *Sequence) Partial(lin Lineage) string {
	if s == nil {
		return ""
	}
	switch lin {
	case Paternal:
		return s.ydna
	case Maternal:
		return s.mtdna
	case Full:
		return s.String()
	}
	return ""
}

// Pat returns the paternal DNA.
func (s *Sequence) Pat() string {
	return s.Partial(Paternal)
}

// Mat returns the maternal DNA.
func (s *Sequence) Mat() string {
	return s.Partial(Maternal)
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
