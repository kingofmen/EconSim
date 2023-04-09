// Package dna models two-part identity strings.
package dna

import (
	"fmt"
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

func (s *Sequence) Copy() *Sequence {
	if s == nil {
		return nil
	}
	return New(s.ydna, s.mtdna)
}

// String returns the full sequence.
func (s *Sequence) String() string {
	if err := s.Valid(); err != nil {
		return err.Error()
	}
	return fmt.Sprintf("%s %s", s.ydna, s.mtdna)
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
func (s *Sequence) Match(seq *Sequence, lin Lineage) bool {
	switch lin {
	case Paternal:
		return seq.ydna == s.ydna
	case Maternal:
		return seq.mtdna == s.mtdna
	case Full:
		return seq.mtdna == s.mtdna && seq.ydna == s.ydna
	}
	return false
}

// Valid returns an error if the sequence is not valid.
func (s *Sequence) Valid() error {
	if s == nil {
		return fmt.Errorf("nil sequence")
	}
	if s.Pat() == "" {
		return fmt.Errorf("sequence %q has no paternal DNA", s.String())
	}
	if s.Mat() == "" {
		return fmt.Errorf("sequence %q has no maternal DNA", s.String())
	}
	return nil
}
