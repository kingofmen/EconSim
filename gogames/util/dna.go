// Package DNA contains a simple model of DNA as strings.
package dna

import (
	"image/color"
)

const (
	Size = 8
)

type Sequence [Size]byte

// FromString creates a Sequence from the first 8 bytes of the string,
// padding with '0' characters if needed.
func FromString(s string) Sequence {
	var ret Sequence
	for idx := range ret {
		if idx < len(s) {
			ret[idx] = s[idx]
		} else {
			ret[idx] = '0'
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
