// Package DNA contains a simple model of DNA as strings.
package dna

import (
	"fmt"
	"image/color"
)

const (
	Size = 8
)

type Sequence [Size]byte

// FromString creates a Sequence from the first 8 bytes of the string,
// padding with space characters if needed. Extended ASCII is ignored
// and printable characters are spaced out to make a neat 0-255 range.
func FromString(s string) Sequence {
	if len(s) < Size {
		return FromString(fmt.Sprintf("%-*s", Size, s))
	}
	var ret Sequence
	for idx := range ret {
		ascii := s[idx]
		// Map space to zero.
		if ascii > 31 {
			ascii -= 32
		} else {
			ascii = 0
		}
		if ascii > 94 {
			ascii = 94
		}
		// Tilde is 94 - add 33 for 127.
		ascii += ascii/32 + ascii/3
		ret[idx] = ascii * 2
		if s[idx]-32 > 64 {
			ret[idx]++
		}
	}
	return ret
}

// Distance returns the sum of point distances of the sequences.
func (s1 Sequence) Distance(s2 Sequence) int {
	ret := 0
	for idx, a := range s1 {
		b := s2[idx]
		if a > b {
			ret += int(a - b)
			continue
		}
		ret += int(b - a)
	}
	return ret
}

// RGB interprets the sequence as a color.
func (s1 Sequence) RGBA() color.NRGBA64 {
	return color.NRGBA64{
		R: uint16(s1[0])<<8 | uint16(s1[1]),
		G: uint16(s1[2])<<8 | uint16(s1[3]),
		B: uint16(s1[4])<<8 | uint16(s1[5]),
		A: uint16(s1[6])<<8 | uint16(s1[7]),
	}
}

// String converts a Sequence to a string for human readability.
func (seq Sequence) String() string {
	return string(seq[:])
}
